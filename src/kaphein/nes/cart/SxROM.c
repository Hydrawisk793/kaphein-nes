#include "kaphein/mem/utils.h"
#include "kaphein/nes/ShiftRegister8.h"
#include "kaphein/nes/cart/SxROM.h"

//TODO : 오류 코드 처리

enum HELoader
{
    HELoader_LDR_RESET = 0x80,
    HELoader_LDR_UNUSED = 0x7E,
    HELoader_LDR_DATA = 0x01,
    HELoader_LDR_VALUES = HELoader_LDR_RESET | HELoader_LDR_DATA
};

enum HEControl
{
    HEControl_CTR_CHR_MODE = 0x10,
    HEControl_CTR_PRG_MODE = 0x0C,
    HEControl_CTR_MIRRORING = 0x03
};

enum HECHRBank
{
    HECHRBank_CB_CHR_BANK_NO = 0x1F
};

enum HEPRGBank
{
    HEPRGBank_PB_DISABLE_PRG_RAM = 0x10,
    HEPRGBank_PB_PRG_BANK_NO = 0x0F
};

void mapPRGROMBank(
    struct kaphein_nes_cart_SxROM * thisPtr
)
{
    switch((thisPtr->control & HEControl_CTR_PRG_MODE) >> 2) {
    case 0:
    case 1:
        //TODO : 동작 확인
        thisPtr->parent.prgBankAddresses[0] = (((thisPtr->prgBank & HEPRGBank_PB_PRG_BANK_NO) % thisPtr->prgRomBankCount) >> 1) * 0x8000;
        thisPtr->parent.prgBankAddresses[1] = thisPtr->parent.prgBankAddresses[0] + 0x2000;
        thisPtr->parent.prgBankAddresses[2] = thisPtr->parent.prgBankAddresses[1] + 0x2000;
        thisPtr->parent.prgBankAddresses[3] = thisPtr->parent.prgBankAddresses[2] + 0x2000;
    break;
    case 2:
        thisPtr->parent.prgBankAddresses[0] = 0;
        thisPtr->parent.prgBankAddresses[1] = 0x2000;
        thisPtr->parent.prgBankAddresses[2] = ((thisPtr->prgBank & HEPRGBank_PB_PRG_BANK_NO) % thisPtr->prgRomBankCount) * 0x4000;
        thisPtr->parent.prgBankAddresses[3] = thisPtr->parent.prgBankAddresses[2] + 0x2000;
    break;
    case 3:
        thisPtr->parent.prgBankAddresses[0] = ((thisPtr->prgBank & HEPRGBank_PB_PRG_BANK_NO) % thisPtr->prgRomBankCount) * 0x4000;
        thisPtr->parent.prgBankAddresses[1] = thisPtr->parent.prgBankAddresses[0] + 0x2000;
        thisPtr->parent.prgBankAddresses[2] = (thisPtr->prgRomBankCount - 1) * 0x4000;
        thisPtr->parent.prgBankAddresses[3] = thisPtr->parent.prgBankAddresses[2] + 0x2000;
    break;
    }
}

static
void
mapCHRROMBank0(
    struct kaphein_nes_cart_SxROM * thisPtr
)
{
    int r1;
    
    //4KB 모드
    if(thisPtr->control & HEControl_CTR_CHR_MODE) {
        for(r1 = 0; r1 < 4; ++r1) {
            thisPtr->parent.chrBankAddresses[r1] = (thisPtr->chrBank0 & 0x1F) * 0x1000 + r1 * 0x400;
        }
    }
    //8KB 모드
    else {
        for(r1 = 0; r1 < 8; ++r1) {
            thisPtr->parent.chrBankAddresses[r1] = ((thisPtr->chrBank0 & 0x1F) >> 1) * 0x2000 + r1 * 0x400;
        }
    }
}

static
void mapCHRROMBank1(
    struct kaphein_nes_cart_SxROM * thisPtr
)
{
    int r1;

    //4KB 모드
    if(thisPtr->control & HEControl_CTR_CHR_MODE) {
        for(r1 = 4; r1 < 8; ++r1) {
            thisPtr->parent.chrBankAddresses[r1] = (thisPtr->chrBank1 & 0x1F) * 0x1000 + (r1 - 4) * 0x400;
        }
    }
}

