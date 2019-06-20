#ifndef KAPHEIN_HGRD_kaphein_nes_Cartridge_h
#define KAPHEIN_HGRD_kaphein_nes_Cartridge_h

#include "kaphein/io/Stream.h"
#include "NesRomHeader.h"
#include "MemoryChipArray.h"

struct kaphein_nes_Cartridge_VTable
{
    enum kaphein_ErrorCode
    (* destruct) (
        void * thisObj
    );

    enum kaphein_ErrorCode
    (* setNametableMemory)(
        void * thisObj
        , kaphein_UInt8 * memory
        , kaphein_SSize memorySize
    );

    enum kaphein_ErrorCode
    (* setBuses)(
        void * thisObj
        , kaphein_UInt16 * cpuAddressBus
        , kaphein_UInt8 * cpuDataBus
        , enum kaphein_nes_InterruptSignal * cpuIrqBus
        , kaphein_UInt16 * ppuAddressBus
        , kaphein_UInt8 * ppuDataBus
    );
    
    enum kaphein_ErrorCode
    (* powerUp) (
        void * thisObj
    );

    enum kaphein_ErrorCode
    (* reset) (
        void * thisObj
    );
    
    enum kaphein_ErrorCode
    (* run) (
        void * thisObj
    );

    enum kaphein_ErrorCode
    (* doCpuRead) (
        void * thisObj
        , kaphein_UInt16 address
        , kaphein_UInt8 * valueOut
    );

    enum kaphein_ErrorCode
    (* doCpuWrite) (
        void * thisObj
        , kaphein_UInt16 address
        , kaphein_UInt8 value
    );

    enum kaphein_ErrorCode
    (* doPpuRead) (
        void * thisObj
        , kaphein_UInt16 address
        , kaphein_UInt8 * valueOut
    );

    enum kaphein_ErrorCode
    (* doPpuWrite) (
        void * thisObj
        , kaphein_UInt16 address
        , kaphein_UInt8 value
    );
};

struct kaphein_nes_Cartridge
{
    const struct kaphein_nes_Cartridge_VTable * vTable;
    
    void * allocator;
    
    struct kaphein_nes_NesRomHeader romHeader;

    struct kaphein_nes_MemoryChipArray memoryChipArray;

    kaphein_UInt16 * cpuAddressBus;

    kaphein_UInt8 * cpuDataBus;

    enum kaphein_nes_InterruptSignal * cpuIrqBus;

    kaphein_UInt16 * ppuAddressBus;

    kaphein_UInt8 * ppuDataBus;

    kaphein_UInt8 * nametableMemory;

    kaphein_SSize nametableMemorySize;
    
    //An array of addresses that point the start of 1KB CHR banks.
    kaphein_SSize chrBankAddresses[8];

    //An array of addresses that point the start of 1KB nametables.
    kaphein_SSize nametableAddresses[4];
    
    //An array of addresses that point the start of 8KB PRG banks.
    kaphein_SSize prgBankAddresses[4];

    //An array of addresses that point the start of 8KB PRG banks.
    kaphein_SSize lastPrgBankAddresses[2];
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_Cartridge_construct(
    struct kaphein_nes_Cartridge * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_Cartridge_destruct(
    struct kaphein_nes_Cartridge * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_Cartridge_setNametableMemory(
    void * thisObj
    , kaphein_UInt8 * memory
    , kaphein_SSize memorySize
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_Cartridge_setBuses(
    void * thisObj
    , kaphein_UInt16 * cpuAddressBus
    , kaphein_UInt8 * cpuDataBus
    , enum kaphein_nes_InterruptSignal * cpuIrqBus
    , kaphein_UInt16 * ppuAddressBus
    , kaphein_UInt8 * ppuDataBus
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_Cartridge_powerUp(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_Cartridge_reset(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_Cartridge_run(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_Cartridge_doCpuRead(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_Cartridge_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_Cartridge_doPpuRead(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_Cartridge_doPpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
);

#endif
