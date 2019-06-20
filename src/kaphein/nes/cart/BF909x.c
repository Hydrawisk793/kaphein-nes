#include "kaphein/nes/cart/BF909x.h"

static
const struct kaphein_nes_Cartridge_VTable cartVTable = {
    kaphein_nes_cart_BF909x_destruct
    , kaphein_nes_Cartridge_setNametableMemory
    , kaphein_nes_Cartridge_setBuses
    , kaphein_nes_Cartridge_powerUp
    , kaphein_nes_Cartridge_reset
    , kaphein_nes_Cartridge_run
    , kaphein_nes_Cartridge_doCpuRead
    , kaphein_nes_cart_BF909x_doCpuWrite
    , kaphein_nes_Cartridge_doPpuRead
    , kaphein_nes_Cartridge_doPpuWrite
};

static
void setOneScreenMirroring(
    struct kaphein_nes_cart_BF909x* thisPtr
    , kaphein_UInt8 writtenValue
)
{
    thisPtr->parent.parent.nametableAddresses[0] = ((writtenValue & 0x10) >> 4) * 0x400;
    thisPtr->parent.parent.nametableAddresses[1] = thisPtr->parent.parent.nametableAddresses[0];
    thisPtr->parent.parent.nametableAddresses[2] = thisPtr->parent.parent.nametableAddresses[0];
    thisPtr->parent.parent.nametableAddresses[3] = thisPtr->parent.parent.nametableAddresses[0];
}

static
void selectLast16KbPrgBank(
    struct kaphein_nes_cart_BF909x* thisPtr
)
{
    thisPtr->parent.parent.prgBankAddresses[2] = ((((thisPtr->prgBankNumber & 0x0C) | 0x03) % thisPtr->parent.prgROMBankCount) << 14);
    thisPtr->parent.parent.prgBankAddresses[3] = thisPtr->parent.parent.prgBankAddresses[2] + 0x2000;
}

enum kaphein_ErrorCode
kaphein_nes_cart_BF909x_construct(
    struct kaphein_nes_cart_BF909x* thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
)
{
    int mapper, subMapper;

    kaphein_nes_cart_UxROM_construct(
        &thisObj->parent
        , stream
        , romHeader
        , allocator
    );
    thisObj->parent.parent.vTable = &cartVTable;
    
    kaphein_nes_NesRomHeader_getMapper(&thisObj->parent.parent.romHeader, &mapper, &subMapper);
    switch(mapper) {
    case 71:
        switch(subMapper) {
        case 0:
            thisObj->bf909xMode = kaphein_nes_cart_BF909x_Mode_BF9093;
        break;
        case 1:
            thisObj->bf909xMode = kaphein_nes_cart_BF909x_Mode_BF9097;
        break;
        default:
            thisObj->bf909xMode = kaphein_nes_cart_BF909x_Mode_BF9093;
        break;
        }
    break;
    case 232:
        thisObj->bf909xMode = kaphein_nes_cart_BF909x_Mode_BF9096;
    break;
    default:
        thisObj->bf909xMode = kaphein_nes_cart_BF909x_Mode_BF9093;
    break;
    }

    return kapheinErrorCodeNoError;   
}

enum kaphein_ErrorCode
kaphein_nes_cart_BF909x_destruct(
    void * thisObj
)
{
    struct kaphein_nes_cart_BF909x *const thisPtr = (struct kaphein_nes_cart_BF909x *)thisObj;
    
    return kaphein_nes_cart_UxROM_destruct(&thisPtr->parent);
}

enum kaphein_ErrorCode
kaphein_nes_cart_BF909x_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
)
{
    struct kaphein_nes_cart_BF909x *const thisPtr = (struct kaphein_nes_cart_BF909x *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    switch(thisPtr->bf909xMode) {
    case kaphein_nes_cart_BF909x_Mode_BF9093:
        switch(address >> 13) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            resultErrorCode = kaphein_nes_cart_UxROM_doCpuWrite(&thisPtr->parent, address, value);
        break;
        case 0x04:
            //For iNES header compatibility.
            setOneScreenMirroring(thisPtr, value);
        break;
        case 0x05:
        break;
        case 0x06:
        case 0x07:
            resultErrorCode = kaphein_nes_cart_UxROM_doCpuWrite(&thisPtr->parent, address, value);
        break;
        }
    break;
    case kaphein_nes_cart_BF909x_Mode_BF9096:
        switch(address >> 14) {
        case 0x00:
        case 0x01:
            resultErrorCode = kaphein_nes_cart_UxROM_doCpuWrite(&thisPtr->parent, address, value);
        break;
        case 0x02:
            thisPtr->prgBankNumber = (thisPtr->prgBankNumber & 0x03) | ((value & 0x18) >> 1);

            resultErrorCode = kaphein_nes_cart_UxROM_doCpuWrite(
                &thisPtr->parent
                , 0xC000
                , thisPtr->prgBankNumber
            );
            selectLast16KbPrgBank(thisPtr);
        break;
        case 0x03:
            thisPtr->prgBankNumber = (thisPtr->prgBankNumber & 0x0C) | (value & 0x03);
            
            resultErrorCode = kaphein_nes_cart_UxROM_doCpuWrite(
                &thisPtr->parent
                , 0xC000
                , thisPtr->prgBankNumber
            );
        break;
        }
    break;
    case kaphein_nes_cart_BF909x_Mode_BF9097:
        switch(address >> 13) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            resultErrorCode = kaphein_nes_cart_UxROM_doCpuWrite(&thisPtr->parent, address, value);
        break;
        case 0x04:
            setOneScreenMirroring(thisPtr, value);
        break;
        case 0x05:
        break;
        case 0x06:
        case 0x07:
            //The maximum PRG ROM size of BF9097 is 128KB.
            resultErrorCode = kaphein_nes_cart_UxROM_doCpuWrite(&thisPtr->parent, address, value & 0x07);
        break;
        }
    break;
    }

    return resultErrorCode;
}
