#include "kaphein/mem/utils.h"
#include "kaphein/nes/Cartridge.h"

enum
{
    nesRomTrainerSize = 512
};

static
const struct kaphein_nes_Cartridge_VTable vTable = {
    kaphein_nes_Cartridge_destruct
    , kaphein_nes_Cartridge_setNametableMemory
    , kaphein_nes_Cartridge_setBuses
    , kaphein_nes_Cartridge_powerUp
    , kaphein_nes_Cartridge_reset
    , kaphein_nes_Cartridge_run
    , kaphein_nes_Cartridge_doCpuRead
    , kaphein_nes_Cartridge_doCpuWrite
    , kaphein_nes_Cartridge_doPpuRead
    , kaphein_nes_Cartridge_doPpuWrite
};

enum kaphein_ErrorCode
kaphein_nes_Cartridge_construct(
    struct kaphein_nes_Cartridge * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
)
{
    static const char signature[] = {'N', 'E', 'S', 0x1A};
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        kaphein_SSize prgRomChipSize = 0;
        kaphein_SSize chrRomChipSize = 0;
        kaphein_SSize readByteCount = 0;
        kaphein_SSize bankCount;
        int version = 0;
        int i;
        int state;
        bool hasTrainer = false;
        bool hasPrgNvRam;
        bool isVerticalMirroring;
        
        kaphein_mem_fillZero(thisObj, KAPHEIN_ssizeof(*thisObj), KAPHEIN_ssizeof(*thisObj));
        thisObj->vTable = &vTable;
        thisObj->allocator = allocator;

        for(state = 16; kapheinErrorCodeNoError == resultErrorCode && state > 0; ) {
            switch(state) {
            case 16:
                if(KAPHEIN_NULL == romHeader) {
                    resultErrorCode = (*stream->vTable->read)(
                        stream
                        , thisObj->romHeader.u.a, KAPHEIN_ssizeof(thisObj->romHeader.u.a)
                        , &readByteCount
                    );
                }
                else {
                    resultErrorCode = kaphein_mem_copy(
                        &thisObj->romHeader
                        , KAPHEIN_ssizeof(thisObj->romHeader)
                        , romHeader
                        , KAPHEIN_ssizeof(romHeader)
                    );
                    readByteCount = KAPHEIN_ssizeof(*romHeader);
                }
            break;
            case 15:
                if(readByteCount < KAPHEIN_ssizeof(thisObj->romHeader.u.a)) {
                    resultErrorCode = kapheinErrorCodeNotEnoughBufferSpace;
                }
            break;
            case 14:
                if(
                    kaphein_mem_compare(
                        signature
                        , KAPHEIN_ssizeof(signature)
                        , thisObj->romHeader.u.m.signature
                        , KAPHEIN_ssizeof(thisObj->romHeader.u.m.signature)
                    ) != 0
                ) {
                    resultErrorCode = kapheinErrorCodeArgumentFormatInvalid;
                }
            break;
            case 13:
                kaphein_nes_NesRomHeader_getVersion(&thisObj->romHeader, &version);
                
                kaphein_nes_NesRomHeader_hasTrainer(&thisObj->romHeader, &hasTrainer);
                if(hasTrainer) {
                    thisObj->memoryChipArray.trainer = (kaphein_UInt8 *)kaphein_mem_allocate(
                        allocator
                        , nesRomTrainerSize
                        , KAPHEIN_NULL
                        , &resultErrorCode
                    );
                    
                    if(KAPHEIN_NULL != thisObj->memoryChipArray.trainer) {
                        resultErrorCode = (*stream->vTable->read)(
                            stream
                            , thisObj->memoryChipArray.trainer, nesRomTrainerSize
                            , &readByteCount
                        );
                    }
                }
            break;
            case 12:
                if(hasTrainer) {
                    if(readByteCount < nesRomTrainerSize) {
                        resultErrorCode = kapheinErrorCodeNotEnoughBufferSpace;
                    }
                }
            break;
            case 11:
                kaphein_nes_NesRomHeader_getPrgRomSize(&thisObj->romHeader, &prgRomChipSize);
                if(prgRomChipSize < 1) {
                    resultErrorCode = kapheinErrorCodeArgumentOutOfRange;
                }
            break;
            case 10:
                thisObj->memoryChipArray.prgRom = (kaphein_UInt8 *)kaphein_mem_allocate(
                    allocator
                    , prgRomChipSize
                    , KAPHEIN_NULL
                    , &resultErrorCode
                );
                
                if(KAPHEIN_NULL != thisObj->memoryChipArray.prgRom) {
                    thisObj->memoryChipArray.prgRomSize = prgRomChipSize;
                    
                    resultErrorCode = (*stream->vTable->read)(
                        stream
                        , thisObj->memoryChipArray.prgRom, prgRomChipSize
                        , &readByteCount
                    );
                }
            break;
            case 9:
                if(readByteCount < prgRomChipSize) {
                    resultErrorCode = kapheinErrorCodeNotEnoughBufferSpace;
                }
            break;
            case 8:
                kaphein_nes_NesRomHeader_getChrRomSize(&thisObj->romHeader, &chrRomChipSize);
                if(chrRomChipSize < 0) {
                    resultErrorCode = kapheinErrorCodeArgumentOutOfRange;
                }
            break;
            case 7:
                if(chrRomChipSize > 0) {
                    thisObj->memoryChipArray.chrRom = (kaphein_UInt8 *)kaphein_mem_allocate(
                        allocator
                        , chrRomChipSize
                        , KAPHEIN_NULL
                        , &resultErrorCode
                    );
                
                    if(KAPHEIN_NULL != thisObj->memoryChipArray.chrRom) {
                        thisObj->memoryChipArray.chrRomSize = chrRomChipSize;
                        
                        resultErrorCode = (*stream->vTable->read)(
                            stream
                            , thisObj->memoryChipArray.chrRom, chrRomChipSize
                            , &readByteCount
                        );
                    }
                }
            break;
            case 6:
                if(readByteCount < chrRomChipSize) {
                    resultErrorCode = kapheinErrorCodeNotEnoughBufferSpace;
                }
            break;
            case 5:
                switch(version) {
                case 0:
                break;
                case 2:
                    kaphein_nes_NesRomHeader_getPrgRamSize(&thisObj->romHeader, &thisObj->memoryChipArray.prgRamSize);
                break;
                }

                if(thisObj->memoryChipArray.prgRamSize > 0) {
                    thisObj->memoryChipArray.prgRam = (kaphein_UInt8 *)kaphein_mem_allocate(
                        allocator
                        , thisObj->memoryChipArray.prgRamSize
                        , KAPHEIN_NULL
                        , &resultErrorCode
                    );
                }
            break;
            case 4:
                kaphein_nes_NesRomHeader_hasPrgNvRam(&thisObj->romHeader, &hasPrgNvRam);

                switch(version) {
                case 0:
                    if(hasPrgNvRam) {
                        thisObj->memoryChipArray.prgNvRamSize = 8192;
                    }
                break;
                case 2:
                    kaphein_nes_NesRomHeader_getPrgNvRamSize(&thisObj->romHeader, &thisObj->memoryChipArray.prgNvRamSize);

                    if(thisObj->memoryChipArray.prgNvRamSize > 0 && !hasPrgNvRam) {
                        thisObj->memoryChipArray.prgNvRamSize = 0;
                        
                        resultErrorCode = kapheinErrorCodeArgumentFormatInvalid;
                    }
                break;
                }

                if(thisObj->memoryChipArray.prgNvRamSize > 0) {
                    thisObj->memoryChipArray.prgNvRam = (kaphein_UInt8 *)kaphein_mem_allocate(
                        allocator
                        , thisObj->memoryChipArray.prgNvRamSize
                        , KAPHEIN_NULL
                        , &resultErrorCode
                    );
                }
            break;
            case 3:
                switch(version) {
                case 0:
                    if(chrRomChipSize < 1) {
                        thisObj->memoryChipArray.chrRamSize = 8192;
                    }
                break;
                case 2:
                    kaphein_nes_NesRomHeader_getChrRamSize(&thisObj->romHeader, &thisObj->memoryChipArray.chrRamSize);
                break;
                }

                if(thisObj->memoryChipArray.chrRamSize > 0) {
                    thisObj->memoryChipArray.chrRam = (kaphein_UInt8 *)kaphein_mem_allocate(
                        allocator
                        , thisObj->memoryChipArray.chrRamSize
                        , KAPHEIN_NULL
                        , &resultErrorCode
                    );
                }
            break;
            case 2:
                switch(version) {
                case 0:
                break;
                case 2:
                    kaphein_nes_NesRomHeader_getChrNvRamSize(&thisObj->romHeader, &thisObj->memoryChipArray.chrNvRamSize);
                break;
                }

                if(thisObj->memoryChipArray.chrNvRamSize > 0) {
                    thisObj->memoryChipArray.chrNvRam = (kaphein_UInt8 *)kaphein_mem_allocate(
                        allocator
                        , thisObj->memoryChipArray.chrNvRamSize
                        , KAPHEIN_NULL
                        , &resultErrorCode
                    );
                }
            break;
            case 1:
                kaphein_nes_NesRomHeader_getPrgRomBankCount(&thisObj->romHeader, &bankCount);
                
                thisObj->lastPrgBankAddresses[0] = (bankCount - 1) * 0x4000;
                thisObj->lastPrgBankAddresses[1] = thisObj->lastPrgBankAddresses[0] + 0x2000;

                thisObj->prgBankAddresses[2] = thisObj->lastPrgBankAddresses[0];
                thisObj->prgBankAddresses[3] = thisObj->lastPrgBankAddresses[1];

                switch(bankCount) {
                case 0:
                    resultErrorCode = kapheinErrorCodeArgumentInvalid;
                break;
                case 1:
                    thisObj->prgBankAddresses[0] = 0;
                    thisObj->prgBankAddresses[1] = 0x2000;
                break;
                default:
                    thisObj->prgBankAddresses[1] = thisObj->prgBankAddresses[2] - 0x2000;
                    thisObj->prgBankAddresses[0] = thisObj->prgBankAddresses[1] - 0x2000;
                break;
                }

                if(thisObj->memoryChipArray.chrRomSize > 0) {
                    for(i = 0; i < 8; ++i) {
                        thisObj->chrBankAddresses[i] = (i << 10);
                    }
                }

                thisObj->nametableAddresses[0] = 0x0000;
                thisObj->nametableAddresses[3] = 0x0400;
                
                kaphein_nes_NesRomHeader_isVerticalMirroring(&thisObj->romHeader, &isVerticalMirroring);
	            if(isVerticalMirroring) {
		            thisObj->nametableAddresses[1] = 0x0400;
		            thisObj->nametableAddresses[2] = 0x0000;
	            }
	            else{
		            thisObj->nametableAddresses[1] = 0x0000;
		            thisObj->nametableAddresses[2] = 0x0400;
	            }
            break;
            }

            if(kapheinErrorCodeNoError == resultErrorCode) {
                --state;
            }
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_Cartridge_destruct(
    struct kaphein_nes_Cartridge * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        int state;

        for(state = 8; kapheinErrorCodeNoError == resultErrorCode && state > 0; ) {
            switch(state) {
            case 8:
                if(KAPHEIN_NULL != thisObj->memoryChipArray.trainer) {
                    resultErrorCode = kaphein_mem_deallocate(
                        thisObj->allocator
                        , thisObj->memoryChipArray.trainer
                        , nesRomTrainerSize
                    );

                    if(kapheinErrorCodeNoError == resultErrorCode) {
                        thisObj->memoryChipArray.trainer = KAPHEIN_NULL;
                    }
                }
            break;
            case 7:
                resultErrorCode = kaphein_mem_deallocate(
                    thisObj->allocator
                    , thisObj->memoryChipArray.prgRom
                    , thisObj->memoryChipArray.prgRomSize
                );

                if(kapheinErrorCodeNoError == resultErrorCode) {
                    thisObj->memoryChipArray.prgRom = KAPHEIN_NULL;
                }
            break;
            case 6:
                resultErrorCode = kaphein_mem_deallocate(
                    thisObj->allocator
                    , thisObj->memoryChipArray.chrRom
                    , thisObj->memoryChipArray.chrRomSize
                );

                if(kapheinErrorCodeNoError == resultErrorCode) {
                    thisObj->memoryChipArray.chrRom = KAPHEIN_NULL;
                }
            break;
            case 5:
                if(KAPHEIN_NULL != thisObj->memoryChipArray.prgRam) {
                    resultErrorCode = kaphein_mem_deallocate(
                        thisObj->allocator
                        , thisObj->memoryChipArray.prgRam
                        , thisObj->memoryChipArray.prgRamSize
                    );
                        
                    if(kapheinErrorCodeNoError == resultErrorCode) {
                        thisObj->memoryChipArray.prgRam = KAPHEIN_NULL;
                    }
                }
            break;
            case 4:
                if(KAPHEIN_NULL != thisObj->memoryChipArray.prgNvRam) {
                    resultErrorCode = kaphein_mem_deallocate(
                        thisObj->allocator
                        , thisObj->memoryChipArray.prgNvRam
                        , thisObj->memoryChipArray.prgNvRamSize
                    );
                        
                    if(kapheinErrorCodeNoError == resultErrorCode) {
                        thisObj->memoryChipArray.prgNvRam = KAPHEIN_NULL;
                    }
                }
            break;
            case 3:
                if(KAPHEIN_NULL != thisObj->memoryChipArray.chrRam) {
                    resultErrorCode = kaphein_mem_deallocate(
                        thisObj->allocator
                        , thisObj->memoryChipArray.chrRam
                        , thisObj->memoryChipArray.chrRamSize
                    );

                    if(kapheinErrorCodeNoError == resultErrorCode) {
                        thisObj->memoryChipArray.chrRam = KAPHEIN_NULL;
                    }
                }
            break;
            case 2:
                if(KAPHEIN_NULL != thisObj->memoryChipArray.chrNvRam) {
                    resultErrorCode = kaphein_mem_deallocate(
                        thisObj->allocator
                        , thisObj->memoryChipArray.chrNvRam
                        , thisObj->memoryChipArray.chrNvRamSize
                    );

                    if(kapheinErrorCodeNoError == resultErrorCode) {
                        thisObj->memoryChipArray.chrNvRam = KAPHEIN_NULL;
                    }
                }
            break;
            case 1:
                thisObj->vTable = KAPHEIN_NULL;
            break;
            }

            if(kapheinErrorCodeNoError == resultErrorCode) {
                --state;
            }
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_Cartridge_setNametableMemory(
    void * thisObj
    , kaphein_UInt8 * memory
    , kaphein_SSize memorySize
)
{
    struct kaphein_nes_Cartridge *const thisPtr = (struct kaphein_nes_Cartridge *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    thisPtr->nametableMemory = memory;
    thisPtr->nametableMemorySize = memorySize;

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_Cartridge_setBuses(
    void * thisObj
    , kaphein_UInt16 * cpuAddressBus
    , kaphein_UInt8 * cpuDataBus
    , enum kaphein_nes_InterruptSignal * cpuIrqBus
    , kaphein_UInt16 * ppuAddressBus
    , kaphein_UInt8 * ppuDataBus
)
{
    struct kaphein_nes_Cartridge *const thisPtr = (struct kaphein_nes_Cartridge *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    thisPtr->cpuAddressBus = cpuAddressBus;
    thisPtr->cpuDataBus = cpuDataBus;
    thisPtr->cpuIrqBus = cpuIrqBus;
    thisPtr->ppuAddressBus = ppuAddressBus;
    thisPtr->ppuDataBus = ppuDataBus;

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_Cartridge_powerUp(
    void * thisObj
)
{
    return kaphein_nes_Cartridge_reset(thisObj);
}

enum kaphein_ErrorCode
kaphein_nes_Cartridge_reset(
    void * thisObj
)
{
    KAPHEIN_x_UNUSED_PARAMETER(thisObj)

    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    //Does nothing.

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_Cartridge_run(
    void * thisObj
)
{
    KAPHEIN_x_UNUSED_PARAMETER(thisObj)

    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    //Does nothing.

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_Cartridge_doCpuRead(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
)
{
    struct kaphein_nes_Cartridge *const thisPtr = (struct kaphein_nes_Cartridge *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    const kaphein_SSize bank8KbNumber = address >> 13;
    kaphein_UInt8 value = 0x00;

    switch(bank8KbNumber >> 2) {
    case 0x00:
        value = *thisPtr->cpuDataBus;
    break;
    case 0x01:
        value = thisPtr->memoryChipArray.prgRom[thisPtr->prgBankAddresses[bank8KbNumber & 0x03] + (address & 0x1FFF)];
    break;
    }

    *valueOut = value;

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_Cartridge_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
)
{
    KAPHEIN_x_UNUSED_PARAMETER(thisObj)
    KAPHEIN_x_UNUSED_PARAMETER(address)
    KAPHEIN_x_UNUSED_PARAMETER(value)

    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    //Does nothing.

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_Cartridge_doPpuRead(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
)
{
    struct kaphein_nes_Cartridge *const thisPtr = (struct kaphein_nes_Cartridge *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    const kaphein_UInt16 addressShiftRight10 = address >> 10;
    kaphein_UInt8 value = 0x00;

    switch((addressShiftRight10 >> 3) & 0x01) {
    case 0x00:
        if(thisPtr->memoryChipArray.chrRomSize > 0) {
            value = thisPtr->memoryChipArray.chrRom[thisPtr->chrBankAddresses[addressShiftRight10 & 0x07] + (address & 0x3FF)];
        }
        else {
            value = thisPtr->memoryChipArray.chrRam[address & 0x1FFF];
        }
    break;
    case 0x01:
        value = thisPtr->nametableMemory[thisPtr->nametableAddresses[addressShiftRight10 & 0x03] + (address & 0x03FF)];
    break;
    }

    *valueOut = value;

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_Cartridge_doPpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
)
{
    struct kaphein_nes_Cartridge *const thisPtr = (struct kaphein_nes_Cartridge *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    const kaphein_UInt16 addressShiftRight10 = address >> 10;

    switch((addressShiftRight10 >> 3) & 0x01) {
    case 0x00:
        if(thisPtr->memoryChipArray.chrRamSize > 0) {
            thisPtr->memoryChipArray.chrRam[address & 0x1FFF] = value;
        }
    break;
    case 0x01:
        thisPtr->nametableMemory[thisPtr->nametableAddresses[addressShiftRight10 & 0x03] + (address & 0x03FF)] = value;
    break;
    }

    return resultErrorCode;
}
