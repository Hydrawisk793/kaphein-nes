#ifndef KAPHEIN_HGRD_kaphein_nes_LeSr16_h
#define KAPHEIN_HGRD_kaphein_nes_LeSr16_h

#include "kaphein/def.h"

////////////////////////////
//  0000 0000   0000 0000
//  LOW         HIGH
////////////////////////////

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
kaphein_UInt8
kaphein_nes_LeSr16_getLowByte(
    kaphein_UInt16 reg
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
void
kaphein_nes_LeSr16_setLowByte(
    kaphein_UInt16 * reg
    , kaphein_UInt8 value
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
kaphein_UInt8
kaphein_nes_LeSr16_getHighByte(
    kaphein_UInt16 reg
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
void
kaphein_nes_LeSr16_setHighByte(
    kaphein_UInt16 * reg
    , kaphein_UInt8 value
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
bool
kaphein_nes_LeSr16_shiftLeft(
    kaphein_UInt16 * reg
    , bool value
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
bool
kaphein_nes_LeSr16_shiftRight(
    kaphein_UInt16 * reg
    , bool value
);

#endif
