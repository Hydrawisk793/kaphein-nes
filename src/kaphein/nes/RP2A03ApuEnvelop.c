#include "kaphein/nes/RP2A03ApuEnvelop.h"

#define MAX_COUNTER_VALUE (0x0F)

enum HEParam
{
    HEParam_LOOP_ENVELOPE = 0x20
    , HEParam_USE_CONSTANT_VOLUME = 0x10
    , HEParam_VOLUME = 0x0F
    , HEParam_PERIOD = 0x0F
};

//TODO : �����ڵ� ó��

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuEnvelop_construct(
    struct kaphein_nes_RP2A03ApuEnvelop * thisObj
    , kaphein_UInt8 * parameterSource
)
{
    thisObj->pParam_ = parameterSource;
    //thisObj->divider_.counter = 0;
    //thisObj->divider_.period = 0;
    thisObj->counter_ = 0;
    thisObj->startFlag_ = false;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuEnvelop_getOutput(
    struct kaphein_nes_RP2A03ApuEnvelop * thisObj
    , kaphein_UInt8 * valueOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    kaphein_UInt8 param = *thisObj->pParam_;
    
    *valueOut = (
        ((param & HEParam_USE_CONSTANT_VOLUME) != 0)
        ? (param & HEParam_VOLUME)
        : (thisObj->counter_)
    );

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuEnvelop_setStartFlag(
    struct kaphein_nes_RP2A03ApuEnvelop * thisObj
)
{
    thisObj->startFlag_ = true;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuEnvelop_reset(
    struct kaphein_nes_RP2A03ApuEnvelop * thisObj
)
{
    thisObj->counter_ = 0;
    thisObj->startFlag_ = false;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuEnvelop_run(
    struct kaphein_nes_RP2A03ApuEnvelop * thisObj
)
{
    bool dividerResult;
    
    if(thisObj->startFlag_) {
        thisObj->startFlag_ = false;

        //ī���� ���ε�
        thisObj->counter_ = MAX_COUNTER_VALUE;

        //����̴� �ֱ� ���� �� ����
        thisObj->divider_.period = (*thisObj->pParam_ & HEParam_PERIOD) + 1;  
        kaphein_nes_ClockDivider_reset(&thisObj->divider_);
    }
    //���� �÷��� Ŭ����� ���¿��� ����̴��κ��� Ŭ���� �߻��ϸ�
    else if(kaphein_nes_ClockDivider_run(&thisObj->divider_, &dividerResult), dividerResult) {
        //ī���� ����
        if(thisObj->counter_ > 0) {
            --thisObj->counter_;
        }
        //ī���Ͱ� 0�̰� ���� �÷��װ� �� �Ǿ�������
        else if((*thisObj->pParam_ & HEParam_LOOP_ENVELOPE) != 0) {
            //ī���� ���ε�
            thisObj->counter_ = MAX_COUNTER_VALUE;
        }
    }

    return kapheinErrorCodeNoError;
}
