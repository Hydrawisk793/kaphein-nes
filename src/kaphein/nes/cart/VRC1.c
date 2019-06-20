#include "kaphein/nes/cart/VRC1.h"

enum HEConfig
{
    HEConfig_BANK_NO = 0x0F
    , HEConfig_CHR1_HIGH_BIT = 0x04
    , HEConfig_CHR0_HIGH_BIT = 0x02
    , HEConfig_MIRRORING = 0x01
};

static
void
mapPRGBank(
    struct kaphein_nes_cart_VRC1 * thisPtr
    , unsigned int index
    , kaphein_UInt8 v
){
    v %= HEConfig_BANK_NO;
    v %= thisPtr->prgROMBankCount;

    thisPtr->parent.prgBankAddresses[index] = v * 0x2000;
}

static
void
mapCHRBank(
    struct kaphein_nes_cart_VRC1 * thisPtr
    , unsigned int index
    , kaphein_UInt8 v
)
{
    v &= HEConfig_BANK_NO;
    v %= thisPtr->chrROMBankCount;

    if(index) {
        v |= ((thisPtr->chr1MSB)?(0x10):(0));
        thisPtr->parent.chrBankAddresses[4] = v*0x1000;
        thisPtr->parent.chrBankAddresses[5] = thisPtr->parent.chrBankAddresses[4] + 0x400;
        thisPtr->parent.chrBankAddresses[6] = thisPtr->parent.chrBankAddresses[5] + 0x400;
        thisPtr->parent.chrBankAddresses[7] = thisPtr->parent.chrBankAddresses[6] + 0x400;
    }
    else{
        v |= ((thisPtr->chr0MSB)?(0x10):(0));
        thisPtr->parent.chrBankAddresses[0] = v*0x1000;
        thisPtr->parent.chrBankAddresses[1] = thisPtr->parent.chrBankAddresses[0] + 0x400;
        thisPtr->parent.chrBankAddresses[2] = thisPtr->parent.chrBankAddresses[1] + 0x400;
        thisPtr->parent.chrBankAddresses[3] = thisPtr->parent.chrBankAddresses[2] + 0x400;    
    }
}

static
void
mapNameTable(
    struct kaphein_nes_cart_VRC1 * thisPtr
    , kaphein_UInt8 v
)
{
    if(v & HEConfig_MIRRORING) {
        //Horizontal
        thisPtr->parent.nametableAddresses[1] = 0x0000;
        thisPtr->parent.nametableAddresses[2] = 0x0400;
    }
    else {
        //Vertical
        thisPtr->parent.nametableAddresses[1] = 0x0400;
        thisPtr->parent.nametableAddresses[2] = 0x0000;
    }
};

static
const struct kaphein_nes_Cartridge_VTable cartVTable = {
    kaphein_nes_cart_VRC1_destruct
    , kaphein_nes_Cartridge_setNametableMemory
    , kaphein_nes_Cartridge_setBuses
    , kaphein_nes_cart_VRC1_powerUp
    , kaphein_nes_cart_VRC1_reset
    , kaphein_nes_Cartridge_run
    , kaphein_nes_Cartridge_doCpuRead
    , kaphein_nes_cart_VRC1_doCpuWrite
    , kaphein_nes_Cartridge_doPpuRead
    , kaphein_nes_Cartridge_doPpuWrite
};

enum kaphein_ErrorCode
kaphein_nes_cart_VRC1_construct(
    struct kaphein_nes_cart_VRC1 * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
)
{
    kaphein_nes_Cartridge_construct(
        &thisObj->parent
        , stream
        , romHeader
        , allocator
    );
    thisObj->parent.vTable = &cartVTable;

    thisObj->prgROMBankCount = 0;
    thisObj->chrROMBankCount = 0;
    thisObj->hasFourScreens = false;
    thisObj->chr0MSB = false;
    thisObj->chr1MSB = false;

    kaphein_nes_NesRomHeader_hasFourScreens(&thisObj->parent.romHeader, &thisObj->hasFourScreens);

    kaphein_nes_NesRomHeader_getPrgRomBankCount(&thisObj->parent.romHeader, &thisObj->prgROMBankCount);
    thisObj->prgROMBankCount <<= 1;

    kaphein_nes_NesRomHeader_getChrRomBankCount(&thisObj->parent.romHeader, &thisObj->chrROMBankCount);


    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_cart_VRC1_destruct(
    void * thisObj
)
{
    struct kaphein_nes_cart_VRC1 *const thisPtr = (struct kaphein_nes_cart_VRC1 *)thisObj;

    return kaphein_nes_Cartridge_destruct(&thisPtr->parent);
}

enum kaphein_ErrorCode
kaphein_nes_cart_VRC1_powerUp(
    void * thisObj
)
{
    return kaphein_nes_cart_VRC1_reset(thisObj);
}

enum kaphein_ErrorCode
kaphein_nes_cart_VRC1_reset(
    void * thisObj
)
{
    struct kaphein_nes_cart_VRC1 *const thisPtr = (struct kaphein_nes_cart_VRC1 *)thisObj;
    
    kaphein_nes_Cartridge_reset(&thisPtr->parent);

    mapNameTable(thisPtr, 0);

    thisPtr->chr0MSB = false;
    thisPtr->chr1MSB = false;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_cart_VRC1_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
)
{
    enum kaphein_ErrorCode errorCode = kapheinErrorCodeNoError;
    struct kaphein_nes_cart_VRC1 *const thisPtr = (struct kaphein_nes_cart_VRC1 *)thisObj;
    
    switch(address >> 12) {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
        errorCode = kaphein_nes_Cartridge_doCpuWrite(&thisPtr->parent, address, value);
    break;
    case 0x08:
        mapPRGBank(thisPtr, 0, value);
    break;
    case 0x09:
        thisPtr->chr1MSB = !!(value & HEConfig_CHR1_HIGH_BIT);
        thisPtr->chr0MSB = !!(value & HEConfig_CHR0_HIGH_BIT);
        
        if(!thisPtr->hasFourScreens) {
            mapNameTable(thisPtr, value);
        }
    break;
    case 0x0A:
        mapPRGBank(thisPtr, 1, value);
    break;
    case 0x0B:
        errorCode = kaphein_nes_Cartridge_doCpuWrite(&thisPtr->parent, address, value);
    break;
    case 0x0C:
        mapPRGBank(thisPtr, 2, value);
    break;
    case 0x0D:
        errorCode = kaphein_nes_Cartridge_doCpuWrite(&thisPtr->parent, address, value);
    break;
    case 0x0E:
        mapCHRBank(thisPtr, 0, value);
    break;
    case 0x0F:
        mapCHRBank(thisPtr, 1, value);
    break;
    }
    
    return errorCode;
}
