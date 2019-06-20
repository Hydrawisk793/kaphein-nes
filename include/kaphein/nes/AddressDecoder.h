#ifndef KAPHEIN_HGRD_kaphein_nes_AddressDecoder_h
#define KAPHEIN_HGRD_kaphein_nes_AddressDecoder_h

#include "def.h"
#include "kaphein/ErrorCode.h"

struct kaphein_nes_AddressDecoder_VTable
{
    enum kaphein_ErrorCode
    (* read) (
        void * thisObj
        , kaphein_UInt16 address
        , kaphein_UInt8 * valueOut
    );
    
    enum kaphein_ErrorCode
    (* write) (
        void * thisObj
        , kaphein_UInt16 address
        , kaphein_UInt8 value
    );
};

struct kaphein_nes_AddressDecoder
{
    const struct kaphein_nes_AddressDecoder_VTable * vTable;
};

#endif
