#ifndef KAPHEIN_HGRD_kaphein_nes_RP2A03ApuNoiseSequencer_h
#define KAPHEIN_HGRD_kaphein_nes_RP2A03ApuNoiseSequencer_h

#include "def.h"
#include "kaphein/ErrorCode.h"

struct kaphein_nes_RP2A03ApuNoiseSequencer
{
    kaphein_UInt8 * pParam_;

    kaphein_UInt16 timer_;
    
    kaphein_UInt16 prn_;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuNoiseSequencer_construct(
    struct kaphein_nes_RP2A03ApuNoiseSequencer * thisObj
    , kaphein_UInt8 * parameterSource
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuNoiseSequencer_getOutput(
    const struct kaphein_nes_RP2A03ApuNoiseSequencer * thisObj
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuNoiseSequencer_reset(
    struct kaphein_nes_RP2A03ApuNoiseSequencer * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuNoiseSequencer_run(
    struct kaphein_nes_RP2A03ApuNoiseSequencer * thisObj
);

#endif
