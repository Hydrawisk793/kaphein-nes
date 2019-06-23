#include "kaphein/mem/utils.h"
#include "kaphein/nes/cart/TxROM.h"

//TODO : 오류 코드 처리

/* **************************************************************** */
/* Internal declarations */

enum HEBankConfig
{
    HEBankConfig_CHR_MODE = 0x80
    , HEBankConfig_PRG_MODE = 0x40
    , HEBankConfig_BANK_SELECT = 0x07
};

enum HEPRGRAMConfig
{
    HEPRGRAMConfig_ENABLE = 0x80
    , HEPRGRAMConfig_DISABLE_WRITING = 0x40
    , HEPRGRAMConfig_VALUES = 0xC0
};

enum HEBankSelectConst
{
    HEBankSelectConst_CHR00 = 0
    , HEBankSelectConst_CHR08
    , HEBankSelectConst_CHR10
    , HEBankSelectConst_CHR14
    , HEBankSelectConst_CHR18
    , HEBankSelectConst_CHR1C
    , HEBankSelectConst_PRG8
    , HEBankSelectConst_PRGA 
};

static
void
mapBank(
    struct kaphein_nes_cart_TxROM * thisPtr
    , kaphein_UInt8 v
);

static
void
setBankMode(
    struct kaphein_nes_cart_TxROM * thisPtr
    , kaphein_UInt8 v
);

static
void
mapNameTable(
    struct kaphein_nes_cart_TxROM * thisPtr
    , kaphein_UInt8 v
);

static
void
clockIrqCounterA(
    struct kaphein_nes_cart_TxROM * thisPtr
);

static
void
clockIrqCounterB(
    struct kaphein_nes_cart_TxROM * thisPtr
);

static
const struct kaphein_nes_Cartridge_VTable cartVTable = {
    kaphein_nes_cart_TxROM_destruct
    , kaphein_nes_Cartridge_setNametableMemory
    , kaphein_nes_Cartridge_setBuses
    , kaphein_nes_cart_TxROM_powerUp
    , kaphein_nes_cart_TxROM_reset
    , kaphein_nes_cart_TxROM_run
    , kaphein_nes_cart_TxROM_doCpuRead
    , kaphein_nes_cart_TxROM_doCpuWrite
    , kaphein_nes_Cartridge_doPpuRead
    , kaphein_nes_Cartridge_doPpuWrite
};

/* **************************************************************** */

/* **************************************************************** */
/* Function Definitions */

enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_construct(
    struct kaphein_nes_cart_TxROM * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
)
{
    kaphein_SSize prgRamSize = 0;
    int version;
    bool hasPrgNvRam;
    
    kaphein_nes_Cartridge_construct(
        &thisObj->parent
        , stream
        , romHeader
        , allocator
    );
    thisObj->parent.vTable = &cartVTable;

    kaphein_nes_NesRomHeader_getPrgRomBankCount(&thisObj->parent.romHeader, &thisObj->prgRomBankCount);
    thisObj->prgRomBankCount <<= 1;
    kaphein_nes_NesRomHeader_getChrRomBankCount(&thisObj->parent.romHeader, &thisObj->chrRomBankCount);
    thisObj->chrRomBankCount <<= 3;

    kaphein_nes_NesRomHeader_getVersion(&thisObj->parent.romHeader, &version);
    kaphein_nes_NesRomHeader_hasPrgNvRam(&thisObj->parent.romHeader, &hasPrgNvRam);
    switch(version) {
    case 0:
        prgRamSize = 0x2000;
    break;
    case 2:
        if(hasPrgNvRam) {
            kaphein_nes_NesRomHeader_getPrgNvRamSize(&thisObj->parent.romHeader, &prgRamSize);
        }
        else {
            kaphein_nes_NesRomHeader_getPrgRamSize(&thisObj->parent.romHeader, &prgRamSize);
        }
    break;
    default:
        prgRamSize = 0x2000;
    break;
    }

    if(prgRamSize > 0) {
        if(hasPrgNvRam) {
            thisObj->parent.memoryChipArray.prgNvRam = (kaphein_UInt8 *)kaphein_mem_allocate(
                thisObj->parent.allocator
                , prgRamSize
                , KAPHEIN_NULL
                , KAPHEIN_NULL
            );
            thisObj->parent.memoryChipArray.prgNvRamSize = prgRamSize;
        }
        else {
            thisObj->parent.memoryChipArray.prgRam = (kaphein_UInt8 *)kaphein_mem_allocate(
                thisObj->parent.allocator
                , prgRamSize
                , KAPHEIN_NULL
                , KAPHEIN_NULL
            );
            thisObj->parent.memoryChipArray.prgRamSize = prgRamSize;
        }
    }

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_destruct(
    void * thisObj
)
{
    struct kaphein_nes_cart_TxROM *const thisPtr = (struct kaphein_nes_cart_TxROM *)thisObj;

    if(KAPHEIN_NULL != thisPtr->parent.memoryChipArray.prgRam) {
        kaphein_mem_deallocate(
            thisPtr->parent.allocator
            , thisPtr->parent.memoryChipArray.prgRam
            , thisPtr->parent.memoryChipArray.prgRamSize
        );

        thisPtr->parent.memoryChipArray.prgRam = KAPHEIN_NULL;
        thisPtr->parent.memoryChipArray.prgRamSize = 0;
    }

    if(KAPHEIN_NULL != thisPtr->parent.memoryChipArray.prgNvRam) {
        kaphein_mem_deallocate(
            thisPtr->parent.allocator
            , thisPtr->parent.memoryChipArray.prgNvRam
            , thisPtr->parent.memoryChipArray.prgNvRamSize
        );

        thisPtr->parent.memoryChipArray.prgNvRam = KAPHEIN_NULL;
        thisPtr->parent.memoryChipArray.prgNvRamSize = 0;
    }

    return kaphein_nes_Cartridge_destruct(&thisPtr->parent);
}

enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_powerUp(
    void * thisObj
)
{
    return kaphein_nes_cart_TxROM_reset(thisObj);
}

enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_reset(
    void * thisObj
)
{
    struct kaphein_nes_cart_TxROM *const thisPtr = (struct kaphein_nes_cart_TxROM *)thisObj;
    int i;

    kaphein_nes_Cartridge_reset(&thisPtr->parent);

    //뱅크 번호 레지스터 초기화
    for(i = 0; i < 6; ++i) {
        thisPtr->chrAddressLatch[i] = 0;
    }
    thisPtr->prgRomAddressLatch = thisPtr->parent.prgBankAddresses[0];

    //레지스터 셋
    thisPtr->bankConfig = 0;
    thisPtr->prgRamConfig = 0;
    thisPtr->irqCounter = 0;
    thisPtr->irqReloadValue = 0;
    thisPtr->reloadValueSignal = false;
    thisPtr->irqReloadFlag = false;
    thisPtr->irqEnableFlag = false;
    thisPtr->isPpuA12Set = ((*thisPtr->parent.ppuAddressBus & 0x1000)? true : false);
    thisPtr->irqFlag = false;
    thisPtr->singleIrqFlag = false;
    thisPtr->irqDetectedFlag = false;
    thisPtr->irqIgnoranceCounter = 0;
    thisPtr->irqOccurenceDelayCounter = 0;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_run(
    void * thisObj
)
{
    struct kaphein_nes_cart_TxROM *const thisPtr = (struct kaphein_nes_cart_TxROM *)thisObj;
    kaphein_UInt16 addr = *thisPtr->parent.ppuAddressBus;
    
    if(thisPtr->irqIgnoranceCounter > 0) {
        --thisPtr->irqIgnoranceCounter;
    }

    //(0x0000 ~ 0x1FFF일 때만 감지)
    if(addr < 0x2000) {
        if(addr & 0x1000) {
            //PPU A12가 0->1이 되면(상승 엣지)
            if(!thisPtr->irqIgnoranceCounter && !thisPtr->isPpuA12Set) {
                thisPtr->irqDetectedFlag = true;
                thisPtr->irqOccurenceDelayCounter = 4;
            }

            //신호 레벨(High) 기억
            thisPtr->isPpuA12Set = true;
        }
        else {
            //신호 레벨(Low) 기억
            thisPtr->isPpuA12Set = false;
        }
    }

    if(thisPtr->irqOccurenceDelayCounter > 0) {
        --thisPtr->irqOccurenceDelayCounter;
    }

    if(thisPtr->irqDetectedFlag && !thisPtr->irqOccurenceDelayCounter) {
        //카운터를 클럭시킴
        clockIrqCounterB(thisPtr);

        //이후 발생하는 7개 신호 무시
        thisPtr->irqIgnoranceCounter = 52;
        thisPtr->irqDetectedFlag = false;
    }

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_doCpuRead(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
)
{
    enum kaphein_ErrorCode errorCode = kapheinErrorCodeNoError;
    struct kaphein_nes_cart_TxROM *const thisPtr = (struct kaphein_nes_cart_TxROM *)thisObj;

    switch(address >> 13) {
    case 0x00:
    case 0x01:
    case 0x02:
        errorCode = kaphein_nes_Cartridge_doCpuRead(&thisPtr->parent, address, valueOut);
    break;
    case 0x03:
        if((thisPtr->prgRamConfig & HEPRGRAMConfig_ENABLE) != 0) {
            if(thisPtr->parent.memoryChipArray.prgRamSize > 0) {
                *valueOut = thisPtr->parent.memoryChipArray.prgRam[address & 0x1FFF];
            }
            else if(thisPtr->parent.memoryChipArray.prgNvRamSize > 0) {
                *valueOut = thisPtr->parent.memoryChipArray.prgNvRam[address & 0x1FFF];
            }
            else {
                *valueOut = *thisPtr->parent.cpuDataBus;
            }
        }
    break;
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
        errorCode = kaphein_nes_Cartridge_doCpuRead(&thisPtr->parent, address, valueOut);
    break;
    }

    return errorCode;
}

enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
)
{
    enum kaphein_ErrorCode errorCode = kapheinErrorCodeNoError;
    struct kaphein_nes_cart_TxROM *const thisPtr = (struct kaphein_nes_cart_TxROM *)thisObj;

    switch(address >> 12) {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
        errorCode = kaphein_nes_Cartridge_doCpuWrite(&thisPtr->parent, address, value);
    break;
    case 0x06:
    case 0x07:
        if((thisPtr->prgRamConfig & HEPRGRAMConfig_VALUES) == HEPRGRAMConfig_ENABLE) {
            if(thisPtr->parent.memoryChipArray.prgRamSize > 0) {
                thisPtr->parent.memoryChipArray.prgRam[address & 0x1FFF] = value;
            }
            else if(thisPtr->parent.memoryChipArray.prgNvRamSize > 0) {
                thisPtr->parent.memoryChipArray.prgNvRam[address & 0x1FFF] = value;
            }
        }
    break;
    case 0x08:
    case 0x09:
        switch(address & 0x01) {
        case 0x00:
            //뱅크 변경 모드 입력
            setBankMode(thisPtr, value);
        break;
        case 0x01:
            //뱅크 변경
            mapBank(thisPtr, value);
        break;
        }
    break;
    case 0x0A:
    case 0x0B:
        if(address & 0x0001) {
            //배터리 PRG RAM 설정
            thisPtr->prgRamConfig = value;
        }
        else {
            //네임 테이블 미러링 설정
            mapNameTable(thisPtr, value);
        }
    break;
    case 0x0C:
    case 0x0D:
        if(address & 0x0001) {
            thisPtr->irqCounter = 0;
            thisPtr->irqReloadFlag = true;
        }
        else {
            if(value) {
                thisPtr->reloadValueSignal = true;
            }
            else {
                if(thisPtr->reloadValueSignal) {
                    thisPtr->singleIrqFlag = true;
                }

                thisPtr->reloadValueSignal = false;
            }

            thisPtr->irqReloadValue = value;
        }
    break;
    case 0x0E:
    case 0x0F:
        if(address & 0x0001) {
            thisPtr->irqEnableFlag = true;
        }
        else {
            thisPtr->irqEnableFlag = false;

            if(thisPtr->irqFlag) {
                thisPtr->irqFlag = false;
                *thisPtr->parent.cpuIrqBus = kaphein_nes_InterruptSignal_NONE;
            }
        }
    break;
    }

    return errorCode;
}

/* **************************************************************** */

/* **************************************************************** */
/* Internal Definitions */

static
void
mapBank(
    struct kaphein_nes_cart_TxROM * thisPtr
    , kaphein_UInt8 v
)
{
    switch(thisPtr->bankConfig & HEBankConfig_BANK_SELECT) {
    case HEBankSelectConst_CHR00:
        if(thisPtr->parent.memoryChipArray.chrRomSize > 0) {
            thisPtr->chrAddressLatch[0] = thisPtr->parent.chrBankAddresses[0] = (((kaphein_SSize)(v & 0xFE)) % thisPtr->chrRomBankCount) << 10;
            thisPtr->parent.chrBankAddresses[1] = thisPtr->parent.chrBankAddresses[0] + 0x400;
        }
    break;
    case HEBankSelectConst_CHR08:
        if(thisPtr->parent.memoryChipArray.chrRomSize > 0) {
            thisPtr->chrAddressLatch[1] = thisPtr->parent.chrBankAddresses[2] = (((kaphein_SSize)(v & 0xFE)) % thisPtr->chrRomBankCount) << 10;
            thisPtr->parent.chrBankAddresses[3] = thisPtr->parent.chrBankAddresses[2] + 0x400;
        }
    break;
    case HEBankSelectConst_CHR10:
        if(thisPtr->parent.memoryChipArray.chrRomSize > 0) {
            thisPtr->chrAddressLatch[2] = thisPtr->parent.chrBankAddresses[4] = ((kaphein_SSize)v % thisPtr->chrRomBankCount) << 10;
        }
    break;
    case HEBankSelectConst_CHR14:
        if(thisPtr->parent.memoryChipArray.chrRomSize > 0) {
            thisPtr->chrAddressLatch[3] = thisPtr->parent.chrBankAddresses[5] = ((kaphein_SSize)v % thisPtr->chrRomBankCount) << 10;
        }
    break;
    case HEBankSelectConst_CHR18:
        if(thisPtr->parent.memoryChipArray.chrRomSize > 0) {
            thisPtr->chrAddressLatch[4] = thisPtr->parent.chrBankAddresses[6] = ((kaphein_SSize)v % thisPtr->chrRomBankCount) << 10;
        }
    break;
    case HEBankSelectConst_CHR1C:
        if(thisPtr->parent.memoryChipArray.chrRomSize > 0) {
            thisPtr->chrAddressLatch[5] = thisPtr->parent.chrBankAddresses[7] = ((kaphein_SSize)v % thisPtr->chrRomBankCount) << 10;
        }
    break;
    case HEBankSelectConst_PRG8:
        thisPtr->prgRomAddressLatch = thisPtr->parent.prgBankAddresses[((thisPtr->bankConfig & HEBankConfig_PRG_MODE)?(2):(0))] = (((kaphein_SSize)(v & 0x3F)) % thisPtr->prgRomBankCount) << 13;
    break;
    case HEBankSelectConst_PRGA:
        thisPtr->parent.prgBankAddresses[1] = (((kaphein_SSize)(v & 0x3F)) % thisPtr->prgRomBankCount) << 13;
    break;
    }
}

static
void
setBankMode(
    struct kaphein_nes_cart_TxROM * thisPtr
    , kaphein_UInt8 v
)
{
    //뱅크 변경 모드 입력
    thisPtr->bankConfig = v;
    
    if(v & HEBankConfig_PRG_MODE) {
        thisPtr->parent.prgBankAddresses[0] = thisPtr->parent.lastPrgBankAddresses[0];
        thisPtr->parent.prgBankAddresses[2] = thisPtr->prgRomAddressLatch;
    }
    else {
        thisPtr->parent.prgBankAddresses[0] = thisPtr->prgRomAddressLatch;
        thisPtr->parent.prgBankAddresses[2] = thisPtr->parent.lastPrgBankAddresses[0];
    }
    
    if(v & HEBankConfig_CHR_MODE) {
        thisPtr->parent.chrBankAddresses[0] = thisPtr->chrAddressLatch[2];
        thisPtr->parent.chrBankAddresses[1] = thisPtr->chrAddressLatch[3];
        thisPtr->parent.chrBankAddresses[2] = thisPtr->chrAddressLatch[4];
        thisPtr->parent.chrBankAddresses[3] = thisPtr->chrAddressLatch[5];
        thisPtr->parent.chrBankAddresses[4] = thisPtr->chrAddressLatch[0];
        thisPtr->parent.chrBankAddresses[5] = thisPtr->parent.chrBankAddresses[4] + 0x400;
        thisPtr->parent.chrBankAddresses[6] = thisPtr->chrAddressLatch[1];
        thisPtr->parent.chrBankAddresses[7] = thisPtr->parent.chrBankAddresses[6] + 0x400;
    }
    else {
        thisPtr->parent.chrBankAddresses[0] = thisPtr->chrAddressLatch[0];
        thisPtr->parent.chrBankAddresses[1] = thisPtr->parent.chrBankAddresses[0] + 0x400;
        thisPtr->parent.chrBankAddresses[2] = thisPtr->chrAddressLatch[1];
        thisPtr->parent.chrBankAddresses[3] = thisPtr->parent.chrBankAddresses[2] + 0x400;
        thisPtr->parent.chrBankAddresses[4] = thisPtr->chrAddressLatch[2];
        thisPtr->parent.chrBankAddresses[5] = thisPtr->chrAddressLatch[3];
        thisPtr->parent.chrBankAddresses[6] = thisPtr->chrAddressLatch[4];
        thisPtr->parent.chrBankAddresses[7] = thisPtr->chrAddressLatch[5];
    }
}

static
void
mapNameTable(
    struct kaphein_nes_cart_TxROM * thisPtr
    , kaphein_UInt8 v
)
{
    switch(v & 0x01) {
    case 0x00:
        //Vertical
        thisPtr->parent.nametableAddresses[1] = 0x0400;
        thisPtr->parent.nametableAddresses[2] = 0x0000;
    break;
    case 0x01:
        //Horizontal
        thisPtr->parent.nametableAddresses[1] = 0x0000;
        thisPtr->parent.nametableAddresses[2] = 0x0400;
    break;
    }
}

static
void
clockIrqCounterA(
    struct kaphein_nes_cart_TxROM * thisPtr
)
{
    //static const nes::console::HRP2C02 * ppu = shell::wcMainConsole->getPPUPointer();

    if(thisPtr->irqReloadFlag) {                  //리로드 플래그가 셋 되어 있으면
        thisPtr->irqCounter = thisPtr->irqReloadValue;    //카운터 리로드
        thisPtr->irqReloadFlag = false;          //리로드 플래그 클리어
    }
    else if(thisPtr->irqCounter) {
        if(!(--thisPtr->irqCounter)) {            //카운터 감소, 0이면
            thisPtr->irqReloadFlag = true;       //리로드 플래그 셋

            if(thisPtr->irqEnableFlag) {          //IRQ가 활성화되어 있으면
                thisPtr->irqFlag = true;         //IRQ 플래그 셋
                *thisPtr->parent.cpuIrqBus = kaphein_nes_InterruptSignal_OCCUR;
                thisPtr->singleIrqFlag = false;
                //_tprintf(_T("MMC3 IRQ on scanline == %3d, cycles == %3d\n"), ppu->getScanline(), ppu->getCycles());
            }
        }
    }

    if(thisPtr->singleIrqFlag) {
        if(thisPtr->irqEnableFlag) {              //IRQ가 활성화되어 있으면
            thisPtr->irqFlag = true;             //IRQ 플래그 셋
            *thisPtr->parent.cpuIrqBus = kaphein_nes_InterruptSignal_OCCUR;
        }

        thisPtr->singleIrqFlag = false;
    }

}

static
void
clockIrqCounterB(
    struct kaphein_nes_cart_TxROM * thisPtr
)
{
    if(thisPtr->irqReloadFlag) {                  //리로드 플래그가 셋 되어 있으면
        thisPtr->irqCounter = thisPtr->irqReloadValue;    //카운터 리로드
        thisPtr->irqReloadFlag = false;          //리로드 플래그 클리어
    }
    else if(thisPtr->irqCounter) --thisPtr->irqCounter;   //아니면 카운터 감소

    if(!thisPtr->irqCounter) {                    //카운터가 0이면
        thisPtr->irqReloadFlag = true;           //리로드 플래그 셋
        
        if(thisPtr->irqEnableFlag){              //IRQ가 활성화되어 있으면
            thisPtr->irqFlag = true;             //IRQ 플래그 셋
            *thisPtr->parent.cpuIrqBus = kaphein_nes_InterruptSignal_OCCUR;
        }
    }
}

/* **************************************************************** */