static
void
mapNameTable(
    struct kaphein_nes_cart_SxROM * thisPtr
)
{
    switch(thisPtr->control & HEControl_CTR_MIRRORING) {
    case 0:
        thisPtr->parent.nametableAddresses[0] = 0x0000;
        thisPtr->parent.nametableAddresses[1] = 0x0000;
        thisPtr->parent.nametableAddresses[2] = 0x0000;
        thisPtr->parent.nametableAddresses[3] = 0x0000;
    break;
    case 1:
        thisPtr->parent.nametableAddresses[0] = 0x0400;
        thisPtr->parent.nametableAddresses[1] = 0x0400;
        thisPtr->parent.nametableAddresses[2] = 0x0400;
        thisPtr->parent.nametableAddresses[3] = 0x0400;
    break;
    case 2:
        thisPtr->parent.nametableAddresses[0] = 0x0000;
        thisPtr->parent.nametableAddresses[1] = 0x0400;
        thisPtr->parent.nametableAddresses[2] = 0x0000;
        thisPtr->parent.nametableAddresses[3] = 0x0400;
    break;
    case 3:
        thisPtr->parent.nametableAddresses[0] = 0x0000;
        thisPtr->parent.nametableAddresses[1] = 0x0000;
        thisPtr->parent.nametableAddresses[2] = 0x0400;
        thisPtr->parent.nametableAddresses[3] = 0x0400;
    break;
    }
};

static
bool
loadBit(
    struct kaphein_nes_cart_SxROM * thisPtr
    , kaphein_UInt8 * destRegister
    , kaphein_UInt8 v
)
{
    //리셋 비트가 1이면
    if(v & HELoader_LDR_RESET) {
        thisPtr->serialPort = 0;
        thisPtr->writeCount = 0;

        //컨트롤 레지스터 리셋
        thisPtr->control |= HEControl_CTR_PRG_MODE;

        return true;
    }
    //아니면 LSB 쓰기
    else {
        kaphein_nes_ShiftRegister8_shiftRight(&thisPtr->serialPort, !!(v & HELoader_LDR_DATA));

        //쓰기 카운터 증가, 5이면
        if(++thisPtr->writeCount >= 5) {
            //목적지 레지스터에 값 쓰기
            *destRegister = thisPtr->serialPort >> 3;
            thisPtr->writeCount = 0;

            return true;
        }
    }

    return false;
}

static
const struct kaphein_nes_Cartridge_VTable cartVTable = {
    kaphein_nes_cart_SxROM_destruct
    , kaphein_nes_Cartridge_setNametableMemory
    , kaphein_nes_Cartridge_setBuses
    , kaphein_nes_cart_SxROM_powerUp
    , kaphein_nes_cart_SxROM_reset
    , kaphein_nes_Cartridge_run
    , kaphein_nes_cart_SxROM_doCpuRead
    , kaphein_nes_cart_SxROM_doCpuWrite
    , kaphein_nes_Cartridge_doPpuRead
    , kaphein_nes_Cartridge_doPpuWrite
};

