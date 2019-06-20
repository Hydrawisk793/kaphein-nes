#include "kaphein/mem/utils.h"
#include "kaphein/nes/LeSr16.h"
#include "kaphein/nes/ShiftRegister8.h"
#include "kaphein/nes/RP2C02.h"

//TODO : 시퀀싱 타이밍 재조정

/* **************************************************************** */
/* Internal declarations */

enum HECommand
{
    HECommand_NONE = 0
    , HECommand_VRAM_READ
    , HECommand_VRAM_WRITE
};

enum HEVRAMAddr
{
    HEVRAMAddr_W_FINE_Y = 0x7000
    , HEVRAMAddr_W_NAMETABLE_XY = 0x0C00
    , HEVRAMAddr_W_NAMETABLE_Y = 0x0800
    , HEVRAMAddr_W_NAMETABLE_X = 0x0400
    , HEVRAMAddr_W_COARSE_Y = 0x03E0
    , HEVRAMAddr_W_COARSE_Y_HIGH3BITS = 0x380
    , HEVRAMAddr_W_COARSE_X = 0x001F    
};

enum HEVRAMAddrL
{
    HEVRAMAddrL_COARSE_Y_L = 0xE0
    , HEVRAMAddrL_COARSE_X = 0x1F
};

enum HEVRAMAddrH
{
    HEVRAMAddrH_FINE_Y = 0x70
    , HEVRAMAddrH_NAMETABLE_XY = 0x0C
    , HEVRAMAddrH_NAMETABLE_Y = 0x08
    , HEVRAMAddrH_NAMETABLE_X = 0x04
    , HEVRAMAddrH_COARSE_Y_H = 0x03
    , HEVRAMAddrH_Y_VALUES = HEVRAMAddrH_FINE_Y
        | HEVRAMAddrH_NAMETABLE_Y
        | HEVRAMAddrH_COARSE_Y_H
};

struct kaphein_nes_RP2C02_Impl
{
    //시퀀서 코드 블록
    void (* sequencerCodeBlock) (
        struct kaphein_nes_RP2C02 * thisObj
    );
    //VRAM 코드 블록
    void (* bgCodeBlock) (
        struct kaphein_nes_RP2C02 * thisObj
    );
    //OAM 코드 블록
    void (* sprCodeBlock) (
        struct kaphein_nes_RP2C02 * thisObj
    );

    void * allocator;

    ////////////////////////////////
    //Buses
    
    //kaphein_UInt8 * addrBusH;
    //kaphein_UInt8 * addrLLatch;
    kaphein_UInt16 * addrBus;                       //(6) Address high byte
    kaphein_UInt8 * dataBus;                        //(8) Data bus, address low byte
    enum kaphein_nes_InterruptSignal * vBlank;      //(1) 2A03 NMI 버스로 연결
    struct kaphein_nes_AddressDecoder * decoder;    //주소 디코더

    ////////////////////////////////

    ////////////////////////////////
    //Output Device
    
    kaphein_UInt32 * _rgbTable;             //RGB 색상 테이블
    kaphein_UInt32 * _surface;              //서페이스
    kaphein_UInt32 * _surfacePtr;           //서페이스 포인터

    ////////////////////////////////

    ////////////////////////////////
    //Internal Memory Chips

    kaphein_UInt8 oam [SPR_COUNT*4];
    kaphein_UInt8 tempOAM [SPR_PER_SCANLINE*4];
    kaphein_UInt8 palette [0x10*2];
    kaphein_UInt16 addrLatch;               //(14) (External Chip)

    ////////////////////////////////

    ////////////////////////////////
    //PPU Register Open Bus

    kaphein_UInt8 regDecayedValue;

    ////////////////////////////////

    ////////////////////////////////
    //PPU Controller Registers
    
    kaphein_UInt8 regController;    //(0, W)
    kaphein_UInt8 regMask;          //(1, W)
    kaphein_UInt8 regStatus;        //(2, R)
    kaphein_UInt8 vBlankFlagSignal; //VBlank 플래그 신호

    ////////////////////////////////

    ////////////////////////////////
    //VRAM Registers

    kaphein_UInt8 regVRAMAddrL;     //(8, W)
    kaphein_UInt8 regVRAMAddrH;     //(6, W)
    kaphein_UInt16 regVRAMAddr;     //(14, R/W)
    kaphein_UInt8 tempVRAMAddrL;
    kaphein_UInt8 tempVRAMAddrH;
    kaphein_UInt16 tempVRAMAddr;
    kaphein_UInt8 regVRAMFinePosX;  //(3)
    kaphein_UInt8 regVRAMLatch;     //(1)
    kaphein_UInt8 regVRAMData;      //(7, R/W)

    ////////////////////////////////

    ////////////////////////////////
    //OAM Registers

    kaphein_UInt8 regOAMAddr;       //(3, W)
    kaphein_UInt8 tempOAMAddr;
    kaphein_UInt8 regOAMData;       //(4, R/W)
    kaphein_UInt8 sprInRangeCount;
    bool sprZeroFound;

    ////////////////////////////////

    ////////////////////////////////
    //Registers For Rendering

    kaphein_UInt16 tileBitmapL;         //ShiftRegister16
    kaphein_UInt16 tileBitmapH;         //ShiftRegister16
    kaphein_UInt8 tileAttrL;            //ShiftRegister8
    kaphein_UInt8 tileAttrH;            //ShiftRegister8
    kaphein_UInt8 tileAttrLLatch;
    kaphein_UInt8 tileAttrHLatch;
    kaphein_UInt8 tempTileNo;
    kaphein_UInt8 tempAttrNo;
    kaphein_UInt8 tempBitmapL;
    kaphein_UInt8 tempBitmapH;
    kaphein_UInt8 tempPosY;
    kaphein_UInt8 tempPosX;
    
    int sprPosXCounter[SPR_PER_SCANLINE];               //(8)
    kaphein_UInt8 sprTileBitmapL[SPR_PER_SCANLINE];     //ShiftRegister8
    kaphein_UInt8 sprTileBitmapH[SPR_PER_SCANLINE];     //ShiftRegister8
    kaphein_UInt8 sprAttr[SPR_PER_SCANLINE];
    kaphein_UInt8 sprPixelIndex[SPR_PER_SCANLINE];
    kaphein_UInt8 sprCount;
    kaphein_UInt8 sprZeroPosXCounter;
    bool hasSprZero;

    ////////////////////////////////

    ////////////////////////////////
    //Internal Status

    unsigned int cycles;            //사이클 카운터
    unsigned int sqDelayCounter;
    unsigned int bgDelayCounter;
    unsigned int sprDelayCounter;
    kaphein_UInt16 scanline;        //스캔라인 카운터
    kaphein_UInt8 pixel;            //생성된 픽셀
    bool oddFrame;                  //짝/홀 프레임 플래그
    bool renderingFlag;             //렌더링 플래그
    bool bgBitmapShiftFlag;         //BG 비트맵 시프트 플래그

    ////////////////////////////////
};

static const kaphein_UInt8 paletteRWIndexTable[0x20] = {
    0x00, 0x01, 0x02, 0x03
    , 0x04, 0x05, 0x06, 0x07
    , 0x08, 0x09, 0x0A, 0x0B
    , 0x0C, 0x0D, 0x0E, 0x0F
    , 0x00, 0x11, 0x12, 0x13
    , 0x04, 0x15, 0x16, 0x17
    , 0x08, 0x19, 0x1A, 0x1B
    , 0x0C, 0x1D, 0x1E, 0x1F
};

/* **************************************************************** */

