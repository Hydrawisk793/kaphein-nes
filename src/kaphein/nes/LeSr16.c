#include "kaphein/nes/LeSr16.h"

kaphein_UInt8
kaphein_nes_LeSr16_getLowByte(
    kaphein_UInt16 reg
)
{
    return ((reg & 0xFF00) >> 8);
}

void
kaphein_nes_LeSr16_setLowByte(
    kaphein_UInt16 * reg
    , kaphein_UInt8 value
)
{
    if(KAPHEIN_NULL != reg) {
        kaphein_UInt16 r = *reg;
    
        r = ((((kaphein_UInt16)value) << 8) | (r & 0x00FF));

        *reg = r;
    }
}

kaphein_UInt8
kaphein_nes_LeSr16_getHighByte(
    kaphein_UInt16 reg
)
{
    return (reg & 0x00FF);
}

void
kaphein_nes_LeSr16_setHighByte(
    kaphein_UInt16 * reg
    , kaphein_UInt8 value
)
{
    if(KAPHEIN_NULL != reg) {
        kaphein_UInt16 r = *reg;
        
        r = (r & 0xFF00) | value;

        *reg = r;
    }
}

bool
kaphein_nes_LeSr16_shiftLeft(
    kaphein_UInt16 * reg
    , bool value
)
{
    bool result = false;
    
    if(KAPHEIN_NULL != reg) {
        kaphein_UInt16 r = *reg;
        
        result = (r & 0x8000) != 0;

        r = (r << 1) | (value & 0x01);

        *reg = r;
    }

    return result;
}

bool
kaphein_nes_LeSr16_shiftRight(
    kaphein_UInt16 * reg
    , bool value
)
{
    bool result = false;

    if(KAPHEIN_NULL != reg) {
        kaphein_UInt16 r = *reg;
        
        result = (r & 0x0001) != 0;

        r = (r >> 1) | ((value & 0x01) << 15);

        *reg = r;
    }

    return result;
}
