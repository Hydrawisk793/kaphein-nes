#include "kaphein/mem/utils.h"
#include "kaphein/nes/CpuAddressDecoder.h"

struct kaphein_nes_CpuAddressDecoder_Impl
{
    void * allocator_;
    
    struct kaphein_nes_RP2A03 * mpu_;

    struct kaphein_nes_RP2C02 * ppu_;

    struct kaphein_nes_Cartridge * cart_;

    kaphein_UInt8 * ram_;

    kaphein_SSize ramSize_;
};

static const struct kaphein_nes_AddressDecoder_VTable parentVTable = {
    kaphein_nes_CpuAddressDecoder_read
    , kaphein_nes_CpuAddressDecoder_write
};

enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_construct(
    struct kaphein_nes_CpuAddressDecoder * thisObj
    , void * allocator
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_CpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_CpuAddressDecoder_Impl *)kaphein_mem_allocate(
            allocator
            , KAPHEIN_ssizeof(*implPtr)
            , KAPHEIN_NULL
            ,  &resultErrorCode
        );

        if(KAPHEIN_NULL != implPtr) {
            thisObj->parent.vTable = &parentVTable;
            thisObj->impl_ = implPtr;

            implPtr->allocator_ = allocator;
            implPtr->mpu_ = KAPHEIN_NULL;
            implPtr->ppu_ = KAPHEIN_NULL;
            implPtr->cart_ = KAPHEIN_NULL;
            implPtr->ram_ = KAPHEIN_NULL;
            implPtr->ramSize_ = 0;
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_destruct(
    struct kaphein_nes_CpuAddressDecoder * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_CpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_CpuAddressDecoder_Impl *)thisObj->impl_;
        
        resultErrorCode = kaphein_mem_deallocate(
            implPtr->allocator_
            , implPtr
            , KAPHEIN_ssizeof(*implPtr)
        );
        if(resultErrorCode == kapheinErrorCodeNoError) {
            thisObj->impl_  = KAPHEIN_NULL;
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_connectCpu(
    struct kaphein_nes_CpuAddressDecoder * thisObj
    , struct kaphein_nes_RP2A03 * mpu
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == mpu
    ) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_CpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_CpuAddressDecoder_Impl *)thisObj->impl_;

        implPtr->mpu_ = mpu;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_connectPpu(
    struct kaphein_nes_CpuAddressDecoder * thisObj
    , struct kaphein_nes_RP2C02 * ppu
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == ppu
    ) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_CpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_CpuAddressDecoder_Impl *)thisObj->impl_;

        implPtr->ppu_ = ppu;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_connectMainMemory(
    struct kaphein_nes_CpuAddressDecoder * thisObj
    , kaphein_UInt8 * ram
    , kaphein_SSize ramSize
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == ram) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else if(ramSize < 0 || ramSize != 0x800) {
        resultErrorCode = kapheinErrorCodeArgumentOutOfRange;
    }
    else {
        struct kaphein_nes_CpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_CpuAddressDecoder_Impl *)thisObj->impl_;
        
        implPtr->ram_ = ram;
        implPtr->ramSize_ = ramSize;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_connectCartridge(
    struct kaphein_nes_CpuAddressDecoder * thisObj
    , struct kaphein_nes_Cartridge * cart
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == cart) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_CpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_CpuAddressDecoder_Impl *)thisObj->impl_;

        implPtr->cart_ = cart;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_read(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
)
{
    struct kaphein_nes_CpuAddressDecoder *const thisPtr = (struct kaphein_nes_CpuAddressDecoder *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    kaphein_UInt8 value = 0xFF;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == valueOut
    ) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_CpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_CpuAddressDecoder_Impl *)thisPtr->impl_;

        switch(address >> 12) {
	    //RAM
	    case 0x00:
	    case 0x01:
            value = implPtr->ram_[address & 0x7FF];
	    break;

        //PPU Registers
	    case 0x02:
	    case 0x03:
            resultErrorCode = kaphein_nes_RP2C02_readRegister(
                implPtr->ppu_
                , address & 0x07
                , &value
            );
	    break;

	    //Cartridge Area
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            resultErrorCode = (*implPtr->cart_->vTable->doCpuRead)(
                implPtr->cart_
                , address
                , &value
            );
	    break;
	    }

        *valueOut = value;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_CpuAddressDecoder_write(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
)
{
    struct kaphein_nes_CpuAddressDecoder *const thisPtr = (struct kaphein_nes_CpuAddressDecoder *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_CpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_CpuAddressDecoder_Impl *)thisPtr->impl_;

	    switch(address >> 12){
	    //RAM
	    case 0x0:
	    case 0x1:
            implPtr->ram_[address & 0x07FF] = value;
	    break;

	    //PPU Registers
	    case 0x2:
	    case 0x3:
            resultErrorCode = kaphein_nes_RP2C02_writeRegister(
                implPtr->ppu_
                , address & 0x07
                , value
            );
	    break;

	    //Cartridge Area
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            resultErrorCode = (*implPtr->cart_->vTable->doCpuWrite)(
                implPtr->cart_
                , address
                , value
            );
        break;
	    }
    }

    return resultErrorCode;
}
