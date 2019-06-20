#ifndef KAPHEIN_HGRD_kaphein_nes_def_h
#define KAPHEIN_HGRD_kaphein_nes_def_h

#include "kaphein/def.h"

enum kaphein_nes_InterruptSignal
{
	kaphein_nes_InterruptSignal_OCCUR = 0
	, kaphein_nes_InterruptSignal_NONE = 1
};

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
kaphein_UInt16
makeWord(
    kaphein_UInt8 l
    , kaphein_UInt8 h
)
{
    return (h << 8) | l;
}

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
kaphein_UInt8
lowByte(
    kaphein_UInt16 v
)
{
    return v & 0xFF;
}

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
kaphein_UInt8
highByte(
    kaphein_UInt16 v
)
{
    return v >> 8;
}

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
kaphein_UInt8
getSign(
    kaphein_UInt8 v
)
{
    return !!(v & 0x80);
}

#endif
