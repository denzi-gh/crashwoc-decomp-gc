#include "dolphin.h"
#include "dolphin/os.h"
#include "dolphin/dvd.h"

#include "__os.h"
#include "__dvd.h"

#define OFFSET_APPLOADER 0x2440

u32 RebootResetCode : 0x817FFFFC;
u32 RebootBootDol : 0x817FFFF8;
u8 RebootFlag : 0x800030E2;
void* RebootSaveStart : 0x812FDFF0;
void* RebootSaveEnd : 0x812FDFEC;

static void* SaveStart;
static void* SaveEnd;
static volatile BOOL Prepared;
static u8 Header[32] ATTRIBUTE_ALIGN(32);

static asm void Run(register void* entry) {
    // clang-format off
    fralloc
    bl      OSDisableInterrupts
    bl      ICFlashInvalidate
    sync
    isync
    mtlr    entry
    blr
    // clang-format on
}

static void Callback(s32 result, DVDCommandBlock* block) {
    Prepared = TRUE;
}

static void WaitPrepared(void) {
    while (!Prepared) {
        ;
    }
}

static void WaitRead(DVDCommandBlock* block) {
    while (TRUE) {
        switch (block->state) {
        case DVD_STATE_END:
            return;
        case DVD_STATE_FATAL_ERROR:
        case DVD_STATE_WAITING:
        case DVD_STATE_COVER_CLOSED:
        case DVD_STATE_NO_DISK:
        case DVD_STATE_COVER_OPEN:
        case DVD_STATE_WRONG_DISK:
        case DVD_STATE_MOTOR_STOPPED:
        case DVD_STATE_PAUSING:
        case DVD_STATE_IGNORED:
        case DVD_STATE_CANCELED:
        case DVD_STATE_RETRY:
            __OSDoHotReset(*(u32*)0x817FFFFC);
            break;
        }
    }
}

void __OSReboot(u32 resetCode, u32 bootDol) {
    OSContext exceptionContext;
    DVDCommandBlock block;
    DVDCommandBlock block2;
    u32 offset;
    u32 size;

    OSDisableInterrupts();

    RebootResetCode = resetCode;
    RebootBootDol = 0;
    RebootFlag = 1;
    RebootSaveStart = SaveStart;
    RebootSaveEnd = SaveEnd;

    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);

    DVDInit();
    DVDSetAutoInvalidation(TRUE);
    __DVDPrepareResetAsync(Callback);
    if (!DVDCheckDisk()) {
        __OSDoHotReset(*(u32*)0x817FFFFC);
    }
    __OSMaskInterrupts(0xFFFFFFE0);
    __OSUnmaskInterrupts(0x400);
    OSEnableInterrupts();

    WaitPrepared();

    DVDReadAbsAsyncForBS(&block, Header, 32, OFFSET_APPLOADER, NULL);
    WaitRead(&block);
    offset = *(u32*)&Header[0x14] + 0x20;
    size = OSRoundUp32B(*(u32*)&Header[0x18]);

    WaitPrepared();

    DVDReadAbsAsyncForBS(&block2, (void*)0x81300000, size, OFFSET_APPLOADER + offset, NULL);
    WaitRead(&block2);
    ICInvalidateRange((void*)0x81300000, size);
    Run((void*)0x81300000);
}
