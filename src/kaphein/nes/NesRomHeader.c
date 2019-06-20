#include "kaphein/nes/NesRomHeader.h"

enum
{
    prgRomBankSize = 16384
    , chrRomBankSize = 8192
};

/*
enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_unserialize(
    struct kaphein_nes_NesRomHeader * thisObj
    , const char * bytes
    , kaphein_SSize byteCount
)
{
	static const char firstString[] = {'N', 'E', 'S', 0x1A};
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    int headerVersion;
    int state = 0;
    int i;
    bool isValid;
    bool isLooping;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == bytes
    ) {
       resultErrorCode = kapheinErrorCodeArgumentNull; 
    }
    else if(byteCount < 16) {
       resultErrorCode = kapheinErrorCodeArgumentOutOfRange;
    }
    else {
        for(isLooping = true, isValid = true; isLooping && isValid; ) {
            switch(state) {
            case 0:
	            for(i = (int)sizeof(firstString); isValid && i > 0; ) {
                    --i;

                    isValid = bytes[i] == firstString[i];
                }

                if(isValid) {
                    ++state;
                }
            break;
            case 1:
                thisObj->prgRomBankCount = bytes[4];
                thisObj->chrRomBankCount = bytes[5];
                
                thisObj->isVerticalMirroring = !!(bytes[6] & 0x01);
                thisObj->hasBatteryRam = !!(bytes[6] & 0x02);
                thisObj->hasTrainer = !!(bytes[6] & 0x04);
                thisObj->hasFourScreen = !!(bytes[6] & 0x08);
                
                thisObj->mapper = ((bytes[6] & 0xF0) >> 4) | (bytes[7] & 0xF0);

                thisObj->consoleType = (enum kaphein_nes_ConsoleType)(bytes[7] & 0x03);
                
                headerVersion = (bytes[7] & 0x0C) >> 2;

                switch(headerVersion) {
                //Ver 1.0
                case 0:
                    thisObj->subMapper = 0;

                    thisObj->prgRomBankCount = bytes[4];
                    thisObj->chrRomBankCount = bytes[5];

                    //TODO : 맵퍼별로 특수 처리
                    {
                        thisObj->nvPrgRamSegmentCount = 0;
                        thisObj->prgRamSegmentCount = 0;
                        thisObj->nvChrRamSegmentCount = 0;
                        thisObj->chrRamSegmentCount = 0;

                        thisObj->timingMode = kaphein_nes_TimingMode_ntsc;

                        thisObj->vsSystemHardwareType = (enum kaphein_nes_VsSystemHardwareType)0;
                        thisObj->vsSystemPpuType = (enum kaphein_nes_VsSystemPpuType)0;
                    
                        thisObj->extendedConsoleType = (enum kaphein_nes_ExtendedConsoleType)0;

                        thisObj->miscRomCount = 0;

                        thisObj->defaultExpansionDevice = kaphein_nes_ExpansionDevice_none;
                    }
                break;
                //Ver 2.0
                case 2:
                    thisObj->mapper |= ((bytes[8] & 0x0F) << 8);
                    thisObj->subMapper = ((bytes[8] & 0xF0) >> 4);

                    thisObj->prgRomBankCount |= ((bytes[9] & 0x0F) << 8);
                    thisObj->chrRomBankCount |= ((bytes[9] & 0xF0) << 4);

                    thisObj->nvPrgRamSegmentCount = (bytes[10] & 0xF0) >> 4;
                    thisObj->prgRamSegmentCount = (bytes[10] & 0x0F);
                    thisObj->nvChrRamSegmentCount = (bytes[11] & 0xF0) >> 4;
                    thisObj->chrRamSegmentCount = (bytes[11] & 0x0F);

                    thisObj->timingMode = (enum kaphein_nes_TimingMode)(bytes[12] & 0x03);

                    switch(thisObj->consoleType) {
                    case kaphein_nes_ConsoleType_playChoice:
                        thisObj->vsSystemHardwareType = (enum kaphein_nes_VsSystemHardwareType)((bytes[13] & 0xF0) >> 4);
                        thisObj->vsSystemPpuType = (enum kaphein_nes_VsSystemPpuType)(bytes[13] & 0x0F);

                        thisObj->extendedConsoleType = (enum kaphein_nes_ExtendedConsoleType)0;
                    break;
                    case kaphein_nes_ConsoleType_extendedType:
                        thisObj->vsSystemHardwareType = (enum kaphein_nes_VsSystemHardwareType)0;
                        thisObj->vsSystemPpuType = (enum kaphein_nes_VsSystemPpuType)0;

                        thisObj->extendedConsoleType = (enum kaphein_nes_ExtendedConsoleType)(bytes[13] & 0x0F);
                    break;
                    }

                    thisObj->miscRomCount = (bytes[14] & 0x03);

                    thisObj->defaultExpansionDevice = (enum kaphein_nes_ExpansionDevice)(bytes[15] & 0x3F);
                break;
                default:
                    isValid = false;
                break;
                }

                if(isValid) {
                    ++state;
                }
            break;
            case 2:
                isValid = 
                    (headerVersion == 0 || headerVersion == 2)
                    && thisObj->consoleType != kaphein_nes_ConsoleType_extendedType
                ;
                
                //TODO : Do more validation checks.

                if(isValid) {
                    isLooping = false;
                }
            break;
            }
        }

        if(!isValid) {
            resultErrorCode = kapheinErrorCodeArgumentFormatInvalid;
        }
    }

    return resultErrorCode;
}
*/

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getVersion(
    const struct kaphein_nes_NesRomHeader * thisObj
    , int * valueOut
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
        *valueOut = (thisObj->u.a[7] & 0x0C) >> 2;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getMapper(
    const struct kaphein_nes_NesRomHeader * thisObj
    , int * mapperOut
    , int * subMapperOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == mapperOut
    ) {
       resultErrorCode = kapheinErrorCodeArgumentNull; 
    }
    else {
        int version;
        int mapper;
        int subMapper = 0;

        mapper = ((thisObj->u.a[6] & 0xF0) >> 4) | (thisObj->u.a[7] & 0xF0);;

        kaphein_nes_NesRomHeader_getVersion(thisObj, &version);
        if(version == 2) {
            mapper |= ((thisObj->u.a[8] & 0x0F) << 8);
            subMapper = ((thisObj->u.a[8] & 0xF0) >> 4);
        }

        *mapperOut = mapper;
        if(KAPHEIN_NULL != subMapperOut) {
            *subMapperOut = subMapper;
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getPrgRomBankCount(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * countOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == countOut
    ) {
       resultErrorCode = kapheinErrorCodeArgumentNull; 
    }
    else {
        kaphein_SSize bankCount;
        int version;
        
        bankCount = thisObj->u.a[4];

        kaphein_nes_NesRomHeader_getVersion(thisObj, &version);
        switch(version) {
        case 0:
        break;
        case 2:
            bankCount |= ((thisObj->u.a[9] & 0x0F) << 8);
        break;
        default:
            resultErrorCode = kapheinErrorCodeArgumentFormatInvalid;; 
        }

        *countOut = bankCount;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getPrgRomSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;;
    kaphein_SSize bankCount;

    if(KAPHEIN_NULL == sizeOut) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        resultErrorCode = kaphein_nes_NesRomHeader_getPrgRomBankCount(thisObj, &bankCount);
        if(kapheinErrorCodeNoError == resultErrorCode) {
            *sizeOut = bankCount * prgRomBankSize;
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getPrgRamSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == sizeOut
    ) {
       resultErrorCode = kapheinErrorCodeArgumentNull; 
    }
    else {
        int shiftCount;
        int version;
        
        shiftCount = thisObj->u.a[4];

        kaphein_nes_NesRomHeader_getVersion(thisObj, &version);
        switch(version) {
        case 0:
        break;
        case 2:
            shiftCount |= (thisObj->u.a[10] & 0x0F);
        break;
        default:
            resultErrorCode = kapheinErrorCodeArgumentFormatInvalid;; 
        }

        *sizeOut = (shiftCount > 0 ? (64 << shiftCount) : 0);
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getPrgNvRamSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == sizeOut
    ) {
       resultErrorCode = kapheinErrorCodeArgumentNull; 
    }
    else {
        int shiftCount;
        int version;
        
        shiftCount = thisObj->u.a[4];

        kaphein_nes_NesRomHeader_getVersion(thisObj, &version);
        switch(version) {
        case 0:
        break;
        case 2:
            shiftCount |= ((thisObj->u.a[10] & 0xF0) >> 4);
        break;
        default:
            resultErrorCode = kapheinErrorCodeArgumentFormatInvalid;; 
        }

        *sizeOut = (shiftCount > 0 ? (64 << shiftCount) : 0);
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getChrRomBankCount(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * countOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    int version;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == countOut
    ) {
       resultErrorCode = kapheinErrorCodeArgumentNull; 
    }
    else {
        kaphein_SSize bankCount;
        
        bankCount = thisObj->u.a[5];

        kaphein_nes_NesRomHeader_getVersion(thisObj, &version);
        switch(version) {
        case 0:
        break;
        case 2:
            bankCount |= ((thisObj->u.a[9] & 0xF0) << 4);
        break;
        default:
            resultErrorCode = kapheinErrorCodeArgumentFormatInvalid;; 
        }

        *countOut = bankCount;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getChrRomSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;;
    kaphein_SSize bankCount;

    if(KAPHEIN_NULL == sizeOut) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        resultErrorCode = kaphein_nes_NesRomHeader_getChrRomBankCount(thisObj, &bankCount);
        if(kapheinErrorCodeNoError == resultErrorCode) {
            *sizeOut = bankCount * chrRomBankSize;
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getChrRamSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == sizeOut
    ) {
       resultErrorCode = kapheinErrorCodeArgumentNull; 
    }
    else {
        int shiftCount;
        int version;
        
        shiftCount = thisObj->u.a[4];

        kaphein_nes_NesRomHeader_getVersion(thisObj, &version);
        switch(version) {
        case 0:
        break;
        case 2:
            shiftCount |= (thisObj->u.a[11] & 0x0F);
        break;
        default:
            resultErrorCode = kapheinErrorCodeArgumentFormatInvalid;; 
        }

        *sizeOut = (shiftCount > 0 ? (64 << shiftCount) : 0);
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_getChrNvRamSize(
    const struct kaphein_nes_NesRomHeader * thisObj
    , kaphein_SSize * sizeOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == sizeOut
    ) {
       resultErrorCode = kapheinErrorCodeArgumentNull; 
    }
    else {
        int shiftCount;
        int version;
        
        shiftCount = thisObj->u.a[4];

        kaphein_nes_NesRomHeader_getVersion(thisObj, &version);
        switch(version) {
        case 0:
        break;
        case 2:
            shiftCount |= ((thisObj->u.a[11] & 0xF0) >> 4);
        break;
        default:
            resultErrorCode = kapheinErrorCodeArgumentFormatInvalid;; 
        }

        *sizeOut = (shiftCount > 0 ? (64 << shiftCount) : 0);
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_isVerticalMirroring(
    const struct kaphein_nes_NesRomHeader * thisObj
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
        *truthOut = !!(thisObj->u.a[6] & 0x01);
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_hasPrgNvRam(
    const struct kaphein_nes_NesRomHeader * thisObj
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
        *truthOut = !!(thisObj->u.a[6] & 0x02);
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_hasTrainer(
    const struct kaphein_nes_NesRomHeader * thisObj
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
        *truthOut = !!(thisObj->u.a[6] & 0x04);
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_NesRomHeader_hasFourScreens(
    const struct kaphein_nes_NesRomHeader * thisObj
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
        *truthOut = !!(thisObj->u.a[6] & 0x08);
    }

    return resultErrorCode;
}
