#include "kaphein/nes/debug.h"
#include "kaphein/mem/utils.h"
#include "kaphein/nes/ShiftRegister16.h"
#include "kaphein/nes/RP2A03ApuEnvelop.h"
#include "kaphein/nes/RP2A03ApuLengthCounter.h"
#include "kaphein/nes/RP2A03ApuLinearCounter.h"
#include "kaphein/nes/RP2A03ApuPulseSequencer.h"
#include "kaphein/nes/RP2A03ApuTriangleSequencer.h"
#include "kaphein/nes/RP2A03ApuNoiseSequencer.h"
#include "kaphein/nes/RP2A03ApuDmcOutputUnit.h"
#include "kaphein/nes/RP2A03ApuFrameCounter.h"
#include "kaphein/nes/RP2A03Apu.h"

enum HEAPULengthCounterFixBit
{
    HEAPULengthCounterFixBit_SQX = 0x20
    , HEAPULengthCounterFixBit_TRI = 0x80
    , HEAPULengthCounterFixBit_NOI = 0x20
};

enum HEAPUReigster3
{
    HEAPUReigster3_LENGTH_INDEX = 0xF8
    , HEAPUReigster3_TIMER_H = 0x07
};

enum HEAPUSequenceCycles
{
    HEAPUSequenceCycles_SEQUENCE0 = 3728
    , HEAPUSequenceCycles_SEQUENCE1 = (3728 * 2)
    , HEAPUSequenceCycles_SEQUENCE2 = (3728 * 3 + 1)
    , HEAPUSequenceCycles_SEQUENCE3 = (3728 * 4 + 2)
    , HEAPUSequenceCycles_SEQUENCE4 = (3728 * 5)
};

enum HEAPUChannelFlag
{
    HEAPUChannelFlag_ENABLE_SQ1 = 0x01
    , HEAPUChannelFlag_ENABLE_SQ2 = 0x02
    , HEAPUChannelFlag_ENABLE_TRI = 0x04
    , HEAPUChannelFlag_ENABLE_NOI = 0x08
    , HEAPUChannelFlag_ENABLE_DMC = 0x10
    , HEAPUChannelFlag_ALL_CHANNELS = HEAPUChannelFlag_ENABLE_SQ1
        | HEAPUChannelFlag_ENABLE_SQ2
        | HEAPUChannelFlag_ENABLE_TRI
        | HEAPUChannelFlag_ENABLE_NOI
        | HEAPUChannelFlag_ENABLE_DMC
};

struct kaphein_nes_RP2A03Apu_Impl
{
    void (* apuCodeBlock)(
        struct kaphein_nes_RP2A03Apu * thisObj
    );

    void * allocator;

    kaphein_UInt8 * dataBus;

    kaphein_UInt8 apuSQ1Register0;          //��Ƽ ����Ŭ, �������� �Ķ����
    kaphein_UInt8 apuSQ1Register1;          //������ �Ķ����
    kaphein_UInt8 apuSQ1Register2;          //Ÿ�̸�(L) �Ķ����
    kaphein_UInt8 apuSQ1Register3;          //���� ī���� �ε���, Ÿ�̸�(H) �Ķ����
    struct kaphein_nes_RP2A03ApuEnvelop apuSQ1Envelope;
    struct kaphein_nes_RP2A03ApuPulseSequencer apuSQ1Sequencer;
    struct kaphein_nes_RP2A03ApuLengthCounter apuSQ1LengthCounter;

    kaphein_UInt8 apuSQ2Register0;          //��Ƽ ����Ŭ, �������� �Ķ����
    kaphein_UInt8 apuSQ2Register1;          //������ �Ķ����
    kaphein_UInt8 apuSQ2Register2;          //Ÿ�̸�(L) �Ķ����
    kaphein_UInt8 apuSQ2Register3;          //���� ī���� �ε���, Ÿ�̸�(H) �Ķ����
    struct kaphein_nes_RP2A03ApuEnvelop apuSQ2Envelope;
    struct kaphein_nes_RP2A03ApuPulseSequencer apuSQ2Sequencer;
    struct kaphein_nes_RP2A03ApuLengthCounter apuSQ2LengthCounter;

    kaphein_UInt8 apuTRIRegister0;          //���Ͼ� ī���� �Ķ����
    kaphein_UInt8 apuTRIRegister1;          //����
    kaphein_UInt8 apuTRIRegister2;          //Ÿ�̸�(L) �Ķ����
    kaphein_UInt8 apuTRIRegister3;          //���� ī���� �ε���, Ÿ�̸�(H) �Ķ����
    struct kaphein_nes_RP2A03ApuLinearCounter apuTRILinearCounter;
    struct kaphein_nes_RP2A03ApuTriangleSequencer apuTRISequencer;
    struct kaphein_nes_RP2A03ApuLengthCounter apuTRILengthCounter;

    kaphein_UInt8 apuNOIRegister0;
    kaphein_UInt8 apuNOIRegister1;          //����
    kaphein_UInt8 apuNOIRegister2;
    kaphein_UInt8 apuNOIRegister3;

