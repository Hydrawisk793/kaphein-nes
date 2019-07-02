#include <assert.h>

#include "kaphein/mem/utils.h"
#include "kaphein/nes/AddressDecoder.h"
#include "kaphein/nes/RP2A03.h"
#include "kaphein/nes/ShiftRegister16.h"
#include "kaphein/nes/debug.h"

/* **************************************************************** */
/* Internal declarations */

enum RP2A03_Status
{
    RP2A03_Status_P_CARRY = 0x01
    , RP2A03_Status_P_ZERO = 0x02
    , RP2A03_Status_P_DISABLE_IRQ = 0x04
    , RP2A03_Status_P_DECIMAL = 0x08
    , RP2A03_Status_P_BRK = 0x10
    , RP2A03_Status_P_UNUSED = 0x20
    , RP2A03_Status_P_UB = RP2A03_Status_P_BRK | RP2A03_Status_P_UNUSED
    , RP2A03_Status_P_OVERFLOW = 0x40
    , RP2A03_Status_P_NEGATIVE = 0x80
};

enum RP2A03_CoreSeq
{
    RP2A03_CoreSeq_onResetEntry0 = 0
    , RP2A03_CoreSeq_onResetEntry1
    , RP2A03_CoreSeq_onResetEntry2
    , RP2A03_CoreSeq_onIntReset0
    , RP2A03_CoreSeq_onIntReset1
    , RP2A03_CoreSeq_onIntReset2
    , RP2A03_CoreSeq_onIntReset3
    , RP2A03_CoreSeq_onIntReset4
};

enum RP2A03_IoMode
{
    RP2A03_IoMode_write = 0
    , RP2A03_IoMode_read = 1
};

enum HEInst
{
    //Read
    HEInst_LDA, HEInst_LDX, HEInst_LDY
    , HEInst_LAX, HEInst_XAA, HEInst_NOP

    //Read-Modify
    , HEInst_CMP, HEInst_CPX, HEInst_CPY, HEInst_BIT
    , HEInst_ORA, HEInst_AND, HEInst_EOR
    , HEInst_ADC, HEInst_SBC
    , HEInst_ANC, HEInst_AXS, HEInst_ATX
    , HEInst_ASR, HEInst_ARR, HEInst_LAR
    
    //Read-Modify-Write
    , HEInst_ASL, HEInst_LSR, HEInst_ROL, HEInst_ROR
    , HEInst_INC, HEInst_DEC
    , HEInst_SLO, HEInst_SRE, HEInst_RLA, HEInst_RRA
    , HEInst_ISC, HEInst_DCP

    //Write
    , HEInst_STA, HEInst_STX, HEInst_STY
    , HEInst_SXA, HEInst_SYA, HEInst_XAS
    , HEInst_AXA, HEInst_AAX

    //IMP
    , HEInst_TXA, HEInst_TAX, HEInst_TXS, HEInst_TSX, HEInst_TYA, HEInst_TAY
    , HEInst_CLC, HEInst_SEC, HEInst_CLI, HEInst_SEI, HEInst_CLV, HEInst_CLD, HEInst_SED
    , HEInst_DEY, HEInst_DEX, HEInst_INY, HEInst_INX, HEInst_KIL

    //REL
    , HEInst_BPL, HEInst_BMI, HEInst_BVC, HEInst_BVS
    , HEInst_BNE, HEInst_BEQ, HEInst_BCC, HEInst_BCS

    //IND
    , HEInst_JMP

    //Stack Modifier
    , HEInst_JSR, HEInst_BRK, HEInst_PHP, HEInst_PHA
    , HEInst_RTI, HEInst_RTS, HEInst_PLP, HEInst_PLA
};

enum HEInstType
{
    HEInstType_READ = 0
    , HEInstType_READ_MODIFY
    , HEInstType_READ_MODIFY_WRITE
    , HEInstType_WRITE
    , HEInstType_ETC
};

enum HEAddrMode
{
    HEAddrMode_IMP = 0
    , HEAddrMode_IMM
    , HEAddrMode_ZPG
    , HEAddrMode_ZPX
    , HEAddrMode_ZPY
    , HEAddrMode_ABS
    , HEAddrMode_ABX
    , HEAddrMode_ABY
    , HEAddrMode_IND
    , HEAddrMode_IZX
    , HEAddrMode_IZY
    , HEAddrMode_REL
    , HEAddrMode_IMPBRK
    , HEAddrMode_IMPSTK
    , HEAddrMode_ABSJMP
    , HEAddrMode_IMPSFT
};

enum HEInterrupt
{
    HEInterrupt_NONE = 0
    , HEInterrupt_RESET = 1
    , HEInterrupt_NMI = 2
    , HEInterrupt_IRQ = 3
};

enum RP2A03_InstIoSeq
{
    RP2A03_InstIoSeq_readFromPc
    , RP2A03_InstIoSeq_readFromAddressVar
    , RP2A03_InstIoSeq_readFromPointerVar
    , RP2A03_InstIoSeq_readFromStack
    , RP2A03_InstIoSeq_writeToStack
};

enum RP2A03_InstSeq
{
    RP2A03_InstSeq_decodeOpcode
    , RP2A03_InstSeq_imp
    , RP2A03_InstSeq_imm
    , RP2A03_InstSeq_zpg
    , RP2A03_InstSeq_zpx0
    , RP2A03_InstSeq_zpx1
    , RP2A03_InstSeq_zpy0
    , RP2A03_InstSeq_zpy1
    , RP2A03_InstSeq_abs0
    , RP2A03_InstSeq_abs1
    , RP2A03_InstSeq_abx0
    , RP2A03_InstSeq_abx1
    , RP2A03_InstSeq_aby0
    , RP2A03_InstSeq_aby1
    , RP2A03_InstSeq_ind0
    , RP2A03_InstSeq_ind1
    , RP2A03_InstSeq_izx0
    , RP2A03_InstSeq_izx1
    , RP2A03_InstSeq_izx2
    , RP2A03_InstSeq_izx3
    , RP2A03_InstSeq_izy0
    , RP2A03_InstSeq_izy1
    , RP2A03_InstSeq_izy2
    , RP2A03_InstSeq_rel0
    , RP2A03_InstSeq_rel1
    , RP2A03_InstSeq_rel2
    , RP2A03_InstSeq_impBrk
    , RP2A03_InstSeq_impStk
    , RP2A03_InstSeq_absJmp
    , RP2A03_InstSeq_impSft
    , RP2A03_InstSeq_fetchOperandWithCarry
    , RP2A03_InstSeq_fetchOperandWithNoCarry
    , RP2A03_InstSeq_rmwInstFetchOperand
    , RP2A03_InstSeq_writeInstWriteToAddressVar
    , RP2A03_InstSeq_rmwInstWriteOriginalValue
    , RP2A03_InstSeq_rmwInstWriteNewValue
    , RP2A03_InstSeq_jmpInd0
    , RP2A03_InstSeq_jmpInd1
    , RP2A03_InstSeq_jmpAbs0
    , RP2A03_InstSeq_jsr0
    , RP2A03_InstSeq_jsr1
    , RP2A03_InstSeq_jsr2
    , RP2A03_InstSeq_jsr3
    , RP2A03_InstSeq_brk1
    , RP2A03_InstSeq_brk2
    , RP2A03_InstSeq_brk3
    , RP2A03_InstSeq_brk4
    , RP2A03_InstSeq_brk5
    , RP2A03_InstSeq_brk6
    , RP2A03_InstSeq_php
    , RP2A03_InstSeq_pha
    , RP2A03_InstSeq_rti0
    , RP2A03_InstSeq_rti1
    , RP2A03_InstSeq_rti2
    , RP2A03_InstSeq_rti3
    , RP2A03_InstSeq_rts0
    , RP2A03_InstSeq_rts1
    , RP2A03_InstSeq_rts2
    , RP2A03_InstSeq_rts3
    , RP2A03_InstSeq_plp0
    , RP2A03_InstSeq_plp1
    , RP2A03_InstSeq_pla0
    , RP2A03_InstSeq_pla1
    , RP2A03_InstSeq_reset0
    , RP2A03_InstSeq_reset1
    , RP2A03_InstSeq_reset2
    , RP2A03_InstSeq_reset3
    , RP2A03_InstSeq_reset4
};

struct kaphein_nes_RP2A03_Impl;

typedef void (RP2A03_SequenceFunction) (
    struct kaphein_nes_RP2A03_Impl * impl
);

struct kaphein_nes_RP2A03_Impl
{
    void * allocator;
    
    struct kaphein_nes_RP2A03Apu * apu;

    struct kaphein_nes_AddressDecoder * decoder;
    
    struct
    {
        //(1, I) RESET Signal
	    enum kaphein_nes_InterruptSignal * reset;
        
        //(1, I) NMI Signal
        enum kaphein_nes_InterruptSignal * nmi;

        //(1, I) IRQ Signal
	    enum kaphein_nes_InterruptSignal * irq;

        //(1, O) ShiftRegister16, 4016R, P1 Button Input
	    kaphein_UInt16 * p1Ctrl;

        //(1, O) ShiftRegister16, 4017R, P2 Button Input
	    kaphein_UInt16 * p2Ctrl;

        //(16, O) Address bus
	    kaphein_UInt16 * ioAddress;

        //(8, I/O) Data bus
	    kaphein_UInt8 * ioData;

        //(3, O) 4016W, Controller
	    kaphein_UInt8 * controller;
    } bus;

    struct
    {
        struct
        {
            kaphein_UInt16 pc;

            kaphein_UInt8 a;

            kaphein_UInt8 dummy;

            kaphein_UInt8 x;

            kaphein_UInt8 y;

            kaphein_UInt8 p;

            kaphein_UInt8 s;
        } reg;

        //타겟 주소
	    kaphein_UInt16 ioAddress;

        //하위 주소 변수
	    kaphein_UInt8 addressVarL;
        
        //상위 주소 변수
	    kaphein_UInt8 addressVarH;

        //하위 포인터 변수
	    kaphein_UInt8 pointerVarL;
        
        //상위 포인터 변수
	    kaphein_UInt8 pointerVarH;

        //I/O 데이터 임시 변수
	    kaphein_UInt8 ioData;
        
        //결과 오퍼랜드
	    kaphein_UInt8 lOperand;

        //읽어들인 오퍼랜드
	    kaphein_UInt8 rOperand;

        //캐리 플래그
	    kaphein_UInt8 carry;

        //오버플로우 플래그
	    kaphein_UInt8 overflow;

        //디버그용 현재 실행중인 Opcode
	    kaphein_UInt8 debugOpcode;

        //명령어
	    enum HEInst inst;

        //명령어 종류
	    enum HEInstType instType;

        //어드레싱 모드
	    enum HEAddrMode addrMode;

        enum HEInterrupt occuredInterrupt;
        
        enum RP2A03_IoMode ioMode;

        enum RP2A03_InstSeq instSeq;
        
        //디버그용 사이클 카운터
        int debugCycleCounter;

        bool isInOddCycle;

        bool isHalted;
    } core;

    struct
    {
        struct {
            kaphein_Int16 * buffer;

            kaphein_UInt8 * flagParam;

            kaphein_UInt8 * addrParam;

            kaphein_UInt8 * lenParam;

            int dummyCycleCounter;

            kaphein_UInt16 address;

            kaphein_UInt16 remainingByteCount;

            bool hasIrqOccured;

            bool isReading;

            bool isEnabled;
        } dmc;

        struct 
        {
            kaphein_UInt16 counter;

            kaphein_UInt8 addrH;

            bool isReading;

            bool isEnabled;
        } spr;

        kaphein_UInt8 flag;

        kaphein_UInt8 ioData;

        RP2A03_SequenceFunction * seq;
    } dma;

    kaphein_UInt8 regStrobe;
};

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setFlagParameter(
    void * thisObj
    , kaphein_UInt8 * pParam
);

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setAddressParameter(
    void * thisObj
    , kaphein_UInt8 * pParam
);

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setLengthParameter(
    void * thisObj
    , kaphein_UInt8 * pParam
);

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setSampleBuffer(
    void * thisObj
    , kaphein_Int16 * pBuffer
);

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_reset(
    void * thisObj
);

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_hasRemainingBytesToRead(
    void * thisObj
    , bool * truthOut
);

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_isReadingEnabled(
    void * thisObj
    , bool * truthOut
);

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setReadingEnabled(
    void * thisObj
    , bool enabled
);

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_isIrqEnabled(
    void * thisObj
    , bool * truthOut
);

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setIrqEnabled(
    void * thisObj
    , bool enabled
);

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_hasIrqOccured(
    void * thisObj
    , bool * truthOut
);

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_clearIrqOccuredFlag(
    void * thisObj
);

