#include "dolphin.h"
#include "dolphin/os.h"
#include "dolphin/si.h"
#include "dolphin/gba.h"

#include "__os.h"
#include "__gba.h"

static void __GBAHandler(s32 chan, u32 error, OSContext *context)
{
    GBAControl *gba;
    GBATransferCallback proc;
    GBACallback callback;
    OSContext exceptionContext;

    gba = &__GBA[chan];
    if (__GBAReset) {
        return;
    }
    if (error & 0xF) {
        gba->ret = GBA_NOT_READY;
    } else {
        gba->ret = GBA_READY;
    }
    if (gba->proc) {
        proc = gba->proc;
        gba->proc = NULL;
        proc(chan);
    }
    if (gba->callback) {
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);
        callback = gba->callback;
        gba->callback = NULL;
        callback(chan, gba->ret);
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
    }
}

void __GBASyncCallback(s32 chan, s32 ret)
{
    GBAControl *gba = &__GBA[chan];

    OSWakeupThread(&gba->threadQueue);
}

s32 __GBASync(s32 chan)
{
    GBAControl *gba;
    BOOL enabled;
    s32 ret;

    gba = &__GBA[chan];
    enabled = OSDisableInterrupts();
    while (gba->callback) {
        OSSleepThread(&gba->threadQueue);
    }
    ret = gba->ret;
    OSRestoreInterrupts(enabled);
    return ret;
}

static void TypeAndStatusCallback(s32 chan, u32 type)
{
    GBAControl *gba;
    GBATransferCallback proc;
    GBACallback callback;
    OSContext exceptionContext;
    OSContext *context;

    gba = &__GBA[chan];
    if (__GBAReset) {
        return;
    }
    if ((type & 0xFF) || (type & 0xFFFF0000) != SI_GBA) {
        gba->ret = GBA_NOT_READY;
    } else {
        if (SITransfer(chan, gba->output, gba->outputBytes, gba->input, gba->inputBytes, __GBAHandler, gba->delay)) {
            return;
        }
        gba->ret = GBA_BUSY;
    }
    if (gba->proc) {
        proc = gba->proc;
        gba->proc = NULL;
        proc(chan);
    }
    if (gba->callback) {
        context = OSGetCurrentContext();
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);
        callback = gba->callback;
        gba->callback = NULL;
        callback(chan, gba->ret);
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
        __OSReschedule();
    }
}

s32 __GBATransfer(s32 chan, u32 outputBytes, u32 inputBytes, GBATransferCallback proc)
{
    BOOL enabled;
    GBAControl *gba;

    gba = &__GBA[chan];
    enabled = OSDisableInterrupts();
    gba->proc = proc;
    gba->outputBytes = outputBytes;
    gba->inputBytes = inputBytes;
    SIGetTypeAsync(chan, TypeAndStatusCallback);
    OSRestoreInterrupts(enabled);
    return GBA_READY;
}

OSTime __GBASetDelay(s32 chan, OSTime delay)
{
    GBAControl *gba;
    OSTime oldDelay;

    gba = &__GBA[chan];
    oldDelay = gba->delay;
    gba->delay = delay;
    return oldDelay;
}
