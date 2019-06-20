#ifndef KAPHEIN_HGRD_kaphein_nes_RP2A03ApuLinearCounter_h
#define KAPHEIN_HGRD_kaphein_nes_RP2A03ApuLinearCounter_h

#include "def.h"
#include "kaphein/ErrorCode.h"

struct kaphein_nes_RP2A03ApuLinearCounter
{
    kaphein_UInt8 * pParam_;

    kaphein_UInt8 counter_;

    bool reloadFlag_;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLinearCounter_construct(
    struct kaphein_nes_RP2A03ApuLinearCounter * thisObj
    , kaphein_UInt8 * parameterSource
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLinearCounter_setReloadFlag(
    struct kaphein_nes_RP2A03ApuLinearCounter * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLinearCounter_reset(
    struct kaphein_nes_RP2A03ApuLinearCounter * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLinearCounter_isNonZero(
    const struct kaphein_nes_RP2A03ApuLinearCounter * thisObj
    , bool * truthOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLinearCounter_run(
    struct kaphein_nes_RP2A03ApuLinearCounter * thisObj
);

#endif