static
void
RP2A03_powerUp(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_reset(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_triggerDmcDmaIfNecessary(
    struct kaphein_nes_RP2A03_Impl * impl
    , int delayCycleCount
);

static
void
RP2A03_run(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
kaphein_UInt8
RP2A03_read(
    struct kaphein_nes_RP2A03_Impl * impl
    , const kaphein_UInt16 address
);

static
void
RP2A03_write(
    struct kaphein_nes_RP2A03_Impl * impl
    , kaphein_UInt16 address
    , kaphein_UInt8 data
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
kaphein_UInt8
RP2A03_readRegister(
    struct kaphein_nes_RP2A03_Impl * impl
    , kaphein_UInt8 address
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void
RP2A03_writeRegister(
    struct kaphein_nes_RP2A03_Impl * impl
    , kaphein_UInt8 address
    , kaphein_UInt8 data
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void
RP2A03_doLastInstructionCycle(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_pollInterrupts(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_updateStatusFlagN(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_updateStatusFlagV(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_updateStatusFlagZ(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_updateStatusFlagC(
    struct kaphein_nes_RP2A03_Impl * impl
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void
RP2A03_updateStatusFlagNZ(
    struct kaphein_nes_RP2A03_Impl * impl
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void
RP2A03_updateStatusFlagCV(
    struct kaphein_nes_RP2A03_Impl * impl
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void
RP2A03_updateStatusFlagCNZ(
    struct kaphein_nes_RP2A03_Impl * impl
);

KAPHEIN_ATTRIBUTE_FORCE_INLINE
static
void
RP2A03_updateStatusFlagCVNZ(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_testBranchCondition(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_executeInstruction(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_dmc_setReadingEnabled(
    struct kaphein_nes_RP2A03_Impl * impl
    , bool enabled
);

static
void
RP2A03_alu_add(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_alu_sub(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
kaphein_UInt8
RP2A03_rwSeq_read(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_rwSeq_write(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_dmaSeq_dmcHaltCore(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_dmaSeq_sprHaltCore(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_dmaSeq_alignment0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_dmaSeq_alignment1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_dmaSeq_alignment2(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_dmaSeq_read(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_dmaSeq_write(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_dmaSeq_readDmcSample(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_doesNothing(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_readFromPc(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_readFromAddressVar(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_readFromPointerVar(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_readFromStack(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_rmwInstFetchOperand(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_rmwInstWriteOriginalValue(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_rmwInstWriteNewValue(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_writeInstWriteToAddressVar(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_writeToAddressVar(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_jsr1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_jsr2(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_brk2(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_brk3(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_brk4(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_brk5(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_brk6(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_php(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instIoSeq_pha(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_decodeOpcode(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_imp(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_imm(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_zpg(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_zpx0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_zpx1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_zpy0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_zpy1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_abs0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_abs1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_abx0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_abx1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_aby0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_aby1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_ind0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_ind1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_izx0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_izx1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_izx2(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_izx3(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_izy0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_izy1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_izy2(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rel0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rel1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rel2(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_impBrk(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_impStk(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_absJmp(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_impSft(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_fetchOperandWithCarry(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_fetchOperandWithNoCarry(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rmwInstFetchOperand(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_writeToAddressVar(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rmwInstWriteOriginalValue(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rmwInstWriteNewValue(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_jmpInd0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_jmpInd1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_jmpAbs(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_jsr0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_jsr1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_jsr2(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_jsr3(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_brk1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_brk2(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_brk3(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_brk4(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_brk5(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_brk6(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_php(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_pha(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rti0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rti1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rti2(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rti3(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rts0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rts1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rts2(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_rts3(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_plp0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_plp1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_pla0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_pla1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_reset0(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_reset1(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_reset2(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_reset3(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
void
RP2A03_instBodySeq_reset4(
    struct kaphein_nes_RP2A03_Impl * impl
);

static
const enum HEInst
RP2A03_instTable[] = {
	HEInst_BRK, HEInst_ORA, HEInst_KIL, HEInst_SLO, HEInst_NOP, HEInst_ORA, HEInst_ASL, HEInst_SLO, HEInst_PHP, HEInst_ORA, HEInst_ASL, HEInst_ANC, HEInst_NOP, HEInst_ORA, HEInst_ASL, HEInst_SLO,
	HEInst_BPL, HEInst_ORA, HEInst_KIL, HEInst_SLO, HEInst_NOP, HEInst_ORA, HEInst_ASL, HEInst_SLO, HEInst_CLC, HEInst_ORA, HEInst_NOP, HEInst_SLO, HEInst_NOP, HEInst_ORA, HEInst_ASL, HEInst_SLO,
	HEInst_JSR, HEInst_AND, HEInst_KIL, HEInst_RLA, HEInst_BIT, HEInst_AND, HEInst_ROL, HEInst_RLA, HEInst_PLP, HEInst_AND, HEInst_ROL, HEInst_ANC, HEInst_BIT, HEInst_AND, HEInst_ROL, HEInst_RLA,
	HEInst_BMI, HEInst_AND, HEInst_KIL, HEInst_RLA, HEInst_NOP, HEInst_AND, HEInst_ROL, HEInst_RLA, HEInst_SEC, HEInst_AND, HEInst_NOP, HEInst_RLA, HEInst_NOP, HEInst_AND, HEInst_ROL, HEInst_RLA,

	HEInst_RTI, HEInst_EOR, HEInst_KIL, HEInst_SRE, HEInst_NOP, HEInst_EOR, HEInst_LSR, HEInst_SRE, HEInst_PHA, HEInst_EOR, HEInst_LSR, HEInst_ASR, HEInst_JMP, HEInst_EOR, HEInst_LSR, HEInst_SRE,
	HEInst_BVC, HEInst_EOR, HEInst_KIL, HEInst_SRE, HEInst_NOP, HEInst_EOR, HEInst_LSR, HEInst_SRE, HEInst_CLI, HEInst_EOR, HEInst_NOP, HEInst_SRE, HEInst_NOP, HEInst_EOR, HEInst_LSR, HEInst_SRE,
	HEInst_RTS, HEInst_ADC, HEInst_KIL, HEInst_RRA, HEInst_NOP, HEInst_ADC, HEInst_ROR, HEInst_RRA, HEInst_PLA, HEInst_ADC, HEInst_ROR, HEInst_ARR, HEInst_JMP, HEInst_ADC, HEInst_ROR, HEInst_RRA,
	HEInst_BVS, HEInst_ADC, HEInst_KIL, HEInst_RRA, HEInst_NOP, HEInst_ADC, HEInst_ROR, HEInst_RRA, HEInst_SEI, HEInst_ADC, HEInst_NOP, HEInst_RRA, HEInst_NOP, HEInst_ADC, HEInst_ROR, HEInst_RRA,

	HEInst_NOP, HEInst_STA, HEInst_NOP, HEInst_AAX, HEInst_STY, HEInst_STA, HEInst_STX, HEInst_AAX, HEInst_DEY, HEInst_NOP, HEInst_TXA, HEInst_XAA, HEInst_STY, HEInst_STA, HEInst_STX, HEInst_AAX,
	HEInst_BCC, HEInst_STA, HEInst_KIL, HEInst_AXA, HEInst_STY, HEInst_STA, HEInst_STX, HEInst_AAX, HEInst_TYA, HEInst_STA, HEInst_TXS, HEInst_XAS, HEInst_SYA, HEInst_STA, HEInst_SXA, HEInst_AXA,
	HEInst_LDY, HEInst_LDA, HEInst_LDX, HEInst_LAX, HEInst_LDY, HEInst_LDA, HEInst_LDX, HEInst_LAX, HEInst_TAY, HEInst_LDA, HEInst_TAX, HEInst_ATX, HEInst_LDY, HEInst_LDA, HEInst_LDX, HEInst_LAX,
	HEInst_BCS, HEInst_LDA, HEInst_KIL, HEInst_LAX, HEInst_LDY, HEInst_LDA, HEInst_LDX, HEInst_LAX, HEInst_CLV, HEInst_LDA, HEInst_TSX, HEInst_LAR, HEInst_LDY, HEInst_LDA, HEInst_LDX, HEInst_LAX,

	HEInst_CPY, HEInst_CMP, HEInst_NOP, HEInst_DCP, HEInst_CPY, HEInst_CMP, HEInst_DEC, HEInst_DCP, HEInst_INY, HEInst_CMP, HEInst_DEX, HEInst_AXS, HEInst_CPY, HEInst_CMP, HEInst_DEC, HEInst_DCP,
	HEInst_BNE, HEInst_CMP, HEInst_KIL, HEInst_DCP, HEInst_NOP, HEInst_CMP, HEInst_DEC, HEInst_DCP, HEInst_CLD, HEInst_CMP, HEInst_NOP, HEInst_DCP, HEInst_NOP, HEInst_CMP, HEInst_DEC, HEInst_DCP,
	HEInst_CPX, HEInst_SBC, HEInst_NOP, HEInst_ISC, HEInst_CPX, HEInst_SBC, HEInst_INC, HEInst_ISC, HEInst_INX, HEInst_SBC, HEInst_NOP, HEInst_SBC, HEInst_CPX, HEInst_SBC, HEInst_INC, HEInst_ISC,
	HEInst_BEQ, HEInst_SBC, HEInst_KIL, HEInst_ISC, HEInst_NOP, HEInst_SBC, HEInst_INC, HEInst_ISC, HEInst_SED, HEInst_SBC, HEInst_NOP, HEInst_ISC, HEInst_NOP, HEInst_SBC, HEInst_INC, HEInst_ISC
};

static
const enum HEInstType
RP2A03_instTypeTable[] = {
    //Read
    HEInstType_READ, HEInstType_READ, HEInstType_READ, 
    HEInstType_READ, HEInstType_READ, HEInstType_READ, 

    //Read-Modify
    HEInstType_READ_MODIFY, HEInstType_READ_MODIFY, HEInstType_READ_MODIFY, HEInstType_READ_MODIFY,
    HEInstType_READ_MODIFY, HEInstType_READ_MODIFY, HEInstType_READ_MODIFY,
    HEInstType_READ_MODIFY, HEInstType_READ_MODIFY,
    HEInstType_READ_MODIFY, HEInstType_READ_MODIFY, HEInstType_READ_MODIFY,
    HEInstType_READ_MODIFY, HEInstType_READ_MODIFY, HEInstType_READ_MODIFY,

    //Read-Modify-Write
    HEInstType_READ_MODIFY_WRITE, HEInstType_READ_MODIFY_WRITE, HEInstType_READ_MODIFY_WRITE, HEInstType_READ_MODIFY_WRITE,
    HEInstType_READ_MODIFY_WRITE, HEInstType_READ_MODIFY_WRITE,
    HEInstType_READ_MODIFY_WRITE, HEInstType_READ_MODIFY_WRITE, HEInstType_READ_MODIFY_WRITE, HEInstType_READ_MODIFY_WRITE,
    HEInstType_READ_MODIFY_WRITE, HEInstType_READ_MODIFY_WRITE,

    //Write
    HEInstType_WRITE, HEInstType_WRITE, HEInstType_WRITE,
    HEInstType_WRITE, HEInstType_WRITE, HEInstType_WRITE,
    HEInstType_WRITE, HEInstType_WRITE,

    //IMP
    HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC,
    HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC,
    HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC,

    //REL
    HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC,

    //IND
    HEInstType_ETC,

    //Stack Modifier
    HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC,
    HEInstType_ETC, HEInstType_ETC, HEInstType_ETC, HEInstType_ETC,
    HEInstType_ETC, HEInstType_ETC
};

static
const enum HEAddrMode
RP2A03_addressingModeTable[] = {
    HEAddrMode_IMPBRK, HEAddrMode_IZX, HEAddrMode_IMP, HEAddrMode_IZX, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_IMPSTK,  HEAddrMode_IMM, HEAddrMode_IMPSFT,    HEAddrMode_IMM, HEAddrMode_ABS,     HEAddrMode_ABS, HEAddrMode_ABS, HEAddrMode_ABS,
    HEAddrMode_REL,    HEAddrMode_IZY, HEAddrMode_IMP, HEAddrMode_IZY, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_IMP,     HEAddrMode_ABY, HEAddrMode_IMPSFT,    HEAddrMode_ABY, HEAddrMode_ABX,     HEAddrMode_ABX, HEAddrMode_ABX, HEAddrMode_ABX,
    HEAddrMode_ABSJMP, HEAddrMode_IZX, HEAddrMode_IMP, HEAddrMode_IZX, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_IMPSTK,  HEAddrMode_IMM, HEAddrMode_IMPSFT,    HEAddrMode_IMM, HEAddrMode_ABS,     HEAddrMode_ABS, HEAddrMode_ABS, HEAddrMode_ABS,
    HEAddrMode_REL,    HEAddrMode_IZY, HEAddrMode_IMP, HEAddrMode_IZY, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_IMP,     HEAddrMode_ABY, HEAddrMode_IMPSFT,    HEAddrMode_ABY, HEAddrMode_ABX,     HEAddrMode_ABX, HEAddrMode_ABX, HEAddrMode_ABX,

    HEAddrMode_IMPSTK, HEAddrMode_IZX, HEAddrMode_IMP, HEAddrMode_IZX, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_IMPSTK,  HEAddrMode_IMM, HEAddrMode_IMPSFT,    HEAddrMode_IMM, HEAddrMode_ABSJMP,  HEAddrMode_ABS, HEAddrMode_ABS, HEAddrMode_ABS,
    HEAddrMode_REL,    HEAddrMode_IZY, HEAddrMode_IMP, HEAddrMode_IZY, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_IMP,     HEAddrMode_ABY, HEAddrMode_IMPSFT,    HEAddrMode_ABY, HEAddrMode_ABX,     HEAddrMode_ABX, HEAddrMode_ABX, HEAddrMode_ABX,
    HEAddrMode_IMPSTK, HEAddrMode_IZX, HEAddrMode_IMP, HEAddrMode_IZX, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_IMPSTK,  HEAddrMode_IMM, HEAddrMode_IMPSFT,    HEAddrMode_IMM, HEAddrMode_IND,     HEAddrMode_ABS, HEAddrMode_ABS, HEAddrMode_ABS,
    HEAddrMode_REL,    HEAddrMode_IZY, HEAddrMode_IMP, HEAddrMode_IZY, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_IMP,     HEAddrMode_ABY, HEAddrMode_IMPSFT,    HEAddrMode_ABY, HEAddrMode_ABX,     HEAddrMode_ABX, HEAddrMode_ABX, HEAddrMode_ABX,

    HEAddrMode_IMM,    HEAddrMode_IZX, HEAddrMode_IMM, HEAddrMode_IZX, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_IMP,     HEAddrMode_IMM, HEAddrMode_IMP,       HEAddrMode_IMM, HEAddrMode_ABS,     HEAddrMode_ABS, HEAddrMode_ABS, HEAddrMode_ABS,
    HEAddrMode_REL,    HEAddrMode_IZY, HEAddrMode_IMP, HEAddrMode_IZY, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPY, HEAddrMode_ZPY, HEAddrMode_IMP,     HEAddrMode_ABY, HEAddrMode_IMP,       HEAddrMode_ABY, HEAddrMode_ABX,     HEAddrMode_ABX, HEAddrMode_ABY, HEAddrMode_ABY,
    HEAddrMode_IMM,    HEAddrMode_IZX, HEAddrMode_IMM, HEAddrMode_IZX, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_IMP,     HEAddrMode_IMM, HEAddrMode_IMP,       HEAddrMode_IMM, HEAddrMode_ABS,     HEAddrMode_ABS, HEAddrMode_ABS, HEAddrMode_ABS,
    HEAddrMode_REL,    HEAddrMode_IZY, HEAddrMode_IMP, HEAddrMode_IZY, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPY, HEAddrMode_ZPY, HEAddrMode_IMP,     HEAddrMode_ABY, HEAddrMode_IMP,       HEAddrMode_ABY, HEAddrMode_ABX,     HEAddrMode_ABX, HEAddrMode_ABY, HEAddrMode_ABY,

    HEAddrMode_IMM,    HEAddrMode_IZX, HEAddrMode_IMM, HEAddrMode_IZX, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_IMP,     HEAddrMode_IMM, HEAddrMode_IMP,       HEAddrMode_IMM, HEAddrMode_ABS,     HEAddrMode_ABS, HEAddrMode_ABS, HEAddrMode_ABS,
    HEAddrMode_REL,    HEAddrMode_IZY, HEAddrMode_IMP, HEAddrMode_IZY, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_IMP,     HEAddrMode_ABY, HEAddrMode_IMP,       HEAddrMode_ABY, HEAddrMode_ABX,     HEAddrMode_ABX, HEAddrMode_ABX, HEAddrMode_ABX,
    HEAddrMode_IMM,    HEAddrMode_IZX, HEAddrMode_IMM, HEAddrMode_IZX, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_ZPG, HEAddrMode_IMP,     HEAddrMode_IMM, HEAddrMode_IMP,       HEAddrMode_IMM, HEAddrMode_ABS,     HEAddrMode_ABS, HEAddrMode_ABS, HEAddrMode_ABS,
    HEAddrMode_REL,    HEAddrMode_IZY, HEAddrMode_IMP, HEAddrMode_IZY, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_ZPX, HEAddrMode_IMP,     HEAddrMode_ABY, HEAddrMode_IMP,       HEAddrMode_ABY, HEAddrMode_ABX,     HEAddrMode_ABX, HEAddrMode_ABX, HEAddrMode_ABX
};

static
RP2A03_SequenceFunction *
RP2A03_rwSeqTable[] = {
    RP2A03_rwSeq_write
    , RP2A03_rwSeq_read
};

static
RP2A03_SequenceFunction *
RP2A03_instIoSeqTable[] = {
    RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromAddressVar
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromAddressVar
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPointerVar
    , RP2A03_instIoSeq_readFromPointerVar
    , RP2A03_instIoSeq_readFromPointerVar
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPointerVar
    , RP2A03_instIoSeq_readFromPointerVar
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc //TODO : 확인
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromAddressVar
    , RP2A03_instIoSeq_readFromAddressVar
    , RP2A03_instIoSeq_rmwInstFetchOperand
    , RP2A03_instIoSeq_writeInstWriteToAddressVar
    , RP2A03_instIoSeq_rmwInstWriteOriginalValue
    , RP2A03_instIoSeq_rmwInstWriteNewValue
    , RP2A03_instIoSeq_readFromPointerVar
    , RP2A03_instIoSeq_readFromPointerVar
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromStack
    , RP2A03_instIoSeq_jsr1
    , RP2A03_instIoSeq_jsr2
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_brk2
    , RP2A03_instIoSeq_brk3
    , RP2A03_instIoSeq_brk4
    , RP2A03_instIoSeq_brk5
    , RP2A03_instIoSeq_brk6
    , RP2A03_instIoSeq_php
    , RP2A03_instIoSeq_pha
    , RP2A03_instIoSeq_readFromStack
    , RP2A03_instIoSeq_readFromStack
    , RP2A03_instIoSeq_readFromStack
    , RP2A03_instIoSeq_readFromStack
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromStack
    , RP2A03_instIoSeq_readFromStack
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromStack
    , RP2A03_instIoSeq_readFromStack
    , RP2A03_instIoSeq_readFromStack
    , RP2A03_instIoSeq_readFromStack
    , RP2A03_instIoSeq_doesNothing
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
    , RP2A03_instIoSeq_readFromPc
};

static
RP2A03_SequenceFunction *
RP2A03_instBodySeqTable[] = {
    RP2A03_instBodySeq_decodeOpcode
    , RP2A03_instBodySeq_imp
    , RP2A03_instBodySeq_imm
    , RP2A03_instBodySeq_zpg
    , RP2A03_instBodySeq_zpx0
    , RP2A03_instBodySeq_zpx1
    , RP2A03_instBodySeq_zpy0
    , RP2A03_instBodySeq_zpy1
    , RP2A03_instBodySeq_abs0
    , RP2A03_instBodySeq_abs1
    , RP2A03_instBodySeq_abx0
    , RP2A03_instBodySeq_abx1
    , RP2A03_instBodySeq_aby0
    , RP2A03_instBodySeq_aby1
    , RP2A03_instBodySeq_ind0
    , RP2A03_instBodySeq_ind1
    , RP2A03_instBodySeq_izx0
    , RP2A03_instBodySeq_izx1
    , RP2A03_instBodySeq_izx2
    , RP2A03_instBodySeq_izx3
    , RP2A03_instBodySeq_izy0
    , RP2A03_instBodySeq_izy1
    , RP2A03_instBodySeq_izy2
    , RP2A03_instBodySeq_rel0
    , RP2A03_instBodySeq_rel1
    , RP2A03_instBodySeq_rel2
    , RP2A03_instBodySeq_impBrk
    , RP2A03_instBodySeq_impStk
    , RP2A03_instBodySeq_absJmp
    , RP2A03_instBodySeq_impSft
    , RP2A03_instBodySeq_fetchOperandWithCarry
    , RP2A03_instBodySeq_fetchOperandWithNoCarry
    , RP2A03_instBodySeq_rmwInstFetchOperand
    , RP2A03_instBodySeq_writeToAddressVar
    , RP2A03_instBodySeq_rmwInstWriteOriginalValue
    , RP2A03_instBodySeq_rmwInstWriteNewValue
    , RP2A03_instBodySeq_jmpInd0
    , RP2A03_instBodySeq_jmpInd1
    , RP2A03_instBodySeq_jmpAbs
    , RP2A03_instBodySeq_jsr0
    , RP2A03_instBodySeq_jsr1
    , RP2A03_instBodySeq_jsr2
    , RP2A03_instBodySeq_jsr3
    , RP2A03_instBodySeq_brk1
    , RP2A03_instBodySeq_brk2
    , RP2A03_instBodySeq_brk3
    , RP2A03_instBodySeq_brk4
    , RP2A03_instBodySeq_brk5
    , RP2A03_instBodySeq_brk6
    , RP2A03_instBodySeq_php
    , RP2A03_instBodySeq_pha
    , RP2A03_instBodySeq_rti0
    , RP2A03_instBodySeq_rti1
    , RP2A03_instBodySeq_rti2
    , RP2A03_instBodySeq_rti3
    , RP2A03_instBodySeq_rts0
    , RP2A03_instBodySeq_rts1
    , RP2A03_instBodySeq_rts2
    , RP2A03_instBodySeq_rts3
    , RP2A03_instBodySeq_plp0
    , RP2A03_instBodySeq_plp1
    , RP2A03_instBodySeq_pla0
    , RP2A03_instBodySeq_pla1
    , RP2A03_instBodySeq_reset0
    , RP2A03_instBodySeq_reset1
    , RP2A03_instBodySeq_reset2
    , RP2A03_instBodySeq_reset3
    , RP2A03_instBodySeq_reset4
};

static
enum RP2A03_InstSeq
RP2A03_instAddressingModeSeqTable[] = {
    RP2A03_InstSeq_imp
    , RP2A03_InstSeq_imm
    , RP2A03_InstSeq_zpg, RP2A03_InstSeq_zpx0, RP2A03_InstSeq_zpy0
    , RP2A03_InstSeq_abs0, RP2A03_InstSeq_abx0, RP2A03_InstSeq_aby0
    , RP2A03_InstSeq_ind0
    , RP2A03_InstSeq_izx0
    , RP2A03_InstSeq_izy0
    , RP2A03_InstSeq_rel0
    , RP2A03_InstSeq_impBrk
    , RP2A03_InstSeq_impStk
    , RP2A03_InstSeq_absJmp
    , RP2A03_InstSeq_impSft
};

static
const struct kaphein_nes_RP2A03ApuDmcReader_VTable
parentVTable = {
    RP2A03ApuDmcReader_setFlagParameter
    , RP2A03ApuDmcReader_setAddressParameter
    , RP2A03ApuDmcReader_setLengthParameter
    , RP2A03ApuDmcReader_setSampleBuffer
    , RP2A03ApuDmcReader_reset
    , RP2A03ApuDmcReader_hasRemainingBytesToRead
    , RP2A03ApuDmcReader_isReadingEnabled
    , RP2A03ApuDmcReader_setReadingEnabled
    , RP2A03ApuDmcReader_isIrqEnabled
    , RP2A03ApuDmcReader_setIrqEnabled
    , RP2A03ApuDmcReader_hasIrqOccured
    , RP2A03ApuDmcReader_clearIrqOccuredFlag
};

static
const char * instNameTexts[0x80] = {
    "LDA", "LDX", "LDY"
    , "LAX", "XAA", "NOP"

    , "CMP", "CPX", "CPY", "BIT"
    , "ORA", "AND", "EOR"
    , "ADC", "SBC"
    , "ANC", "AXS", "ATX"
    , "ASR", "ARR", "LAR"

    , "ASL", "LSR", "ROL", "ROR"
    , "INC", "DEC"
    , "SLO", "SRE", "RLA", "RRA"
    , "ISC", "DCP"

    , "STA", "STX", "STY"
    , "SXA", "SYA", "XAS"
    , "AXA", "AAX"

    , "TXA", "TAX", "TXS", "TSX", "TYA", "TAY"
    , "CLC", "SEC", "CLI", "SEI", "CLV", "CLD", "SED"
    , "DEY", "DEX", "INY", "INX", "KIL"

    , "BPL", "BMI", "BVC", "BVS", "BNE", "BEQ", "BCC", "BCS"

    , "JMP"

    , "JSR", "BRK", "PHP", "PHA"
    , "RTI", "RTS", "PLP", "PLA"
};

/* **************************************************************** */

/* **************************************************************** */
/* Function Definitions */

enum kaphein_ErrorCode
kaphein_nes_RP2A03_construct(
    struct kaphein_nes_RP2A03 * thisObj
    , void * allocator
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)kaphein_mem_allocate(
            allocator
            , KAPHEIN_ssizeof(*impl)
            , KAPHEIN_NULL
            , &resultErrorCode
        );

        if(KAPHEIN_NULL != impl) {
            kaphein_mem_fillZero(impl, KAPHEIN_ssizeof(impl), KAPHEIN_ssizeof(impl));
            
            thisObj->parent.vTable = &parentVTable;
            thisObj->impl_ = impl;

            impl->allocator = allocator;

            impl->apu = KAPHEIN_NULL;
            impl->decoder = KAPHEIN_NULL;

            impl->core.instSeq = RP2A03_InstSeq_reset0;
            impl->core.isHalted = false;

            impl->dma.seq = KAPHEIN_NULL;
            impl->dma.spr.isReading = false;
            impl->dma.dmc.isReading = false;

            impl->bus.ioAddress = KAPHEIN_NULL;
            impl->bus.ioData = KAPHEIN_NULL;
            impl->bus.nmi = KAPHEIN_NULL;
            impl->bus.irq = KAPHEIN_NULL;
            impl->bus.reset = KAPHEIN_NULL;
            impl->bus.controller = KAPHEIN_NULL;
            impl->bus.p1Ctrl = KAPHEIN_NULL;
            impl->bus.p2Ctrl = KAPHEIN_NULL;
            
            impl->core.isInOddCycle = false;
            impl->core.occuredInterrupt = HEInterrupt_NONE;
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03_destruct(
    struct kaphein_nes_RP2A03 * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisObj->impl_;
        
        if(KAPHEIN_NULL != impl) {
            resultErrorCode = kaphein_mem_deallocate(impl->allocator, impl, KAPHEIN_ssizeof(*impl));
            
            if(kapheinErrorCodeNoError == resultErrorCode) {
                thisObj->impl_ = KAPHEIN_NULL;
            }
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03_setInterruptBuses(
    struct kaphein_nes_RP2A03 * thisObj
    , enum kaphein_nes_InterruptSignal * nmiBus
    , enum kaphein_nes_InterruptSignal * irqBus
    , enum kaphein_nes_InterruptSignal * resetBus
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisObj->impl_;

        impl->bus.nmi = nmiBus;
        impl->bus.irq = irqBus;
        impl->bus.reset = resetBus;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03_setIoBuses(
    struct kaphein_nes_RP2A03 * thisObj
    , kaphein_UInt16 * addrBus
    , kaphein_UInt8 * dataBus
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisObj->impl_;

        impl->bus.ioAddress = addrBus;
        impl->bus.ioData = dataBus;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03_setPeripheralBuses(
    struct kaphein_nes_RP2A03 * thisObj
    , kaphein_UInt8 * controllerBus
    , kaphein_UInt16 * p1CtrlBus
    , kaphein_UInt16 * p2CtrlBus
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisObj->impl_;

        impl->bus.controller = controllerBus;
        impl->bus.p1Ctrl = p1CtrlBus;
        impl->bus.p2Ctrl = p2CtrlBus;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03_setApu(
    struct kaphein_nes_RP2A03 * thisObj
    , struct kaphein_nes_RP2A03Apu * apu
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisObj->impl_;

        impl->apu = apu;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03_setAddressDecoder(
    struct kaphein_nes_RP2A03 * thisObj
    , struct kaphein_nes_AddressDecoder * decoder
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisObj->impl_;

        impl->decoder = decoder;
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03_powerUp(
    struct kaphein_nes_RP2A03 * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisObj->impl_;

        RP2A03_powerUp(impl);
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03_reset(
    struct kaphein_nes_RP2A03 * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisObj->impl_;
        
        RP2A03_reset(impl);
    }
    
    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03_run(
    struct kaphein_nes_RP2A03 * thisObj
    , int cycleCount
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    int i;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisObj->impl_;

        for(i = cycleCount; i > 0; ) {
            --i;

            RP2A03_run(impl);
        }
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03_dumpMainRegisters(
    const struct kaphein_nes_RP2A03 * thisObj
    , struct kaphein_nes_Mos6502MainRegisterSet * resultOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(
        KAPHEIN_NULL == thisObj
        || KAPHEIN_NULL == resultOut
    ) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisObj->impl_;

        resultOut->u.m.regA = impl->core.reg.a;
        resultOut->u.m.regDummy = 0x00;
        resultOut->u.m.regX = impl->core.reg.x;
        resultOut->u.m.regY = impl->core.reg.y;
        resultOut->u.m.regP = impl->core.reg.p;
        resultOut->u.m.regS = impl->core.reg.s;
        resultOut->u.m.pcL = lowByte(impl->core.reg.pc);
        resultOut->u.m.pcH = highByte(impl->core.reg.pc);
    }

    return resultErrorCode;
}

/* **************************************************************** */

/* **************************************************************** */
/* Internal Definitions */

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setFlagParameter(
    void * thisObj
    , kaphein_UInt8 * pParam
)
{
    struct kaphein_nes_RP2A03 * const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;

    impl->dma.dmc.flagParam = pParam;

    return kapheinErrorCodeNoError;
}

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setAddressParameter(
    void * thisObj
    , kaphein_UInt8 * pParam
)
{
    struct kaphein_nes_RP2A03 * const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;

    impl->dma.dmc.addrParam = pParam;

    return kapheinErrorCodeNoError;
}

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setLengthParameter(
    void * thisObj
    , kaphein_UInt8 * pParam
)
{
    struct kaphein_nes_RP2A03 * const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;

    impl->dma.dmc.lenParam = pParam;

    return kapheinErrorCodeNoError;
}

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setSampleBuffer(
    void * thisObj
    , kaphein_Int16 * pBuffer
)
{
    struct kaphein_nes_RP2A03 * const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;

    impl->dma.dmc.buffer = pBuffer;

    return kapheinErrorCodeNoError;
}

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_reset(
    void * thisObj
)
{
    struct kaphein_nes_RP2A03 * const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;

    impl->dma.dmc.isEnabled = false;

    impl->dma.dmc.address = 0x8000;
    impl->dma.dmc.remainingByteCount = 0;

    impl->dma.dmc.hasIrqOccured = false;

    return kapheinErrorCodeNoError;
}

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_hasRemainingBytesToRead(
    void * thisObj
    , bool * truthOut
)
{
    struct kaphein_nes_RP2A03 * const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;

    *truthOut = impl->dma.dmc.remainingByteCount > 0;

    return kapheinErrorCodeNoError;
}

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_isReadingEnabled(
    void * thisObj
    , bool * truthOut
)
{
    struct kaphein_nes_RP2A03 * const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;
    
    *truthOut = impl->dma.dmc.isEnabled;

    return kapheinErrorCodeNoError;
}

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setReadingEnabled(
    void * thisObj
    , bool enabled
)
{
    struct kaphein_nes_RP2A03 *const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;

    RP2A03_dmc_setReadingEnabled(impl, enabled);

    return kapheinErrorCodeNoError;
}

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_isIrqEnabled(
    void * thisObj
    , bool * truthOut
)
{
    struct kaphein_nes_RP2A03 * const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;

    *truthOut = !!(*impl->dma.dmc.flagParam & 0x80);

    return kapheinErrorCodeNoError;
}

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_setIrqEnabled(
    void * thisObj
    , bool enabled
)
{
    struct kaphein_nes_RP2A03 * const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;

    if(enabled) {
        *impl->dma.dmc.flagParam |= 0x80;
    }
    else {
        *impl->dma.dmc.flagParam &= ~0x80;
    }

    return kapheinErrorCodeNoError;
}

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_hasIrqOccured(
    void * thisObj
    , bool * truthOut
)
{
    struct kaphein_nes_RP2A03 * const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;

    *truthOut = impl->dma.dmc.hasIrqOccured;

    return kapheinErrorCodeNoError;
}

static
enum kaphein_ErrorCode
RP2A03ApuDmcReader_clearIrqOccuredFlag(
    void * thisObj
)
{
    struct kaphein_nes_RP2A03 * const thisPtr = (struct kaphein_nes_RP2A03 *)thisObj;
    struct kaphein_nes_RP2A03_Impl *const impl = (struct kaphein_nes_RP2A03_Impl *)thisPtr->impl_;

    impl->dma.dmc.hasIrqOccured = false;

    return kapheinErrorCodeNoError;
}

static
void
RP2A03_powerUp(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    kaphein_mem_fillZero(
        &impl->core
        , KAPHEIN_ssizeof(impl->core)
        , KAPHEIN_ssizeof(impl->core)
    );

    impl->core.ioData = 0x00;
    
    RP2A03_reset(impl);
}

static
void
RP2A03_reset(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->dma.seq = KAPHEIN_NULL;
    impl->dma.spr.isReading = false;
    impl->dma.spr.isEnabled = false;
    impl->dma.dmc.isReading = false;
    impl->dma.dmc.isEnabled = false;
    impl->dma.dmc.dummyCycleCounter = 0;
    impl->dma.dmc.remainingByteCount = 0;
    impl->dma.dmc.hasIrqOccured = false;
    
    impl->core.isHalted = false;
    impl->core.isInOddCycle = false;
    impl->core.debugCycleCounter = 0;

    impl->core.ioMode = RP2A03_IoMode_write;
    impl->core.ioAddress = 0x4017;

    //BRK instruction
    impl->core.inst = 0x00;
    impl->core.instType = RP2A03_instTypeTable[impl->core.inst];
    impl->core.addrMode = RP2A03_addressingModeTable[impl->core.inst];
    impl->core.occuredInterrupt = HEInterrupt_RESET;
    impl->core.instSeq = RP2A03_InstSeq_reset0;
}

static
void
RP2A03_triggerDmcDmaIfNecessary(
    struct kaphein_nes_RP2A03_Impl * impl
    , int delayCycleCount
)
{
    if(
        impl->dma.dmc.isEnabled
        && !impl->dma.dmc.isReading
        && *impl->dma.dmc.buffer < 0
        && impl->dma.dmc.remainingByteCount > 0
    ) {
        impl->dma.dmc.isReading = true;
        impl->dma.dmc.dummyCycleCounter = delayCycleCount;

        if(KAPHEIN_NULL == impl->dma.seq) {
            impl->dma.seq = RP2A03_dmaSeq_dmcHaltCore;
        }

        //outputDebugString(
        //    "%16d %04X DMC DMA has been triggered. (lastOpcode == %02X(%s))\n"
        //    , impl->core.debugCycleCounter
        //    , impl->core.reg.pc
        //    , impl->core.debugOpcode
        //    , instNameTexts[impl->core.inst]
        //);
    }
}

static
void
RP2A03_run(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    if(KAPHEIN_NULL == impl->dma.seq) {
        impl->core.isHalted = false;
    }

    if(!impl->core.isHalted) {
        (*RP2A03_instIoSeqTable[impl->core.instSeq])(impl);
        (*RP2A03_rwSeqTable[impl->core.ioMode])(impl);
    }

    if(impl->core.isInOddCycle) {
        RP2A03_triggerDmcDmaIfNecessary(impl, (impl->core.ioMode == RP2A03_IoMode_write ? 0 : 1));
    }

    if(impl->dma.spr.isEnabled) {
        impl->dma.spr.isReading = true;

        if(KAPHEIN_NULL == impl->dma.seq) {
            impl->dma.seq = RP2A03_dmaSeq_sprHaltCore;
        }
    }
    
    if(KAPHEIN_NULL != impl->dma.seq) {
        (*impl->dma.seq)(impl);
    }

    if(!impl->core.isHalted) {
        (*RP2A03_instBodySeqTable[impl->core.instSeq])(impl);
    }

    impl->core.isInOddCycle = !impl->core.isInOddCycle;
    ++impl->core.debugCycleCounter;
}

static
kaphein_UInt8
RP2A03_read(
    struct kaphein_nes_RP2A03_Impl * impl
    , const kaphein_UInt16 address
)
{
    kaphein_UInt8 result = 0x00;

    *impl->bus.ioAddress = address;

    if(address >= 0x4000 && address < 0x4020) {
        result = RP2A03_readRegister(impl, lowByte(address));
    }
    else {
        (*impl->decoder->vTable->read)(impl->decoder, address, &result);
    }

    *impl->bus.ioData = result;

    return result;
}

static
void
RP2A03_write(
    struct kaphein_nes_RP2A03_Impl * impl
    , kaphein_UInt16 address
    , kaphein_UInt8 data
)
{
    *impl->bus.ioAddress = address;
    *impl->bus.ioData = data;

    if(address >= 0x4000 && address < 0x4020) {
        RP2A03_writeRegister(impl, lowByte(address), data);
    }
    else {
        (*impl->decoder->vTable->write)(impl->decoder, address, data);
    }
}

static
kaphein_UInt8
RP2A03_readRegister(
    struct kaphein_nes_RP2A03_Impl * impl
    , kaphein_UInt8 address
)
{
    kaphein_UInt8 value = 0x00;

    switch(address) {
    //P1 Controller
	case 0x16:
        value = (*impl->bus.ioData & 0xE0) | kaphein_nes_ShiftRegister16_shiftLeft(impl->bus.p1Ctrl, false);
    break;

    //P2 Controller
	case 0x17:
        value = (*impl->bus.ioData & 0xE0) | kaphein_nes_ShiftRegister16_shiftLeft(impl->bus.p2Ctrl, false);
    break;
    default:
        kaphein_nes_RP2A03Apu_readRegister(
            impl->apu
            , address
            , &value
        );
    break;
    }

    return value;
}

static
void
RP2A03_writeRegister(
    struct kaphein_nes_RP2A03_Impl * impl
    , kaphein_UInt8 address
    , kaphein_UInt8 data
)
{
    switch(address) {
    //OAM DMA
	case 0x14:
        if(!impl->dma.spr.isEnabled) {
            impl->dma.spr.addrH = data;
            impl->dma.spr.counter = 0;
            
            impl->dma.spr.isEnabled = true;

            //outputDebugString(
            //    "%16d %04X writeRegister(0x4014)\n"
            //    , impl->core.debugCycleCounter
            //    , impl->core.reg.pc
            //);
        }
	break;
    //Controller
	case 0x16:
        impl->regStrobe = data & 0x07;

        *impl->bus.controller = impl->regStrobe;
    break;
    default:
        kaphein_nes_RP2A03Apu_writeRegister(
            impl->apu
            , address
            , data
        );
    break;
    }
}

static
void
RP2A03_doLastInstructionCycle(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_pollInterrupts(impl);

    impl->core.instSeq = RP2A03_InstSeq_decodeOpcode;
}

static
void
RP2A03_pollInterrupts(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    if(kaphein_nes_InterruptSignal_OCCUR == *impl->bus.reset) {
        impl->core.occuredInterrupt = HEInterrupt_RESET;
    }
    else if(kaphein_nes_InterruptSignal_OCCUR == *impl->bus.nmi){
        impl->core.occuredInterrupt = HEInterrupt_NMI;
        *impl->bus.nmi = kaphein_nes_InterruptSignal_NONE;
    }

    if(
        HEInterrupt_NMI != impl->core.occuredInterrupt
        && (impl->core.reg.p & RP2A03_Status_P_DISABLE_IRQ) == 0
    ) {
        bool truth;
        
        //External IRQ
        //APU Frame Counter IRQ
        //APU DMC IRQ
        if(
            (*impl->bus.irq == kaphein_nes_InterruptSignal_OCCUR)
            || impl->dma.dmc.hasIrqOccured
            || (kaphein_nes_RP2A03Apu_isInterruptRequested(impl->apu, &truth), truth)
        ){
            impl->core.occuredInterrupt = HEInterrupt_IRQ;
        }
    }
}

static
void
RP2A03_updateStatusFlagN(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    if((impl->core.lOperand & 0x80) != 0) {
        impl->core.reg.p |= RP2A03_Status_P_NEGATIVE;
    }
    else {
        impl->core.reg.p &= ~RP2A03_Status_P_NEGATIVE;
    }
};

static
void
RP2A03_updateStatusFlagV(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    if(impl->core.overflow) {
        impl->core.reg.p |= RP2A03_Status_P_OVERFLOW;
    }
    else {
        impl->core.reg.p &= ~RP2A03_Status_P_OVERFLOW;
    }
};

static
void
RP2A03_updateStatusFlagZ(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    if(impl->core.lOperand == 0) {
        impl->core.reg.p |= RP2A03_Status_P_ZERO;
    }
    else {
        impl->core.reg.p &= ~RP2A03_Status_P_ZERO;
    }
};

static
void
RP2A03_updateStatusFlagC(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    if(impl->core.carry) {
        impl->core.reg.p |= RP2A03_Status_P_CARRY;
    }
    else {
        impl->core.reg.p &= ~RP2A03_Status_P_CARRY;
    }
};

static
void
RP2A03_updateStatusFlagNZ(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_updateStatusFlagN(impl);
    RP2A03_updateStatusFlagZ(impl);
};

static
void
RP2A03_updateStatusFlagCV(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_updateStatusFlagC(impl);
    RP2A03_updateStatusFlagV(impl);
};

static
void
RP2A03_updateStatusFlagCNZ(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_updateStatusFlagC(impl);
    RP2A03_updateStatusFlagNZ(impl);
}

static
void
RP2A03_updateStatusFlagCVNZ(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_updateStatusFlagCV(impl);
    RP2A03_updateStatusFlagNZ(impl);
}

static
void
RP2A03_testBranchCondition(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    switch(impl->core.inst) {
	case HEInst_BPL:
		impl->core.lOperand = !(impl->core.reg.p & RP2A03_Status_P_NEGATIVE);
	break;
	case HEInst_BMI:
		impl->core.lOperand = !!(impl->core.reg.p & RP2A03_Status_P_NEGATIVE);
	break;
	case HEInst_BVC:
		impl->core.lOperand = !(impl->core.reg.p & RP2A03_Status_P_OVERFLOW);
	break;
	case HEInst_BVS:
		impl->core.lOperand = !!(impl->core.reg.p & RP2A03_Status_P_OVERFLOW);
	break;
	case HEInst_BNE:
		impl->core.lOperand = !(impl->core.reg.p & RP2A03_Status_P_ZERO);
	break;
	case HEInst_BEQ:
		impl->core.lOperand = !!(impl->core.reg.p & RP2A03_Status_P_ZERO);
	break;
	case HEInst_BCC:
		impl->core.lOperand = !(impl->core.reg.p & RP2A03_Status_P_CARRY);
	break;
	case HEInst_BCS:
		impl->core.lOperand = !!(impl->core.reg.p & RP2A03_Status_P_CARRY);
	break;
    }
}

#ifdef IMPLEMENT_PIPELINING
static
void
executeInstruction(
    struct kaphein_nes_RP2A03_Impl * impl
    , HUInt cycles /*=0*/
)
#else
static
void
RP2A03_executeInstruction(
    struct kaphein_nes_RP2A03_Impl * impl
)
#endif
{
	switch(impl->core.inst) {
	////////////////////////////////////////////////////////////////
	//Read Instructions
	case HEInst_LDA:
        impl->core.lOperand = impl->core.reg.a = impl->core.rOperand;

        RP2A03_updateStatusFlagNZ(impl);

        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
    break;
	case HEInst_LDX:
        impl->core.lOperand = impl->core.reg.x = impl->core.rOperand;

        RP2A03_updateStatusFlagNZ(impl);

        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
    break;
	case HEInst_LDY:
        impl->core.lOperand = impl->core.reg.y = impl->core.rOperand;

        RP2A03_updateStatusFlagNZ(impl);

        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
    break;
    case HEInst_LAX:
        impl->core.lOperand = impl->core.reg.a = impl->core.reg.x = impl->core.rOperand;

        RP2A03_updateStatusFlagNZ(impl);

        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
    break;
    case HEInst_XAA:
        
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif        
    break;
	case HEInst_NOP:
        
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//Read-Modify Instructions
	case HEInst_CMP:
        impl->core.lOperand = impl->core.reg.a;
        impl->core.carry = true;      //자리 내림 없음
        RP2A03_alu_sub(impl);
		RP2A03_updateStatusFlagCNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_CPX:
        impl->core.lOperand = impl->core.reg.x;
        impl->core.carry = true;      //자리 내림 없음
        RP2A03_alu_sub(impl);
		RP2A03_updateStatusFlagCNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_CPY:
        impl->core.lOperand = impl->core.reg.y;
        impl->core.carry = true;      //자리 내림 없음
        RP2A03_alu_sub(impl);
		RP2A03_updateStatusFlagCNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_BIT:
        //P 레지스터에 오퍼랜드의 6, 7번 비트를 복사
		impl->core.reg.p = (impl->core.reg.p & (~0xC0)) | (impl->core.rOperand & 0xC0);

        //오퍼랜드와 A 레지스터 AND 연산
		impl->core.lOperand = (impl->core.rOperand & impl->core.reg.a);

		RP2A03_updateStatusFlagZ(impl);

        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_ORA:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0)
        #endif
            impl->core.reg.a |= impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
		else {
        #endif
            impl->core.lOperand = impl->core.reg.a;
			RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
	break;
	case HEInst_AND:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0)
        #endif
            impl->core.reg.a &= impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
		else {
        #endif
            impl->core.lOperand = impl->core.reg.a;
			RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
	break;
	case HEInst_EOR:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0)
        #endif
            impl->core.reg.a ^= impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
		else {
        #endif
            impl->core.lOperand = impl->core.reg.a;
			RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
	break;
	case HEInst_ADC:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0) {
        #endif
            impl->core.lOperand = impl->core.reg.a;
            impl->core.carry = (impl->core.reg.p & RP2A03_Status_P_CARRY) != 0;
            RP2A03_alu_add(impl);
        #ifdef IMPLEMENT_PIPELINING
        }
		else {
        #endif
            RP2A03_updateStatusFlagCVNZ(impl);
			impl->core.reg.a = impl->core.lOperand;
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
	break;
	case HEInst_SBC:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0) {
        #endif
            impl->core.lOperand = impl->core.reg.a;
            impl->core.carry = (impl->core.reg.p & RP2A03_Status_P_CARRY) != 0;
            RP2A03_alu_sub(impl);
        #ifdef IMPLEMENT_PIPELINING
        }
        else {
        #endif
            RP2A03_updateStatusFlagCVNZ(impl);
            impl->core.reg.a = impl->core.lOperand;
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
	break;
    case HEInst_ANC:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0)
        #endif
            impl->core.reg.a &= impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
		else {
        #endif
            impl->core.lOperand = impl->core.reg.a;
			RP2A03_updateStatusFlagNZ(impl);
            if((impl->core.lOperand & 0x80) != 0) {
                impl->core.reg.p |= RP2A03_Status_P_CARRY;
            }
            else {
                impl->core.reg.p &= ~RP2A03_Status_P_CARRY;
            }
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
    break;
    case HEInst_AXS:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0)
        #endif
            impl->core.reg.x &= impl->core.reg.a;
        #ifdef IMPLEMENT_PIPELINING
		else {
        #endif
            impl->core.lOperand = impl->core.reg.x;
            impl->core.carry = true;      //자리 내림 없음
            RP2A03_alu_sub(impl);
		    impl->core.reg.x = (impl->core.lOperand & 0xFF);
            RP2A03_updateStatusFlagCNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
    break;
    case HEInst_ATX:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0)
        #endif
            impl->core.reg.a = impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
		else {
        #endif
            impl->core.lOperand = impl->core.reg.x = impl->core.reg.a;
            RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
    break;
    case HEInst_ASR:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0)
        #endif
            impl->core.reg.a &= impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
		else {
        #endif
            impl->core.carry = (impl->core.reg.a & 0x01) != 0; //LSB 기억
		    impl->core.lOperand = impl->core.reg.a;
            impl->core.lOperand >>= 1;	        //시프트 연산
            impl->core.reg.a = impl->core.lOperand;                    //결과를 A 레지스터에 기억
            RP2A03_updateStatusFlagCNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
    break;
    case HEInst_ARR:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0)
        #endif
            impl->core.reg.a &= impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
		else {
        #endif
            impl->core.carry = (impl->core.reg.a & 0x01) != 0; //LSB 기억
			impl->core.lOperand = impl->core.reg.a;
            impl->core.lOperand >>= 1;	        //9bit 로테이트 연산
            impl->core.lOperand |= ((impl->core.reg.p&RP2A03_Status_P_CARRY)?(0x80):(0));
            impl->core.reg.a = impl->core.lOperand;                    //결과를 A 레지스터에 기억
		    RP2A03_updateStatusFlagNZ(impl);                        //N, Z 플래그 셋
            switch((impl->core.reg.a&0x60) >> 5){
            case 0:
                impl->core.reg.p &= ~(RP2A03_Status_P_CARRY|RP2A03_Status_P_OVERFLOW);
            break;
            case 1:
                impl->core.reg.p |= RP2A03_Status_P_OVERFLOW;
                impl->core.reg.p &= ~RP2A03_Status_P_CARRY;
            break;
            case 2:
                impl->core.reg.p |= (RP2A03_Status_P_CARRY|RP2A03_Status_P_OVERFLOW);
            break;
            case 3:
                impl->core.reg.p |= RP2A03_Status_P_CARRY;
                impl->core.reg.p &= ~RP2A03_Status_P_OVERFLOW;
            break;
            }
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
    break;
    case HEInst_LAR:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0)
        #endif
            impl->core.reg.s &= impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
		else {
        #endif
            impl->core.lOperand = impl->core.reg.a = impl->core.reg.x = impl->core.reg.s;
            RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
    break;
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//Read-Modify-Write Instructions
	//If the addressing mode is IMM,
	//Instructions will behave as a Read Instruction.
	//It means they will be pipelined.
	case HEInst_ASL:
        #ifdef IMPLEMENT_PIPELINING
        if(impl->cycles == 0) {
        #endif
            impl->core.carry = (impl->core.rOperand & 0x80) != 0; //MSB 기억
			impl->core.lOperand = impl->core.rOperand;
            impl->core.lOperand <<= 1;	        //시프트 연산
        #ifdef IMPLEMENT_PIPELINING
        }
        else {
        #endif
            RP2A03_updateStatusFlagCNZ(impl);
			if(impl->core.addrMode == HEAddrMode_IMPSFT)  //어드레싱 모드가 IMPSFT이면
			#ifdef IMPLEMENT_PIPELINING
            {
            #endif
            	impl->core.reg.a = impl->core.lOperand;                //결과를 A 레지스터에 기억
            #ifdef IMPLEMENT_PIPELINING
                impl->pipelined = false;              //파이프라이닝 해제
            }
            #endif
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false;  //원인을 찾을때 까지 이 문장 유지
		}
        #endif
	break;
	case HEInst_LSR:
        #ifdef IMPLEMENT_PIPELINING
        if(impl->cycles == 0) {
        #endif
            impl->core.carry = (impl->core.rOperand & 0x01) != 0; //LSB 기억
			impl->core.lOperand = impl->core.rOperand;
            impl->core.lOperand >>= 1;	        //시프트 연산
        #ifdef IMPLEMENT_PIPELINING
        }
        else {
        #endif
            RP2A03_updateStatusFlagCNZ(impl);
			if(impl->core.addrMode == HEAddrMode_IMPSFT)  //어드레싱 모드가 IMPSFT이면
			#ifdef IMPLEMENT_PIPELINING
            {
            #endif
            	impl->core.reg.a = impl->core.lOperand;                //결과를 A 레지스터에 기억
            #ifdef IMPLEMENT_PIPELINING
                impl->pipelined = false;              //파이프라이닝 해제
            }
            #endif
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false;  //원인을 찾을때 까지 이 문장 유지
		}
        #endif
	break;
	case HEInst_ROL:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0) {
        #endif
            impl->core.carry = (impl->core.rOperand & 0x80) != 0; //MSB 기억
			impl->core.lOperand = impl->core.rOperand;
            impl->core.lOperand <<= 1;	        //9bit 로테이트 연산
            impl->core.lOperand |= ((impl->core.reg.p&RP2A03_Status_P_CARRY)?(0x01):(0));
        #ifdef IMPLEMENT_PIPELINING
        }
        else {
        #endif
            RP2A03_updateStatusFlagCNZ(impl);
			if(impl->core.addrMode == HEAddrMode_IMPSFT)  //어드레싱 모드가 IMPSFT이면
			#ifdef IMPLEMENT_PIPELINING
            {
            #endif
            	impl->core.reg.a = impl->core.lOperand;                //결과를 A 레지스터에 기억
            #ifdef IMPLEMENT_PIPELINING
                impl->pipelined = false;              //파이프라이닝 해제
            }
            #endif
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false;  //원인을 찾을때 까지 이 문장 유지
		}
        #endif
	break;
	case HEInst_ROR:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0) {
        #endif
            impl->core.carry = (impl->core.rOperand & 0x01) != 0; //LSB 기억
			impl->core.lOperand = impl->core.rOperand;
            impl->core.lOperand >>= 1;	        //9bit 로테이트 연산
            impl->core.lOperand |= ((impl->core.reg.p&RP2A03_Status_P_CARRY)?(0x80):(0));
        #ifdef IMPLEMENT_PIPELINING
        }
        else {
        #endif
            RP2A03_updateStatusFlagCNZ(impl);
			if(impl->core.addrMode == HEAddrMode_IMPSFT)  //어드레싱 모드가 IMPSFT이면
			#ifdef IMPLEMENT_PIPELINING
            {
            #endif
            	impl->core.reg.a = impl->core.lOperand;                //결과를 A 레지스터에 기억
            #ifdef IMPLEMENT_PIPELINING
                impl->pipelined = false;              //파이프라이닝 해제
            }
            #endif
        #ifdef IMPLEMENT_PIPELINING
           impl->pipelined = false;  //원인을 찾을때 까지 이 문장 유지
		}
        #endif
	break;
	case HEInst_INC:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0)
        #endif
			impl->core.lOperand = ++impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
        else {
        #endif
			RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
	break;
	case HEInst_DEC:
        #ifdef IMPLEMENT_PIPELINING
		if(impl->cycles == 0)
        #endif
			impl->core.lOperand = --impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
        else {
        #endif
			RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false; //파이프라이닝 해제
		}
        #endif
	break;
	case HEInst_SLO:
        #ifdef IMPLEMENT_PIPELINING
        if(impl->cycles == 0) {
        #endif
            impl->core.carry = (impl->core.rOperand & 0x80) != 0; //MSB 기억
			impl->core.lOperand = impl->core.rOperand;
            impl->core.lOperand <<= 1;	        //시프트 연산
        #ifdef IMPLEMENT_PIPELINING
        }
        else {
        #endif
            impl->core.reg.a |= (impl->core.lOperand & 0xFF);
            RP2A03_updateStatusFlagC(impl);
            ((impl->core.reg.a & 0x80)?(impl->core.reg.p |= RP2A03_Status_P_NEGATIVE):(impl->core.reg.p &= ~RP2A03_Status_P_NEGATIVE));
            ((impl->core.reg.a)?(impl->core.reg.p &= ~RP2A03_Status_P_ZERO):(impl->core.reg.p |= RP2A03_Status_P_ZERO));
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false;  //원인을 찾을때 까지 이 문장 유지
		}
        #endif
	break;
	case HEInst_SRE:
        #ifdef IMPLEMENT_PIPELINING
        if(impl->cycles == 0) {
        #endif
            impl->core.carry = (impl->core.rOperand & 0x01) != 0; //LSB 기억
		    impl->core.lOperand = impl->core.rOperand;
            impl->core.lOperand >>= 1;	        //시프트 연산
        #ifdef IMPLEMENT_PIPELINING
        }
        else {
        #endif
            impl->core.reg.a ^= (impl->core.lOperand & 0xFF);
            RP2A03_updateStatusFlagC(impl);
            ((impl->core.reg.a & 0x80)?(impl->core.reg.p |= RP2A03_Status_P_NEGATIVE):(impl->core.reg.p &= ~RP2A03_Status_P_NEGATIVE));
            ((impl->core.reg.a)?(impl->core.reg.p &= ~RP2A03_Status_P_ZERO):(impl->core.reg.p |= RP2A03_Status_P_ZERO));
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false;  //원인을 찾을때 까지 이 문장 유지
		}
        #endif
	break;
    case HEInst_RLA:
        #ifdef IMPLEMENT_PIPELINING
        if(impl->cycles == 0) {
        #endif
            impl->core.carry = (impl->core.rOperand & 0x80) != 0; //MSB 기억
			impl->core.lOperand = impl->core.rOperand;
            impl->core.lOperand <<= 1;	        //9bit 로테이트 연산
            impl->core.lOperand |= ((impl->core.reg.p&RP2A03_Status_P_CARRY)?(0x01):(0));
            impl->core.rOperand = lowByte(impl->core.lOperand);
        #ifdef IMPLEMENT_PIPELINING
        }
        else {
        #endif
            impl->core.reg.a &= impl->core.rOperand;
            impl->core.lOperand = impl->core.reg.a;
            RP2A03_updateStatusFlagCNZ(impl);

            impl->core.lOperand = impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false;
        }
        #endif
    break;
    case HEInst_RRA:
        #ifdef IMPLEMENT_PIPELINING
        if(impl->cycles == 0) {
        #endif
            impl->core.carry = (impl->core.rOperand & 0x01) != 0; //LSB 기억
			impl->core.lOperand = impl->core.rOperand;
            impl->core.lOperand >>= 1;	        //9bit 로테이트 연산
            impl->core.lOperand |= ((impl->core.reg.p&RP2A03_Status_P_CARRY)?(0x80):(0));
            impl->core.rOperand = lowByte(impl->core.lOperand);
        #ifdef IMPLEMENT_PIPELINING
        }
        else {
        #endif
            impl->core.lOperand = impl->core.reg.a;
            //carry = ((regP&RP2A03_Status_P_CARRY)?(true):(false));
            RP2A03_alu_add(impl);
            impl->core.reg.a = impl->core.lOperand;
            RP2A03_updateStatusFlagCVNZ(impl);

            impl->core.lOperand = impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false;
        }
        #endif
    break;
    case HEInst_ISC:
        #ifdef IMPLEMENT_PIPELINING
        if(impl->cycles == 0) {
        #endif
			impl->core.lOperand = ++impl->core.rOperand;
			RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        }
        else {
        #endif
            impl->core.lOperand = impl->core.reg.a;
            impl->core.carry = (impl->core.reg.p & RP2A03_Status_P_CARRY) != 0;
            RP2A03_alu_sub(impl);
            RP2A03_updateStatusFlagCV(impl);
            impl->core.reg.a = impl->core.lOperand;
            RP2A03_updateStatusFlagNZ(impl);

            impl->core.lOperand = impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false;  //원인을 찾을때 까지 이 문장 유지
		}
        #endif
    break;
    case HEInst_DCP:
        #ifdef IMPLEMENT_PIPELINING
        if(impl->cycles == 0) {
        #endif
			impl->core.lOperand = --impl->core.rOperand;
			RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        }
        else {
        #endif
            impl->core.lOperand = impl->core.reg.a;
            impl->core.carry = true;      //자리 내림 없음
            RP2A03_alu_sub(impl);
		    RP2A03_updateStatusFlagCNZ(impl);

            impl->core.lOperand = impl->core.rOperand;
        #ifdef IMPLEMENT_PIPELINING
            impl->pipelined = false;  //원인을 찾을때 까지 이 문장 유지
		}
        #endif
    break;
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//Write Instructions
	case HEInst_STA:
		impl->core.lOperand = impl->core.reg.a;
	break;
	case HEInst_STX:
		impl->core.lOperand = impl->core.reg.x;
	break;
	case HEInst_STY:
		impl->core.lOperand = impl->core.reg.y;
	break;
    case HEInst_SXA:
        impl->core.lOperand = (impl->core.addressVarH &= impl->core.reg.x);
    break;
    case HEInst_SYA:
        impl->core.lOperand = (impl->core.addressVarH &= impl->core.reg.y);
    break;
    case HEInst_XAS:
        impl->core.reg.s = impl->core.reg.x & impl->core.reg.a;
        impl->core.lOperand = (impl->core.reg.s & impl->core.addressVarH);
    break;
    case HEInst_AXA:
        impl->core.lOperand = (impl->core.reg.x & impl->core.reg.a) & 0x80;
    break;
    case HEInst_AAX:
        impl->core.lOperand = (impl->core.reg.x & impl->core.reg.a);
    break;
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//Transference Instructions
	case HEInst_TXA:
        impl->core.lOperand = impl->core.reg.a = impl->core.reg.x;
        RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_TAX:
        impl->core.lOperand = impl->core.reg.x = impl->core.reg.a;
        RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_TXS:
        impl->core.lOperand = impl->core.reg.s = impl->core.reg.x;
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_TSX:
        impl->core.lOperand = impl->core.reg.x = impl->core.reg.s;
        RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_TYA:
        impl->core.lOperand = impl->core.reg.a = impl->core.reg.y;
        RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_TAY:
        impl->core.lOperand = impl->core.reg.y = impl->core.reg.a;
        RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//Status flag modification Instructions
	case HEInst_CLV:
		impl->core.reg.p &= ~RP2A03_Status_P_OVERFLOW;
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_CLD:
		impl->core.reg.p &= ~RP2A03_Status_P_DECIMAL;
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_SED:
		impl->core.reg.p |= RP2A03_Status_P_DECIMAL;
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_CLI:
		impl->core.reg.p &= ~RP2A03_Status_P_DISABLE_IRQ;
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_SEI:
		impl->core.reg.p |= RP2A03_Status_P_DISABLE_IRQ;
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_CLC:
		impl->core.reg.p &= ~RP2A03_Status_P_CARRY;
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_SEC:
		impl->core.reg.p |= RP2A03_Status_P_CARRY;
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	////////////////////////////////////////////////////////////////

	////////////////////////////////
	//Register Increment / Decrement Instructions
	case HEInst_DEY:
		impl->core.lOperand = --impl->core.reg.y;
        RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_DEX:
		impl->core.lOperand = --impl->core.reg.x;
        RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_INY:
		impl->core.lOperand = ++impl->core.reg.y;
        RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
	case HEInst_INX:
		impl->core.lOperand = ++impl->core.reg.x;
        RP2A03_updateStatusFlagNZ(impl);
        #ifdef IMPLEMENT_PIPELINING
        impl->pipelined = false; //파이프라이닝 해제
        #endif
	break;
    case HEInst_KIL:
    break;
	////////////////////////////////////////////////////////////////
	}
}

static
void
RP2A03_dmc_setReadingEnabled(
    struct kaphein_nes_RP2A03_Impl * impl
    , bool enabled
)
{
    impl->dma.dmc.isEnabled = !!enabled;
    
    if(impl->dma.dmc.isEnabled) {
        if(impl->dma.dmc.remainingByteCount < 1) {
            impl->dma.dmc.address = 0xC000 | (((kaphein_UInt16)(*impl->dma.dmc.addrParam)) << 6);

            impl->dma.dmc.remainingByteCount =
                (((kaphein_UInt16)(*impl->dma.dmc.lenParam)) << 4) | 0x0001
            ;
        }

        RP2A03_triggerDmcDmaIfNecessary(impl, 0);

        //outputDebugString(
        //    "%16d %04X impl->dma.dmc.isEnabled == true\n"
        //    , impl->core.debugCycleCounter
        //    , impl->core.reg.pc
        //);
    }
    else {
        impl->dma.dmc.remainingByteCount = 0;
    }
}

static
void
RP2A03_alu_add(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    const kaphein_UInt8 rOperandMsb = impl->core.rOperand & 0x80;
    const kaphein_UInt16 result = impl->core.lOperand + impl->core.rOperand + (!!impl->core.carry);

    impl->core.overflow = ((impl->core.lOperand & 0x80) == rOperandMsb) && (rOperandMsb != (result & 0x80));
    impl->core.carry = result > 0xFF;
    impl->core.lOperand = result & 0xFF;
}

static
void
RP2A03_alu_sub(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    const kaphein_UInt8 rOperandMsb = impl->core.rOperand & 0x80;
    const kaphein_UInt16 result = impl->core.lOperand + ((impl->core.rOperand ^ 0xFF) + (!!impl->core.carry));
    
    impl->core.overflow = ((impl->core.lOperand & 0x80) != rOperandMsb) && (rOperandMsb == (result & 0x80));
    impl->core.carry = result > 0xFF;
    impl->core.lOperand = result & 0xFF;
};

static
kaphein_UInt8
RP2A03_rwSeq_read(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.ioData = RP2A03_read(impl, impl->core.ioAddress);
    
    return impl->core.ioData;
}

static
void
RP2A03_rwSeq_write(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_write(impl, impl->core.ioAddress, impl->core.ioData);
}

static
void
RP2A03_dmaSeq_dmcHaltCore(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    switch(impl->core.ioMode) {
    case RP2A03_IoMode_write:
        //outputDebugString(
        //    "%16d %04X SPR == %d, DMC == %d RP2A03_dmaSeq_dmcHaltCore FAILED! Because the CPU is trying to write something!\n"
        //    , impl->core.debugCycleCounter
        //    , impl->core.reg.pc
        //    , impl->dma.spr.isReading
        //    , impl->dma.dmc.isReading
        //);
    break;
    case RP2A03_IoMode_read:
        impl->core.isHalted = true;

        impl->dma.seq = RP2A03_dmaSeq_alignment0;

        //outputDebugString(
        //    "%16d %04X SPR == %d, DMC == %d RP2A03_dmaSeq_dmcHaltCore\n"
        //    , impl->core.debugCycleCounter
        //    , impl->core.reg.pc
        //    , impl->dma.spr.isReading
        //    , impl->dma.dmc.isReading
        //);
    break;
    }
}

static
void
RP2A03_dmaSeq_sprHaltCore(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    switch(impl->core.ioMode) {
    case RP2A03_IoMode_write:
        //outputDebugString(
        //    "%16d %04X SPR == %d, DMC == %d RP2A03_dmaSeq_sprHaltCore FAILED! Because the CPU is trying to write something!\n"
        //    , impl->core.debugCycleCounter
        //    , impl->core.reg.pc
        //    , impl->dma.spr.isReading
        //    , impl->dma.dmc.isReading
        //);
    break;
    case RP2A03_IoMode_read:
        impl->core.isHalted = true;

        impl->dma.seq = RP2A03_dmaSeq_alignment1;

        //outputDebugString(
        //    "%16d %04X SPR == %d, DMC == %d RP2A03_dmaSeq_sprHaltCore\n"
        //    , impl->core.debugCycleCounter
        //    , impl->core.reg.pc
        //    , impl->dma.spr.isReading
        //    , impl->dma.dmc.isReading
        //);
    break;
    }
}

static
void
RP2A03_dmaSeq_alignment0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    if(impl->dma.dmc.isReading) {
        impl->dma.seq = RP2A03_dmaSeq_alignment1;

        //outputDebugString(
        //    "%16d %04X SPR == %d, DMC == %d DMC dummy read.\n"
        //    , impl->core.debugCycleCounter
        //    , impl->core.reg.pc
        //    , impl->dma.spr.isReading
        //    , impl->dma.dmc.isReading
        //);
    }
    else if(impl->dma.spr.isReading) {
        if(!impl->core.isInOddCycle) {
            RP2A03_dmaSeq_read(impl);
        }
        else {
            impl->dma.seq = RP2A03_dmaSeq_read;

            //outputDebugString(
            //    "%16d %04X SPR == %d, DMC == %d SPR DMA alignment.\n"
            //    , impl->core.debugCycleCounter
            //    , impl->core.reg.pc
            //    , impl->dma.spr.isReading
            //    , impl->dma.dmc.isReading
            //);
        }
    }
    else {
        impl->dma.spr.isEnabled = false;
        impl->dma.seq = KAPHEIN_NULL;
    }
}

static
void
RP2A03_dmaSeq_alignment1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    if(impl->dma.dmc.isReading) {
        impl->dma.seq = RP2A03_dmaSeq_readDmcSample;

        if(impl->dma.dmc.dummyCycleCounter > 0) {
            --impl->dma.dmc.dummyCycleCounter;

            //outputDebugString(
            //    "%16d %04X SPR == %d, DMC == %d DMC DMA alignment.\n"
            //    , impl->core.debugCycleCounter
            //    , impl->core.reg.pc
            //    , impl->dma.spr.isReading
            //    , impl->dma.dmc.isReading
            //);
        }
        else {
            RP2A03_dmaSeq_readDmcSample(impl);
        }
    }
    else if(impl->dma.spr.isReading) {
        if(!impl->core.isInOddCycle) {
            RP2A03_dmaSeq_read(impl);
        }
        else {
            impl->dma.seq = RP2A03_dmaSeq_read;

            //outputDebugString(
            //    "%16d %04X SPR == %d, DMC == %d SPR DMA alignment.\n"
            //    , impl->core.debugCycleCounter
            //    , impl->core.reg.pc
            //    , impl->dma.spr.isReading
            //    , impl->dma.dmc.isReading
            //);
        }
    }
    else {
        impl->dma.spr.isEnabled = false;
        impl->dma.seq = KAPHEIN_NULL;
    }
}

static
void
RP2A03_dmaSeq_alignment2(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->dma.seq = RP2A03_dmaSeq_alignment1;
}

static
void
RP2A03_dmaSeq_read(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.ioMode = RP2A03_IoMode_read;
    impl->dma.ioData = RP2A03_read(impl, makeWord((impl->dma.spr.counter & 0xFF), impl->dma.spr.addrH));

    //outputDebugString(
    //    "%16d %04X SPR == %d, DMC == %d SPR DMA #%4d %s\n"
    //    , impl->core.debugCycleCounter
    //    , impl->core.reg.pc
    //    , impl->dma.spr.isReading
    //    , impl->dma.dmc.isReading
    //    , impl->dma.spr.counter
    //    , "read"
    //);

    impl->dma.seq = RP2A03_dmaSeq_write;
}

static
void
RP2A03_dmaSeq_write(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.ioMode = RP2A03_IoMode_write;
    RP2A03_write(impl, makeWord(0x04, 0x20), impl->dma.ioData);

    //outputDebugString(
    //    "%16d %04X SPR == %d, DMC == %d SPR DMA #%4d %s\n"
    //    , impl->core.debugCycleCounter
    //    , impl->core.reg.pc
    //    , impl->dma.spr.isReading
    //    , impl->dma.dmc.isReading
    //    , impl->dma.spr.counter
    //    , "write"
    //);
    
    ++impl->dma.spr.counter;
    
    if(impl->dma.dmc.isReading) {
        impl->dma.seq = RP2A03_dmaSeq_readDmcSample;
    }
    else {
        if(impl->dma.spr.counter > 255) {
            impl->dma.spr.isReading = false;
            impl->dma.spr.isEnabled = false;
            impl->dma.seq = KAPHEIN_NULL;
        }
        else {
            impl->dma.seq = RP2A03_dmaSeq_read;
        }
    }
}

static
void
RP2A03_dmaSeq_readDmcSample(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    kaphein_UInt16 address;

    if(impl->dma.spr.isReading && impl->dma.spr.counter <= 255) {
        address = makeWord((impl->dma.spr.counter & 0xFF), impl->dma.spr.addrH);
    }
    else {
        address = impl->dma.dmc.address;
    }
    
    impl->core.ioMode = RP2A03_IoMode_read;
    *impl->dma.dmc.buffer = RP2A03_read(impl, address);

    //outputDebugString(
    //    "%16d %04X SPR == %d, DMC == %d RP2A03_dmaSeq_readDmcSample\n"
    //    , impl->core.debugCycleCounter
    //    , impl->core.reg.pc
    //    , impl->dma.spr.isReading
    //    , impl->dma.dmc.isReading
    //);

    if(impl->dma.dmc.address >= 0xFFFF) {
        impl->dma.dmc.address = 0x8000;
    }
    else {
        ++impl->dma.dmc.address;
    }

    --impl->dma.dmc.remainingByteCount;
    if(impl->dma.dmc.remainingByteCount < 1) {
        const kaphein_UInt8 flag = *impl->dma.dmc.flagParam;
        
        if((flag & 0x40) != 0) {
            RP2A03_dmc_setReadingEnabled(impl, true);
        }
        else if((flag & 0x80) != 0) {
            impl->dma.dmc.hasIrqOccured = true;
        }
    }

    impl->dma.dmc.isReading = false;
    if(impl->dma.spr.isReading) {
        if(impl->dma.spr.counter > 255) {
            impl->dma.spr.isReading = false;
            impl->dma.spr.isEnabled = false;

            impl->dma.seq = RP2A03_dmaSeq_alignment2;
        }
        else {
            switch(impl->dma.spr.counter) {
            case 255:
                impl->dma.seq = RP2A03_dmaSeq_read;
            break;
            default:
                impl->dma.seq = RP2A03_dmaSeq_alignment1;
            //
            }
        }
    }
    else {
        impl->dma.seq = KAPHEIN_NULL;
    }
}

static
void
RP2A03_instIoSeq_doesNothing(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    KAPHEIN_x_UNUSED_PARAMETER(impl)
    
    //Does nothing.
}

static
void
RP2A03_instIoSeq_readFromPc(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.ioAddress = impl->core.reg.pc;
    impl->core.ioMode = RP2A03_IoMode_read;
}

static
void
RP2A03_instIoSeq_readFromAddressVar(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.ioAddress = makeWord(impl->core.addressVarL, impl->core.addressVarH);
    impl->core.ioMode = RP2A03_IoMode_read;
}

static
void
RP2A03_instIoSeq_readFromPointerVar(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.ioAddress = makeWord(impl->core.pointerVarL, impl->core.pointerVarH);
    impl->core.ioMode = RP2A03_IoMode_read;
}

static
void
RP2A03_instIoSeq_readFromStack(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.ioAddress = makeWord(impl->core.reg.s, 0x01);
    impl->core.ioMode = RP2A03_IoMode_read;
}

static
void
RP2A03_instIoSeq_rmwInstFetchOperand(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    KAPHEIN_x_UNUSED_PARAMETER(impl)
}

static
void
RP2A03_instIoSeq_rmwInstWriteOriginalValue(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //오퍼랜드(기존값) 쓰기
    impl->core.ioAddress = makeWord(impl->core.addressVarL, impl->core.addressVarH);
    impl->core.ioData = impl->core.rOperand;
    impl->core.ioMode = RP2A03_IoMode_write;
}

static
void
RP2A03_instIoSeq_rmwInstWriteNewValue(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //결과값 쓰기
    impl->core.ioAddress = makeWord(impl->core.addressVarL, impl->core.addressVarH);
    impl->core.ioData = impl->core.lOperand;
    impl->core.ioMode = RP2A03_IoMode_write;
}


static
void
RP2A03_instIoSeq_writeInstWriteToAddressVar(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_executeInstruction(impl);

    RP2A03_instIoSeq_writeToAddressVar(impl);
}

static
void
RP2A03_instIoSeq_writeToAddressVar(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.ioAddress = makeWord(impl->core.addressVarL, impl->core.addressVarH);
    impl->core.ioData = impl->core.lOperand;
    impl->core.ioMode = RP2A03_IoMode_write;
}

static
void
RP2A03_instIoSeq_jsr1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //PC 상위 값 push
    impl->core.ioAddress = makeWord(impl->core.reg.s, 0x01);
    impl->core.ioData = highByte(impl->core.reg.pc);
    impl->core.ioMode = RP2A03_IoMode_write;
}

static
void
RP2A03_instIoSeq_jsr2(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //PC 하위 값 push
    impl->core.ioAddress = makeWord(impl->core.reg.s, 0x01);
    impl->core.ioData = lowByte(impl->core.reg.pc);
    impl->core.ioMode = RP2A03_IoMode_write;
}

static
void
RP2A03_instIoSeq_brk2(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //PC 상위 값 push
    impl->core.ioAddress = makeWord(impl->core.reg.s, 0x01);
    impl->core.ioData = highByte(impl->core.reg.pc);
    impl->core.ioMode = RP2A03_IoMode_write;
}

static
void
RP2A03_instIoSeq_brk3(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //PC 하위 값 push
    impl->core.ioAddress = makeWord(impl->core.reg.s, 0x01);
    impl->core.ioData = lowByte(impl->core.reg.pc);
    impl->core.ioMode = RP2A03_IoMode_write;
}

static
void
RP2A03_instIoSeq_brk4(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    switch(impl->core.inst) {
    //Opcode 0x00
    case HEInst_BRK:
        impl->core.reg.p |= RP2A03_Status_P_BRK;
    break;
    default:
        impl->core.reg.p &= ~RP2A03_Status_P_BRK;
    }

    //U 플래그 셋
    impl->core.reg.p |= RP2A03_Status_P_UNUSED;

    //P 레지스터 값 push
    impl->core.ioAddress = makeWord(impl->core.reg.s, 0x01);
    impl->core.ioData = impl->core.reg.p;
    impl->core.ioMode = RP2A03_IoMode_write;
}

static
void
RP2A03_instIoSeq_brk5(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    static const kaphein_UInt8 lowByteAddr[] = {0xFE, 0xFC, 0xFA, 0xFE};

    //PC 하위 값 읽기
    impl->core.ioAddress = makeWord(lowByteAddr[((kaphein_UInt8)impl->core.occuredInterrupt)], 0xFF);
    impl->core.ioMode = RP2A03_IoMode_read;
}

static
void
RP2A03_instIoSeq_brk6(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    static const kaphein_UInt8 highByteAddr[] = {0xFF, 0xFD, 0xFB, 0xFF};
    
    //PC 상위 값 읽기
    impl->core.ioAddress = makeWord(highByteAddr[((kaphein_UInt8)impl->core.occuredInterrupt)], 0xFF);
    impl->core.ioMode = RP2A03_IoMode_read;
}

static
void
RP2A03_instIoSeq_php(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.reg.p |= (RP2A03_Status_P_BRK | RP2A03_Status_P_UNUSED);
    
    //P 레지스터 값 push
    impl->core.ioAddress = makeWord(impl->core.reg.s, 0x01);
    impl->core.ioData = impl->core.reg.p;
    impl->core.ioMode = RP2A03_IoMode_write;
}

static
void
RP2A03_instIoSeq_pha(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //A 레지스터 값 push
    impl->core.ioAddress = makeWord(impl->core.reg.s, 0x01);
    impl->core.ioData = impl->core.reg.a;
    impl->core.ioMode = RP2A03_IoMode_write;
}

static
void
RP2A03_instBodySeq_decodeOpcode(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    switch(impl->core.occuredInterrupt) {
    case HEInterrupt_NONE:
        //if(
        //    (impl->core.reg.pc >= 0xE200 && impl->core.reg.pc <= 0xE24F)
        //    || (impl->core.reg.pc >= 0xE280 && impl->core.reg.pc <= 0xE2C6)
        //    || (impl->core.reg.pc >= 0xE4F0 && impl->core.reg.pc <= 0xE507)
        //) {
            //outputDebugString(
            //    "%16d %04X %02X(%s) %02X %02X %02X %02X %c%c%c%c%c%c%c%c\n"
            //    , impl->core.debugCycleCounter
            //    , impl->core.reg.pc
            //    , impl->core.ioData
            //    , instNameTexts[RP2A03_instTable[impl->core.ioData]]
            //    , impl->core.reg.a
            //    , impl->core.reg.x
            //    , impl->core.reg.y
            //    , impl->core.reg.s
            //    , (!!(impl->core.reg.p & RP2A03_Status_P_NEGATIVE) ? 'N' : 'n')
            //    , (!!(impl->core.reg.p & RP2A03_Status_P_OVERFLOW) ? 'V' : 'v')
            //    , (!!(impl->core.reg.p & RP2A03_Status_P_UNUSED) ? 'U' : 'u')
            //    , (!!(impl->core.reg.p & RP2A03_Status_P_BRK) ? 'B' : 'b')
            //    , (!!(impl->core.reg.p & RP2A03_Status_P_DECIMAL) ? 'D' : 'd')
            //    , (!!(impl->core.reg.p & RP2A03_Status_P_DISABLE_IRQ) ? 'I' : 'i')
            //    , (!!(impl->core.reg.p & RP2A03_Status_P_ZERO) ? 'Z' : 'z')
            //    , (!!(impl->core.reg.p & RP2A03_Status_P_CARRY) ? 'C' : 'c')
            //);
        //}

        ++impl->core.reg.pc;
        
        impl->core.inst = RP2A03_instTable[impl->core.ioData];
        impl->core.instType = RP2A03_instTypeTable[impl->core.inst];
        impl->core.addrMode = RP2A03_addressingModeTable[impl->core.ioData];
        impl->core.instSeq = RP2A03_instAddressingModeSeqTable[impl->core.addrMode];
        
        impl->core.debugOpcode = impl->core.ioData;
    break;
    default:
        impl->core.instSeq = RP2A03_InstSeq_brk1;

        //outputDebugString(
        //    "%16d %04X An interrupt has occured.(%d)\n"
        //    , impl->core.debugCycleCounter
        //    , impl->core.reg.pc
        //    , impl->core.occuredInterrupt
        //);
    break;
    }
}

static
void
RP2A03_instBodySeq_imp(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    RP2A03_executeInstruction(impl);
}

static
void
RP2A03_instBodySeq_imm(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    ++impl->core.reg.pc;

    impl->core.rOperand = impl->core.ioData;
    
    RP2A03_executeInstruction(impl);
}

static
void
RP2A03_instBodySeq_zpg(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //하위 주소를 읽음
    impl->core.addressVarL = impl->core.ioData;
    impl->core.addressVarH = 0x00;
    
    switch(impl->core.instType) {
    case HEInstType_WRITE:
        impl->core.instSeq = RP2A03_InstSeq_writeInstWriteToAddressVar;
    break;
    default:
        impl->core.instSeq = RP2A03_InstSeq_fetchOperandWithNoCarry;                
    }
}

static
void
RP2A03_instBodySeq_zpx0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //하위 주소를 읽음
    impl->core.addressVarL = impl->core.ioData;
    impl->core.addressVarH = 0x00;
    
    impl->core.instSeq = RP2A03_InstSeq_zpx1;
}

static
void
RP2A03_instBodySeq_zpx1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //하위 주소에 인덱스를 가산
    impl->core.addressVarL += impl->core.reg.x;
    
    switch(impl->core.instType) {
    case HEInstType_WRITE:
        impl->core.instSeq = RP2A03_InstSeq_writeInstWriteToAddressVar;
    break;
    default:
        impl->core.instSeq = RP2A03_InstSeq_fetchOperandWithNoCarry;                
    }
}

static
void
RP2A03_instBodySeq_zpy0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //하위 주소를 읽음
    impl->core.addressVarL = impl->core.ioData;
    impl->core.addressVarH = 0x00;
    
    impl->core.instSeq = RP2A03_InstSeq_zpy1;
}

static
void
RP2A03_instBodySeq_zpy1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //하위 주소에 인덱스를 가산
    impl->core.addressVarL += impl->core.reg.y;
    
    switch(impl->core.instType) {
    case HEInstType_WRITE:
        impl->core.instSeq = RP2A03_InstSeq_writeInstWriteToAddressVar;
    break;
    default:
        impl->core.instSeq = RP2A03_InstSeq_fetchOperandWithNoCarry;                
    }
}

static
void
RP2A03_instBodySeq_abs0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //하위 주소를 읽음
    impl->core.addressVarL = impl->core.ioData;
    
    impl->core.instSeq = RP2A03_InstSeq_abs1;
}

static
void
RP2A03_instBodySeq_abs1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;
    
    //상위 주소를 읽음
    impl->core.addressVarH = impl->core.ioData;
    
    switch(impl->core.instType) {
    case HEInstType_WRITE:
        impl->core.instSeq = RP2A03_InstSeq_writeInstWriteToAddressVar;
    break;
    default:
        impl->core.instSeq = RP2A03_InstSeq_fetchOperandWithNoCarry;                
    }
}

static
void
RP2A03_instBodySeq_abx0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //하위 주소를 읽음
    impl->core.addressVarL = impl->core.ioData;
    
    impl->core.instSeq = RP2A03_InstSeq_abx1;
}

static
void
RP2A03_instBodySeq_abx1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //상위 주소를 읽음
    impl->core.addressVarH = impl->core.ioData;
    
    //하위 주소에 인덱스를 가산
    impl->core.lOperand = impl->core.addressVarL;
    impl->core.rOperand = impl->core.reg.x;
    impl->core.carry = 0;
    RP2A03_alu_add(impl);
    impl->core.addressVarL = impl->core.lOperand;

    impl->core.instSeq = RP2A03_InstSeq_fetchOperandWithCarry;
}

static
void
RP2A03_instBodySeq_aby0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //하위 주소를 읽음
    impl->core.addressVarL = impl->core.ioData;
    
    impl->core.instSeq = RP2A03_InstSeq_aby1;
}

static
void
RP2A03_instBodySeq_aby1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //상위 주소를 읽음
    impl->core.addressVarH = impl->core.ioData;
    
    //하위 주소에 인덱스를 가산
    impl->core.lOperand = impl->core.addressVarL;
    impl->core.rOperand = impl->core.reg.y;
    impl->core.carry = 0;
    RP2A03_alu_add(impl);
    impl->core.addressVarL = impl->core.lOperand;

    impl->core.instSeq = RP2A03_InstSeq_fetchOperandWithCarry;
}

static
void
RP2A03_instBodySeq_ind0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //포인터 하위 값 읽기
    impl->core.pointerVarL = impl->core.ioData;
    
    impl->core.instSeq = RP2A03_InstSeq_ind1;
}

static
void
RP2A03_instBodySeq_ind1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //포인터 상위 값 읽기
    impl->core.pointerVarH = impl->core.ioData;
    
    impl->core.instSeq =  RP2A03_InstSeq_jmpInd0;
}

static
void
RP2A03_instBodySeq_izx0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //포인터 하위 값을 읽음
    impl->core.pointerVarL = impl->core.ioData;
    impl->core.pointerVarH = 0x00;
    
    impl->core.instSeq = RP2A03_InstSeq_izx1;
}

static
void
RP2A03_instBodySeq_izx1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //포인터 하위 값에 인덱스 가산(캐리 처리 안 됨)
    impl->core.pointerVarL += impl->core.reg.x;
    
    impl->core.instSeq = RP2A03_InstSeq_izx2;
}

static
void
RP2A03_instBodySeq_izx2(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //하위 주소 읽기 후 포인터 증가(캐리 처리 안 됨)
    impl->core.addressVarL = impl->core.ioData;
    ++impl->core.pointerVarL;
    
    impl->core.instSeq = RP2A03_InstSeq_izx3;
}

static
void
RP2A03_instBodySeq_izx3(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //상위 주소 읽기
    impl->core.addressVarH = impl->core.ioData;
    
    switch(impl->core.instType) {
    case HEInstType_WRITE:
        impl->core.instSeq = RP2A03_InstSeq_writeInstWriteToAddressVar;
    break;
    default:
        impl->core.instSeq = RP2A03_InstSeq_fetchOperandWithNoCarry;                
    }
}

static
void
RP2A03_instBodySeq_izy0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    impl->core.pointerVarL = impl->core.ioData;
    impl->core.pointerVarH = 0x00;
    
    impl->core.instSeq = RP2A03_InstSeq_izy1;

    ////////////////////////////////
}

static
void
RP2A03_instBodySeq_izy1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //하위 주소 읽기 후 포인터 증가(캐리 처리 안 됨)
    impl->core.addressVarL = impl->core.ioData;
    ++impl->core.pointerVarL;
    
    impl->core.instSeq = RP2A03_InstSeq_izy2;
}

static
void
RP2A03_instBodySeq_izy2(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //상위 주소 읽기
    impl->core.addressVarH = impl->core.ioData;

    //하위 주소에 인덱스를 가산
    impl->core.lOperand = impl->core.addressVarL;
    impl->core.rOperand = impl->core.reg.y;
    impl->core.carry = 0;
    RP2A03_alu_add(impl);
    impl->core.addressVarL = impl->core.lOperand;
    
    impl->core.instSeq = RP2A03_InstSeq_fetchOperandWithCarry;
}

static
void
RP2A03_instBodySeq_rel0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ////////////////////////////////
    //cycles 1 - PC 오프셋 읽기, 명령어 실행(조건 검사)
    
    ++impl->core.reg.pc;
    
    //PC 오프셋 읽기
    impl->core.rOperand = impl->core.ioData;

    //명령어 실행(조건 검사)
    RP2A03_testBranchCondition(impl);

    //조건이 참이면
    if(impl->core.lOperand) {
        RP2A03_pollInterrupts(impl);
        
        impl->core.instSeq = RP2A03_InstSeq_rel1;
    }
    //조건이 거짓이면
    else {
        RP2A03_doLastInstructionCycle(impl);
    }
    
    ////////////////////////////////
}

static
void
RP2A03_instBodySeq_rel1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ////////////////////////////////
    //cycles 2 - PC 이동
    
    //PC 하위 값에 오퍼랜드 가산
    impl->core.lOperand = lowByte(impl->core.reg.pc);
    impl->core.carry = 0;
    RP2A03_alu_add(impl);
    impl->core.reg.pc = makeWord(impl->core.lOperand, highByte(impl->core.reg.pc));
    
    impl->core.instSeq = RP2A03_InstSeq_rel2;

    ////////////////////////////////
}

static
void
RP2A03_instBodySeq_rel2(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ////////////////////////////////
    //cycles 3 - 조건 참
    
    //자리 올림이 발생했으면
    if(!(impl->core.rOperand & 0x80) && impl->core.carry) {
        //PC 상위 값 자리 올림
        impl->core.reg.pc = makeWord(lowByte(impl->core.reg.pc), highByte(impl->core.reg.pc) + 1);
        
        RP2A03_doLastInstructionCycle(impl);
    }
    //자리 내림이 발생했으면
    else if((impl->core.rOperand & 0x80) && (!impl->core.carry)) {
        //PC 상위 값 자리 내림
        impl->core.reg.pc = makeWord(lowByte(impl->core.reg.pc), highByte(impl->core.reg.pc) - 1);
        
        RP2A03_doLastInstructionCycle(impl);
    }
    //자릿수가 그대로이면
    else {
        //이번 사이클에서 읽은 데이터를 opcode로 취급하고 처리
        RP2A03_instBodySeq_decodeOpcode(impl);
    }
    
    ////////////////////////////////
}

static
void
RP2A03_instBodySeq_impBrk(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;
     
    impl->core.instSeq = RP2A03_InstSeq_brk2;
}

static
void
RP2A03_instBodySeq_impStk(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    switch(impl->core.inst) {
    case HEInst_BRK:
        impl->core.instSeq = RP2A03_InstSeq_brk2;
    break;
    case HEInst_PHP:
        impl->core.instSeq = RP2A03_InstSeq_php;
    break;
    case HEInst_PHA:
        impl->core.instSeq = RP2A03_InstSeq_pha;
    break;
    case HEInst_RTI:
        impl->core.instSeq = RP2A03_InstSeq_rti0;
    break;
    case HEInst_RTS:
        impl->core.instSeq = RP2A03_InstSeq_rts0;
    break;    
    case HEInst_PLP:
        impl->core.instSeq = RP2A03_InstSeq_plp0;
    break;
    case HEInst_PLA:
        impl->core.instSeq = RP2A03_InstSeq_pla0;
    break;
    //default:
    //    impl->core.instSeq = KAPHEIN_NULL;
    }
}

static
void
RP2A03_instBodySeq_absJmp(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.pc;

    //하위 주소 읽기
    impl->core.addressVarL = impl->core.ioData;

    switch(impl->core.inst) {
    case HEInst_JMP:
        impl->core.instSeq = RP2A03_InstSeq_jmpAbs0;
    break;
    default:
        impl->core.instSeq = RP2A03_InstSeq_jsr0;
    }
}

static
void
RP2A03_instBodySeq_impSft(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    //A 레지스터를 오퍼랜드로 함
    impl->core.rOperand = impl->core.reg.a;

    #ifdef IMPLEMENT_PIPELINING
    impl->pipelined = true;       //명령어 파이프라이닝
    #else
    RP2A03_executeInstruction(impl);  //명령어 실행
    #endif
}

static
void
RP2A03_instBodySeq_fetchOperandWithCarry(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ////////////////////////////
    //Read, Read-Modify는 캐리가 없는 경우 4 사이클만 소요

    //오퍼랜드 읽기
    impl->core.rOperand = impl->core.ioData;

    //상위 주소 자리 올림
    impl->core.addressVarH += impl->core.carry;

    switch(impl->core.carry) {
        case 0:
            switch(impl->core.instType) {

            #ifdef IMPLEMENT_PIPELINING
            case HEInstType_READ:
                RP2A03_doLastInstructionCycle(impl);        //최종 사이클 공통 처리
                executeInstruction(impl, 0);  //명령어 실행
            break;    
            case HEInstType_READ_MODIFY:
                RP2A03_doLastInstructionCycle(impl);        //최종 사이클 공통 처리
                impl->pipelined = true;       //명령어 파이프라이닝
            //break;
            #else
            case HEInstType_READ:
            case HEInstType_READ_MODIFY:
                RP2A03_executeInstruction(impl);
                
                RP2A03_doLastInstructionCycle(impl);
            break;
            #endif
            case HEInstType_READ_MODIFY_WRITE:
                impl->core.instSeq = RP2A03_InstSeq_fetchOperandWithNoCarry;
            break;
            case HEInstType_WRITE:
                impl->core.instSeq = RP2A03_InstSeq_writeInstWriteToAddressVar;
            break;
            }
        break;
    case 1:
        switch(impl->core.instType) {
        case HEInstType_WRITE:
            impl->core.instSeq = RP2A03_InstSeq_writeInstWriteToAddressVar;
        break;
        default:
            impl->core.instSeq = RP2A03_InstSeq_fetchOperandWithNoCarry;                
        }
    break;
    }

    ////////////////////////////
}

static
void
RP2A03_instBodySeq_fetchOperandWithNoCarry(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ////////////////////////////
    //공통

    switch(impl->core.instType) {
    
    #ifdef IMPLEMENT_PIPELINING
    case HEInstType_READ:
        RP2A03_doLastInstructionCycle(thisObj);        //최종 사이클 공통 처리
        impl->core.rOperand = read(thisObj, impl->core.addressVarL, impl->core.addressVarH);    //오퍼랜드 읽기
        executeInstruction(thisObj, 0);  //명령어 실행
    break;
    case HEInstType_READ_MODIFY:
        RP2A03_doLastInstructionCycle(thisObj);        //최종 사이클 공통 처리
        impl->core.rOperand = read(thisObj, impl->core.addressVarL, impl->core.addressVarH);    //오퍼랜드 읽기
        thisObj->pipelined = true;       //명령어 파이프라이닝
    break;
    #else
    case HEInstType_READ:
    case HEInstType_READ_MODIFY:
        RP2A03_doLastInstructionCycle(impl);

        //오퍼랜드 읽기
        impl->core.rOperand = impl->core.ioData;
        
        RP2A03_executeInstruction(impl);
    break;
    #endif

    case HEInstType_READ_MODIFY_WRITE:
        //오퍼랜드 읽기
        impl->core.rOperand = impl->core.ioData;

        impl->core.instSeq = RP2A03_InstSeq_rmwInstWriteOriginalValue;
    break;
    case HEInstType_WRITE:
        //MUST NEVER reach this case.
        assert(false);
    break;
    }

    ////////////////////////////
}

static
void
RP2A03_instBodySeq_rmwInstFetchOperand(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    KAPHEIN_x_UNUSED_PARAMETER(impl)
}

static
void
RP2A03_instBodySeq_writeToAddressVar(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);
}

static
void
RP2A03_instBodySeq_rmwInstWriteOriginalValue(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    #ifdef IMPLEMENT_PIPELINING
	executeInstruction(impl, 0);		//명령어 실행
    #else
    RP2A03_executeInstruction(impl);
    #endif

    impl->core.instSeq = RP2A03_InstSeq_rmwInstWriteNewValue;
}

static
void
RP2A03_instBodySeq_rmwInstWriteNewValue(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    #ifdef IMPLEMENT_PIPELINING
	executeInstruction(thisObj, 1);      //명령어 실행
    #else
    //executeInstruction(impl);
    #endif

    RP2A03_doLastInstructionCycle(impl);
}

static
void
RP2A03_instBodySeq_jmpInd0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //PC 하위 값 읽기 후 포인터 증가(캐리 처리 안 됨)
    impl->core.addressVarL = impl->core.ioData;
    ++impl->core.pointerVarL;

    impl->core.instSeq = RP2A03_InstSeq_jmpInd1;
}

static
void
RP2A03_instBodySeq_jmpInd1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    //PC 상위 값 읽기
    impl->core.reg.pc = makeWord(impl->core.addressVarL, impl->core.ioData);

    ////////////////////////////////////////
}

static
void
RP2A03_instBodySeq_jmpAbs(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    //주소 상위 값 읽기
    impl->core.reg.pc = makeWord(impl->core.addressVarL, impl->core.ioData);
}

static
void
RP2A03_instBodySeq_jsr0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.instSeq = RP2A03_InstSeq_jsr1;
}

static
void
RP2A03_instBodySeq_jsr1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    --impl->core.reg.s;
    
    impl->core.instSeq = RP2A03_InstSeq_jsr2;
}

static
void
RP2A03_instBodySeq_jsr2(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    --impl->core.reg.s;

    impl->core.instSeq = RP2A03_InstSeq_jsr3;
}

static
void
RP2A03_instBodySeq_jsr3(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    //주소 상위 값 읽기
    impl->core.reg.pc = makeWord(impl->core.addressVarL, impl->core.ioData);
}

static
void
RP2A03_instBodySeq_brk1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.instSeq = RP2A03_InstSeq_brk2;
}

static
void
RP2A03_instBodySeq_brk2(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    --impl->core.reg.s;

    impl->core.instSeq = RP2A03_InstSeq_brk3;
}

static
void
RP2A03_instBodySeq_brk3(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    --impl->core.reg.s;

    impl->core.instSeq = RP2A03_InstSeq_brk4;
}

static
void
RP2A03_instBodySeq_brk4(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    --impl->core.reg.s;

    //인터럽트 폴링, 여기서 하이재킹이 발생
    RP2A03_pollInterrupts(impl);

    impl->core.instSeq = RP2A03_InstSeq_brk5;
}

static
void
RP2A03_instBodySeq_brk5(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.reg.pc = makeWord(impl->core.ioData, highByte(impl->core.reg.pc));

    impl->core.reg.p |= RP2A03_Status_P_DISABLE_IRQ;
    
    impl->core.instSeq = RP2A03_InstSeq_brk6;
}

static
void
RP2A03_instBodySeq_brk6(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.reg.pc = makeWord(lowByte(impl->core.reg.pc), impl->core.ioData);
    
    //인터럽트를 폴링하지 않음
    impl->core.occuredInterrupt = HEInterrupt_NONE;

    impl->core.instSeq = RP2A03_InstSeq_decodeOpcode;
}

static
void
RP2A03_instBodySeq_php(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    --impl->core.reg.s;
}

static
void
RP2A03_instBodySeq_pha(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    --impl->core.reg.s;
}

static
void
RP2A03_instBodySeq_rti0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.s;
    
    impl->core.instSeq = RP2A03_InstSeq_rti1;
}

static
void
RP2A03_instBodySeq_rti1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //P 레지스터 값 pop (U, B 무시) 후 스택 포인터 증가
    impl->core.reg.p = (impl->core.ioData & (~RP2A03_Status_P_UB));
    ++impl->core.reg.s;

    impl->core.instSeq = RP2A03_InstSeq_rti2;
}

static
void
RP2A03_instBodySeq_rti2(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //PC 하위 값 pop 후 스택 포인터 증가
    impl->core.reg.pc = makeWord(impl->core.ioData, highByte(impl->core.reg.pc));
    ++impl->core.reg.s;
    
    impl->core.instSeq = RP2A03_InstSeq_rti3;
}

static
void
RP2A03_instBodySeq_rti3(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    //PC 상위 값 pop
    impl->core.reg.pc = makeWord(lowByte(impl->core.reg.pc), impl->core.ioData);
}

static
void
RP2A03_instBodySeq_rts0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    ++impl->core.reg.s;
    
    impl->core.instSeq = RP2A03_InstSeq_rts1;
}

static
void
RP2A03_instBodySeq_rts1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //PC 하위 값 pop 후 스택 포인터 증가
    impl->core.reg.pc = makeWord(impl->core.ioData, highByte(impl->core.reg.pc));
    ++impl->core.reg.s;

    impl->core.instSeq = RP2A03_InstSeq_rts2;
}

static
void
RP2A03_instBodySeq_rts2(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //PC 상위 값 pop
    impl->core.reg.pc = makeWord(lowByte(impl->core.reg.pc), impl->core.ioData);
    
    impl->core.instSeq = RP2A03_InstSeq_rts3;
}

static
void
RP2A03_instBodySeq_rts3(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    ++impl->core.reg.pc;
}

static
void
RP2A03_instBodySeq_plp0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //쓰레기 값 pop
    ++impl->core.reg.s;

    impl->core.instSeq = RP2A03_InstSeq_plp1;
}

static
void
RP2A03_instBodySeq_plp1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    //P 레지스터 값 pop
    impl->core.reg.p = (impl->core.ioData & (~RP2A03_Status_P_UB));
}

static
void
RP2A03_instBodySeq_pla0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    //쓰레기 값 pop
    ++impl->core.reg.s;
    
    impl->core.instSeq = RP2A03_InstSeq_pla1;
}

static
void
RP2A03_instBodySeq_pla1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    RP2A03_doLastInstructionCycle(impl);

    //A 레지스터 값 pop
    impl->core.reg.a = impl->core.ioData;

    impl->core.lOperand = impl->core.reg.a;
    RP2A03_updateStatusFlagNZ(impl);
}

static
void
RP2A03_instBodySeq_reset0(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.instSeq = RP2A03_InstSeq_reset1;
}

static
void
RP2A03_instBodySeq_reset1(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    impl->core.instSeq = RP2A03_InstSeq_reset2;
}

static
void
RP2A03_instBodySeq_reset2(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    --impl->core.reg.s;
    
    impl->core.instSeq = RP2A03_InstSeq_reset3;
}

static
void
RP2A03_instBodySeq_reset3(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    --impl->core.reg.s;
    
    impl->core.instSeq = RP2A03_InstSeq_reset4;
}

static
void
RP2A03_instBodySeq_reset4(
    struct kaphein_nes_RP2A03_Impl * impl
)
{
    --impl->core.reg.s;

    impl->core.reg.p |= RP2A03_Status_P_UB;
    
    impl->core.instSeq = RP2A03_InstSeq_brk5;
}

/* **************************************************************** */