    struct kaphein_nes_RP2A03ApuEnvelop apuNOIEnvelope;
    struct kaphein_nes_RP2A03ApuNoiseSequencer apuNOISequencer;
    struct kaphein_nes_RP2A03ApuLengthCounter apuNOILengthCounter;
    kaphein_UInt8 apuDMCParam0;             //(00xx 0000) �÷���, ���ļ�
    kaphein_UInt8 apuDMCParam1;             //DMA �Ķ����
    kaphein_UInt8 apuDMCParam2;             //DMC ���� �ּ�
    kaphein_UInt8 apuDMCParam3;             //DMC ���� ����

    struct kaphein_nes_RP2A03ApuDmcOutputUnit apuDMCOutput;     //DMC ��� ����
    struct kaphein_nes_RP2A03ApuDmcReader * apuDMCReader;
    kaphein_Int16 apuDMCSampleBuffer;
    
    kaphein_UInt8 apuChannelFlag;           //(000x xxxx)

    ////////////////////////////////
    //RP2A03 APU Frame Counter Registers

    struct kaphein_nes_RP2A03ApuFrameCounter apuFrameCounter;
    bool apuFrameCounterResetFlag;
    enum kaphein_nes_InterruptSignal apuIRQSignal;
    enum kaphein_nes_InterruptSignal apuDMCIRQSignal;
    kaphein_UInt8 apuIRQCounter;

    ////////////////////////////////

    ////////////////////////////////
    //RP2A03 APU Status Variables

    unsigned int apuDelayCounter;
    bool execEnvFlag;
    bool execSweepFlag;
    bool isInOddCycle;

    int debugCycleCounter;

    ////////////////////////////////
};

static
void
ON_RESET_0(
    struct kaphein_nes_RP2A03Apu * thisObj
);

static
void
APU_STATE0(
    struct kaphein_nes_RP2A03Apu * thisObj
);

static
void
APU_STATE1(
    struct kaphein_nes_RP2A03Apu * thisObj
);

static
void
APU_STATE2(
    struct kaphein_nes_RP2A03Apu * thisObj
);

static
void
APU_STATE3(
    struct kaphein_nes_RP2A03Apu * thisObj
);

static
void
APU_STATE4(
    struct kaphein_nes_RP2A03Apu * thisObj
);

