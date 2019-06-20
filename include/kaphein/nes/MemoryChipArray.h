#ifndef KAPHEIN_HGRD_kaphein_nes_MemoryChipArray_h
#define KAPHEIN_HGRD_kaphein_nes_MemoryChipArray_h

#include "def.h"

struct kaphein_nes_MemoryChipArray
{
    void * allocator;

    kaphein_UInt8 * trainer;

    kaphein_UInt8 * prgRom;

    kaphein_UInt8 * chrRom;

    kaphein_UInt8 * prgNvRam;

    kaphein_UInt8 * chrNvRam;

    kaphein_UInt8 * prgRam;

    kaphein_UInt8 * chrRam;

    kaphein_SSize trainerSize;

    kaphein_SSize prgRomSize;

    kaphein_SSize prgRamSize;

    kaphein_SSize prgNvRamSize;

    kaphein_SSize chrRomSize;

    kaphein_SSize chrRamSize;

    kaphein_SSize chrNvRamSize;
};

#endif
