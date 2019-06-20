#ifndef KAPHEIN_HGRD_kaphein_nes_ClockDivider_h
#define KAPHEIN_HGRD_kaphein_nes_ClockDivider_h

#include "kaphein/ErrorCode.h"

struct kaphein_nes_ClockDivider
{
    unsigned int period;

    unsigned int counter;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_ClockDivider_reset(
    struct kaphein_nes_ClockDivider * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_ClockDivider_run(
    struct kaphein_nes_ClockDivider * thisObj
    , bool * resultOut
);

#endif
