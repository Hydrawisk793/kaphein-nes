#ifndef KAPHEIN_HGRD_kaphein_nes_cart_BF909x_h
#define KAPHEIN_HGRD_kaphein_nes_cart_BF909x_h

#include "UxROM.h"

enum kaphein_nes_cart_BF909x_Mode
{
    kaphein_nes_cart_BF909x_Mode_BF9093 = 0
    , kaphein_nes_cart_BF909x_Mode_BF9096
    , kaphein_nes_cart_BF909x_Mode_BF9097
};

/**
 *  @brief Cameria/Codemasters BF9093, BF9096 and BF9097
 */
struct kaphein_nes_cart_BF909x
{
    struct kaphein_nes_cart_UxROM parent;
    
    enum kaphein_nes_cart_BF909x_Mode bf909xMode;

    kaphein_UInt8 prgBankNumber;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_BF909x_construct(
    struct kaphein_nes_cart_BF909x * thisObj
    , struct kaphein_io_Stream * stream
    , const struct kaphein_nes_NesRomHeader * romHeader
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_BF909x_destruct(
    void * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_cart_BF909x_doCpuWrite(
    void * thisObj
    , kaphein_UInt16 address
    , kaphein_UInt8 value
);

#endif