static
void
APU_BLANK(
    struct kaphein_nes_RP2A03Apu * thisObj
);

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_construct(
    struct kaphein_nes_RP2A03Apu * thisObj
    , void * allocator
)
{
    enum kaphein_ErrorCode resultErrorCode;
    struct kaphein_nes_RP2A03Apu_Impl * impl;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        impl = (struct kaphein_nes_RP2A03Apu_Impl * )kaphein_mem_allocate(
            (struct kaphein_mem_Allocator *)allocator
            , KAPHEIN_ssizeof(*impl)
            , KAPHEIN_NULL
            , &resultErrorCode
        );
        if(KAPHEIN_NULL != impl) {
            kaphein_mem_fillZero(impl, KAPHEIN_ssizeof(*impl), KAPHEIN_ssizeof(*impl));
            
            thisObj->impl_ = impl;
            impl->allocator = allocator;

            impl->dataBus = KAPHEIN_NULL;
            
            kaphein_nes_RP2A03ApuEnvelop_construct(
                &impl->apuSQ1Envelope
                , &impl->apuSQ1Register0
            );
            kaphein_nes_RP2A03ApuPulseSequencer_construct(
                &impl->apuSQ1Sequencer
                , &impl->apuSQ1Register0
                , &impl->apuSQ1Register1
                , &impl->apuSQ1Register2
                , &impl->apuSQ1Register3
                , false
            );
            kaphein_nes_RP2A03ApuLengthCounter_construct(
                &impl->apuSQ1LengthCounter
                , HEAPULengthCounterFixBit_SQX
                , &impl->apuSQ1Register0
            );
        
            kaphein_nes_RP2A03ApuEnvelop_construct(
                &impl->apuSQ2Envelope
                , &impl->apuSQ2Register0
            );
            kaphein_nes_RP2A03ApuPulseSequencer_construct(
                &impl->apuSQ2Sequencer
                , &impl->apuSQ2Register0
                , &impl->apuSQ2Register1
                , &impl->apuSQ2Register2
                , &impl->apuSQ2Register3
                , true
            );
            kaphein_nes_RP2A03ApuLengthCounter_construct(
                &impl->apuSQ2LengthCounter
                , HEAPULengthCounterFixBit_SQX
                , &impl->apuSQ2Register0
            );
        
            kaphein_nes_RP2A03ApuLinearCounter_construct(
                &impl->apuTRILinearCounter
                , &impl->apuTRIRegister0
            );
            kaphein_nes_RP2A03ApuTriangleSequencer_construct(
                &impl->apuTRISequencer
                , &impl->apuTRIRegister2
                , &impl->apuTRIRegister3
            );
            kaphein_nes_RP2A03ApuLengthCounter_construct(
                &impl->apuTRILengthCounter
                , HEAPULengthCounterFixBit_TRI
                , &impl->apuTRIRegister0
            );
        
            kaphein_nes_RP2A03ApuEnvelop_construct(
                &impl->apuNOIEnvelope
                , &impl->apuNOIRegister0
            );
            kaphein_nes_RP2A03ApuNoiseSequencer_construct(
                &impl->apuNOISequencer
                , &impl->apuNOIRegister2
            );
            kaphein_nes_RP2A03ApuLengthCounter_construct(
                &impl->apuNOILengthCounter
                , HEAPULengthCounterFixBit_NOI
                , &impl->apuNOIRegister0
            );

            kaphein_nes_RP2A03ApuDmcOutputUnit_construct(
                &impl->apuDMCOutput
                , &impl->apuDMCParam0
                , &impl->apuDMCParam0
                , &impl->apuDMCSampleBuffer
            );
            impl->apuDMCReader = KAPHEIN_NULL;

            impl->apuFrameCounterResetFlag = false;
            kaphein_nes_RP2A03ApuFrameCounter_construct(
                &impl->apuFrameCounter
                , 2
            );
            
            impl->apuIRQSignal = kaphein_nes_InterruptSignal_NONE;
            impl->apuDMCIRQSignal = kaphein_nes_InterruptSignal_NONE;

            impl->execEnvFlag = false;
            impl->execSweepFlag = false;
            impl->apuCodeBlock = KAPHEIN_NULL;
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_destruct(
    struct kaphein_nes_RP2A03Apu * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
        
        if(KAPHEIN_NULL != impl) {
            kaphein_nes_RP2A03Apu_setDmcReader(thisObj, KAPHEIN_NULL);

            resultErrorCode = kaphein_mem_deallocate(
                (struct kaphein_mem_Allocator *)impl->allocator
                , impl
                , KAPHEIN_ssizeof(*impl)
            );
            if(kapheinErrorCodeNoError == resultErrorCode) {
                thisObj->impl_ = KAPHEIN_NULL;
            }
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_setIoBuses(
    struct kaphein_nes_RP2A03Apu * thisObj
    , kaphein_UInt16 * addrBus
    , kaphein_UInt8 * dataBus
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;

        KAPHEIN_x_UNUSED_PARAMETER(addrBus)

        //impl->addrBus = addrBus;
        impl->dataBus = dataBus;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_setDmcReader(
    struct kaphein_nes_RP2A03Apu * thisObj
    , struct kaphein_nes_RP2A03ApuDmcReader * reader
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
        
        if(KAPHEIN_NULL == impl) {
            resultErrorCode = kapheinErrorCodeOperationInvalid;
        }
        else {
            if(KAPHEIN_NULL != impl->apuDMCReader) {
                (*impl->apuDMCReader->vTable->setFlagParameter)(
                    impl->apuDMCReader
                    , KAPHEIN_NULL
                );
                (*impl->apuDMCReader->vTable->setAddressParameter)(
                    impl->apuDMCReader
                    , KAPHEIN_NULL
                );
                (*impl->apuDMCReader->vTable->setLengthParameter)(
                    impl->apuDMCReader
                    , KAPHEIN_NULL
                );
                (*impl->apuDMCReader->vTable->setSampleBuffer)(
                    impl->apuDMCReader
                    , KAPHEIN_NULL
                );
            }

            impl->apuDMCReader = reader;

            if(KAPHEIN_NULL != reader) {
                (*reader->vTable->setFlagParameter)(
                    reader
                    , &impl->apuDMCParam0
                );
                (*reader->vTable->setAddressParameter)(
                    reader
                    , &impl->apuDMCParam2
                );
                (*reader->vTable->setLengthParameter)(
                    reader
                    , &impl->apuDMCParam3
                );
                (*reader->vTable->setSampleBuffer)(
                    reader
                    , &impl->apuDMCSampleBuffer
                );
            }

            kaphein_nes_RP2A03ApuDmcOutputUnit_setDmcReader(
                &impl->apuDMCOutput
                , reader
            );
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_isInterruptRequested(
    struct kaphein_nes_RP2A03Apu * thisObj
    , bool * truthOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
        
        *truthOut = 
            (impl->apuIRQSignal == kaphein_nes_InterruptSignal_OCCUR)
            || (impl->apuDMCIRQSignal == kaphein_nes_InterruptSignal_OCCUR)
        ;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_getROut(
    struct kaphein_nes_RP2A03Apu * thisObj
    , double * valueOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == valueOut
    ) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
        kaphein_UInt8 env, seq;
        kaphein_UInt8 sq1 = 0;
        kaphein_UInt8 sq2 = 0;
        bool truth;
        
        if(kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuSQ1LengthCounter, &truth), truth) {
            kaphein_nes_RP2A03ApuEnvelop_getOutput(&impl->apuSQ1Envelope, &env);
            kaphein_nes_RP2A03ApuPulseSequencer_getOutput(&impl->apuSQ1Sequencer, &seq);
            
            sq1 = env * seq;
        }

        if(kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuSQ2LengthCounter, &truth), truth) {
            kaphein_nes_RP2A03ApuEnvelop_getOutput(&impl->apuSQ2Envelope, &env);
            kaphein_nes_RP2A03ApuPulseSequencer_getOutput(&impl->apuSQ2Sequencer, &seq);
            
            sq2 = env * seq;
        }
        
        *valueOut = 0.00752 * (sq1 + sq2);
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_getCOut(
    struct kaphein_nes_RP2A03Apu * thisObj
    , double * valueOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == valueOut
    ) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
        kaphein_UInt8 tri = 0;
        kaphein_UInt8 noi = 0;
        kaphein_UInt8 env, seq;
        kaphein_UInt8 dmc;
        bool isLinCntZero, isLenCntZero;
        
        kaphein_nes_RP2A03ApuLinearCounter_isNonZero(&impl->apuTRILinearCounter, &isLinCntZero);
        kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuTRILengthCounter, &isLenCntZero);
        kaphein_nes_RP2A03ApuTriangleSequencer_getOutput(&impl->apuTRISequencer, &seq);
        tri = (kaphein_UInt8)((isLinCntZero * isLenCntZero * seq) * 1.3);

        kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuNOILengthCounter, &isLenCntZero);
        kaphein_nes_RP2A03ApuEnvelop_getOutput(&impl->apuNOIEnvelope, &env);
        kaphein_nes_RP2A03ApuNoiseSequencer_getOutput(&impl->apuNOISequencer, &seq);
        noi = (kaphein_UInt8)((isLenCntZero * env * seq) * 1.2);
        
        kaphein_nes_RP2A03ApuDmcOutputUnit_getOutput(&impl->apuDMCOutput, &dmc);
        
        *valueOut = 0.00851 * tri + 0.00494 * noi + 0.00335 * dmc;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_isInterruptFlagSet(
    struct kaphein_nes_RP2A03Apu * thisObj
    , bool * truthOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == truthOut
    ) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;

        *truthOut = 
            (impl->apuIRQSignal == kaphein_nes_InterruptSignal_OCCUR)
            || (impl->apuDMCIRQSignal == kaphein_nes_InterruptSignal_OCCUR)
        ;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_powerUp(
    struct kaphein_nes_RP2A03Apu * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;

        //APU �ʱ�ȭ
        kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuSQ1LengthCounter);
        kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuSQ2LengthCounter);
        kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuTRILengthCounter);
        kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuNOILengthCounter);

        //APU ä�� �Ķ���� �ʱ�ȭ
        impl->apuSQ1Register0 = 0;
        impl->apuSQ2Register0 = 0;
        impl->apuTRIRegister0 = 0;
        impl->apuNOIRegister0 = 0;
        impl->apuDMCParam0 = 0;
        impl->apuDMCParam1 = 0;
        impl->apuDMCParam2 = 0;
        impl->apuDMCParam3 = 0;

        impl->apuDMCSampleBuffer = 0;

	    //����
        resultErrorCode = kaphein_nes_RP2A03Apu_reset(thisObj);
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_reset(
    struct kaphein_nes_RP2A03Apu * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;

        //���ͷ�Ʈ �ñ׳� �ʱ�ȭ
        impl->apuIRQSignal = kaphein_nes_InterruptSignal_NONE;
        impl->apuDMCIRQSignal = kaphein_nes_InterruptSignal_NONE;

        //ī���� �ʱ�ȭ
        impl->apuIRQCounter = 0;
        impl->apuDelayCounter = 0;
        impl->isInOddCycle = false;
        impl->debugCycleCounter = 0;

        kaphein_nes_RP2A03Apu_writeRegister(thisObj, 0x15, 0x00);

        //���� ��� ����
        impl->apuCodeBlock = ON_RESET_0;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_run(
    struct kaphein_nes_RP2A03Apu * thisObj
    , int cycleCount
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    int i;
    bool truth;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;

        for(i = cycleCount; i > 0; ) {
            --i;

            if(impl->apuDelayCounter > 0) {
                --impl->apuDelayCounter;
            }
            else {
                if(impl->execEnvFlag) {
                    //�������� ó��
                    kaphein_nes_RP2A03ApuEnvelop_run(&impl->apuSQ1Envelope);
                    kaphein_nes_RP2A03ApuEnvelop_run(&impl->apuSQ2Envelope);
                    kaphein_nes_RP2A03ApuEnvelop_run(&impl->apuNOIEnvelope);
                    
                    //TRI ���Ͼ� ī���� ó��
                    kaphein_nes_RP2A03ApuLinearCounter_run(&impl->apuTRILinearCounter);
                    
                    impl->execEnvFlag = false;
                }
                else if(impl->execSweepFlag) {
                    //�������� ó��
                    kaphein_nes_RP2A03ApuEnvelop_run(&impl->apuSQ1Envelope);
                    kaphein_nes_RP2A03ApuEnvelop_run(&impl->apuSQ2Envelope);
                    kaphein_nes_RP2A03ApuEnvelop_run(&impl->apuNOIEnvelope);
                    
                    //TRI ���Ͼ� ī���� ó��
                    kaphein_nes_RP2A03ApuLinearCounter_run(&impl->apuTRILinearCounter);

                    //���� ī���� ó��
                    if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_SQ1)) {
                        kaphein_nes_RP2A03ApuLengthCounter_run(&impl->apuSQ1LengthCounter);
                    }
                    if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_SQ2)) {
                        kaphein_nes_RP2A03ApuLengthCounter_run(&impl->apuSQ2LengthCounter);
                    }
                    if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_TRI)) {
                        kaphein_nes_RP2A03ApuLengthCounter_run(&impl->apuTRILengthCounter);
                    }
                    if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_NOI)) {
                        kaphein_nes_RP2A03ApuLengthCounter_run(&impl->apuNOILengthCounter);
                    }

                    //������ ó��
                    kaphein_nes_RP2A03ApuPulseSequencer_runSweep(&impl->apuSQ1Sequencer);
                    kaphein_nes_RP2A03ApuPulseSequencer_runSweep(&impl->apuSQ2Sequencer);
                    
                    impl->execSweepFlag = false;
                }
                
                if(kaphein_nes_RP2A03ApuFrameCounter_run(&impl->apuFrameCounter, &truth), truth) {
                    (*impl->apuCodeBlock)(thisObj);

                    if(impl->apuFrameCounterResetFlag) {
                        kaphein_nes_RP2A03ApuFrameCounter_resetSequencer(&impl->apuFrameCounter);
                        impl->apuCodeBlock = APU_STATE0;
                        impl->apuFrameCounterResetFlag = false;
                    }
                    else {
                        kaphein_nes_RP2A03ApuFrameCounter_incrementSequencer(&impl->apuFrameCounter);
                    }
                    
                    if(kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuSQ1LengthCounter, &truth), truth) {
                        kaphein_nes_RP2A03ApuPulseSequencer_runSequencer(&impl->apuSQ1Sequencer);
                    }

                    if(kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuSQ2LengthCounter, &truth), truth) {
                        kaphein_nes_RP2A03ApuPulseSequencer_runSequencer(&impl->apuSQ2Sequencer);
                    }

                    if(kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuNOILengthCounter, &truth), truth) {
                        kaphein_nes_RP2A03ApuNoiseSequencer_run(&impl->apuNOISequencer);
                    }

                    kaphein_nes_RP2A03ApuDmcOutputUnit_run(&impl->apuDMCOutput, impl->debugCycleCounter);
                }

                if(
                    (kaphein_nes_RP2A03ApuLinearCounter_isNonZero(&impl->apuTRILinearCounter, &truth), truth)
                    && (kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuTRILengthCounter, &truth), truth)
                ) {
                    kaphein_nes_RP2A03ApuTriangleSequencer_run(&impl->apuTRISequencer);
                }
                
                //APU ������ ī���� ���ͷ�Ʈ
                if(
                    impl->apuIRQCounter > 0  //���ͷ�Ʈ ī���Ͱ� 0���� ũ��
                    && (
                        kaphein_nes_RP2A03ApuFrameCounter_getDisableIRQFlag(
                            &impl->apuFrameCounter
                            , &truth
                        )
                        , !truth
                    )   //���ͷ�Ʈ ���� �÷��װ� Ŭ���� �Ǿ�������
                ) {
                    if(impl->apuIRQCounter <= 2) {
                        impl->apuIRQSignal = kaphein_nes_InterruptSignal_OCCUR;
                    }

                    //���ͷ�Ʈ �÷��� ��
                    kaphein_nes_RP2A03ApuFrameCounter_setInterruptFlag(&impl->apuFrameCounter);

                    //���ͷ�Ʈ ī���� ����
                    --impl->apuIRQCounter;
                }
                
                if((*impl->apuDMCReader->vTable->hasIrqOccured)(impl->apuDMCReader, &truth), truth) {
                    impl->apuDMCIRQSignal = kaphein_nes_InterruptSignal_OCCUR;
                }
            }

            impl->isInOddCycle = !impl->isInOddCycle;
            ++impl->debugCycleCounter;
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_readRegister(
    struct kaphein_nes_RP2A03Apu * thisObj
    , kaphein_UInt8 address
    , kaphein_UInt8 * valueOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
    kaphein_UInt8 returnValue = *impl->dataBus;
    bool truth;

	switch(address & 0x1F) {
	case 0x00:
    case 0x01:
	case 0x02:
	case 0x03:
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
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
        //Does nothing. it's an intention.
	break;

    //APU Status
	case 0x15:
        //���ͷ�Ʈ �÷���, �� ä�ε��� Length ī���� ���� ����
        returnValue = 0x00;
        
        (*impl->apuDMCReader->vTable->hasIrqOccured)(impl->apuDMCReader, &truth);
        returnValue |= !!truth;
        
        kaphein_nes_RP2A03ApuFrameCounter_isInterruptFlagSet(&impl->apuFrameCounter, &truth);
        returnValue <<= 1;
        returnValue |= !!truth;

        (*impl->apuDMCReader->vTable->hasRemainingBytesToRead)(impl->apuDMCReader, &truth);
        returnValue <<= 2;
        returnValue |= !!truth;

        kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuNOILengthCounter, &truth);
        returnValue <<= 1;
        returnValue |= !!truth;

        kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuTRILengthCounter, &truth);
        returnValue <<= 1;
        returnValue |= !!truth;

        kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuSQ2LengthCounter, &truth);
        returnValue <<= 1;
        returnValue |= !!truth;

        kaphein_nes_RP2A03ApuLengthCounter_isNonZero(&impl->apuSQ1LengthCounter, &truth);
        returnValue <<= 1;
        returnValue |= !!truth;

        //������ ī���� ���ͷ�Ʈ �÷��� Ŭ����
        kaphein_nes_RP2A03ApuFrameCounter_clearInterruptFlag(&impl->apuFrameCounter);
        impl->apuIRQSignal = kaphein_nes_InterruptSignal_NONE;
	break;
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
        //Does nothing. it's an intention.
	break;
    }

    *valueOut = returnValue;

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03Apu_writeRegister(
    struct kaphein_nes_RP2A03Apu * thisObj
    , kaphein_UInt8 address
    , kaphein_UInt8 value
)
{
    struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
    kaphein_UInt8 mode;
    bool truth;
    
    switch(address & 0x1F) {
	case 0x00:  //Rectangle Wave 1
        impl->apuSQ1Register0 = value;
    break;
	case 0x01:  //Rectangle Wave 1
        impl->apuSQ1Register1 = value;

        //������ ���ε� �÷��� ��
        kaphein_nes_RP2A03ApuPulseSequencer_setSweepReloadFlag(&impl->apuSQ1Sequencer);
    break;
	case 0x02:  //Rectangle Wave 1
        impl->apuSQ1Register2 = value;
    break;
	case 0x03:  //Rectangle Wave 1
        impl->apuSQ1Register3 = value;

        //���� ī���� ��
        if(impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_SQ1) {
            kaphein_nes_RP2A03ApuLengthCounter_set(&impl->apuSQ1LengthCounter, (value & HEAPUReigster3_LENGTH_INDEX) >> 3);
        }

        //�������� ���� �÷��� ��
        kaphein_nes_RP2A03ApuEnvelop_setStartFlag(&impl->apuSQ1Envelope);

        //������ �����
        kaphein_nes_RP2A03ApuPulseSequencer_resetSequencer(&impl->apuSQ1Sequencer);
        kaphein_nes_RP2A03ApuPulseSequencer_resetTimer(&impl->apuSQ1Sequencer);
    break;
	case 0x04:  //Rectangle Wave 2
        impl->apuSQ2Register0 = value;
    break;
	case 0x05:  //Rectangle Wave 2
        impl->apuSQ2Register1 = value;

        //������ ���ε� �÷��� ��
        kaphein_nes_RP2A03ApuPulseSequencer_setSweepReloadFlag(&impl->apuSQ2Sequencer);
    break;
	case 0x06:  //Rectangle Wave 2
        impl->apuSQ2Register2 = value;
	break;
    case 0x07:  //Rectangle Wave 2
        impl->apuSQ2Register3 = value;

        //���� ī���� ��
        if(impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_SQ2) {
            kaphein_nes_RP2A03ApuLengthCounter_set(&impl->apuSQ2LengthCounter, (value & HEAPUReigster3_LENGTH_INDEX) >> 3);
        }

        //�������� ���� �÷��� ��
        kaphein_nes_RP2A03ApuEnvelop_setStartFlag(&impl->apuSQ2Envelope);

        //������ �����
        kaphein_nes_RP2A03ApuPulseSequencer_resetSequencer(&impl->apuSQ2Sequencer);
        kaphein_nes_RP2A03ApuPulseSequencer_resetTimer(&impl->apuSQ2Sequencer);
    break;
	case 0x08:  //Triangle Wave
        impl->apuTRIRegister0 = value;
    break;
	case 0x09:  //Triangle Wave
        //Does nothing. It's an intention.
    break;
	case 0x0A:  //Triangle Wave
        impl->apuTRIRegister2 = value;
    break;
	case 0x0B:  //Triangle Wave
        impl->apuTRIRegister3 = value;

        //���� ī���� ��
        if(impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_TRI) {
            kaphein_nes_RP2A03ApuLengthCounter_set(&impl->apuTRILengthCounter, (value & HEAPUReigster3_LENGTH_INDEX) >> 3);
        }

        //���Ͼ� ī���� ���ε� �÷��� ��
        kaphein_nes_RP2A03ApuLinearCounter_setReloadFlag(&impl->apuTRILinearCounter);
    break;
	case 0x0C:  //Noise Wave
        impl->apuNOIRegister0 = value;
    break;
    case 0x0D:  //Noise Wave
        //Does nothing. It's an intention.
    break;
	case 0x0E:  //Noise Wave
        impl->apuNOIRegister2 = value;
    break;
	case 0x0F:  //Noise Wave
        impl->apuNOIRegister3 = value;

        //���� ī���� ��
        if(impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_NOI) {
            kaphein_nes_RP2A03ApuLengthCounter_set(&impl->apuNOILengthCounter, (value & HEAPUReigster3_LENGTH_INDEX) >> 3);
        }
        
        //�������� ���� �÷��� ��
        kaphein_nes_RP2A03ApuEnvelop_setStartFlag(&impl->apuNOIEnvelope);
    break;
	case 0x10:  //DPCM
        impl->apuDMCParam0 = value;
        if((value & 0x80) == 0){
            (*impl->apuDMCReader->vTable->clearIrqOccuredFlag)(impl->apuDMCReader);
            impl->apuDMCIRQSignal = kaphein_nes_InterruptSignal_NONE;
        }
    break;
	case 0x11:  //DPCM
        impl->apuDMCParam1 = value;
        kaphein_nes_RP2A03ApuDmcOutputUnit_setDeltaCounter(&impl->apuDMCOutput, value);
    break;
	case 0x12:  //DPCM
        impl->apuDMCParam2 = value;
    break;
	case 0x13:  //DPCM
        impl->apuDMCParam3 = value;
    break;
	case 0x14:
        //Does nothing. It's an intention.
	break;
	case 0x15:  //APU Status
        //ä�� �÷��� ��
        impl->apuChannelFlag = value & 0x1F;

        //ä���� ��Ȱ��ȭ �Ǿ������� ���� ī���͸� 0���� ����
        if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_SQ1) == 0) {
            kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuSQ1LengthCounter);
        }
        if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_SQ2) == 0) {
            kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuSQ2LengthCounter);
        }
        if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_TRI) == 0) {
            kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuTRILengthCounter);
        }
        if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_NOI) == 0) {
            kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuNOILengthCounter);
        }

        //����� DMC Ȱ�� �÷��׿� ���� ó��
        if(impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_DMC) {
            if((*impl->apuDMCReader->vTable->hasRemainingBytesToRead)(impl->apuDMCReader, &truth), !truth) {
                (*impl->apuDMCReader->vTable->setReadingEnabled)(impl->apuDMCReader, true);
            }
        }
        else {
            (*impl->apuDMCReader->vTable->setReadingEnabled)(impl->apuDMCReader, false);
        }

        //DMC IRQ �÷��� Ŭ����
        (*impl->apuDMCReader->vTable->clearIrqOccuredFlag)(impl->apuDMCReader);
        impl->apuDMCIRQSignal = kaphein_nes_InterruptSignal_NONE;
	break;
	case 0x16:
        //Does nothing.
    break;
	case 0x17:  //APU Frame Counter Config
        //�÷��� ��
        kaphein_nes_RP2A03ApuFrameCounter_setConfig(&impl->apuFrameCounter, value);

        //APU ���ν��� ��
        impl->apuCodeBlock = APU_BLANK;

        //������ ���� �÷��� ��
        impl->apuFrameCounterResetFlag = true;

        //���� ������ ��尡 1�̸� APU�� ��� ó�� ����
        if((kaphein_nes_RP2A03ApuFrameCounter_getMode(&impl->apuFrameCounter, &mode), mode) != 0) {
            //�������� ó��
            kaphein_nes_RP2A03ApuEnvelop_run(&impl->apuSQ1Envelope);
            kaphein_nes_RP2A03ApuEnvelop_run(&impl->apuSQ2Envelope);
            kaphein_nes_RP2A03ApuEnvelop_run(&impl->apuNOIEnvelope);
            kaphein_nes_RP2A03ApuLinearCounter_run(&impl->apuTRILinearCounter);

            //���� ī���� ó��
            if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_SQ1) != 0) {
                kaphein_nes_RP2A03ApuLengthCounter_run(&impl->apuSQ1LengthCounter);
            }
            if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_SQ2) != 0) {
                kaphein_nes_RP2A03ApuLengthCounter_run(&impl->apuSQ2LengthCounter);
            }
            if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_TRI) != 0) {
                kaphein_nes_RP2A03ApuLengthCounter_run(&impl->apuTRILengthCounter);
            }
            if((impl->apuChannelFlag & HEAPUChannelFlag_ENABLE_NOI) != 0) {
                kaphein_nes_RP2A03ApuLengthCounter_run(&impl->apuNOILengthCounter);
            }

            //������ ó��
            kaphein_nes_RP2A03ApuPulseSequencer_runSweep(&impl->apuSQ1Sequencer);
            kaphein_nes_RP2A03ApuPulseSequencer_runSweep(&impl->apuSQ2Sequencer);
        }

        //���ͷ�Ʈ ���� �÷��װ� �� �Ǹ�
        if(kaphein_nes_RP2A03ApuFrameCounter_getDisableIRQFlag(&impl->apuFrameCounter, &truth), truth){
            //���ͷ�Ʈ �÷��� Ŭ����
            kaphein_nes_RP2A03ApuFrameCounter_clearInterruptFlag(&impl->apuFrameCounter);
            impl->apuIRQSignal = kaphein_nes_InterruptSignal_NONE;
        }

        //���ͷ�Ʈ ī���� Ŭ����
        impl->apuIRQCounter = 0;
    break;
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
        //Does nothing. it's an intention.
	break;
    }

    return kapheinErrorCodeNoError;
}

