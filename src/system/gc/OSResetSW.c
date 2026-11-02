#include "dolphin.h"
#include "dolphin/os.h"

#include "__os.h"

static OSResetCallback ResetCallback;
static int Down;
static int LastState;
static long long HoldUp;
static long long HoldDown;

void __OSResetSWInterruptHandler(short exception, struct OSContext *context) {
    OSResetCallback callback;

    HoldDown = __OSGetSystemTime();
    while (__OSGetSystemTime() - HoldDown < OSMicrosecondsToTicks(100) && !(__PIRegs[0] & 0x10000)) {
        ;
    }
    if (!(__PIRegs[0] & 0x10000)) {
        Down = 1;
        LastState = 1;
        __OSMaskInterrupts(0x200);

        if (ResetCallback) {
            callback = ResetCallback;
            ResetCallback = NULL;
            callback();
        }
    }
    __PIRegs[0] = 2;
}

int OSGetResetButtonState() {
    int enabled;
    int state;
    long long now;
    long long fire;
    int minutes;

    enabled = OSDisableInterrupts();
    now = __OSGetSystemTime();

    if (!(__PIRegs[0] & 0x10000)) {
        if (!Down) {
            Down = 1;
            state = (HoldUp) ? 1 : 0;
            HoldDown = now;
        } else {
            state = (HoldUp || OSMicrosecondsToTicks(100) < (now - HoldDown)) ? 1 : 0;
        }
    } else {
        if (Down) {
            Down = 0;
            state = LastState;
            if (state) {
                HoldUp = now;
            } else {
                HoldUp = 0;
            }
        } else if (HoldUp && (now - HoldUp) < OSMillisecondsToTicks(40)) {
            state = 1;
        } else {
            state = 0;
            HoldUp = 0;
        }
    }
    LastState = state;

    minutes = *(u8*)OSPhysicalToCached(0x30E3) & 0x3F;
    if (minutes) {
        fire = __OSStartTime + OSSecondsToTicks((long long)(minutes * 60));
        if (fire < now) {
            if ((((now - fire) / OS_TIMER_CLOCK / 2) & 1) == 0) {
                state = 1;
            } else {
                state = 0;
            }
        }
    }

    OSRestoreInterrupts(enabled);
    return state;
}
