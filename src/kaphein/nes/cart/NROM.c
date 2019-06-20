#include "kaphein/mem/utils.h"
#include "kaphein/nes/cart/NROM.h"

//TODO : 오류 코드 처리

static
const struct kaphein_nes_Cartridge_VTable parentVTable = {
    kaphein_nes_cart_NROM_destruct
    , kaphein_nes_Cartridge_setNametableMemory
    , kaphein_nes_Cartridge_setBuses
    , kaphein_nes_Cartridge_powerUp
    , kaphein_nes_Cartridge_reset
    , kaphein_nes_Cartridge_run
    , kaphein_nes_Cartridge_doCpuRead
    , kaphein_nes_Cartridge_doCpuWrite
    , kaphein_nes_Cartridge_doPpuRead
    , kaphein_nes_Cartridge_doPpuWrite
};

enum kaphein_ErrorCode
kaphein_nes_cart_NROM_construct(
    struct kaphein_nes_cart_NROM * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    resultErrorCode = kaphein_nes_Cartridge_construct(
        &thisObj->parent
        , stream
        , romHeader
        , allocator
    );
    thisObj->parent.vTable = &parentVTable;

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_cart_NROM_destruct(
    void * thisObj
)
{
    struct kaphein_nes_cart_NROM *const thisPtr = (struct kaphein_nes_cart_NROM *)thisObj;

    return kaphein_nes_Cartridge_destruct(&thisPtr->parent);
}
