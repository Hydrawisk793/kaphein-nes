#ifndef KAPHEIN_HGRD_kaphein_nes_ShiftRegister15_h
#define KAPHEIN_HGRD_kaphein_nes_ShiftRegister15_h

#include "kaphein/def.h"

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
bool
kaphein_nes_ShiftRegister15_shiftLeft(
    kaphein_UInt16 * reg
    , bool feed
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
bool
kaphein_nes_ShiftRegister15_shiftRight(
    kaphein_UInt16 * reg
    , bool feed
);

#endif
