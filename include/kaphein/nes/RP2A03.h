#ifndef KAPHEIN_HGRD_kaphein_nes_RP2A03_h
#define KAPHEIN_HGRD_kaphein_nes_RP2A03_h

#include "kaphein/ErrorCode.h"
#include "def.h"
#include "RP2A03Apu.h"
#include "RP2A03ApuDmcReader.h"

//#define IMPLEMENT_PIPELINING

struct kaphein_nes_RP2A03
{
    struct kaphein_nes_RP2A03ApuDmcReader parent;
    
    void * impl_;
};

struct kaphein_nes_Mos6502MainRegisterSet
{
    union
    {
        kaphein_UInt8 a[8];

        struct
        {
            //A ��������(�����)
            kaphein_UInt8 regA;

            //����
            kaphein_UInt8 regDummy;

            //X ��������(�ε���)
            kaphein_UInt8 regX;

            //Y ��������(�ε���)
            kaphein_UInt8 regY;

            //P ��������(����)
            kaphein_UInt8 regP;

            //S ��������(���� ������)
            kaphein_UInt8 regS;

            //PC ���� ����Ʈ
            kaphein_UInt8 pcH;

            //PC ���� ����Ʈ
            kaphein_UInt8 pcL;
        } m;
    } u;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03_construct(
    struct kaphein_nes_RP2A03 * thisObj
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03_destruct(
    struct kaphein_nes_RP2A03 * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03_setInterruptBuses(
    struct kaphein_nes_RP2A03 * thisObj
    , enum kaphein_nes_InterruptSignal * nmiBus
    , enum kaphein_nes_InterruptSignal * irqBus
    , enum kaphein_nes_InterruptSignal * resetBus
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03_setIoBuses(
    struct kaphein_nes_RP2A03 * thisObj
    , kaphein_UInt16 * addrBus
    , kaphein_UInt8 * dataBus
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03_setPeripheralBuses(
    struct kaphein_nes_RP2A03 * thisObj
    , kaphein_UInt8 * controllerBus
    , kaphein_UInt16 * p1CtrlBus
    , kaphein_UInt16 * p2CtrlBus
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03_setApu(
    struct kaphein_nes_RP2A03 * thisObj
    , struct kaphein_nes_RP2A03Apu * apu
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03_setAddressDecoder(
    struct kaphein_nes_RP2A03 * thisObj
    , struct kaphein_nes_AddressDecoder * decoder
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03_powerUp(
    struct kaphein_nes_RP2A03 * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03_reset(
    struct kaphein_nes_RP2A03 * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03_run(
    struct kaphein_nes_RP2A03 * thisObj
    , int cycleCount
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03_dumpMainRegisters(
    const struct kaphein_nes_RP2A03 * thisObj
    , struct kaphein_nes_Mos6502MainRegisterSet * resultOut
);

#endif