/* **************************************************************** */
/* Internal function declarations */

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void
increaseVramAddress(
    struct kaphein_nes_RP2C02 * thisObj
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void
increaseVramPosX(
    struct kaphein_nes_RP2C02 * thisObj
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void increaseVramPosY(
    struct kaphein_nes_RP2C02 * thisObj
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void resetVramXValues(
    struct kaphein_nes_RP2C02 * thisObj
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void resetVramYValues(
    struct kaphein_nes_RP2C02 * thisObj
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
kaphein_UInt16 testPosY(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt8 v
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
bool
isInRange(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt16 v
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
kaphein_UInt8
selectPalette(
    struct kaphein_nes_RP2C02 * thisObj
    ,kaphein_UInt8 v
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void
setAddrBus(
    struct kaphein_nes_RP2C02 * thisObj
    , const kaphein_UInt8 l
    , const kaphein_UInt8 h
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
kaphein_UInt8
read(
    struct kaphein_nes_RP2C02 * thisObj
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void
write(
    struct kaphein_nes_RP2C02 * thisObj
    , const kaphein_UInt8 v
);

static
void
renderPixel(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
vBlankFlagSignalProc(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
loadBGBitmapIntoShiftRegister(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
loadSPRBitmapIntoShiftRegister(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SQ_onPreRendering(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SQ_onRendering(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SQ_onPostRendering(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SQ_onVBlankStart(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SQ_onVBlank(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchBGBitmap_0(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchBGBitmap_1(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchBGBitmap_2(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchBGBitmap_3(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchBGBitmap_4(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchBGBitmap_5(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchBGBitmap_6(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchBGBitmap_7(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_resetVRAMYValues(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchDummyBitmap_0(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchDummyBitmap_1(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchDummyBitmap_2(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
BG_fetchDummyBitmap_3(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_startInitialization(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_initializeTempOAM_0(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_startEvaluation(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_NORMAL_evaluateSprites_0(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_NORMAL_evaluateSprites_1(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_NORMAL_copyData_0(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_NORMAL_copyData_1(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_OVERFLOW_evaluateSprites_0(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_OVERFLOW_evaluateSprites_1(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_OVERFLOW_copyData_0(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_OVERFLOW_copyData_1(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_endOfEvaluation_0(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_startFetching(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_fetchSPRBitmap_0(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_fetchSPRBitmap_4(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_fetchSPRBitmap_7(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_startFetchingDummyData(
    struct kaphein_nes_RP2C02 * thisObj
);

static
void
SPR_fetchDummyData(
    struct kaphein_nes_RP2C02 * thisObj
);

/* **************************************************************** */

/* **************************************************************** */
/* Function definitions */

enum kaphein_ErrorCode
kaphein_nes_RP2C02_construct(
    struct kaphein_nes_RP2C02 * thisObj
    , void * allocator
)
{
    enum kaphein_ErrorCode resultErrorCode;
    struct kaphein_nes_RP2C02_Impl * impl;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        impl = (struct kaphein_nes_RP2C02_Impl * )kaphein_mem_allocate(
            allocator
            , KAPHEIN_ssizeof(*impl)
            , KAPHEIN_NULL
            , &resultErrorCode
        );
        if(KAPHEIN_NULL != impl) {
            kaphein_mem_fillZero(impl, KAPHEIN_ssizeof(*impl), KAPHEIN_ssizeof(*impl));
            
            thisObj->impl_ = impl;
            impl->allocator = allocator;

            impl->sequencerCodeBlock = KAPHEIN_NULL;
            impl->bgCodeBlock = KAPHEIN_NULL;
            impl->sprCodeBlock = KAPHEIN_NULL;
        
            impl->decoder = KAPHEIN_NULL;
            impl->vBlank = KAPHEIN_NULL;
            impl->addrBus = KAPHEIN_NULL;
            impl->dataBus = KAPHEIN_NULL;
        
            impl->_rgbTable = KAPHEIN_NULL;
            impl->_surface = KAPHEIN_NULL;
            impl->_surfacePtr = KAPHEIN_NULL;

            impl->oddFrame = false;
            impl->renderingFlag = false;
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2C02_destruct(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;
        
        if(KAPHEIN_NULL != impl) {
            resultErrorCode = kaphein_mem_deallocate(impl->allocator, impl, KAPHEIN_ssizeof(*impl));
            
            if(kapheinErrorCodeNoError == resultErrorCode) {
                thisObj->impl_ = KAPHEIN_NULL;
            }
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2C02_setOutputDevice(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt32 (surface [256*240])
    , kaphein_UInt32 (rgbTable [64])
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

        impl->_rgbTable = rgbTable;
        impl->_surface = surface;
        impl->_surfacePtr = surface;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2C02_setBuses(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt16 * address
    , kaphein_UInt8 * data
    , enum kaphein_nes_InterruptSignal * nmiBus
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

        impl->addrBus = address;
        impl->dataBus = data;
        impl->vBlank = nmiBus;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2C02_setAddressDecoder(
    struct kaphein_nes_RP2C02 * thisObj
    , struct kaphein_nes_AddressDecoder * decoder
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

        impl->decoder = decoder;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2C02_powerUp(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    static const kaphein_UInt8 powerUpPalette[0x20] = {
        0x09, 0x01, 0x00, 0x01,
        0x00, 0x02, 0x02, 0x0D,
        0x08, 0x10, 0x08, 0x24,
        0x00, 0x00, 0x04, 0x2C, 

        0x09, 0x01, 0x34, 0x03,
        0x00, 0x04, 0x00, 0x14,
        0x08, 0x3A, 0x00, 0x02,
        0x00, 0x20, 0x2C, 0x08
    };
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

	    impl->regStatus &= ~(HEStatus_VBLANK_OCCURED | HEStatus_SPR0_DETECTED);
	    impl->regOAMAddr = 0x00;
	    impl->regVRAMAddrH = 0x00;
        impl->regVRAMAddrL = 0x00;
	    impl->regVRAMData = 0xFF;
        
        kaphein_mem_copy(
            impl->palette
            , KAPHEIN_ssizeof(impl->palette)
            , powerUpPalette
            , KAPHEIN_ssizeof(powerUpPalette)
        );
        kaphein_mem_fillZero(
            impl->oam
            , KAPHEIN_ssizeof(impl->oam)
            , KAPHEIN_ssizeof(impl->oam)
        );
        
	    resultErrorCode = kaphein_nes_RP2C02_reset(thisObj);
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2C02_reset(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

        //서페이스 포인터 초기화
        impl->_surfacePtr = impl->_surface;

        //레지스터 초기화
        impl->regController = 0x00;
        impl->regMask = 0x00;
        impl->regStatus &= 0x7F;
        impl->regVRAMLatch = 0;
	    impl->regVRAMLatch = 0x00;
	    impl->regVRAMData = 0x00;

        //내부 상태 변수 초기화
        impl->sprInRangeCount = 0;
        impl->sprCount = 0;
	    impl->cycles = 0;
	    impl->scanline = 0;
        impl->sqDelayCounter = 0;
        impl->bgDelayCounter = 0;
        impl->sprDelayCounter = 0;
        impl->sprZeroFound = false;
        impl->hasSprZero = false;
	    impl->oddFrame = false;
        impl->renderingFlag = false;
        impl->bgBitmapShiftFlag = false;
        impl->vBlankFlagSignal = 100;
	    //impl->regWriteProtected = true;

        setAddrBus(thisObj, 0, 0);

        //시퀀서 블록 셋
        impl->sequencerCodeBlock = SQ_onRendering;

        //코드 블록 셋
        impl->bgCodeBlock = KAPHEIN_NULL;
        impl->sprCodeBlock = KAPHEIN_NULL;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2C02_run(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

        ////////////////////////////////////////////////
        //스캔라인별 시퀀서 실행

        if(impl->sqDelayCounter < 1) {
            (*impl->sequencerCodeBlock)(thisObj);
        }
        else {
            --impl->sqDelayCounter;
        }

        ////////////////////////////////////////////////

        ////////////////////////////////////////////////
        //픽셀 렌더링

        if(impl->renderingFlag) {
            renderPixel(thisObj);
        }

        ////////////////////////////////////////////////

        ////////////////////////////////////////////////
        //코드 블록 실행

        if(impl->bgBitmapShiftFlag) {
            impl->tileBitmapL <<= 1;
            impl->tileBitmapH <<= 1;
            kaphein_nes_ShiftRegister8_shiftLeft(&impl->tileAttrL, impl->tileAttrLLatch != 0);
            kaphein_nes_ShiftRegister8_shiftLeft(&impl->tileAttrH, impl->tileAttrHLatch != 0);
        }

        if(impl->bgDelayCounter > 0) {
            --impl->bgDelayCounter;
        }
        else if(
            (impl->regMask & HEMask_DISPLAY) != 0
            && KAPHEIN_NULL != impl->bgCodeBlock
        ) {
            (*impl->bgCodeBlock)(thisObj);
        }
        
        if(impl->sprDelayCounter > 0) {
            --impl->sprDelayCounter;
        }
        else if(
            (impl->regMask & HEMask_DISPLAY) != 0
            && KAPHEIN_NULL != impl->sprCodeBlock
        ) {
            (*impl->sprCodeBlock)(thisObj);
        }

        ////////////////////////////////////////////////

        ////////////////////////////////////////////////
        //사이클, 스캔라인 카운트, 홀/짝 프레임 플래그 셋

        if(++impl->cycles == CYCLES_PER_SCANLINE) {
            impl->cycles = 0;
            
            //프레임 하나를 다 처리 했으면
            if(++impl->scanline == TOTAL_SCANLINE_COUNT) {
                impl->scanline = 0;

                impl->oddFrame = !impl->oddFrame;

                impl->regDecayedValue = 0x00;
            }

            switch(impl->scanline) {
            case 0:
                impl->sequencerCodeBlock = SQ_onRendering;
            break;
            case VISIBLE_SCANLINE_COUNT:
                impl->sequencerCodeBlock = SQ_onPostRendering;
            break;
            case (VISIBLE_SCANLINE_COUNT+1):
                impl->sequencerCodeBlock = SQ_onVBlankStart;
            break;
            case (VISIBLE_SCANLINE_COUNT+2):
                impl->sequencerCodeBlock = SQ_onVBlank;
            break;
            case (TOTAL_SCANLINE_COUNT-1):
                impl->sequencerCodeBlock = SQ_onPreRendering;
            break;
            }
        }

        ////////////////////////////////////////////////
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2C02_isInOddFrame(
    struct kaphein_nes_RP2C02 * thisObj
    , bool * truthOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

        *truthOut = impl->oddFrame;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2C02_isRendering(
    struct kaphein_nes_RP2C02 * thisObj
    , bool * truthOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

        *truthOut = impl->renderingFlag;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2C02_readRegister(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt8 addr
    , kaphein_UInt8 * valueOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    kaphein_UInt8 returnValue;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

        returnValue = impl->regDecayedValue;

        switch(addr & 0x07) {
        case 0:
	    case 1:
        break;

        //(R) Status
        case 2:
	        //상태 값 리턴
	        returnValue = (impl->regDecayedValue & (~HEStatus_STATUS_BITS)) | impl->regStatus;

	        //VBlnak 플래그 클리어
	        impl->regStatus &= ~HEStatus_VBLANK_OCCURED;

            if(impl->vBlankFlagSignal < 100){
                *impl->vBlank = kaphein_nes_InterruptSignal_NONE;
                impl->vBlankFlagSignal = 100;
            }

		    //VRAM 레지스터 래치 리셋
		    impl->regVRAMLatch = 0;
	    break;

	    case 3:
        break;

        //(R/W) OAM Data
	    case 4:
            //데이터 반환
            returnValue = impl->oam[impl->regOAMAddr] & (((impl->regOAMAddr&0x03) == 0x02) ? HESprFlag_SPR_FLAG_VALUES : 0xFF);

            //오픈 버스 값 갱신
            if((impl->regOAMAddr & 0x03) == 2) {
                impl->regDecayedValue = (returnValue & HESprFlag_SPR_FLAG_VALUES);
            }
        break;

        case 5:
        case 6:
        break;

        //(R/W) VRAM Data
        case 7:
            if(impl->regVRAMAddrH == 0x3F) {      //팔레트 읽기
                returnValue = (impl->regDecayedValue & 0xC0) | impl->palette[paletteRWIndexTable[impl->regVRAMAddrL & 0x1F]];
            }
            else {
	            returnValue = impl->regDecayedValue = impl->regVRAMData;  //버퍼 값 반환
            }
	    
            impl->regVRAMData = read(thisObj);   //VRAM 읽기
		    increaseVramAddress(thisObj);             //VRAM 주소 증가
            setAddrBus(thisObj, impl->regVRAMAddrL, impl->regVRAMAddrH);  //어드레스 버스 셋
        break;
        }

        *valueOut = returnValue;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2C02_writeRegister(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt8 addr
    , kaphein_UInt8 v
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

        impl->regDecayedValue = v;

        switch(addr & 0x07) {
        //(W) Controller
        case 0:
            //NMI 발생
            if(
                !(impl->regController & HEController_ENABLE_VBLANK)
                && (v & HEController_ENABLE_VBLANK)
                && (impl->regStatus & HEStatus_VBLANK_OCCURED)
            ) {
                impl->vBlankFlagSignal = 2;
            }

		    //값 쓰기
		    impl->regController = v;

		    //임시 VRAM 주소 변경
		    impl->tempVRAMAddrH &= ~HEVRAMAddrH_NAMETABLE_XY;
		    impl->tempVRAMAddrH |= ((impl->regController & HEController_NAMETABLE_WH) << 2);
	    break;
        
        //(W) Mask
	    case 1:
		    //값 쓰기
		    impl->regMask = v;
	    break;

        //Does Nothing. It's an intention.
        case 2:
            
        break;

	    //(W) OAM Address
	    case 3:
		    //값 쓰기
		    impl->regOAMAddr = v;
	    break;

        //(R/W) OAM Data
	    case 4:
            //쓰기 후 주소 이동
            impl->oam[impl->regOAMAddr++] = v;
        break;

        //(W) VRAM Scroll
        case 5:
		    if(impl->regVRAMLatch == 0) {
			    //임시 VRAM X축 주소 셋
			    impl->tempVRAMAddrL &= ~HEVRAMAddrL_COARSE_X;
			    impl->tempVRAMAddrL |= (v >> 3);

			    //FINE X축 좌표 셋
			    impl->regVRAMFinePosX = (v) & 0x07;
		
			    //래치 셋
			    impl->regVRAMLatch = 1;
		    }
		    else {
			    //임시 VRAM Y축 하위 주소 셋
			    impl->tempVRAMAddrL &= ~HEVRAMAddrL_COARSE_Y_L;
			    impl->tempVRAMAddrL |= (v & 0x38) << 2;

			    //임시 VRMA Y축 하위 주소, FINE Y축 좌표 셋
			    impl->tempVRAMAddrH &= ~(HEVRAMAddrH_COARSE_Y_H | HEVRAMAddrH_FINE_Y);
			    impl->tempVRAMAddrH |= (((v & 0xC0) >> 6) | ((v & 0x07) << 4));
			    
			    //래치 클리어
			    impl->regVRAMLatch = 0;
		    }
	    break;

        //(W) VRAM Address
        case 6:
            if(impl->regVRAMLatch == 0) {
	            //6번 비트 클리어 후, 임시 VRAM 상위 주소 바이트 셋
                //impl->tempVRAMAddrH = 0;
	            impl->tempVRAMAddrH = (v & 0x3F);
		        
	            //래치 셋
	            impl->regVRAMLatch = 1;
            }
            else {
	            //임시 VRAM 하위 주소 바이트 셋, VRAM 주소 셋
	            impl->regVRAMAddrL = impl->tempVRAMAddrL = v;
	            impl->regVRAMAddrH = impl->tempVRAMAddrH;
		        
	            //래치 클리어
	            impl->regVRAMLatch = 0;

                //어드레스 버스 셋
		        setAddrBus(thisObj, impl->regVRAMAddrL, impl->regVRAMAddrH);
            }
	    break;

        //(R/W) VRAM Data
        case 7:
            //팔레트 데이터 쓰기
            if(impl->regVRAMAddrH == 0x3F) {
		        impl->palette[paletteRWIndexTable[impl->regVRAMAddrL & 0x1F]] = v;
            }
            //쓰기는 버퍼를 통과하지 않고 바로 실행됨.
            else {
                write(thisObj, v);
            }

            //VRAM 주소 증가
		    increaseVramAddress(thisObj);

            //어드레스 버스 셋
            setAddrBus(thisObj, impl->regVRAMAddrL, impl->regVRAMAddrH);
        break;
        }
    }
    
    return resultErrorCode;
}

/* **************************************************************** */

/* **************************************************************** */
/* Internal function definitions */

static
void
increaseVramAddress(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

	//VRAM 주소 32 증가
	if(impl->regController & HEController_ADDR_INCREMENT){
		//자리 올림 처리
		if(((kaphein_Int16)impl->regVRAMAddrL) + 0x20 >= 0x100) {
            ++impl->regVRAMAddrH;
            impl->regVRAMAddrH &= 0x3F;
        }

		impl->regVRAMAddrL += 0x20;
	}
	//VRAM 주소 1 증가, 최대 값은 0x3FFF로 한정
	else if(!(++impl->regVRAMAddrL)) {
        ++impl->regVRAMAddrH;
        impl->regVRAMAddrH &= 0x3F;
    }
}

static
void
increaseVramPosX(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //값이 최대치인 경우
    if((impl->regVRAMAddrL & HEVRAMAddrL_COARSE_X) == HEVRAMAddrL_COARSE_X){
        impl->regVRAMAddrL &= ~HEVRAMAddrL_COARSE_X;     //Coarse X 초기화
        impl->regVRAMAddrH ^= HEVRAMAddrH_NAMETABLE_X;   //X축 네임 테이블 변경
    }
    //아닌 경우
    else {
        ++impl->regVRAMAddrL;    //Coarse X 증가
    }
}

static
void increaseVramPosY(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //Fine Y가 최대값인 경우
    if((impl->regVRAMAddrH & HEVRAMAddrH_FINE_Y) == HEVRAMAddrH_FINE_Y){
        //VRAM 주소 병합
        impl->regVRAMAddr = makeWord(impl->regVRAMAddrL, impl->regVRAMAddrH);

        //Fine Y 초기화
        impl->regVRAMAddr &= (~HEVRAMAddr_W_FINE_Y);

        switch(impl->regVRAMAddr & HEVRAMAddr_W_COARSE_Y){
        
        //29
        case (HEVRAMAddr_W_COARSE_Y-0x40):
            //Coarse Y 초기화
            impl->regVRAMAddr &= ~HEVRAMAddr_W_COARSE_Y;

            //Y축 네임 테이블 변경
            impl->regVRAMAddr ^= HEVRAMAddr_W_NAMETABLE_Y;
        break;

        //31
        case HEVRAMAddr_W_COARSE_Y:
            //Coarse Y 초기화
            impl->regVRAMAddr &= ~HEVRAMAddr_W_COARSE_Y;
        break;

        //그외에는 Coarse Y 증가
        default:
            impl->regVRAMAddr += 0x20;
        }

        //계산된 VRAM 주소를 분할 후 기록
        impl->regVRAMAddrL = lowByte(impl->regVRAMAddr);
        impl->regVRAMAddrH = highByte(impl->regVRAMAddr);
    }
    //아닌 경우
    else {
        impl->regVRAMAddrH += 0x10;   //Fine Y 증가
    }
}

static
void resetVramXValues(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    impl->regVRAMAddrL &= ~HEVRAMAddrL_COARSE_X;
    impl->regVRAMAddrL |= (impl->tempVRAMAddrL & HEVRAMAddrL_COARSE_X);
    impl->regVRAMAddrH &= ~HEVRAMAddrH_NAMETABLE_X;
    impl->regVRAMAddrH |= (impl->tempVRAMAddrH & HEVRAMAddrH_NAMETABLE_X);
}

static
void resetVramYValues(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    impl->regVRAMAddrL &= ~HEVRAMAddrL_COARSE_Y_L;
    impl->regVRAMAddrL |= (impl->tempVRAMAddrL & HEVRAMAddrL_COARSE_Y_L);
    impl->regVRAMAddrH &= ~HEVRAMAddrH_Y_VALUES;
    impl->regVRAMAddrH |= (impl->tempVRAMAddrH & HEVRAMAddrH_Y_VALUES);
}

static
kaphein_UInt16 testPosY(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt8 v
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    return impl->scanline - v;
};

static
bool
isInRange(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt16 v
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    return v >= 0 && v < ((impl->regController & HEController_SPR_SIZE)?(16):(8));
};

static
kaphein_UInt8
selectPalette(
    struct kaphein_nes_RP2C02 * thisObj
    ,kaphein_UInt8 v
)
{
    static const kaphein_UInt8 indexTable[0x20] = {
        0x00, 0x01, 0x02, 0x03,
        0x00, 0x05, 0x06, 0x07,
        0x00, 0x09, 0x0A, 0x0B,
        0x00, 0x0D, 0x0E, 0x0F,
        0x00, 0x11, 0x12, 0x13,
        0x00, 0x15, 0x16, 0x17,
        0x00, 0x19, 0x1A, 0x1B,
        0x00, 0x1D, 0x1E, 0x1F
    };
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    return impl->palette[indexTable[v & 0x1F]];
}

static
void
setAddrBus(
    struct kaphein_nes_RP2C02 * thisObj
    , const kaphein_UInt8 l
    , const kaphein_UInt8 h
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    impl->addrLatch = makeWord(l, h);
	*impl->addrBus = impl->addrLatch;
}

static
kaphein_UInt8
read(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;
    kaphein_UInt8 value;

    (*impl->decoder->vTable->read)(
        impl->decoder
        , impl->addrLatch
        , &value
    );

    return value;
}

static
void
write(
    struct kaphein_nes_RP2C02 * thisObj
    , const kaphein_UInt8 v
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    (*impl->decoder->vTable->write)(
        impl->decoder
        , impl->addrLatch
        , v
    );
}

static
void
renderPixel(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    enum
    {
        PIXEL_INDEX_BITMAP = 0x03
    };
    static const kaphein_UInt8 BITMASK[8] = {
        0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
    };
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;
    kaphein_UInt8 bgPixelIndex;
    kaphein_UInt8 lastSprPixelIndex;
    kaphein_UInt8 lastSprPriority;
    unsigned int r1;
    const bool isLeftCornerClippingSpritesEnabled = ((impl->regMask & HEMask_DISABLE_CLIP_SPR) == 0) && impl->cycles < 9;

    ////////////////////////////
    //배경 픽셀 생성

    bgPixelIndex = 0;
    
    if((impl->regMask & HEMask_DISPLAY_BG)) {
        //BG 클립핑 영역이 아닌 경우
        if(
            (impl->regMask & HEMask_DISABLE_CLIP_BG) != 0
            || impl->cycles >= 9
        ) {
            //BG 픽셀 제조
            const kaphein_UInt8 BITMASK_VALUE = BITMASK[impl->regVRAMFinePosX];
            
            bgPixelIndex = 
                (!!(highByte(impl->tileBitmapH) & BITMASK_VALUE) << 1)
                | (!!(highByte(impl->tileBitmapL) & BITMASK_VALUE))
            ;

            if(bgPixelIndex != 0) {
                bgPixelIndex |= (!!(impl->tileAttrH & BITMASK_VALUE) << 3)
                    | (!!(impl->tileAttrL & BITMASK_VALUE) << 2)
                ;
            }
        }
    }

    ////////////////////////////

    ////////////////////////////////
    //스프라이트 픽셀 생성

    lastSprPixelIndex = 0;
    lastSprPriority = 0;

    if(impl->regMask & HEMask_DISPLAY_SPR) {
        for(r1 = 0; r1 < impl->sprCount; ++r1) {
            impl->sprPixelIndex[r1] = 0;
            
            if(impl->sprPosXCounter[r1] > -8) {
                if(impl->sprPosXCounter[r1] < 1) {
                    if((impl->sprAttr[r1] & HESprFlag_FLIP_H) != 0) {
                        impl->sprPixelIndex[r1] = 
                            kaphein_nes_ShiftRegister8_shiftRight(impl->sprTileBitmapL + r1, false)
                            | kaphein_nes_ShiftRegister8_shiftRight(impl->sprTileBitmapH + r1, false) << 1
                        ;
                    }
                    else {
                        impl->sprPixelIndex[r1] = 
                            kaphein_nes_ShiftRegister8_shiftLeft(impl->sprTileBitmapL + r1, false)
                            | kaphein_nes_ShiftRegister8_shiftLeft(impl->sprTileBitmapH + r1, false) << 1
                        ;
                    }
                    
                    if((impl->sprPixelIndex[r1] & PIXEL_INDEX_BITMAP) != 0) {
                        impl->sprPixelIndex[r1] |= ((impl->sprAttr[r1] & HESprFlag_ATTRIBUTE) << 2);

                        //우선 순위가 높고 투명하지 않은 픽셀 선택
                        if(lastSprPixelIndex == 0) {
                            lastSprPixelIndex = impl->sprPixelIndex[r1];
                            lastSprPriority = impl->sprAttr[r1] & HESprFlag_PRIORITY;
                        }
                    }
                }
                else {
                    --impl->sprPosXCounter[r1];
                }
            }
        }

        //SPR 클립핑
        if(isLeftCornerClippingSpritesEnabled) {
            lastSprPixelIndex = 0;
        }
    }

    ////////////////////////////////

    ////////////////////////////////
    //Sprite 0 Hit 감지

    if(
        impl->hasSprZero
        && (impl->regStatus & HEStatus_SPR0_DETECTED) == 0
        && (impl->sprPosXCounter[0] < 1 && impl->sprPosXCounter[0] > -8)    //SPR 0이 그려질 때가 되었을때
        && (!isLeftCornerClippingSpritesEnabled && impl->cycles < 256)
        && (bgPixelIndex != 0 && impl->sprPixelIndex[0] != 0)               //BG 픽셀과 SPR 0 픽셀이 충돌하면
    ) {
        impl->regStatus |= HEStatus_SPR0_DETECTED;
    }
    
    ////////////////////////////////

    ////////////////////////////////
    //최종 픽셀 결정 후 렌더링

    if(impl->regMask & HEMask_DISPLAY) {
        //SPR 픽셀이 투명이면
        if(lastSprPixelIndex == 0) {
            //BG 픽셀 표시
            impl->pixel = selectPalette(thisObj, bgPixelIndex);
        }
        //SPR 픽셀이 투명이 아니면
        else {
            //SPR 픽셀이 우선순위가 더 높거나
            //우선순위가 낮지만 BG 픽셀이 투명인 경우
            //SPR 픽셀 표시
            if(lastSprPriority == 0 || bgPixelIndex == 0) {
                impl->pixel = selectPalette(thisObj, lastSprPixelIndex + 0x10);
            }
            //안 그러면 BG 픽셀 표시
            else {
                impl->pixel = selectPalette(thisObj, bgPixelIndex);
            }
        }
    }
    else {
        //렌더링이 꺼진경우 투명색으로 채움
        impl->pixel = selectPalette(thisObj, 0);
    }

    //픽셀 렌더링
    if(
        KAPHEIN_NULL != impl->_surfacePtr
        && KAPHEIN_NULL != impl->_rgbTable
    ) {
        *impl->_surfacePtr++ = impl->_rgbTable[impl->pixel];
    }
    
    ////////////////////////////////
}

static
void
vBlankFlagSignalProc(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    if(
        impl->vBlankFlagSignal < 100
        && impl->vBlankFlagSignal++ == 2
        && (impl->regStatus & HEStatus_VBLANK_OCCURED)
        && (impl->regController & HEController_ENABLE_VBLANK)    //NMI 플래그가 셋 되어있으면
    ) {
        //NMI 발생
        *impl->vBlank = kaphein_nes_InterruptSignal_OCCUR;             
        impl->vBlankFlagSignal = 100;
    }
}

static
void
loadBGBitmapIntoShiftRegister(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //읽은 비트맵 비트 값을 시프트 레지스터 상위 바이트에 셋
    kaphein_nes_LeSr16_setHighByte(&impl->tileBitmapL, impl->tempBitmapL);
    kaphein_nes_LeSr16_setHighByte(&impl->tileBitmapH, impl->tempBitmapH);

    //현재 타일에 해당하는 속성 값 추출 후 ((Y축/16)%2와 ((X축/16)%2)로 계산)
    //읽은 속성 값을 속성 래치에 셋
    switch((((impl->regVRAMAddrL & 0x40) >> 5) | ((impl->regVRAMAddrL & 0x02) >> 1))){
    case 0:
	    impl->tileAttrLLatch = impl->tempAttrNo & 0x01;
	    impl->tileAttrHLatch = (impl->tempAttrNo & 0x02) >> 1;
    break;
    case 1:
	    impl->tileAttrLLatch = (impl->tempAttrNo & 0x04) >> 2;
	    impl->tileAttrHLatch = (impl->tempAttrNo & 0x08) >> 3;
    break;
    case 2:
	    impl->tileAttrLLatch = (impl->tempAttrNo & 0x10) >> 4;
	    impl->tileAttrHLatch = (impl->tempAttrNo & 0x20) >> 5;
    break;
    case 3:
	    impl->tileAttrLLatch = (impl->tempAttrNo & 0x40) >> 6;
	    impl->tileAttrHLatch = (impl->tempAttrNo & 0x80) >> 7;
    break;
    }
}

static
void
loadSPRBitmapIntoShiftRegister(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;
    const int offset = (impl->tempOAMAddr >> 2) - 1;

    impl->sprTileBitmapL[offset] = impl->tempBitmapL;
    impl->sprTileBitmapH[offset] = impl->tempBitmapH;
    impl->sprAttr[offset] = impl->tempAttrNo;
    impl->sprPosXCounter[offset] = impl->tempPosX;
}

static
void
SQ_onPreRendering(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    switch(impl->cycles) {
    case 0:
        impl->_surfacePtr = impl->_surface;       //서페이스 포인터 초기화
        impl->regStatus &= ~HEStatus_STATUS_BITS;    //스테이터스 플래그 클리어
    break;
    case 1:
        //impl->bgBitmapShiftFlag = true;        //BG 비트맵 시프트 플래그 셋
        impl->bgCodeBlock = KAPHEIN_NULL;        //BG_fetchBGBitmap_0;
        kaphein_mem_fill(
            impl->tempOAM
            , KAPHEIN_ssizeof(impl->tempOAM)
            , 0xFF
            , KAPHEIN_ssizeof(impl->tempOAM)
        );  //임시 OAM 초기화
        impl->sprCodeBlock = KAPHEIN_NULL;       //SPR_startInitialization;

        impl->sqDelayCounter = 65-1-1;
    break;
    case 65:
        impl->sprCodeBlock = KAPHEIN_NULL;

        impl->sqDelayCounter = 257-65-1;
    break;
    case 257:
        if(impl->regMask & HEMask_DISPLAY) {
            increaseVramPosY(thisObj);             //256 후반
            resetVramXValues(thisObj);
        }

        //impl->bgBitmapShiftFlag = false;       //BG 비트맵 시프트 플래그 클리어
        impl->bgCodeBlock = KAPHEIN_NULL;
        impl->sprCodeBlock = SPR_startFetching;

        impl->sqDelayCounter = 280-257-1;
    break;
    case 280:
        impl->bgCodeBlock = BG_resetVRAMYValues;

        impl->sqDelayCounter = 305 - 280 - 1;
    break;
    case 305:
        impl->bgCodeBlock = KAPHEIN_NULL;

        impl->sqDelayCounter = 321 - 305 - 1;
    break;
    case 321:
        impl->bgBitmapShiftFlag = true;          //BG 비트맵 시프트 플래그 셋
        impl->bgCodeBlock = BG_fetchBGBitmap_0;
        impl->sprCodeBlock = KAPHEIN_NULL;       //SPR_startFetchingDummyData;

        impl->sqDelayCounter = 337 - 321 - 1;
    break;
    case 337:
        impl->bgBitmapShiftFlag = false;         //BG 비트맵 시프트 플래그 클리어
        impl->bgCodeBlock = BG_fetchDummyBitmap_0;

        //홀 프레임에 렌더링 중이면
        if(impl->oddFrame && (impl->regMask & HEMask_DISPLAY)) {
            //1 사이클 스킵
            ++impl->cycles;
        }
    break;
    }
}

static
void
SQ_onRendering(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    switch(impl->cycles) {
    case 1:
        impl->renderingFlag = true;          //렌더링 플래그 셋
        impl->bgBitmapShiftFlag = true;      //BG 비트맵 시프트 플래그 셋
        impl->bgCodeBlock = BG_fetchBGBitmap_0;
        kaphein_mem_fill(
            impl->tempOAM
            , KAPHEIN_ssizeof(impl->tempOAM)
            , 0xFF
            , KAPHEIN_ssizeof(impl->tempOAM)
        );  //임시 OAM 초기화
        impl->sprCodeBlock = KAPHEIN_NULL;   //SPR_startInitialization;
        
        impl->sqDelayCounter = 65 - 1 - 1;
    break;
    case 65:
        impl->sprCodeBlock = SPR_startEvaluation;

        impl->sqDelayCounter = 257 - 65 - 1;
    break;
    case 257:
        if(impl->regMask & HEMask_DISPLAY) {
            increaseVramPosY(thisObj);         //256 후반
            resetVramXValues(thisObj);
        }

        impl->renderingFlag = false;         //렌더링 플래그 클리어
        impl->bgBitmapShiftFlag = false;     //BG 비트맵 시프트 플래그 클리어
        impl->bgCodeBlock = KAPHEIN_NULL;
        impl->sprCodeBlock = SPR_startFetching;
        
        impl->sqDelayCounter = 321 - 257 - 1;
    break;
    case 321:
        impl->bgBitmapShiftFlag = true;      //BG 비트맵 시프트 플래그 셋
        impl->bgCodeBlock = BG_fetchBGBitmap_0;
        impl->sprCodeBlock = KAPHEIN_NULL;   //SPR_startFetchingDummyData;

        impl->sqDelayCounter = 337 - 321 - 1;
    break;
    case 337:
        impl->bgBitmapShiftFlag = false;     //BG 비트맵 시프트 플래그 클리어
        impl->bgCodeBlock = BG_fetchDummyBitmap_0;
    break;
    }
}

static
void
SQ_onPostRendering(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    if(impl->cycles == 340) {
        //VBlank 플래그 신호 레벨 셋
        impl->vBlankFlagSignal = 0;
    }

    impl->bgCodeBlock = KAPHEIN_NULL;
    impl->sprCodeBlock = KAPHEIN_NULL;
}

static
void
SQ_onVBlankStart(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    if(!impl->cycles) {
        impl->bgCodeBlock = KAPHEIN_NULL;
        impl->sprCodeBlock = KAPHEIN_NULL;
        
        if(!impl->vBlankFlagSignal) {
            //VBlank 플래그
            impl->regStatus |= HEStatus_VBLANK_OCCURED;
        }
    }

    //VBlank 플래그 신호 처리
    vBlankFlagSignalProc(thisObj);
}

static
void
SQ_onVBlank(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    //VBlank 플래그 신호 처리
    vBlankFlagSignalProc(thisObj);
}

static
void
BG_fetchBGBitmap_0(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    ////////////////////////////////
    //cycles 0 - 

    setAddrBus(thisObj, impl->regVRAMAddrL, 0x20 | (impl->regVRAMAddrH & 0x0F));
    
    ////////////////////////////////

    impl->bgCodeBlock = BG_fetchBGBitmap_1;
}

static
void
BG_fetchBGBitmap_1(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    ////////////////////////////////
    //cycles 1 - 

    impl->tempTileNo = read(thisObj);    //타일 번호 읽기

    ////////////////////////////////

    impl->bgCodeBlock = BG_fetchBGBitmap_2;
}

static
void
BG_fetchBGBitmap_2(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;
    kaphein_UInt16 addrBusVar;
    
    ////////////////////////////////
    //cycles 2 - 

    impl->regVRAMAddr = makeWord(impl->regVRAMAddrL, impl->regVRAMAddrH);
    addrBusVar = 0x23C0 | (impl->regVRAMAddr & HEVRAMAddr_W_NAMETABLE_XY)
                | ((impl->regVRAMAddr & HEVRAMAddr_W_COARSE_Y_HIGH3BITS) >> 4)
                | ((impl->regVRAMAddr & HEVRAMAddr_W_COARSE_X) >> 2)
            ;
	setAddrBus(thisObj, lowByte(addrBusVar), highByte(addrBusVar));

    ////////////////////////////////
    
    impl->bgCodeBlock = BG_fetchBGBitmap_3;
}

static
void
BG_fetchBGBitmap_3(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    ////////////////////////////////
    //cycles 3 - 

    impl->tempAttrNo = read(thisObj);    //속성 번호 읽기

    ////////////////////////////////

    impl->bgCodeBlock = BG_fetchBGBitmap_4;
}

static
void
BG_fetchBGBitmap_4(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    ////////////////////////////////
    //cycles 4 -
    
    setAddrBus(
        thisObj
        , ((impl->tempTileNo & 0x0F) << 4) | ((impl->regVRAMAddrH & HEVRAMAddrH_FINE_Y) >> 4)
        , (impl->regController & HEController_BG_PATTERN) | ((impl->tempTileNo & 0xF0) >> 4)
    );

    ////////////////////////////////

    impl->bgCodeBlock = BG_fetchBGBitmap_5;
}

static
void
BG_fetchBGBitmap_5(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    ////////////////////////////////
    //cycles 5 - 

    impl->tempBitmapL = read(thisObj);   //하위 비트맵 읽기

    ////////////////////////////////

    impl->bgCodeBlock = BG_fetchBGBitmap_6;
}

static
void
BG_fetchBGBitmap_6(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    ////////////////////////////////
    //cycles 6 - 
	
    setAddrBus(
        thisObj
        , (((impl->tempTileNo & 0x0F) << 4) | ((impl->regVRAMAddrH & HEVRAMAddrH_FINE_Y) >> 4)) + 8
        , (impl->regController & HEController_BG_PATTERN) | ((impl->tempTileNo & 0xF0) >> 4)
    );
    
    ////////////////////////////////

    impl->bgCodeBlock = BG_fetchBGBitmap_7;
}

static
void
BG_fetchBGBitmap_7(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    ////////////////////////////////
    //cycles 7 - 

    impl->tempBitmapH = read(thisObj);   //상위 비트맵 읽기
    loadBGBitmapIntoShiftRegister(thisObj); //시프트 레지스터에 비트맵 로드
    increaseVramPosX(thisObj);             //VRAM 주소 X축 좌표 증가
    
    ////////////////////////////////

    impl->bgCodeBlock = BG_fetchBGBitmap_0;
}

static
void
BG_resetVRAMYValues(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    resetVramYValues(thisObj);

    impl->bgCodeBlock = KAPHEIN_NULL;    //필요하게되면 삭제
}

static
void
BG_fetchDummyBitmap_0(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    setAddrBus(thisObj, impl->regVRAMAddrL, 0x20 | (impl->regVRAMAddrH & 0x0F));
    impl->bgCodeBlock = BG_fetchDummyBitmap_1;
}

static
void
BG_fetchDummyBitmap_1(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    impl->tempTileNo = read(thisObj);    //타일 번호 읽기
    impl->bgCodeBlock = BG_fetchDummyBitmap_2;
}

static
void
BG_fetchDummyBitmap_2(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    setAddrBus(thisObj, impl->regVRAMAddrL, 0x20 | (impl->regVRAMAddrH & 0x0F));
    impl->bgCodeBlock = BG_fetchDummyBitmap_3;
}

static
void
BG_fetchDummyBitmap_3(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    impl->tempTileNo = read(thisObj);    //타일 번호 읽기
    impl->bgCodeBlock = KAPHEIN_NULL;
}

static
void
SPR_startInitialization(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    impl->tempOAMAddr = 0;

    SPR_initializeTempOAM_0(thisObj);
}

static
void
SPR_initializeTempOAM_0(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    ////////////////////////////////
    //cycles 0 - Load

    impl->regOAMData = 0xFF;

    ////////////////////////////////

    ////////////////////////////////
    //cycles 1 - Store

    impl->tempOAM[impl->tempOAMAddr++] = impl->regOAMData;

    ////////////////////////////////

    impl->sprDelayCounter = 1;
}

static
void
SPR_startEvaluation(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    impl->tempOAMAddr = 0;
    impl->regOAMAddr = 0;
    impl->sprInRangeCount = 0;
    impl->sprZeroFound = false;

    SPR_NORMAL_evaluateSprites_0(thisObj);
}

static
void
SPR_NORMAL_evaluateSprites_0(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //임시 OAM에서 Y축 좌표 읽기
    impl->regOAMData = impl->oam[impl->regOAMAddr];

    //스캔라인 범위 내에 있으면 데이터 복사 시작

    if(isInRange(thisObj, testPosY(thisObj, impl->regOAMData))) {

        //0번 스프라이트인 경우 플래그 셋
        if(impl->regOAMAddr == 0) {
            impl->sprZeroFound = true;
        }

        //찾아낸 스프라이트 카운터 증가
        ++impl->sprInRangeCount;

        //데이터 복사 블록으로 이동
        impl->sprCodeBlock = SPR_NORMAL_copyData_1;

    }
    else {
        impl->sprCodeBlock = SPR_NORMAL_evaluateSprites_1;
    }

    /*
    impl->sprCodeBlock = (
        (isInRange(thisObj, testPosY(thisObj, impl->regOAMData)))
        ? (SPR_NORMAL_copyData_1)
        : (SPR_NORMAL_evaluateSprites_1)
    );
    */
}

static
void
SPR_NORMAL_evaluateSprites_1(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //Y축 좌표를 임시 OAM에 복사
    impl->tempOAM[impl->tempOAMAddr] = impl->regOAMData;

    //순회가 끝나면 평가 종료
    impl->sprCodeBlock = (
        (!(impl->regOAMAddr += 4))
        ? KAPHEIN_NULL  //SPR_endOfEvaluation_0
        : SPR_NORMAL_evaluateSprites_0
    );
}

static
void
SPR_NORMAL_copyData_0(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //현재 스프라이트의 다음 데이터 읽기
    impl->regOAMData = impl->oam[impl->regOAMAddr];

    impl->sprCodeBlock = SPR_NORMAL_copyData_1;
}

static
void
SPR_NORMAL_copyData_1(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //임시 OAM에 데이터 복사 후
    //임시 OAM 포인터 이동
    impl->tempOAM[impl->tempOAMAddr++] = impl->regOAMData;

    //OAM 포인터 이동
    ++impl->regOAMAddr;
            
    //임시 OAM 포인터가 범위를 벗어나면
    if(impl->tempOAMAddr >= (SPR_PER_SCANLINE << 2)) {
        //오버플로우 처리 진입
        impl->sprCodeBlock = SPR_OVERFLOW_evaluateSprites_0;
    }    
    //OAM 순회가 끝났으면 평가 종료
    else if(impl->regOAMAddr == 0) {
        impl->sprCodeBlock = KAPHEIN_NULL;//SPR_endOfEvaluation_0;
    }
    //데이터 복사가 끝났으면 평가 프로시저로 복귀
    else if(!(impl->regOAMAddr & 0x03)) {
        impl->sprCodeBlock = SPR_NORMAL_evaluateSprites_0;
    }
    //아니면 복사 재개
    else {
        impl->sprCodeBlock = SPR_NORMAL_copyData_0;
    }
}

static
void
SPR_OVERFLOW_evaluateSprites_0(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //OAM에서 데이터 읽기
    impl->regOAMData = impl->oam[impl->regOAMAddr];

    //값이 스캔라인 범위 내에 있으면
    if(isInRange(thisObj, testPosY(thisObj, impl->regOAMData))) {
        //SPR 오버플로우 플래그 셋
        impl->regStatus |= HEStatus_SPR_OVERFLOW;
                
        //데이터 복사 시작
        impl->sprCodeBlock = SPR_OVERFLOW_copyData_1;
    }
    //아니면 평가 재개
    else {
        impl->sprCodeBlock = SPR_OVERFLOW_evaluateSprites_1;
    }
}

static
void
SPR_OVERFLOW_evaluateSprites_1(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //캐리 판단을 위한 가짜 OAM 주소 사용
    kaphein_UInt16 tempRegOAMAddr = impl->regOAMAddr;
    
    //쓰기금지 되어있으므로
    //데이터 읽기로 대체됨
    impl->regOAMData = impl->tempOAM[(SPR_PER_SCANLINE << 2) - 1];

    //다음 코드 블록 지정
    impl->sprCodeBlock = SPR_OVERFLOW_evaluateSprites_0;

    //M증가, 순회가 끝나면
    if(++tempRegOAMAddr > 0xFF) {
        //M = 0
        tempRegOAMAddr &= 0xFC;
                
        //평가 종료
        impl->sprCodeBlock = KAPHEIN_NULL;//SPR_endOfEvaluation_0;
    }

    //N증가, 순회가 끝나면 평가 종료 
    else if(tempRegOAMAddr += 4 > 0xFF) {
        //M = 0
        tempRegOAMAddr &= 0xFC;

        //평가 종료
        impl->sprCodeBlock = KAPHEIN_NULL;//SPR_endOfEvaluation_0;
    }

    //OAM 주소 반영
    impl->regOAMAddr = tempRegOAMAddr & 0xFF;
}

static
void
SPR_OVERFLOW_copyData_0(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //현재 스프라이트의 다음 데이터 읽기
    impl->regOAMData = impl->oam[impl->regOAMAddr];

    impl->sprCodeBlock = SPR_OVERFLOW_copyData_1;
}

static
void
SPR_OVERFLOW_copyData_1(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //쓰기금지 되어있으므로
    //데이터 읽기로 대체됨
    impl->regOAMData = impl->tempOAM[(SPR_PER_SCANLINE << 2) - 1];

    //순회가 끝나면 평가 종료
    if(!(++impl->regOAMAddr)) {
        impl->sprCodeBlock = KAPHEIN_NULL;//SPR_endOfEvaluation_0;
    }
    //M = 0이면 평가 프로시저로 복귀
    else if(!(impl->regOAMAddr & 0x03)) {
        impl->sprCodeBlock = SPR_OVERFLOW_evaluateSprites_0;
    }
    //아니면 복사 재개
    else {
        impl->sprCodeBlock = SPR_OVERFLOW_copyData_0;
    }
}

static
void
SPR_endOfEvaluation_0(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    impl->regOAMData = impl->oam[impl->regOAMAddr];   //OAM에서 데이터 읽기
    //impl->sprCodeBlock = SPR_endOfEvaluation_1;

    impl->regOAMData = impl->tempOAM[SPR_PER_SCANLINE*4-1]; //쓰기금지 되어있으므로 데이터 읽기로 대체됨
    impl->regOAMAddr += 4;    //OAM 포인터 이동
    //impl->sprCodeBlock = SPR_endOfEvaluation_0;

    impl->sprDelayCounter = 1;
}

static
void
SPR_startFetching(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //임시 OAM 주소 초기화
    impl->tempOAMAddr = 0;

    //파라미터 전송
    impl->sprCount = impl->sprInRangeCount;
    impl->hasSprZero = impl->sprZeroFound;

    //프로시저 시작
    SPR_fetchSPRBitmap_0(thisObj);
}

static
void
SPR_fetchSPRBitmap_0(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    ////////////////////////////////
    //cycles 0 -     

    impl->tempPosY = lowByte(testPosY(thisObj, (impl->tempOAM[impl->tempOAMAddr++])));

    //setAddrBus(thisObj, impl->regVRAMAddrL, 0x20 | (regVRAMAddrH&0x0F));

    ////////////////////////////////

    ////////////////////////////////
    //cycles 1 - 

    impl->tempTileNo = impl->tempOAM[impl->tempOAMAddr++];

    //read(thisObj);
    
    ////////////////////////////////

    ////////////////////////////////
    //cycles 2 - 

    impl->tempAttrNo = impl->tempOAM[impl->tempOAMAddr++];
    impl->tempPosY = ((impl->tempAttrNo & HESprFlag_FLIP_Y) ? (~impl->tempPosY) : (impl->tempPosY));
    
    //setAddrBus(thisObj, impl->regVRAMAddrL, 0x20 | (regVRAMAddrH&0x0F));
    
    ////////////////////////////////

    ////////////////////////////////
    //cycles 3 - 

    impl->tempPosX = impl->tempOAM[impl->tempOAMAddr++];

    //read(thisObj);

    ////////////////////////////////

    impl->sprCodeBlock = SPR_fetchSPRBitmap_4;
    impl->sprDelayCounter = 3;
}

static
void
SPR_fetchSPRBitmap_4(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    ////////////////////////////////
    //cycles 4 - 

    if(impl->regController & HEController_SPR_SIZE) {
	    setAddrBus(
            thisObj
            , ((impl->tempTileNo & 0x0E) << 4) + (impl->tempPosY & 0x07) + (((impl->tempPosY & 0x08)) ? (0x10) : (0))
            , (((impl->tempTileNo & 0x01) << 4) | ((impl->tempTileNo & 0xF0) >> 4) )& 0x1F
        );
    }
    else {
	    setAddrBus(
            thisObj
            , ((impl->tempTileNo & 0x0F) << 4) + (impl->tempPosY & 0x07)
            , (((impl->regController & HEController_SPR_PATTERN) << 1) | ((impl->tempTileNo & 0xF0) >> 4)) & 0x1F
        );
    }
    
    ////////////////////////////////

    ////////////////////////////////
    //cycles 5 - 

    impl->tempBitmapL = read(thisObj);   //하위 비트맵 읽기

    ////////////////////////////////

    ////////////////////////////////
    //cycles 6 -
    
    if(impl->regController & HEController_SPR_SIZE) {
	    setAddrBus(
            thisObj
            , ((impl->tempTileNo & 0x0E) << 4) + 8 + (impl->tempPosY & 0x07) + ((impl->tempPosY & 0x08) ? (0x10) : (0))
            , (((impl->tempTileNo&0x01) << 4) | ((impl->tempTileNo & 0xF0) >> 4))  &0x1F
        );
    }
    else {
	    setAddrBus(
            thisObj
            , ((impl->tempTileNo & 0x0F) << 4) + 8 + (impl->tempPosY & 0x07)
            , (((impl->regController & HEController_SPR_PATTERN) << 1) | ((impl->tempTileNo & 0xF0) >> 4)) & 0x1F
        );
    }
    
    ////////////////////////////////

    impl->sprCodeBlock = SPR_fetchSPRBitmap_7;
    impl->sprDelayCounter = 2;
}

static
void
SPR_fetchSPRBitmap_7(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    ////////////////////////////////
    //cycles 7 - 

    impl->tempBitmapH = read(thisObj);       //상위 비트맵 읽기
    loadSPRBitmapIntoShiftRegister(thisObj);    //비트맵 로드

    ////////////////////////////////

    impl->sprCodeBlock = SPR_fetchSPRBitmap_0;
}

static
void
SPR_startFetchingDummyData(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    impl->regOAMAddr = 0;
    impl->tempOAMAddr = 0;

    impl->sprCodeBlock = KAPHEIN_NULL;
    //SPR_fetchDummyData(thisObj);
}

static
void
SPR_fetchDummyData(
    struct kaphein_nes_RP2C02 * thisObj
)
{
    struct kaphein_nes_RP2C02_Impl *const impl = (struct kaphein_nes_RP2C02_Impl *)thisObj->impl_;

    //임시 OAM의 첫번째 값 읽기
    //impl->regOAMAddr = 0;
    //impl->tempOAMAddr = 0;
    impl->regOAMData = *impl->tempOAM;
}

/* **************************************************************** */
