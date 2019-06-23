#ifndef KAPHEIN_HGRD_kaphein_nes_cart_TxROM_h
#define KAPHEIN_HGRD_kaphein_nes_cart_TxROM_h

#include "../Cartridge.h"

/**
 *  @brief Nintendo MMC3 TxROM
 */
struct kaphein_nes_cart_TxROM
{
    struct kaphein_nes_Cartridge parent;
    
    kaphein_SSize chrRomBankCount;

    kaphein_SSize prgRomBankCount;

    kaphein_SSize chrAddressLatch[6];
    
    kaphein_SSize prgRomAddressLatch;
    
    kaphein_UInt8 bankConfig;

    kaphein_UInt8 prgRamConfig;
    
    kaphein_UInt8 irqCounter;
    
    kaphein_UInt8 irqReloadValue;
    
    bool reloadValueSignal;
    
    bool irqReloadFlag;
    
    bool irqEnableFlag;
    
    bool isPpuA12Set;
    
    bool irqFlag;
    
    bool singleIrqFlag;
    
    bool irqDetectedFlag;
    
    kaphein_UInt8 irqIgnoranceCounter;

    kaphein_UInt8 irqOccurenceDelayCounter;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_construct(
    struct kaphein_nes_cart_TxROM * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_destruct(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_powerUp(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_reset(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_run(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_doCpuRead(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_TxROM_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
);

#endif
