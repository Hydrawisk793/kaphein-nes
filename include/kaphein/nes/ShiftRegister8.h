#ifndef KAPHEIN_HGRD_kaphein_nes_ShiftRegister8_h
#define KAPHEIN_HGRD_kaphein_nes_ShiftRegister8_h

#include "kaphein/def.h"

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
bool
kaphein_nes_ShiftRegister8_shiftLeft(
    kaphein_UInt8 * reg
    , bool feed
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
bool
kaphein_nes_ShiftRegister8_shiftRight(
    kaphein_UInt8 * reg
    , bool feed
);

#endif