static
void
ON_RESET_0(
    struct kaphein_nes_RP2A03Apu * thisObj
)
{
    struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;

    //APU ä�� ���� �ʱ�ȭ
    kaphein_nes_RP2A03ApuEnvelop_reset(&impl->apuSQ1Envelope);
    kaphein_nes_RP2A03ApuPulseSequencer_reset(&impl->apuSQ1Sequencer);
    kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuSQ1LengthCounter);
    kaphein_nes_RP2A03ApuEnvelop_reset(&impl->apuSQ2Envelope);
    kaphein_nes_RP2A03ApuPulseSequencer_reset(&impl->apuSQ2Sequencer);
    kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuSQ2LengthCounter);
    kaphein_nes_RP2A03ApuLinearCounter_reset(&impl->apuTRILinearCounter);
    kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuTRILengthCounter);
    kaphein_nes_RP2A03ApuEnvelop_reset(&impl->apuNOIEnvelope);
    kaphein_nes_RP2A03ApuNoiseSequencer_reset(&impl->apuNOISequencer);
    kaphein_nes_RP2A03ApuLengthCounter_reset(&impl->apuNOILengthCounter);
    kaphein_nes_RP2A03ApuDmcOutputUnit_reset(&impl->apuDMCOutput);
    (*impl->apuDMCReader->vTable->reset)(impl->apuDMCReader);

    //APU ä�� ��Ȱ��ȭ
    impl->apuChannelFlag = 0;

    //APU ������ ī����, Ŭ�� ����̴� �ʱ�ȭ
    kaphein_nes_RP2A03ApuFrameCounter_resetClockDivider(&impl->apuFrameCounter);
    kaphein_nes_RP2A03ApuFrameCounter_resetSequencer(&impl->apuFrameCounter);
    impl->apuFrameCounterResetFlag = false;
    kaphein_nes_RP2A03ApuFrameCounter_clearInterruptFlag(&impl->apuFrameCounter);
    impl->apuIRQCounter = 0;

    //APU ���� ���� �ʱ�ȭ
    impl->execEnvFlag = false;
    impl->execSweepFlag = false;
    
    impl->apuCodeBlock = APU_STATE0;
}

