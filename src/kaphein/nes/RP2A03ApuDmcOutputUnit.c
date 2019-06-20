#include "kaphein/nes/RP2A03ApuDmcOutputUnit.h"
#include "kaphein/nes/debug.h"

static
const kaphein_UInt8
rateTable[0x10] = {
    //NTSC - CPU 클럭 기준
    //428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
    
    //NTSC - APU 클럭(CPU 클럭의 절반) 기준
    214, 190, 170, 160, 143, 127, 113, 107, 95, 80, 71, 64, 53, 42, 36, 27
    
    //PAL
    //...
};

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_construct(
    struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
    , kaphein_UInt8 * flagSource
    , kaphein_UInt8 * freqSource
    , kaphein_Int16 * buffer
)
{
    thisObj->pFlagParam_ = flagSource;
    thisObj->pFreqParam_ = freqSource;
    thisObj->buffer_ = buffer;
    thisObj->reader_ = KAPHEIN_NULL;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_setDmcReader(
    struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
    , struct kaphein_nes_RP2A03ApuDmcReader * reader
)
{
    thisObj->reader_ = reader;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_getOutput(
    const struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
    , kaphein_UInt8 * resultOut
)
{
    *resultOut = thisObj->deltaCounter_;//(thisObj->silenceFlag_ ? 0x00 : thisObj->deltaCounter_);

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_setDeltaCounter(
    struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
    , kaphein_UInt8 v
)
{
    thisObj->deltaCounter_ = v;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_run(
    struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
    , int debugCycleCounter //TODO : 삭제
)
{
    kaphein_Int16 sample;
    
    if(--thisObj->timer_ < 1) {
        //타이머 리로드
        thisObj->timer_ = rateTable[*thisObj->pFreqParam_ & 0x0F];
        
        if(!thisObj->silenceFlag_) {
            //버퍼에서 1비트를 꺼냄
            if(kaphein_nes_ShiftRegister8_shiftRight(&thisObj->sr_, false)) {
                if(thisObj->deltaCounter_ < 126) {
                    thisObj->deltaCounter_ += 2;
                }
            }
            else {
                if(thisObj->deltaCounter_ > 1) {
                    thisObj->deltaCounter_ -= 2;
                }
            }
        }

        --thisObj->remainingBitsCounter_;
        //outputDebugString(
        //    "%16d DMC Output bit counter decrement (%d)\n"
        //    , debugCycleCounter
        //    , thisObj->remainingBitsCounter_
        //);

        if(thisObj->remainingBitsCounter_ < 1) {
            //잔여 비트 카운터 리로드
            thisObj->remainingBitsCounter_ = 8;

            sample = *thisObj->buffer_;
            if(sample < 0) {
                thisObj->silenceFlag_ = true;

                //outputDebugString(
                //    "%16d DMC Output has reloaded an EMPTY sample.\n"
                //    , debugCycleCounter
                //);
            }
            else {
                thisObj->silenceFlag_ = false;

                //시프트 레지스터 리로드 후 버퍼를 비움
                thisObj->sr_ = sample & 0xFF;
                *thisObj->buffer_ = -1;

                //outputDebugString(
                //    "%16d DMC Output has reloaded a sample. (%d)\n"
                //    , debugCycleCounter
                //    , thisObj->sr_
                //);
            }
        }
    }

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_reset(
    struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
)
{
    thisObj->remainingBitsCounter_ = 8;
    thisObj->timer_ = rateTable[0];
    thisObj->sr_ = 0;
    thisObj->deltaCounter_ = 0;
    thisObj->silenceFlag_ = true;

    return kapheinErrorCodeNoError;
}
