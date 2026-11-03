#include "dolphin/exi.h"
#include "dolphin/os.h"
#include "macros.h"

// internal include
#include "__os.h"

static struct SramControl Scb ATTRIBUTE_ALIGN(32);

static int ReadSram(void * buffer);
static void WriteSramCallback();
static int WriteSram(void * buffer, unsigned long offset, unsigned long size);
static void * LockSram(unsigned long offset);
static int UnlockSram(int commit, unsigned long offset);

static int ReadSram(void * buffer) {
    int err;
    unsigned long cmd;

    DCInvalidateRange(buffer, 0x40);
    if (!EXILock(0, 1, NULL) ) {
        return 0;
    }
    if (!EXISelect(0, 1, 3)) {
        EXIUnlock(0);
        return 0;
    }
    cmd = 0x20000100;
    err = 0;
    err |= !EXIImm(0, &cmd, 4, 1, 0);
    err |= !EXISync(0);
    err |= !EXIDma(0, buffer, 0x40, 0, NULL);
    err |= !EXISync(0);
    err |= !EXIDeselect(0);
    EXIUnlock(0);
    return !err;
}

static void WriteSramCallback() {
    int unused;
    ASSERTLINE(0xF0, !Scb.locked);
    Scb.sync = WriteSram(&Scb.sram[Scb.offset], Scb.offset, 0x40 - Scb.offset);
    if (Scb.sync != 0) {
        Scb.offset = 0x40;
    }
    ASSERTLINE(0xF6, Scb.sync);
}

static int WriteSram(void * buffer, unsigned long offset, unsigned long size) {
    int err;
    unsigned long cmd;

    if (!EXILock(0, 1, WriteSramCallback)) {
        return 0;
    }
    if (!EXISelect(0, 1, 3)) {
        EXIUnlock(0);
        return 0;
    }
    offset <<= 6;
    cmd = ((offset + 0x100) | 0xA0000000);
    err = 0;
    err |= !EXIImm(0, &cmd, 4, 1, 0);
    err |= !EXISync(0);
    err |= !EXIImmEx(0, buffer, size, 1);
    err |= !EXIDeselect(0);
    EXIUnlock(0);
    return !err;
}

void __OSInitSram() {
    Scb.locked = Scb.enabled = 0;
    Scb.sync = ReadSram(&Scb);
    ASSERTLINE(0x12C, Scb.sync);
    Scb.offset = 0x40;
}

static void * LockSram(unsigned long offset) {
    int enabled;

    enabled = OSDisableInterrupts();
    ASSERTLINE(0x140, !Scb.locked);
    if (Scb.locked) {
        OSRestoreInterrupts(enabled);
        return NULL;
    }
    Scb.enabled = enabled;
    Scb.locked = 1;
    return &Scb.sram[offset];
}

struct OSSram * __OSLockSram() {
    return (struct OSSram *)LockSram(0);
}

struct OSSramEx * __OSLockSramEx(void) {
    return (struct OSSramEx *)LockSram(0x14);
}

static int UnlockSram(int commit, unsigned long offset) {
    unsigned short * p;

    ASSERTLINE(0x162, Scb.locked);
    if (commit != 0) {
        if (offset == 0) {
            struct OSSram * sram  = (struct OSSram *)&Scb.sram[0];
            if (2 < (u32)(sram->flags & 3)) {
                sram->flags = sram->flags & ~3;
            }
            sram->checkSum = sram->checkSumInv = 0;
            for(p = (unsigned short*)&sram->counterBias; p < ((u16*)&Scb.sram[sizeof (struct OSSram)]); p++) {
                sram->checkSum += *p;
                sram->checkSumInv += ~(*p);
            }
        }
        if (offset < Scb.offset) {
            Scb.offset = offset;
        }
        Scb.sync = WriteSram(&Scb.sram[Scb.offset], Scb.offset, 0x40 - Scb.offset);
        if (Scb.sync != 0) {
            Scb.offset = 0x40;
        }
    }
    Scb.locked = 0;
    OSRestoreInterrupts(Scb.enabled);
    return Scb.sync;
}

int __OSUnlockSram(int commit) {
    UnlockSram(commit, 0);
}

int __OSUnlockSramEx(int commit) {
    UnlockSram(commit, 0x14);
}

int __OSSyncSram() {
    return Scb.sync;
}

u16 OSGetWirelessID(s32 chan) {
    struct OSSramEx * sram;
    u16 id;

    sram = __OSLockSramEx();
    id = sram->wirelessPadID[chan];
    __OSUnlockSramEx(0);
    return id;
}

void OSSetWirelessID(s32 chan, u16 id) {
    struct OSSramEx * sram;

    sram = __OSLockSramEx();
    if (sram->wirelessPadID[chan] != id) {
        sram->wirelessPadID[chan] = id;
        __OSUnlockSramEx(1);
    } else {
        __OSUnlockSramEx(0);
    }
}
