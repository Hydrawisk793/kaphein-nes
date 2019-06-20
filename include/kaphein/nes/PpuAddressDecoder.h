#ifndef KAPHEIN_HGRD_kaphein_nes_PpuAddressDecoder_h
#define KAPHEIN_HGRD_kaphein_nes_PpuAddressDecoder_h

#include "kaphein/ErrorCode.h"
#include "AddressDecoder.h"
#include "RP2C02.h"
#include "Cartridge.h"

struct kaphein_nes_PpuAddressDecoder
{
    struct kaphein_nes_AddressDecoder parent;
    
    void * impl_;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_construct(
    struct kaphein_nes_PpuAddressDecoder * thisObj
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_destruct(
    struct kaphein_nes_PpuAddressDecoder * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_connectPpu(
    struct kaphein_nes_PpuAddressDecoder * thisObj
    , struct kaphein_nes_RP2C02 * ppu
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_connectNametableMemory(
    struct kaphein_nes_PpuAddressDecoder * thisObj
    , kaphein_UInt8 * ram
    , kaphein_SSize ramSize
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_connectCartridge(
    struct kaphein_nes_PpuAddressDecoder * thisObj
    , struct kaphein_nes_Cartridge * cart
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_read(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_write(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
);

#endif
