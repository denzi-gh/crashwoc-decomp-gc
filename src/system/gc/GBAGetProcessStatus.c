#include "dolphin.h"
#include "dolphin/os.h"
#include "dolphin/gba.h"

#include "__gba.h"

s32 GBAGetProcessStatus(s32 chan, u8 *percentp)
{
    GBAControl *gba;
    GBABootInfo *bootInfo;
    BOOL enabled;
    s32 ret;
    u8 percent;
    OSTime t;

    gba = &__GBA[chan];
    bootInfo = &gba->bootInfo;
    enabled = OSDisableInterrupts();
    if (bootInfo->callback) {
        ret = GBA_BUSY;
        percent = (u8)(bootInfo->curOffset * 100 / bootInfo->realLength);
        if (bootInfo->begin != 0) {
            t = OSGetTime() - bootInfo->begin;
            if (OSTicksToMilliseconds(t) < 5500) {
                percent = (u8)(percent * t / OSMillisecondsToTicks((OSTime)5500));
            }
            if (percent >= 100) {
                percent = 100;
            }
        }
        if (percentp) {
            *percentp = percent;
        }
    } else if (gba->callback) {
        ret = GBA_BUSY;
    } else {
        ret = GBA_READY;
    }
    OSRestoreInterrupts(enabled);
    return ret;
}
