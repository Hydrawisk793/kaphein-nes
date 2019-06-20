#include "kaphein/mem/utils.h"
#include "kaphein/nes/PpuAddressDecoder.h"

struct kaphein_nes_PpuAddressDecoder_Impl
{
    void * allocator_;
    
    struct kaphein_nes_RP2C02 * ppu_;

    struct kaphein_nes_Cartridge * cart_;

    kaphein_UInt8 * ram_;

    kaphein_SSize ramSize_;
};

static const struct kaphein_nes_AddressDecoder_VTable parentVTable = {
    kaphein_nes_PpuAddressDecoder_read
    , kaphein_nes_PpuAddressDecoder_write
};

enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_construct(
    struct kaphein_nes_PpuAddressDecoder * thisObj
    , void * allocator
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_PpuAddressDecoder_Impl * implPtr;
        
        implPtr = (struct kaphein_nes_PpuAddressDecoder_Impl *)kaphein_mem_allocate(
            allocator
            , KAPHEIN_ssizeof(*implPtr)
            , KAPHEIN_NULL
            ,  &resultErrorCode
        );

        if(KAPHEIN_NULL != implPtr) {
            thisObj->parent.vTable = &parentVTable;
            thisObj->impl_ = implPtr;

            implPtr->allocator_ = allocator;
            implPtr->ppu_ = KAPHEIN_NULL;
            implPtr->cart_ = KAPHEIN_NULL;
            implPtr->ram_ = KAPHEIN_NULL;
            implPtr->ramSize_ = 0;
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_destruct(
    struct kaphein_nes_PpuAddressDecoder * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_PpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_PpuAddressDecoder_Impl *)thisObj->impl_;
        
        resultErrorCode = kaphein_mem_deallocate(implPtr->allocator_, implPtr, KAPHEIN_ssizeof(*implPtr));
        if(resultErrorCode == kapheinErrorCodeNoError) {
            thisObj->impl_  = KAPHEIN_NULL;
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_connectPpu(
    struct kaphein_nes_PpuAddressDecoder * thisObj
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
        struct kaphein_nes_PpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_PpuAddressDecoder_Impl *)thisObj->impl_;

        implPtr->ppu_ = ppu;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_connectNametableMemory(
    struct kaphein_nes_PpuAddressDecoder * thisObj
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
        struct kaphein_nes_PpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_PpuAddressDecoder_Impl *)thisObj->impl_;
        
        implPtr->ram_ = ram;
        implPtr->ramSize_ = ramSize;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_connectCartridge(
    struct kaphein_nes_PpuAddressDecoder * thisObj
    , struct kaphein_nes_Cartridge * cart
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == cart) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_PpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_PpuAddressDecoder_Impl *)thisObj->impl_;

        implPtr->cart_ = cart;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_read(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 * valueOut
)
{
    struct kaphein_nes_PpuAddressDecoder *const thisPtr = (struct kaphein_nes_PpuAddressDecoder *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == valueOut
    ) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_PpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_PpuAddressDecoder_Impl *)thisPtr->impl_;

        resultErrorCode = (*implPtr->cart_->vTable->doPpuRead)(
            implPtr->cart_
            , address & 0x3FFF
            , valueOut
        );
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_PpuAddressDecoder_write(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
)
{
    struct kaphein_nes_PpuAddressDecoder *const thisPtr = (struct kaphein_nes_PpuAddressDecoder *)thisObj;
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_PpuAddressDecoder_Impl *const implPtr = (struct kaphein_nes_PpuAddressDecoder_Impl *)thisPtr->impl_;

        resultErrorCode = (*implPtr->cart_->vTable->doPpuWrite)(
            implPtr->cart_
            , address & 0x3FFF
            , value
        );
    }

    return resultErrorCode;
}
