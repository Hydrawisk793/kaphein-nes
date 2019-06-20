#include "kaphein/nes/cart/UxROM.h"

//TODO : 오류 코드 처리

static
const struct kaphein_nes_Cartridge_VTable cartVTable = {
    kaphein_nes_cart_UxROM_destruct
    , kaphein_nes_Cartridge_setNametableMemory
    , kaphein_nes_Cartridge_setBuses
    , kaphein_nes_Cartridge_powerUp
    , kaphein_nes_Cartridge_reset
    , kaphein_nes_Cartridge_run
    , kaphein_nes_Cartridge_doCpuRead
    , kaphein_nes_cart_UxROM_doCpuWrite
    , kaphein_nes_Cartridge_doPpuRead
    , kaphein_nes_Cartridge_doPpuWrite
};

enum kaphein_ErrorCode
kaphein_nes_cart_UxROM_construct(
    struct kaphein_nes_cart_UxROM * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
)
{
    int mapper;

    kaphein_nes_Cartridge_construct(
        &thisObj->parent
        , stream
        , romHeader
        , allocator
    );
    thisObj->parent.vTable = &cartVTable;

    kaphein_nes_NesRomHeader_getPrgRomBankCount(&thisObj->parent.romHeader, &thisObj->prgROMBankCount);

    kaphein_nes_NesRomHeader_getMapper(&thisObj->parent.romHeader, &mapper, KAPHEIN_NULL);
    switch(mapper) {
    case 94:
        thisObj->uxromMode = kaphein_nes_cart_UxROM_Mode_UN1ROM;
    break;
    case 180:
        thisObj->uxromMode = kaphein_nes_cart_UxROM_Mode_UNROM02;
    break;
    default:
        thisObj->uxromMode = kaphein_nes_cart_UxROM_Mode_UNROM;
    break;
    }

    switch(thisObj->uxromMode) {
    case kaphein_nes_cart_UxROM_Mode_UNROM:
    break;
    case kaphein_nes_cart_UxROM_Mode_UN1ROM:
    break;
    case kaphein_nes_cart_UxROM_Mode_UNROM02:
        thisObj->parent.prgBankAddresses[0] = 0x0000;
        thisObj->parent.prgBankAddresses[1] = 0x2000;
    break;
    }

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_cart_UxROM_destruct(
    void * thisObj
)
{
    struct kaphein_nes_cart_UxROM *const thisPtr = (struct kaphein_nes_cart_UxROM *)thisObj;
    
    return kaphein_nes_Cartridge_destruct(&thisPtr->parent);
}

enum kaphein_ErrorCode
kaphein_nes_cart_UxROM_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
)
{
    struct kaphein_nes_cart_UxROM *const thisPtr = (struct kaphein_nes_cart_UxROM *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    
    switch(thisPtr->uxromMode) {
    case kaphein_nes_cart_UxROM_Mode_UNROM:
        switch((address & 0x8000) >> 15) {
        case 0x00:
            resultErrorCode = kaphein_nes_Cartridge_doCpuWrite(&thisPtr->parent, address, value);
        break;
        case 0x01:
            thisPtr->parent.prgBankAddresses[0] = (((value & 0x0F) % thisPtr->prgROMBankCount) << 14);
            thisPtr->parent.prgBankAddresses[1] = thisPtr->parent.prgBankAddresses[0] + 0x2000;
        break;
        }
    break;
    case kaphein_nes_cart_UxROM_Mode_UN1ROM:
        switch((address & 0x8000) >> 15) {
        case 0x00:
            resultErrorCode = kaphein_nes_Cartridge_doCpuWrite(&thisPtr->parent, address, value);
        break;
        case 0x01:
            thisPtr->parent.prgBankAddresses[0] = ((((value & 0x3C) >> 2) % thisPtr->prgROMBankCount) << 14);
            thisPtr->parent.prgBankAddresses[1] = thisPtr->parent.prgBankAddresses[0] + 0x2000;
        break;
        }
    break;
    case kaphein_nes_cart_UxROM_Mode_UNROM02:
        switch((address & 0x8000) >> 15) {
        case 0x00:
            resultErrorCode = kaphein_nes_Cartridge_doCpuWrite(&thisPtr->parent, address, value);
        break;
        case 0x01:
            thisPtr->parent.prgBankAddresses[2] = (((value & 0x07) % thisPtr->prgROMBankCount) << 14);
            thisPtr->parent.prgBankAddresses[3] = thisPtr->parent.prgBankAddresses[2] + 0x2000;
        break;
        }
    break;
    }

    return resultErrorCode;
}
