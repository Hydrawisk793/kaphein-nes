#include "kaphein/nes/ShiftRegister15.h"

bool
kaphein_nes_ShiftRegister15_shiftLeft(
    kaphein_UInt16 * reg
    , bool feed
)
{
    bool result = false;
    
    if(KAPHEIN_NULL != reg) {
        kaphein_UInt16 r = *reg;
        
        result = (r & (1 << 14)) != 0;

        r = ((r << 1) | (!!feed)) & 0x7FFF;

        *reg = r;
    }

    return result;
}

bool
kaphein_nes_ShiftRegister15_shiftRight(
    kaphein_UInt16 * reg
    , bool feed
)
{
    bool result = false;
    
    if(KAPHEIN_NULL != reg) {
        kaphein_UInt16 r = *reg;
        
        result = (r & 0x01) != 0;

        r = (r >> 1) | ((!!feed) ? 0x4000 : 0);

        *reg = r;
    }

    return result;
}
