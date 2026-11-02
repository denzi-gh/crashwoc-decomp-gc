#include "dolphin.h"
#include "dolphin/os.h"
#include "dolphin/gba.h"
#include "string.h"

#include "__gba.h"

static void ReadProc(s32 chan)
{
    GBAControl *gba = &__GBA[chan];

    if (gba->ret == GBA_READY) {
        memcpy(gba->ptr, gba->input, 4);
        *gba->status = gba->input[4] & GBA_JSTAT_MASK;
    }
}

s32 GBAReadAsync(s32 chan, u8 *dst, u8 *status, GBACallback callback)
{
    GBAControl *gba = &__GBA[chan];

    if (gba->callback) {
        return GBA_BUSY;
    }
    gba->output[0] = 0x14;
    gba->ptr = dst;
    gba->status = status;
    gba->callback = callback;
    return __GBATransfer(chan, 1, 5, ReadProc);
}

s32 GBARead(s32 chan, u8 *dst, u8 *status)
{
    s32 tmp;
    GBAControl *gba = &__GBA[chan];
    s32 ret;

    ret = GBAReadAsync(chan, dst, status, __GBASyncCallback);
    if (ret != GBA_READY) {
        return ret;
    }
    return __GBASync(chan);
}
