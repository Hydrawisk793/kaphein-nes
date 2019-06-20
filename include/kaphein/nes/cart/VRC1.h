#ifndef KAPHEIN_HGRD_kaphein_nes_cart_VRC1_h
#define KAPHEIN_HGRD_kaphein_nes_cart_VRC1_h

#include "../Cartridge.h"

struct kaphein_nes_cart_VRC1
{
    struct kaphein_nes_Cartridge parent;
    
    kaphein_SSize prgROMBankCount;

    kaphein_SSize chrROMBankCount;

    bool hasFourScreens;

    bool chr0MSB;

    bool chr1MSB;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_VRC1_construct(
    struct kaphein_nes_cart_VRC1 * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_VRC1_destruct(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_VRC1_powerUp(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_VRC1_reset(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_VRC1_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
);

#endif
