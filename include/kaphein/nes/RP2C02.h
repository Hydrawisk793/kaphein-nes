#ifndef KAPHEIN_HGRD_kaphein_nes_RP2C02_h
#define KAPHEIN_HGRD_kaphein_nes_RP2C02_h

#include "AddressDecoder.h"

enum
{
    SPR_COUNT = 64
    , SPR_PER_SCANLINE = 8
    , CYCLES_PER_SCANLINE = 341
    , VBLANK_SCANLINE_COUNT = 20
    , VISIBLE_SCANLINE_COUNT = 240
    , TOTAL_SCANLINE_COUNT = VISIBLE_SCANLINE_COUNT + 1 + VBLANK_SCANLINE_COUNT + 1
    , kaphein_nes_RP2C02_PALETTE_COUNT = 64
    , kaphein_nes_RP2C02_cyclesPerFrame = TOTAL_SCANLINE_COUNT*CYCLES_PER_SCANLINE
};

enum HEController
{
    HEController_NAMETABLE_W = 0x01
    , HEController_NAMETABLE_H = 0x02
    , HEController_NAMETABLE_WH = HEController_NAMETABLE_W
        | HEController_NAMETABLE_H
    , HEController_ADDR_INCREMENT = 0x04
    , HEController_SPR_PATTERN = 0x08
    , HEController_BG_PATTERN = 0x10
    , HEController_SPR_SIZE = 0x20
    , HEController_EXTBUS_DIR = 0x40
    , HEController_ENABLE_VBLANK = 0x80
};

enum HEMask
{
    HEMask_B = 0x80
    , HEMask_G = 0x40
    , HEMask_R = 0x20
    , HEMask_DISPLAY = 0x18
    , HEMask_DISPLAY_SPR = 0x10
    , HEMask_DISPLAY_BG = 0x08
    , HEMask_DISABLE_CLIP = 0x06
    , HEMask_DISABLE_CLIP_SPR = 0x04
    , HEMask_DISABLE_CLIP_BG = 0x02
    , HEMask_GRAYSCALE = 0x01
};

enum HEStatus
{
    HEStatus_STATUS_BITS = 0xE0
    , HEStatus_VBLANK_OCCURED = 0x80
    , HEStatus_SPR0_DETECTED = 0x40
    , HEStatus_SPR_OVERFLOW = 0x20
    , HEStatus_UNUSED = 0x1F
};

enum HESprFlag
{
    HESprFlag_ATTRIBUTE_L = 0x01
    , HESprFlag_ATTRIBUTE_H = 0x02
    , HESprFlag_ATTRIBUTE = HESprFlag_ATTRIBUTE_L | HESprFlag_ATTRIBUTE_H
    , HESprFlag_PRIORITY = 0x20
    , HESprFlag_FLIP_H = 0x40
    , HESprFlag_FLIP_Y = 0x80
    , HESprFlag_SPR_FLAG_VALUES = 0xE3
};

struct kaphein_nes_RP2C02
{
    void * impl_;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_construct(
    struct kaphein_nes_RP2C02 * thisObj
    , void * allocator
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_destruct(
    struct kaphein_nes_RP2C02 * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_setOutputDevice(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt32 (surface [256*240])
    , kaphein_UInt32 (rgbTable [64])
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_setBuses(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt16 * address
    , kaphein_UInt8 * data
    , enum kaphein_nes_InterruptSignal * nmiBus
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_setAddressDecoder(
    struct kaphein_nes_RP2C02 * thisObj
    , struct kaphein_nes_AddressDecoder * decoder
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_powerUp(
    struct kaphein_nes_RP2C02 * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_reset(
    struct kaphein_nes_RP2C02 * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_run(
    struct kaphein_nes_RP2C02 * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_isInOddFrame(
    struct kaphein_nes_RP2C02 * thisObj
    , bool * truthOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_isRendering(
    struct kaphein_nes_RP2C02 * thisObj
    , bool * truthOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_readRegister(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt8 address
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2C02_writeRegister(
    struct kaphein_nes_RP2C02 * thisObj
    , kaphein_UInt8 address
    , kaphein_UInt8 value
);

#endif
