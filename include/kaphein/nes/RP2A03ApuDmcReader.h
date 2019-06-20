#ifndef KAPHEIN_HGRD_kaphein_nes_RP2A03ApuDmcReader_h
#define KAPHEIN_HGRD_kaphein_nes_RP2A03ApuDmcReader_h

#include "kaphein/def.h"
#include "kaphein/ErrorCode.h"

struct kaphein_nes_RP2A03ApuDmcReader_VTable
{
    enum kaphein_ErrorCode
    (* setFlagParameter) (
        void * thisObj
        , kaphein_UInt8 * pParam
    );

    enum kaphein_ErrorCode
    (* setAddressParameter) (
        void * thisObj
        , kaphein_UInt8 * pParam
    );

    enum kaphein_ErrorCode
    (* setLengthParameter) (
        void * thisObj
        , kaphein_UInt8 * pParam
    );

    enum kaphein_ErrorCode
    (* setSampleBuffer) (
        void * thisObj
        , kaphein_Int16 * pBuffer
    );

    enum kaphein_ErrorCode
    (* reset) (
        void * thisObj
    );

    enum kaphein_ErrorCode
    (* hasRemainingBytesToRead) (
        void * thisObj
        , bool * truthOut
    );

    enum kaphein_ErrorCode
    (* isReadingEnabled) (
        void * thisObj
        , bool * truthOut
    );

    enum kaphein_ErrorCode
    (* setReadingEnabled) (
        void * thisObj
        , bool enabled
    );

    enum kaphein_ErrorCode
    (* isIrqEnabled) (
        void * thisObj
        , bool * truthOut
    );

    enum kaphein_ErrorCode
    (* setIrqEnabled) (
        void * thisObj
        , bool enabled
    );

    enum kaphein_ErrorCode
    (* hasIrqOccured) (
        void * thisObj
        , bool * truthOut
    );

    enum kaphein_ErrorCode
    (* clearIrqOccuredFlag) (
        void * thisObj
    );
};

struct kaphein_nes_RP2A03ApuDmcReader
{
    const struct kaphein_nes_RP2A03ApuDmcReader_VTable * vTable;
};

#endif
