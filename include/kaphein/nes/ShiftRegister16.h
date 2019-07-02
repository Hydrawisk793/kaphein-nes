#ifndef KAPHEIN_HGRD_kaphein_nes_ShiftRegister16_h
#define KAPHEIN_HGRD_kaphein_nes_ShiftRegister16_h

#include "kaphein/def.h"

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
bool
kaphein_nes_ShiftRegister16_shiftLeft(
    kaphein_UInt16 * reg
    , bool value
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
bool
kaphein_nes_ShiftRegister16_shiftRight(
    kaphein_UInt16 * reg
    , bool value
);

#endif
