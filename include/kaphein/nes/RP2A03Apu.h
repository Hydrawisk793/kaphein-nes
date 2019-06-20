#ifndef KAPHEIN_HGRD_kaphein_nes_RP2A03Apu_h
#define KAPHEIN_HGRD_kaphein_nes_RP2A03Apu_h

#include "def.h"
#include "kaphein/ErrorCode.h"
#include "RP2A03ApuDmcReader.h"

struct kaphein_nes_RP2A03Apu
{
    void * impl_;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_construct(
    struct kaphein_nes_RP2A03Apu * thisObj
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_destruct(
    struct kaphein_nes_RP2A03Apu * thisObj
);
KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_setIoBuses(
    struct kaphein_nes_RP2A03Apu * thisObj
    , kaphein_UInt16 * addrBus
    , kaphein_UInt8 * dataBus
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_setDmcReader(
    struct kaphein_nes_RP2A03Apu * thisObj
    , struct kaphein_nes_RP2A03ApuDmcReader * reader
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_isInterruptRequested(
    struct kaphein_nes_RP2A03Apu * thisObj
    , bool * truthOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_getROut(
    struct kaphein_nes_RP2A03Apu * thisObj
    , double * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_getCOut(
    struct kaphein_nes_RP2A03Apu * thisObj
    , double * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_isInterruptFlagSet(
    struct kaphein_nes_RP2A03Apu * thisObj
    , bool * truthOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_powerUp(
    struct kaphein_nes_RP2A03Apu * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_reset(
    struct kaphein_nes_RP2A03Apu * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_run(
    struct kaphein_nes_RP2A03Apu * thisObj
    , int cycleCount
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_readRegister(
    struct kaphein_nes_RP2A03Apu * thisObj
    , kaphein_UInt8 address
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_writeRegister(
    struct kaphein_nes_RP2A03Apu * thisObj
    , kaphein_UInt8 address
    , kaphein_UInt8 value
);

#endif