static
void
APU_STATE0(
    struct kaphein_nes_RP2A03Apu * thisObj
)
{
    struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
    unsigned int seq;
    
    if(
        (kaphein_nes_RP2A03ApuFrameCounter_getSequencer(&impl->apuFrameCounter, &seq), seq) != HEAPUSequenceCycles_SEQUENCE0
    ) {
        return;
    }
    
    //apuEnvelopeProc();          //�������� ó��
    //apuTriLengthCounterProc();  //TRI ���� ī���� ó��
    impl->execEnvFlag = true;
            
    impl->apuCodeBlock = APU_STATE1;    //�ڵ� ��� ����
}

static
void
APU_STATE1(
    struct kaphein_nes_RP2A03Apu * thisObj
)
{
    struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
    unsigned int seq;
    
    if(
        (kaphein_nes_RP2A03ApuFrameCounter_getSequencer(&impl->apuFrameCounter, &seq), seq) != HEAPUSequenceCycles_SEQUENCE1
    ) {
        return;
    }
    
    //apuEnvelopeProc();          //�������� ó��
    //apuTriLengthCounterProc();  //TRI ���� ī���� ó��

    //apuLengthCounterProc();     //���� ī���� ó��
    //apuSweepProc();             //������ ó��
    impl->execSweepFlag = true;

    impl->apuCodeBlock = APU_STATE2;    //�ڵ� ��� ����
}

