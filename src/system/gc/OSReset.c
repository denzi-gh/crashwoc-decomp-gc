#include "types.h"

typedef int BOOL;
#define TRUE 1
#define FALSE 0

typedef void (*OSResetCallback)(void);

typedef struct OSResetFunctionInfo {
    int (*func)(int);
    u32 priority;
    struct OSResetFunctionInfo* next;
    struct OSResetFunctionInfo* prev;
} OSResetFunctionInfo;

typedef struct OSThread OSThread;
struct OSThread {
    char context[0x2C8];
    u16 state;
    u16 attr;
    s32 suspend;
    s32 priority;
    s32 base;
    void* val;
    void* queue;
    struct { OSThread* next; OSThread* prev; } link;
    struct { OSThread* head; OSThread* tail; } queueJoin;
    void* mutex;
    struct { void* head; void* tail; } queueMutex;
    struct { OSThread* next; OSThread* prev; } linkActive;
};

s32 OSDisableInterrupts(void);
s32 OSRestoreInterrupts(s32);
void __OSStopAudioSystem(void);
void OSDisableScheduler(void);
void OSEnableScheduler(void);
BOOL __PADDisableRecalibration(BOOL);
void ICFlashInvalidate(void);
void LCDisable(void);
BOOL __OSSyncSram(void);
void* __OSLockSram(void);
BOOL __OSUnlockSram(BOOL);
void __OSMaskInterrupts(u32);
void OSCancelThread(OSThread*);
void __OSReboot(u32, u32);
s64 __OSGetSystemTime(void);
s64 __div2i(s64, s64);

#define OS_BUS_CLOCK (*(volatile u32*)0x800000F8)
#define OS_TIMER_CLOCK (OS_BUS_CLOCK / 4)


static OSResetFunctionInfo* ResetFunctionQueue;

static void Reset(u32 resetCode) {
    /* Cannot be implemented in pure C - requires mfspr/mtspr/isync/sync/mftb */
}

void __OSDoHotReset(u32 resetCode) {
    OSDisableInterrupts();
    *(volatile u16*)0xCC002002 = 0;
    ICFlashInvalidate();
    Reset(resetCode << 3);
}

void OSResetSystem(int reset, u32 resetCode, BOOL forceMenu) {
    BOOL padRecal;
    OSResetFunctionInfo* info;
    BOOL retVal;
    BOOL complete;
    OSThread* thread;
    OSThread* next;
    u32* base;

    OSDisableScheduler();
    __OSStopAudioSystem();

    if (reset == 2) {
        padRecal = __PADDisableRecalibration(TRUE);
    }

    while (TRUE) {
        while (TRUE) {
            info = ResetFunctionQueue;
            retVal = FALSE;

            while (TRUE) {
                while (info != NULL) {
                    if (!info->func(FALSE)) {
                        retVal = TRUE;
                    }
                    info = info->next;
                }

                if (!__OSSyncSram()) {
                    retVal = TRUE;
                }

                if (retVal == FALSE) {
                    complete = TRUE;
                } else {
                    complete = FALSE;
                }

                if (complete) break;
                break;
            }
            if (complete) break;
        }
        break;
    }

    if (reset == 1 && forceMenu != 0) {
        u8* sram = (u8*)__OSLockSram();
        sram[0x13] |= 0x40;
        __OSUnlockSram(TRUE);

        while (TRUE) {
            while (!__OSSyncSram())
                ;
            break;
        }
    }

    OSDisableInterrupts();
    info = (OSResetFunctionInfo*)ResetFunctionQueue;
    retVal = FALSE;

    while (TRUE) {
        while (info != NULL) {
            if (!info->func(TRUE)) {
                retVal = TRUE;
            }
            info = info->next;
        }
        break;
    }

    __OSSyncSram();
    LCDisable();

    if (reset == 1) {
        OSDisableInterrupts();
        *(volatile u16*)0xCC002002 = 0;
        ICFlashInvalidate();
        Reset(resetCode << 3);
    } else if (reset == 0) {
        thread = *(OSThread* volatile*)0x800000DC;

        while (TRUE) {
            while (thread != NULL) {
                u16 state = thread->state;
                next = thread->linkActive.next;
                if (state == 4 || state == 1) {
                    OSCancelThread(thread);
                }
                thread = next;
            }
            break;
        }

        OSEnableScheduler();
        __OSReboot(resetCode, forceMenu);
    }

    thread = *(OSThread* volatile*)0x800000DC;

    while (TRUE) {
        while (thread != NULL) {
            u16 state = thread->state;
            next = thread->linkActive.next;
            if (state == 4 || state == 1) {
                OSCancelThread(thread);
            }
            thread = next;
        }
        break;
    }

    base = (u32*)0x80000000;
    memset(base + 0x10, 0, 0x8C);
    memset(base + 0x35, 0, 0x14);
    memset(base + 0x3D, 0, 0x4);
    memset((void*)(base + 0xC00), 0, 0xC0);
    memset((void*)(base + 0xC32), 0, 0xC);

    __PADDisableRecalibration(padRecal);
}

u32 OSGetResetCode(void) {
    if (*(volatile u8*)0x800030E2 != 0) {
        return (u32)0x80000000;
    }
    return (*(volatile u32*)0xCC003024 & ~0x7) >> 3;
}
