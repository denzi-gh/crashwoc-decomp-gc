#include "dolphin.h"
#include "dolphin/os.h"
#include "dolphin/db.h"
#include "dolphin/hw_regs.h"

#include "__os.h"

#define EXI_CHAN_2_STAT    (0x28 / 4)
#define EXI_CHAN_2_CONTROL (0x34 / 4)
#define EXI_CHAN_2_IMM     (0x38 / 4)

#define ROUND_UP(x, align) (((x) + (align) - 1) & (-(align)))

static MTRCallbackType MTRCallback;
static void (*DBGCallback)(u32, OSContext *);

static u32 SendMailData;
static s32 RecvDataLeng;

static u8 *pEXIInputFlag;
static u8 EXIInputFlag;

static u8 SendCount = 0x80;

static void DBGEXIInit(void)
{
    __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_2);
    __EXIRegs[EXI_CHAN_2_STAT] = 0;
}

static BOOL DBGEXISelect(u32 v)
{
    u32 regs = __EXIRegs[EXI_CHAN_2_STAT];
    regs &= 0x405;
    regs |= 0x80 | (v << 4);
    __EXIRegs[EXI_CHAN_2_STAT] = regs;
    return TRUE;
}

static BOOL DBGEXIDeselect(void)
{
    __EXIRegs[EXI_CHAN_2_STAT] &= 0x405;
    return TRUE;
}

static BOOL DBGEXISync(void)
{
    while (__EXIRegs[EXI_CHAN_2_CONTROL] & 1) {
    }
    return TRUE;
}

static BOOL DBGEXIImm(void *buffer, s32 bytecounter, u32 write)
{
    u8 *tempPointer;
    u32 writeOutValue;
    int i;

    if (write) {
        tempPointer = buffer;
        writeOutValue = 0;
        for (i = 0; i < bytecounter; i++) {
            u8 *temp = ((u8 *)buffer) + i;
            writeOutValue |= *temp << ((3 - i) << 3);
        }
        __EXIRegs[EXI_CHAN_2_IMM] = writeOutValue;
    }

    __EXIRegs[EXI_CHAN_2_CONTROL] = 1 | write << 2 | (bytecounter - 1) << 4;
    DBGEXISync();

    if (!write) {
        writeOutValue = __EXIRegs[EXI_CHAN_2_IMM];
        tempPointer = buffer;
        for (i = 0; i < bytecounter; i++) {
            *tempPointer++ = writeOutValue >> ((3 - i) << 3);
        }
    }
    return TRUE;
}

static BOOL DBGWriteMailbox(u32 p1)
{
    BOOL total = FALSE;
    u32 v;

    if (!DBGEXISelect(4)) {
        return FALSE;
    }
    v = (p1 & 0x1FFFFFFF) | 0xC0000000;
    total |= !DBGEXIImm(&v, sizeof(v), 1);
    total |= !DBGEXISync();
    total |= !DBGEXIDeselect();
    return !total;
}

static BOOL DBGReadMailbox(u32 *p1)
{
    BOOL total = FALSE;
    u32 v;

    if (!DBGEXISelect(4)) {
        return FALSE;
    }
    v = 0x60000000;
    total |= !DBGEXIImm(&v, 2, 1);
    total |= !DBGEXISync();
    total |= !DBGEXIImm(p1, 4, 0);
    total |= !DBGEXISync();
    total |= !DBGEXIDeselect();
    return !total;
}

static BOOL DBGRead(u32 count, void *buffer, s32 param3)
{
    BOOL total = FALSE;
    u32 *buf_p = (u32 *)buffer;
    u32 v1;
    u32 v;

    if (!DBGEXISelect(4)) {
        return FALSE;
    }
    v1 = (count & 0x1FFFC) << 8 | 0x20000000;
    total |= !DBGEXIImm(&v1, sizeof(v1), 1);
    total |= !DBGEXISync();
    while (param3) {
        total |= !DBGEXIImm(&v, sizeof(v), 0);
        total |= !DBGEXISync();
        *buf_p++ = v;
        param3 -= 4;
        if (param3 < 0) {
            param3 = 0;
        }
    }
    total |= !DBGEXIDeselect();
    return !total;
}

