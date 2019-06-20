#include "kaphein/nes/ShiftRegister8.h"

bool
kaphein_nes_ShiftRegister8_shiftLeft(
    kaphein_UInt8 * reg
    , bool feed
)
{
    bool result = false;
    
    if(KAPHEIN_NULL != reg) {
        kaphein_UInt8 r = *reg;
        
        result = (r & (1 << 7)) != 0;

        r = (r << 1) | (feed & 0x01);

        *reg = (r & 0xFF);
    }

    return result;
}

bool
kaphein_nes_ShiftRegister8_shiftRight(
    kaphein_UInt8 * reg
    , bool feed
)
{
    bool result = false;
    
    if(KAPHEIN_NULL != reg) {
        kaphein_UInt8 r = *reg;
        
        result = (r & 0x01) != 0;

        r = (r >> 1) | ((feed & 0x01) << 7);

        *reg = (r & 0xFF);
    }

    return result;
}
