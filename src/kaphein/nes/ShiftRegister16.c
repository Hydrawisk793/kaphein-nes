#include "kaphein/nes/ShiftRegister16.h"

bool
kaphein_nes_ShiftRegister16_shiftLeft(
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
kaphein_nes_ShiftRegister16_shiftRight(
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
