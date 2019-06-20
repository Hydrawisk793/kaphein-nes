#ifndef KAPHEIN_HGRD_kaphein_nes_NesRomHeader_h
#define KAPHEIN_HGRD_kaphein_nes_NesRomHeader_h

#include "def.h"
#include "kaphein/ErrorCode.h"

enum kaphein_nes_ConsoleType
{
    kaphein_nes_ConsoleType_nes = 0
    , kaphein_nes_ConsoleType_vsSystem
    , kaphein_nes_ConsoleType_playChoice
    , kaphein_nes_ConsoleType_extendedType
};

enum kaphein_nes_VsSystemHardwareType
{
    kaphein_nes_VsSystemHardwareType_uniSystemNormal = 0
    , kaphein_nes_VsSystemHardwareType_uniSystemRbiBaseball
    , kaphein_nes_VsSystemHardwareType_uniSystemTkoBoxing
    , kaphein_nes_VsSystemHardwareType_uniSystemSuperXevious
    , kaphein_nes_VsSystemHardwareType_uniSystemIceClimberJapan
    , kaphein_nes_VsSystemHardwareType_dualSystem
    , kaphein_nes_VsSystemHardwareType_dualSystemRaidOnBungelingBay
};

enum kaphein_nes_VsSystemPpuType
{
    kaphein_nes_VsSystemPpuType_RP2C03B = 0
    , kaphein_nes_VsSystemPpuType_RP2C03G
    , kaphein_nes_VsSystemPpuType_RP2C04_0001
    , kaphein_nes_VsSystemPpuType_RP2C04_0002
    , kaphein_nes_VsSystemPpuType_RP2C04_0003
    , kaphein_nes_VsSystemPpuType_RP2C04_0004
    , kaphein_nes_VsSystemPpuType_RC2C03B
    , kaphein_nes_VsSystemPpuType_RC2C03C
    , kaphein_nes_VsSystemPpuType_RC2C05_01
    , kaphein_nes_VsSystemPpuType_RC2C05_02 
    , kaphein_nes_VsSystemPpuType_RC2C05_03
    , kaphein_nes_VsSystemPpuType_RC2C05_04
    , kaphein_nes_VsSystemPpuType_RC2C05_05
};

enum kaphein_nes_TimingMode
{
    kaphein_nes_TimingMode_ntsc = 0
    , kaphein_nes_TimingMode_pal
    , kaphein_nes_TimingMode_multipleRegion
    , kaphein_nes_TimingMode_dendy
};

enum kaphein_nes_ExtendedConsoleType
{
    kaphein_nes_ExtendedConsoleType_normal = 0
    , kaphein_nes_ExtendedConsoleType_vsSystem
    , kaphein_nes_ExtendedConsoleType_playChoice10
    , kaphein_nes_ExtendedConsoleType_cpuWithDecimalModeSupport
    , kaphein_nes_ExtendedConsoleType_VT01Monochrome
    , kaphein_nes_ExtendedConsoleType_VT01RedCyanStnPalette
    , kaphein_nes_ExtendedConsoleType_VT02
    , kaphein_nes_ExtendedConsoleType_VT03
    , kaphein_nes_ExtendedConsoleType_VT09
    , kaphein_nes_ExtendedConsoleType_VT32
    , kaphein_nes_ExtendedConsoleType_VT369
};

enum kaphein_nes_ExpansionDevice
{
    kaphein_nes_ExpansionDevice_none = 0
    , kaphein_nes_ExpansionDevice_standardJoypad
    , kaphein_nes_ExpansionDevice_fourScore
    , kaphein_nes_ExpansionDevice_fourPlayersAdapter
    , kaphein_nes_ExpansionDevice_vsSystem
    , kaphein_nes_ExpansionDevice_vsSystemWithReservedInputs
    , kaphein_nes_ExpansionDevice_vsPinball
    , kaphein_nes_ExpansionDevice_vsZapper
    , kaphein_nes_ExpansionDevice_zapper
    , kaphein_nes_ExpansionDevice_twoZappers
    , kaphein_nes_ExpansionDevice_hyperSlot
    , kaphein_nes_ExpansionDevice_powerPadSideA
    , kaphein_nes_ExpansionDevice_powerPadSideB
    , kaphein_nes_ExpansionDevice_familyTrainerA
    , kaphein_nes_ExpansionDevice_familyTrainerB
    , kaphein_nes_ExpansionDevice_arkanoidVausControllerForNes
    , kaphein_nes_ExpansionDevice_arkanoidVausControllerForFamicom
    , kaphein_nes_ExpansionDevice_barcodeBattler = 0x18
    , kaphein_nes_ExpansionDevice_robGyroSet = 0x1F
    , kaphein_nes_ExpansionDevice_familyBasicKeyboard = 0x23
    , kaphein_nes_ExpansionDevice_snesMouse = 0x29
    , kaphein_nes_ExpansionDevice_multicart
    , kaphein_nes_ExpansionDevice_twoSnesControllers
    , kaphein_nes_ExpansionDevice_uForce
    , kaphein_nes_ExpansionDevice_robStackUp
};

struct kaphein_nes_NesRomHeader
{
    union {
        char a[16];

        struct {
            char signature[4];

            unsigned char prgRomBankCountLowByte;

            unsigned char chrRomBankCountLowByte;

            unsigned char flags6;

            unsigned char flags7;
            
            unsigned char mapperMsbyteAndSubmapper;

            unsigned char romBankCountHighByte;

            unsigned char prgRamBankCount;
            
            unsigned char chrRamBankCount;

            unsigned char timingMode;
            
            unsigned char consoleType;
            
            unsigned char miscRomCount;

            unsigned char defaultExpansionDevice;
        } m;
    } u;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getVersion(
    const struct kaphein_nes_NesRomHeader * thisObj
    , int * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getMapper(
    const struct kaphein_nes_NesRomHeader * thisObj
    , int * mapperOut
    , int * subMapperOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getPrgRomBankCount(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * countOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getPrgRomSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getPrgRamSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getPrgNvRamSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getChrRomBankCount(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * countOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getChrRomSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getChrRamSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getChrNvRamSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_isVerticalMirroring(
    const struct kaphein_nes_NesRomHeader * thisObj
    , bool * truthOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_hasPrgNvRam(
    const struct kaphein_nes_NesRomHeader * thisObj
    , bool * truthOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_hasTrainer(
    const struct kaphein_nes_NesRomHeader * thisObj
    , bool * truthOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_hasFourScreens(
    const struct kaphein_nes_NesRomHeader * thisObj
    , bool * truthOut
);

#endif
