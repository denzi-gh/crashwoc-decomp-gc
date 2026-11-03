#include "dolphin.h"
#include "dolphin/os.h"
#include "dolphin/dsp.h"
#include "dolphin/gba.h"

#include "__gba.h"

static BOOL OnReset(BOOL final);

static OSResetFunctionInfo ResetFunctionInfo = { OnReset, 127, NULL, NULL };

static GBASecParam SecParams[GBA_MAX_CHAN] ATTRIBUTE_ALIGN(32);
GBAControl __GBA[GBA_MAX_CHAN];

BOOL __GBAReset;

static void ShortCommandProc(s32 chan)
{
    GBAControl *gba = &__GBA[chan];

    if (gba->ret == GBA_READY) {
        if (gba->input[0] != 0 || gba->input[1] != 4) {
            gba->ret = GBA_NOT_READY;
        } else {
            *gba->status = gba->input[2] & GBA_JSTAT_MASK;
        }
    }
}

void GBAInit(void)
{
    s32 chan;
    GBAControl *gba;

    for (chan = 0; chan < GBA_MAX_CHAN; chan++) {
        gba = &__GBA[chan];
        gba->delay = OSMicrosecondsToTicks(60);
        OSInitThreadQueue(&gba->threadQueue);
        gba->param = &SecParams[chan];
        ASSERTLINE(90, (u32)gba->param % 32 == 0);
    }
    OSInitAlarm();
    DSPInit();
    __GBAReset = FALSE;
    OSRegisterResetFunction(&ResetFunctionInfo);
}

s32 GBAGetStatusAsync(s32 chan, u8 *status, GBACallback callback)
{
    GBAControl *gba = &__GBA[chan];

    if (gba->callback) {
        return GBA_BUSY;
    }
    gba->output[0] = 0x00;
    gba->status = status;
    gba->callback = callback;
    return __GBATransfer(chan, 1, 3, ShortCommandProc);
}

s32 GBAGetStatus(s32 chan, u8 *status)
{
    GBAControl *gba = &__GBA[chan];
    s32 ret;

    ret = GBAGetStatusAsync(chan, status, __GBASyncCallback);
    if (ret != GBA_READY) {
        return ret;
    }
    return __GBASync(chan);
}

s32 GBAResetAsync(s32 chan, u8 *status, GBACallback callback)
{
    GBAControl *gba = &__GBA[chan];

    if (gba->callback) {
        return GBA_BUSY;
    }
    gba->output[0] = 0xFF;
    gba->status = status;
    gba->callback = callback;
    return __GBATransfer(chan, 1, 3, ShortCommandProc);
}

s32 GBAReset(s32 chan, u8 *status)
{
    GBAControl *gba = &__GBA[chan];
    s32 ret;

    ret = GBAResetAsync(chan, status, __GBASyncCallback);
    if (ret != GBA_READY) {
        return ret;
    }
    return __GBASync(chan);
}

static BOOL OnReset(BOOL final)
{
    __GBAReset = TRUE;
    return TRUE;
}
