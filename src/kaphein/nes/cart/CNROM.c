#include "kaphein/nes/cart/CNROM.h"

//TODO : 오류 코드 처리

static
const struct kaphein_nes_Cartridge_VTable cartVTable = {
    kaphein_nes_cart_CNROM_destruct
    , kaphein_nes_Cartridge_setNametableMemory
    , kaphein_nes_Cartridge_setBuses
    , kaphein_nes_Cartridge_powerUp
    , kaphein_nes_Cartridge_reset
    , kaphein_nes_Cartridge_run
    , kaphein_nes_Cartridge_doCpuRead
    , kaphein_nes_cart_CNROM_doCpuWrite
    , kaphein_nes_Cartridge_doPpuRead
    , kaphein_nes_Cartridge_doPpuWrite
};

enum kaphein_ErrorCode
kaphein_nes_cart_CNROM_construct(
    struct kaphein_nes_cart_CNROM * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
)
{
    kaphein_nes_Cartridge_construct(
        &thisObj->parent
        , stream
        , romHeader
        , allocator
    );
    thisObj->parent.vTable = &cartVTable;

    kaphein_nes_NesRomHeader_getChrRomBankCount(&thisObj->parent.romHeader, &thisObj->chrRomBankCount);

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_cart_CNROM_destruct(
    void * thisObj
)
{
    struct kaphein_nes_cart_CNROM *const thisPtr = (struct kaphein_nes_cart_CNROM *)thisObj;
    
    return kaphein_nes_Cartridge_destruct(&thisPtr->parent);
}

enum kaphein_ErrorCode
kaphein_nes_cart_CNROM_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
)
{
    struct kaphein_nes_cart_CNROM *const thisPtr = (struct kaphein_nes_cart_CNROM *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    int i;

    switch((address & 0x8000) >> 15) {
    case 0x00:
        resultErrorCode = kaphein_nes_Cartridge_doCpuWrite(&thisPtr->parent, address, value);
    break;
    case 0x01:
        thisPtr->parent.chrBankAddresses[0] = ((value & 0x03) % thisPtr->chrRomBankCount) << 13;
        
        for(i = 1; i < 8; ++i) {
            thisPtr->parent.chrBankAddresses[i] = thisPtr->parent.chrBankAddresses[i - 1] + 0x400;
        }
    break;
    }

    return resultErrorCode;
}
