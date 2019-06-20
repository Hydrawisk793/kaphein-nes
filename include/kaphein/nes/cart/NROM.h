#ifndef KAPHEIN_HGRD_kaphein_nes_cart_NROM_h
#define KAPHEIN_HGRD_kaphein_nes_cart_NROM_h

#include "../Cartridge.h"

struct kaphein_nes_cart_NROM
{
    struct kaphein_nes_Cartridge parent;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_NROM_construct(
    struct kaphein_nes_cart_NROM * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_NROM_destruct(
    void * thisObj
);

#endif
