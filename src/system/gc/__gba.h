#ifndef __GBA_H__
#define __GBA_H__

#include "dolphin/types.h"
#include "dolphin/os.h"
#include "dolphin/dsp.h"
#include "dolphin/gba.h"

typedef void (*GBATransferCallback)(s32 chan);

typedef struct GBASecParam {
    u8 readbuf[4];
    s32 paletteColor;
    s32 paletteSpeed;
    s32 length;
    u32 *out;
    u8 _padding0[12];
    u32 keyA;
    s32 keyB;
    u8 _padding1[24];
} GBASecParam;

typedef struct GBABootInfo {
    s32 paletteColor;
    s32 paletteSpeed;
    u8 *programp;
    s32 length;
    u8 *status;
    GBACallback callback;
    u8 readbuf[4];
    u8 writebuf[4];
    int i;
    OSTick start;
    OSTime begin;
    BOOL firstXfer;
    int curOffset;
    u32 crc;
    u32 dummyWord[7];
    u32 keyA;
    s32 keyB;
    u32 initialCode;
    int realLength;
} GBABootInfo;

typedef struct GBAControl {
    u8 output[5];
    u8 input[5];
    u32 outputBytes;
    u32 inputBytes;
    u8 *status;
    void *ptr;
    GBACallback callback;
    s32 ret;
    OSThreadQueue threadQueue;
    OSTime delay;
    GBATransferCallback proc;
    GBABootInfo bootInfo;
    DSPTaskInfo task;
    GBASecParam *param;
} GBAControl;

extern GBAControl __GBA[GBA_MAX_CHAN];
extern BOOL __GBAReset;

void __GBASyncCallback(s32 chan, s32 ret);
s32 __GBASync(s32 chan);
OSTime __GBASetDelay(s32 chan, OSTime delay);
s32 __GBATransfer(s32 chan, u32 outputBytes, u32 inputBytes, GBATransferCallback proc);
void __GBAX01(s32 chan, s32 ret);
void __GBAX02(s32 chan, u8 *readbuf);

#endif