enum kaphein_ErrorCode
kaphein_nes_cart_SxROM_construct(
    struct kaphein_nes_cart_SxROM * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
)
{
    kaphein_SSize prgRamSize;
    int version;
    bool hasPrgNvRam;
    
    kaphein_nes_Cartridge_construct(
        &thisObj->parent
        , stream
        , romHeader
        , allocator
    );
    thisObj->parent.vTable = &cartVTable;

    thisObj->control = 0;
    thisObj->prgBank = 0;
    thisObj->chrBank0 = 0;
    thisObj->chrBank1 = 0;
    thisObj->serialPort = 0;
    thisObj->writeCount = 0;
    kaphein_nes_NesRomHeader_getPrgRomBankCount(&thisObj->parent.romHeader, &thisObj->prgRomBankCount);
    kaphein_nes_NesRomHeader_getChrRomBankCount(&thisObj->parent.romHeader, &thisObj->chrRomBankCount);

    kaphein_nes_NesRomHeader_getVersion(&thisObj->parent.romHeader, &version);
    kaphein_nes_NesRomHeader_hasPrgNvRam(&thisObj->parent.romHeader, &hasPrgNvRam);
    switch(version) {
    case 0:
        prgRamSize = 0x8000;
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
        prgRamSize = 0x8000;
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
kaphein_nes_cart_SxROM_destruct(
    void * thisObj
)
{
    struct kaphein_nes_cart_SxROM *const thisPtr = (struct kaphein_nes_cart_SxROM *)thisObj;

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
kaphein_nes_cart_SxROM_powerUp(
    void * thisObj
)
{
    return kaphein_nes_cart_SxROM_reset(thisObj);
}

enum kaphein_ErrorCode
kaphein_nes_cart_SxROM_reset(
    void * thisObj
)
{
    struct kaphein_nes_cart_SxROM *const thisPtr = (struct kaphein_nes_cart_SxROM *)thisObj;
    
    kaphein_nes_Cartridge_reset(&thisPtr->parent);

    thisPtr->serialPort = 0;
    thisPtr->writeCount = 0;

    //기본 PRG 모드 셋
    thisPtr->control = 0x0C;

    mapNameTable(thisPtr);
    
    //TODO : 초기형 MMC1 칩 디텍션
    //I curse you nintendo who made the original MMC1 chip!!!
    //This behaviour is needed for early MMC1 chips
    //that does not set the last PRG bank on the reset sequence.

    //thisPtr->parent.prgBankAddresses[0] = 0x0000;
    //thisPtr->parent.prgBankAddresses[1] = 0x2000;
    //thisPtr->parent.prgBankAddresses[2] = 0x0000;
    //thisPtr->parent.prgBankAddresses[3] = 0x2000;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_cart_SxROM_doCpuRead(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
)
{
    enum kaphein_ErrorCode errorCode = kapheinErrorCodeNoError;
    struct kaphein_nes_cart_SxROM *const thisPtr = (struct kaphein_nes_cart_SxROM *)thisObj;

    switch(address >> 13) {
    case 0x00:
    case 0x01:
    case 0x02:
        errorCode = kaphein_nes_Cartridge_doCpuRead(&thisPtr->parent, address, valueOut);
    break;
    case 0x03:
        if((thisPtr->prgBank & HEPRGBank_PB_DISABLE_PRG_RAM) == 0) {
            if(thisPtr->parent.memoryChipArray.prgRamSize > 0) {
                *valueOut = thisPtr->parent.memoryChipArray.prgRam[(address & 0x1FFF)];
            }
            else if(thisPtr->parent.memoryChipArray.prgNvRamSize > 0) {
                *valueOut = thisPtr->parent.memoryChipArray.prgNvRam[(address & 0x1FFF)];
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
kaphein_nes_cart_SxROM_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
)
{
    enum kaphein_ErrorCode errorCode = kapheinErrorCodeNoError;
    struct kaphein_nes_cart_SxROM *const thisPtr = (struct kaphein_nes_cart_SxROM *)thisObj;
    
    switch(address >> 13) {
    case 0x00:
    case 0x01:
    case 0x02:
        errorCode = kaphein_nes_Cartridge_doCpuWrite(&thisPtr->parent, address, value);
    break;
    case 0x03:
        if((thisPtr->prgBank & HEPRGBank_PB_DISABLE_PRG_RAM) == 0) {
            if(thisPtr->parent.memoryChipArray.prgRamSize > 0) {
                thisPtr->parent.memoryChipArray.prgRam[(address & 0x1FFF)] = value;
            }
            else if(thisPtr->parent.memoryChipArray.prgNvRamSize > 0) {
                thisPtr->parent.memoryChipArray.prgNvRam[(address & 0x1FFF)] = value;
            }
        }
    break;
    case 0x04:
        if(loadBit(thisPtr, &thisPtr->control, value)) {
            mapPRGROMBank(thisPtr);
            mapNameTable(thisPtr);
            mapCHRROMBank0(thisPtr);
            mapCHRROMBank1(thisPtr);
        }
    break;
    case 0x05:
        if(loadBit(thisPtr, &thisPtr->chrBank0, value)) {
            mapCHRROMBank0(thisPtr);
        }
    break;
    case 0x06:
        if(loadBit(thisPtr, &thisPtr->chrBank1, value)) {
            mapCHRROMBank1(thisPtr);
        }
    break;
    case 0x07:
        if(loadBit(thisPtr, &thisPtr->prgBank, value)) {
            mapPRGROMBank(thisPtr);
        }
    break;
    }
    
    return errorCode;
}
