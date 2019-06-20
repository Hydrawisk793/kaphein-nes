#ifndef KAPHEIN_HGRD_kaphein_nes_CpuAddressDecoder_h
#define KAPHEIN_HGRD_kaphein_nes_CpuAddressDecoder_h

#include "kaphein/ErrorCode.h"
#include "AddressDecoder.h"
#include "RP2A03.h"
#include "RP2A03Apu.h"
#include "RP2C02.h"
#include "Cartridge.h"

struct kaphein_nes_CpuAddressDecoder
{
    struct kaphein_nes_AddressDecoder parent;
    
    void * impl_;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_construct(
    struct kaphein_nes_CpuAddressDecoder * thisObj
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_destruct(
    struct kaphein_nes_CpuAddressDecoder * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_connectCpu(
    struct kaphein_nes_CpuAddressDecoder * thisObj
    , struct kaphein_nes_RP2A03 * mpu
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_connectPpu(
    struct kaphein_nes_CpuAddressDecoder * thisObj
    , struct kaphein_nes_RP2C02 * ppu
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_connectMainMemory(
    struct kaphein_nes_CpuAddressDecoder * thisObj
    , kaphein_UInt8 * ram
    , kaphein_SSize ramSize
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_connectCartridge(
    struct kaphein_nes_CpuAddressDecoder * thisObj
    , struct kaphein_nes_Cartridge * cart
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_read(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_write(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
);

#endif