static
void
APU_STATE2(
    struct kaphein_nes_RP2A03Apu * thisObj
)
{
    struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
    unsigned int seq;
    kaphein_UInt8 mode;
    
    if(
        (kaphein_nes_RP2A03ApuFrameCounter_getSequencer(&impl->apuFrameCounter, &seq), seq) != HEAPUSequenceCycles_SEQUENCE2
    ) {
        return;
    }
    
    //apuEnvelopeProc();          //�������� ó��
    //apuTriLengthCounterProc();  //TRI ���� ī���� ó��
    impl->execEnvFlag = true;

    //�ڵ� ��� ����
    impl->apuCodeBlock = (
        (kaphein_nes_RP2A03ApuFrameCounter_getMode(&impl->apuFrameCounter, &mode), mode) != 0
        ? APU_STATE4
        : APU_STATE3
    );
}

static
void
APU_STATE3(
    struct kaphein_nes_RP2A03Apu * thisObj
)
{
    struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
    unsigned int seq;
    
    if(
        (kaphein_nes_RP2A03ApuFrameCounter_getSequencer(&impl->apuFrameCounter, &seq), seq) != HEAPUSequenceCycles_SEQUENCE3
    ) {
        return;
    }
    
    //apuEnvelopeProc();          //�������� ó��
    //apuTriLengthCounterProc();  //TRI ���� ī���� ó��
    
    //apuLengthCounterProc();     //���� ī���� ó��
    //apuSweepProc();             //������ ó��
    impl->execSweepFlag = true;

    impl->apuIRQCounter = 3;          //���ͷ�Ʈ ī���� ��

    impl->apuFrameCounterResetFlag = true;    //������ ����
    impl->apuCodeBlock = APU_STATE0;    //�ڵ� ��� ����
}

static
void
APU_STATE4(
    struct kaphein_nes_RP2A03Apu * thisObj
)
{
    struct kaphein_nes_RP2A03Apu_Impl *const impl = (struct kaphein_nes_RP2A03Apu_Impl *)thisObj->impl_;
    unsigned int seq;
    
    if(
        (kaphein_nes_RP2A03ApuFrameCounter_getSequencer(&impl->apuFrameCounter, &seq), seq) != HEAPUSequenceCycles_SEQUENCE4
    ) {
        return;
    }

    //apuEnvelopeProc();          //�������� ó��
    //apuTriLengthCounterProc();  //TRI ���� ī���� ó��
    
    //apuLengthCounterProc();     //���� ī���� ó��
    //apuSweepProc();             //������ ó��
    impl->execSweepFlag = true;

    impl->apuFrameCounterResetFlag = true;    //������ ����
    impl->apuCodeBlock = APU_STATE0;    //�ڵ� ��� ����
}

static
void
APU_BLANK(
    struct kaphein_nes_RP2A03Apu * thisObj
)
{
    KAPHEIN_x_UNUSED_PARAMETER(thisObj)
    
    //Does nothing. It's an intention.
}
