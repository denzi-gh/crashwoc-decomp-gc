#include "dolphin.h"
#include "dolphin/os.h"

#include "__os.h"

static BOOL OnReset(BOOL final);

static OSResetFunctionInfo ResetFunctionInfo = { OnReset, 127 };

static BOOL OnReset(BOOL final) {
    if (final != FALSE) {
        __MEMRegs[8] = 0xFF;
        __OSMaskInterrupts(OS_INTERRUPTMASK_MEM_0 | OS_INTERRUPTMASK_MEM_1 | OS_INTERRUPTMASK_MEM_2 | OS_INTERRUPTMASK_MEM_3);
    }
    return TRUE;
}

static void MEMIntrruptHandler(__OSInterrupt interrupt, OSContext* context) {
    u32 addr;
    u32 cause;

    cause = __MEMRegs[15];
    addr = (u32)(__MEMRegs[0x12] & 0x3FF) << 16 | __MEMRegs[0x11];
    __MEMRegs[16] = 0;
    if (__OSErrorTable[OS_ERROR_PROTECTION]) {
        __OSErrorTable[OS_ERROR_PROTECTION](OS_ERROR_PROTECTION, context, cause, addr);
    } else {
        __OSUnhandledException(OS_ERROR_PROTECTION, context, cause, addr);
    }
}

static asm void Config24MB(void) {
    // clang-format off
    nofralloc
    li      r7, 0
    lis     r4, 0x0000
    addi    r4, r4, 0x0002
    lis     r3, 0x8000
    addi    r3, r3, 0x01ff
    lis     r6, 0x0100
    addi    r6, r6, 0x0002
    lis     r5, 0x8100
    addi    r5, r5, 0x00ff
    isync
    mtspr   DBAT0U, r7
    mtspr   DBAT0L, r4
    mtspr   DBAT0U, r3
    isync
    mtspr   IBAT0U, r7
    mtspr   IBAT0L, r4
    mtspr   IBAT0U, r3
    isync
    mtspr   DBAT2U, r7
    mtspr   DBAT2L, r6
    mtspr   DBAT2U, r5
    isync
    mtspr   IBAT2U, r7
    mtspr   IBAT2L, r6
    mtspr   IBAT2U, r5
    isync
    mfmsr   r3
    ori     r3, r3, MSR_DR | MSR_IR
    mtsrr1  r3
    mflr    r3
    mtsrr0  r3
    rfi
    // clang-format on
}

static asm void Config48MB(void) {
    // clang-format off
    nofralloc
    li      r7, 0
    lis     r4, 0x0000
    addi    r4, r4, 0x0002
    lis     r3, 0x8000
    addi    r3, r3, 0x03ff
    lis     r6, 0x0200
    addi    r6, r6, 0x0002
    lis     r5, 0x8200
    addi    r5, r5, 0x01ff
    isync
    mtspr   DBAT0U, r7
    mtspr   DBAT0L, r4
    mtspr   DBAT0U, r3
    isync
    mtspr   IBAT0U, r7
    mtspr   IBAT0L, r4
    mtspr   IBAT0U, r3
    isync
    mtspr   DBAT2U, r7
    mtspr   DBAT2L, r6
    mtspr   DBAT2U, r5
    isync
    mtspr   IBAT2U, r7
    mtspr   IBAT2L, r6
    mtspr   IBAT2U, r5
    isync
    mfmsr   r3
    ori     r3, r3, MSR_DR | MSR_IR
    mtsrr1  r3
    mflr    r3
    mtsrr0  r3
    rfi
    // clang-format on
}

static asm void RealMode(register u32 addr) {
    // clang-format off
    nofralloc
    clrlwi  addr, addr, 2
    mtsrr0  addr
    mfmsr   r3
    rlwinm  r3, r3, 0, 28, 25
    mtsrr1  r3
    rfi
    // clang-format on
}

void __OSInitMemoryProtection(void) {
    u8 padding[48];
    BOOL enabled;
    u32 simulatedSize;

    simulatedSize = __OSSimulatedMemSize;

    enabled = OSDisableInterrupts();

    if (simulatedSize <= 0x1800000) {
        RealMode((u32)Config24MB);
    } else if (simulatedSize <= 0x3000000) {
        RealMode((u32)Config48MB);
    }

    __MEMRegs[16] = 0;
    __MEMRegs[8] = 0xFF;
    __OSMaskInterrupts(OS_INTERRUPTMASK_MEM_0 | OS_INTERRUPTMASK_MEM_1 | OS_INTERRUPTMASK_MEM_2 | OS_INTERRUPTMASK_MEM_3);

    __OSSetInterruptHandler(__OS_INTERRUPT_MEM_0, MEMIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_MEM_1, MEMIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_MEM_2, MEMIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_MEM_3, MEMIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_MEM_ADDRESS, MEMIntrruptHandler);

    OSRegisterResetFunction(&ResetFunctionInfo);

    simulatedSize = __OSSimulatedMemSize;
    if (simulatedSize < __OSPhysicalMemSize && simulatedSize == 0x1800000) {
        __MEMRegs[20] = 2;
    }

    __OSUnmaskInterrupts(OS_INTERRUPTMASK_MEM_ADDRESS);

    OSRestoreInterrupts(enabled);
}
