#ifndef KAPHEIN_HGRD_kaphein_nes_cart_UxROM_h
#define KAPHEIN_HGRD_kaphein_nes_cart_UxROM_h

#include "../Cartridge.h"

enum kaphein_nes_cart_UxROM_Mode
{
    kaphein_nes_cart_UxROM_Mode_UNROM = 0
    , kaphein_nes_cart_UxROM_Mode_UN1ROM
    , kaphein_nes_cart_UxROM_Mode_UNROM02
};

struct kaphein_nes_cart_UxROM
{
    struct kaphein_nes_Cartridge parent;

    kaphein_SSize prgROMBankCount;

    enum kaphein_nes_cart_UxROM_Mode uxromMode;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_UxROM_construct(
    struct kaphein_nes_cart_UxROM * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_UxROM_destruct(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_UxROM_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
);

#endif
