#ifndef KAPHEIN_HGRD_kaphein_nes_RP2A03ApuLengthCounter_h
#define KAPHEIN_HGRD_kaphein_nes_RP2A03ApuLengthCounter_h

#include "def.h"
#include "kaphein/ErrorCode.h"

struct kaphein_nes_RP2A03ApuLengthCounter
{
    kaphein_UInt8 * pParam_;

    kaphein_UInt8 fixLengthBit_;

    kaphein_UInt8 counter_;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_construct(
    struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
    , kaphein_UInt8 stopCountingBit
    , kaphein_UInt8 * parameterSource
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_get(
    const struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_set(
    struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
    , kaphein_UInt8 index
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_reset(
    struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_isNonZero(
    const struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
    , bool * truthOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_run(
    struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
);

#endif
