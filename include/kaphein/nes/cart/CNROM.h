#ifndef KAPHEIN_HGRD_kaphein_nes_cart_CNROM_h
#define KAPHEIN_HGRD_kaphein_nes_cart_CNROM_h

#include "../Cartridge.h"

/**
 *  @brief iNES mapper 003 and 185
 */
struct kaphein_nes_cart_CNROM
{
    struct kaphein_nes_Cartridge parent;

    kaphein_SSize chrRomBankCount;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_CNROM_construct(
    struct kaphein_nes_cart_CNROM * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_CNROM_destruct(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_CNROM_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
);

#endif
