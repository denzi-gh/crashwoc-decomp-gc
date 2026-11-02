#include "types.h"

typedef int BOOL;
#define TRUE 1
#define FALSE 0

typedef void (*OSResetCallback)(void);

s32 OSDisableInterrupts(void);
s32 OSRestoreInterrupts(s32);
void __OSMaskInterrupts(u32);
s64 __OSGetSystemTime(void);

#define OS_BUS_CLOCK (*(volatile u32*)0x800000F8)
#define OS_TIMER_CLOCK (OS_BUS_CLOCK / 4)

extern s64 __OSStartTime;

static OSResetCallback ResetCallback;
static BOOL Down;
static BOOL LastState;
static s64 HoldUp;
static s64 HoldDown;

void __OSResetSWInterruptHandler(void) {
    s64 now;
    u32 threshold;

    now = __OSGetSystemTime();
    HoldDown = now;

    threshold = OS_TIMER_CLOCK / 10000;

    while (TRUE) {
        s64 elapsed;
        now = __OSGetSystemTime();
        elapsed = now - HoldDown;

        if (elapsed >= (s64)threshold) break;
        if (*(volatile u32*)0xCC003000 & 0x00010000) break;
    }

    if (!(*(volatile u32*)0xCC003000 & 0x00010000)) {
        Down = TRUE;
        LastState = TRUE;
        __OSMaskInterrupts(0x200);

        if (ResetCallback != NULL) {
            OSResetCallback cb = ResetCallback;
            ResetCallback = NULL;
            cb();
        }
    }

    *(volatile u32*)0xCC003000 = 2;
}

BOOL OSGetResetButtonState(void) {
    BOOL result;
    s32 old;
    s64 now;

    old = OSDisableInterrupts();
    now = __OSGetSystemTime();

    if (!(*(volatile u32*)0xCC003000 & 0x00010000)) {
        if (Down == 0) {
            BOOL valid = TRUE;
            if (HoldUp != 0) {
                valid = FALSE;
            }
            Down = TRUE;
            result = valid;
            HoldDown = now;
        } else {
            BOOL valid = TRUE;
            if (HoldUp == 0) {
                u32 busClock = OS_BUS_CLOCK;
                u32 timerClock = busClock >> 2;
                u32 ticks100 = timerClock / 125000;
                s64 elapsed = now - HoldDown;
                ticks100 *= 100;
                ticks100 >>= 3;
                if (elapsed < (s64)ticks100) {
                    valid = FALSE;
                }
            }
            if (valid) {
                result = TRUE;
            } else {
                result = FALSE;
            }
        }
    } else {
        if (Down != 0) {
            s32 last = LastState;
            Down = 0;
            result = last;
            if (last) {
                HoldUp = now;
            } else {
                HoldUp = 0;
            }
        } else {
            if (HoldUp != 0) {
                u32 busClock = *(volatile u32*)0x800000F8;
                u32 timerClock = busClock >> 2;
                u32 holdThreshold = timerClock / 1000 * 40;
                s64 elapsed = now - HoldUp;

                if (elapsed >= (s64)holdThreshold) {
                    result = TRUE;
                } else {
                    HoldUp = 0;
                    result = FALSE;
                }
            } else {
                HoldUp = 0;
                result = FALSE;
            }
        }
    }

    LastState = result;

    {
        u8 bootByte = *(volatile u8*)0x800030E3;
        if (bootByte & 0x3F) {
            u32 timerClock = OS_TIMER_CLOCK;
            s64 deadline = __OSStartTime + (s64)timerClock * (s64)(bootByte & 0x3F) * 60;
            if (now >= deadline) {
                s64 diff = now - deadline;
                s64 halved = diff / timerClock / 2;
                if ((halved & 1) == 0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }
            }
        }
    }

    OSRestoreInterrupts(old);
    return result;
}
