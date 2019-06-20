#ifndef KAPHEIN_HGRD_kaphein_nes_cart_SxROM_h
#define KAPHEIN_HGRD_kaphein_nes_cart_SxROM_h

#include "../Cartridge.h"

struct kaphein_nes_cart_SxROM
{
    struct kaphein_nes_Cartridge parent;

    kaphein_SSize prgRomBankCount;

    kaphein_SSize chrRomBankCount;

    kaphein_UInt8 control;

    kaphein_UInt8 prgBank;

    kaphein_UInt8 chrBank0;

    kaphein_UInt8 chrBank1;

    kaphein_UInt8 serialPort;

    kaphein_UInt8 writeCount;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_SxROM_construct(
    struct kaphein_nes_cart_SxROM * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_SxROM_destruct(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_SxROM_powerUp(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_SxROM_reset(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_SxROM_doCpuRead(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_SxROM_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
);

#endif