static BOOL DBGWrite(u32 count, void *buffer, s32 param3)
{
    BOOL total = FALSE;
    u32 *buf_p = (u32 *)buffer;
    u32 v1;
    u32 v;

    if (!DBGEXISelect(4)) {
        return FALSE;
    }
    v1 = (count & 0x1FFFC) << 8 | 0xA0000000;
    total |= !DBGEXIImm(&v1, sizeof(v1), 1);
    total |= !DBGEXISync();
    while (param3 != 0) {
        v = *buf_p++;
        total |= !DBGEXIImm(&v, sizeof(v), 1);
        total |= !DBGEXISync();
        param3 -= 4;
        if (param3 < 0) {
            param3 = 0;
        }
    }
    total |= !DBGEXIDeselect();
    return !total;
}

static BOOL DBGReadStatus(u32 *p1)
{
    BOOL total = FALSE;
    u32 v;

    if (!DBGEXISelect(4)) {
        return FALSE;
    }
    v = 1 << 30;
    total |= !DBGEXIImm((u8 *)&v, 2, 1);
    total |= !DBGEXISync();
    total |= !DBGEXIImm(p1, 4, 0);
    total |= !DBGEXISync();
    total |= !DBGEXIDeselect();
    return !total;
}

static void MWCallback(u32 a, OSContext *b)
{
    EXIInputFlag = TRUE;
    if (MTRCallback) {
        MTRCallback(0);
    }
}

static void DBGHandler(s16 a, OSContext *b)
{
    *__PIRegs = 0x1000;
    if (DBGCallback) {
        DBGCallback(a, b);
    }
}

void DBInitComm(u8 **a, MTRCallbackType b)
{
    BOOL interrupts = OSDisableInterrupts();
    {
        pEXIInputFlag = &EXIInputFlag;
        *a = pEXIInputFlag;
        MTRCallback = b;
        DBGEXIInit();
    }
    OSRestoreInterrupts(interrupts);
}

void DBInitInterrupts(void)
{
    __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_2);
    __OSMaskInterrupts(OS_INTERRUPTMASK_PI_DEBUG);
    DBGCallback = &MWCallback;
    __OSSetInterruptHandler(__OS_INTERRUPT_PI_DEBUG, DBGHandler);
    __OSUnmaskInterrupts(OS_INTERRUPTMASK_PI_DEBUG);
}

static void CheckMailBox(void)
{
    u32 v;

    DBGReadStatus(&v);
    if (v & 1) {
        DBGReadMailbox(&v);
        v &= 0x1FFFFFFF;
        if ((v & 0x1F000000) == 0x1F000000) {
            SendMailData = v;
            RecvDataLeng = v & 0x7FFF;
            EXIInputFlag = 1;
        }
    }
}

int DBQueryData(void)
{
    BOOL interrupts;

    EXIInputFlag = 0;
    if (RecvDataLeng == 0) {
        interrupts = OSDisableInterrupts();
        CheckMailBox();
    }
    // Retail restores unconditionally (interrupts is uninitialized when
    // RecvDataLeng != 0); moving the call inside the branch does not match.
    OSRestoreInterrupts(interrupts);
    return RecvDataLeng;
}

BOOL DBRead(void *buffer, int count)
{
    u32 interrupts = OSDisableInterrupts();
    u32 v = SendMailData & 0x10000 ? 0x1000 : 0;

    DBGRead(v + 0x1E000, buffer, ROUND_UP(count, 4));
    RecvDataLeng = 0;
    EXIInputFlag = 0;
    OSRestoreInterrupts(interrupts);
    return 0;
}

BOOL DBWrite(const void *src, int size)
{
    u32 v;
    u32 busyFlag;
    BOOL interrupts;

    interrupts = OSDisableInterrupts();
    do {
        DBGReadStatus(&busyFlag);
    } while (busyFlag & 2);

    SendCount++;
    v = ((SendCount & 1) ? 0x1000 : 0);
    while (!DBGWrite(v | 0x1C000, (u32 *)src, ROUND_UP(size, 4))) {
    }

    do {
        DBGReadStatus(&busyFlag);
    } while (busyFlag & 2);

    v = (SendCount << 0x10) | 0x1F000000 | size;
    while (!DBGWriteMailbox(v)) {
    }

    do {
        while (!DBGReadStatus(&busyFlag)) {
        }
    } while (busyFlag & 2);

    OSRestoreInterrupts(interrupts);
    return 0;
}

void DBOpen(void)
{
}

void DBClose(void)
{
}
