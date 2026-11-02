#include "dolphin.h"
#include "dolphin/ax.h"

#include "__ax.h"

static unsigned long __AXSrcCycles[5] = {
    0x00000DF8,
    0x00000F78,
    0x000014B8,
    0x000019F8,
    0x000019F8
};

static unsigned long __AXMixCycles[32] = {
    0x000005BE,
    0x00000B7C,
    0x00000B7C,
    0x0000113A,
    0x000008B6,
    0x00000E74,
    0x00000E74,
    0x00001432,
    0x000009A6,
    0x0000134C,
    0x0000134C,
    0x00001CF2,
    0x00000E97,
    0x0000183D,
    0x0000183D,
    0x000021E3,
    0x00000B7C,
    0x00001432,
    0x00000B7C,
    0x00001432,
    0x00000B7C,
    0x00000B7C,
    0x00000B7C,
    0x00000B7C,
    0x0000134C,
    0x000021E3,
    0x0000134C,
    0x000021E3,
    0x0000134C,
    0x000021E3,
    0x0000134C,
    0x000021E3
};

static AXPB __AXPB[AX_MAX_VOICES] ATTRIBUTE_ALIGN(32);
static AXPBITDBUFFER __AXITD[AX_MAX_VOICES] ATTRIBUTE_ALIGN(32);
static AXPBU __AXUpdates[AX_MAX_VOICES] ATTRIBUTE_ALIGN(32);
static AXVPB __AXVPB[AX_MAX_VOICES];

static u32 __AXMaxDspCycles;
static u32 __AXRecDspCycles;
static u32 __AXNumVoices;

static inline void __AXDumpVPB(AXVPB * pvpb);

// __AXDepopVoice is the last function of AXSPB.c in the SDK; the current
// splits.txt assigns it to this unit, so it lives here and must stay
// out of line (AXSPB.c code in the SDK cannot be inlined into AXVPB.c).
#pragma dont_inline on
void __AXDepopVoice(AXPB * p) {
    __AXSpbAL += p->dpop.aL;
    __AXSpbAAL += p->dpop.aAuxAL;
    __AXSpbABL += p->dpop.aAuxBL;
    __AXSpbAR += p->dpop.aR;
    __AXSpbAAR += p->dpop.aAuxAR;
    __AXSpbABR += p->dpop.aAuxBR;
    __AXSpbAS += p->dpop.aS;
    __AXSpbAAS += p->dpop.aAuxAS;
    __AXSpbABS += p->dpop.aAuxBS;
}
#pragma dont_inline reset

u32 __AXGetNumVoices(void) {
    return __AXNumVoices;
}

void __AXServiceVPB(AXVPB *pvpb) {
    AXPB * ppbDsp;
    AXPB * ppbUser;
    u32 sync;

    ASSERTLINE(0xA1, (pvpb->index >= 0) && (pvpb->index < AX_MAX_VOICES));
    __AXNumVoices += 1;
    ppbDsp = &__AXPB[pvpb->index];
    ppbUser = &pvpb->pb;
    sync = pvpb->sync;
    if (sync == 0) {
        ppbUser->state = ppbDsp->state;
        ppbUser->ve.currentVolume = ppbDsp->ve.currentVolume;
        ppbUser->addr.currentAddressHi = ppbDsp->addr.currentAddressHi;
        ppbUser->addr.currentAddressLo = ppbDsp->addr.currentAddressLo;
        return;
    }
    if (sync & AX_SYNC_FLAG_COPYALL) {
        // copy the whole PB struct. (size: 0xC0)
        u32 *src;
        u32 *dst;
        src = (void*)ppbUser;
        dst = (void*)ppbDsp;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src);
        if (pvpb->updateCounter != 0) {
            u32 count;
            src = (void*)&__AXUpdates[pvpb->index];
            dst = (void*)pvpb->updateData;
            for(count = pvpb->updateCounter; count; count--) {
                *(dst) = *(src); dst+=1; src+=1;
            }
        }
        return;
    }
    if (sync & AX_SYNC_FLAG_COPYSELECT) {
        ppbDsp->srcSelect = ppbUser->srcSelect;
        ppbDsp->coefSelect = ppbUser->coefSelect;
    }
    if (sync & AX_SYNC_FLAG_COPYMXRCTRL) {
        ppbDsp->mixerCtrl = ppbUser->mixerCtrl;
    }
    if (sync & AX_SYNC_FLAG_COPYSTATE) {
        ppbDsp->state = ppbUser->state;
    } else {
        ppbUser->state = ppbDsp->state;
    }
    if (sync & AX_SYNC_FLAG_COPYTYPE) {
        ppbDsp->type = ppbUser->type;
    }
    if (sync & AX_SYNC_FLAG_COPYAXPBMIX) {
        // copy AXPBMIX.
        u16 * src;
        u16 * dst;
        src = (void*)&ppbUser->mix;
        dst = (void*)&ppbDsp->mix;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src);
    }
    if (sync & AX_SYNC_FLAG_COPYTSHIFT) {
        ppbDsp->itd.targetShiftL = ppbUser->itd.targetShiftL;
        ppbDsp->itd.targetShiftR = ppbUser->itd.targetShiftR;
    } else if (sync & AX_SYNC_FLAG_COPYITD) {
        // copy ITD struct.
        u16 * src;
        u16 * dst;
        u32 * dst_;
        src = (void*)&ppbUser->itd;
        dst = (void*)&ppbDsp->itd;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); src+=1;
        dst_ = pvpb->itdBuffer;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0; dst_+=1;
        *(dst_) = 0;
    }
    if (sync & AX_SYNC_FLAG_COPYUPDATE) {
        // copy UPDATE struct.
        u16 * src;
        u16 * dst;
        dst = (void*)&ppbDsp->update;
        src = (void*)&ppbUser->update;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); 
        if (pvpb->updateCounter) {
            u32 * src_;
            u32 * dst_;
            u32 count;

            dst_ = (void*)&__AXUpdates[pvpb->index];
            src_ = (void*)&pvpb->updateData;

            for(count = pvpb->updateCounter; count; count--) {
                *(dst_) = *(src_); dst_+=1; src_+=1;
            }
        }
    }
    if (sync & AX_SYNC_FLAG_COPYDPOP) {
        // copy DPOP struct.
        u16 * src;
        u16 * dst;
        dst = (u16*)&ppbDsp->dpop;
        src = (u16*)&ppbUser->dpop;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src);
    }
    if (sync & AX_SYNC_FLAG_SWAPVOL) {
        ppbUser->ve.currentVolume = ppbDsp->ve.currentVolume;
        ppbDsp->ve.currentDelta = ppbUser->ve.currentDelta;
    } else if (sync & AX_SYNC_FLAG_COPYVOL) {
        ppbDsp->ve.currentVolume = ppbUser->ve.currentVolume;
        ppbDsp->ve.currentDelta = ppbUser->ve.currentDelta;
    }
    if (sync & AX_SYNC_FLAG_COPYFIR) {
        // copy FIR struct.
        u16 * src;
        u16 * dst;
        dst = (void*)&ppbDsp->fir;
        src = (void*)&ppbUser->fir;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src);
    }
    if (sync & (AX_SYNC_FLAG_COPYLOOP | AX_SYNC_FLAG_COPYLOOPADDR | AX_SYNC_FLAG_COPYENDADDR | AX_SYNC_FLAG_COPYCURADDR)) {
        if (sync & AX_SYNC_FLAG_COPYLOOP) {
            ppbDsp->addr.loopFlag = ppbUser->addr.loopFlag;
        }
        if (sync & AX_SYNC_FLAG_COPYLOOPADDR) {
            *(u32*)&ppbDsp->addr.loopAddressHi = *(u32*)&ppbUser->addr.loopAddressHi;
        }
        if (sync & AX_SYNC_FLAG_COPYENDADDR) {
            *(u32*)&ppbDsp->addr.endAddressHi = *(u32*)&ppbUser->addr.endAddressHi;
        }
        if (sync & AX_SYNC_FLAG_COPYCURADDR) {
            *(u32*)&ppbDsp->addr.currentAddressHi = *(u32*)&ppbUser->addr.currentAddressHi;
        }
    } else if (sync & AX_SYNC_FLAG_COPYADDR) {
        // copy ADDR struct.
        u32 * src;
        u32 * dst;
        dst = (void*)&ppbDsp->addr;
        src = (void*)&ppbUser->addr;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); 
    } else {
        ppbUser->addr.currentAddressHi = ppbDsp->addr.currentAddressHi;
        ppbUser->addr.currentAddressLo = ppbDsp->addr.currentAddressLo;
    }
    if (sync & AX_SYNC_FLAG_COPYADPCM) {
        // copy ADPCM struct.
        u32 * src;
        u32 * dst;
        dst = (void*)&ppbDsp->adpcm;
        src = (void*)&ppbUser->adpcm;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
    }
    if (sync & AX_SYNC_FLAG_COPYRATIO) {
        ppbDsp->src.ratioHi = ppbUser->src.ratioHi;
        ppbDsp->src.ratioLo = ppbUser->src.ratioLo;
    } else if (sync & AX_SYNC_FLAG_COPYSRC) {
        // copy SRC struct.
        u16 * src;
        u16 * dst;
        dst = (void*)&ppbDsp->src;
        src = (void*)&ppbUser->src;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
    }
    if (sync & AX_SYNC_FLAG_COPYADPCMLOOP) {
        // copy ADPCMLOOP struct.
        u16 * src;
        u16 * dst;
        dst = (void*)&ppbDsp->adpcmLoop;
        src = (void*)&ppbUser->adpcmLoop;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src); dst+=1; src+=1;
        *(dst) = *(src);
    }
}

static inline void __AXDumpVPB(AXVPB * pvpb) {
    AXPB * ppbDsp;

    ppbDsp = &__AXPB[pvpb->index];
    if (ppbDsp->state == 1) {
        __AXDepopVoice(ppbDsp);
    }
    pvpb->pb.state = ppbDsp->state = ppbDsp->update.updNum[0] 
        = ppbDsp->update.updNum[1] = ppbDsp->update.updNum[2] 
        = ppbDsp->update.updNum[3] = ppbDsp->update.updNum[4] = 0;
    __AXPushCallbackStack(pvpb);
}

void __AXSyncPBs(u32 lessDspCycles) {
    u32 cycles;
    u32 i;
    AXVPB * pvpb;

    __AXNumVoices = 0;
    DCInvalidateRange(__AXPB, sizeof(__AXPB));
    DCInvalidateRange(__AXITD, sizeof(__AXITD));
    cycles = (__AXGetCommandListCycles() + 0x10000) - 0x55F0 + lessDspCycles;
    for(i = 31; i; i--) {
        for (pvpb = __AXGetStackHead(i); pvpb; pvpb = pvpb->next) {
            if (pvpb->depop != 0U) {
                __AXDepopVoice(&__AXPB[pvpb->index]);
            }
            if ((pvpb->pb.state == 1) || (pvpb->updateCounter != 0U)) {
                cycles = __AXSrcCycles[pvpb->pb.src.ratioHi] + __AXMixCycles[pvpb->pb.mixerCtrl] + 0x8C + cycles;
                if (__AXMaxDspCycles > cycles) {
                    __AXServiceVPB(pvpb);
                } else {
                    __AXDumpVPB(pvpb);
                }
            } else {
                __AXServiceVPB(pvpb);
            }
            pvpb->sync = 0;
            pvpb->depop = 0;
            pvpb->updateMS = pvpb->updateCounter = 0;
            pvpb->updateWrite = pvpb->updateData;
        }
    }
    __AXRecDspCycles = cycles;
    for (pvpb = __AXGetStackHead(0); pvpb; pvpb = pvpb->next) {
        if (pvpb->depop != 0U) {
            __AXDepopVoice(&__AXPB[pvpb->index]);
        }
        pvpb->depop = 0;
        __AXPB[pvpb->index].state = __AXPB[pvpb->index].update.updNum[0] = __AXPB[pvpb->index].update.updNum[1] 
                                  = __AXPB[pvpb->index].update.updNum[2] = __AXPB[pvpb->index].update.updNum[3] 
                                  = __AXPB[pvpb->index].update.updNum[4] = 0;
    }
    DCFlushRange(__AXPB, sizeof(__AXPB));
    DCFlushRange(__AXITD, sizeof(__AXITD));
    DCFlushRange(__AXUpdates, sizeof(__AXUpdates));
}

AXPB * __AXGetPBs(void) {
    return __AXPB;
}

void __AXSetPBDefault(AXVPB * p) {
    p->pb.state = 0;
    p->pb.itd.flag = 0;
    p->sync = 0xA4;
    p->updateMS = p->updateCounter = 0;
    p->updateWrite = p->updateData;
    p->pb.update.updNum[0] = p->pb.update.updNum[1] = p->pb.update.updNum[2] = p->pb.update.updNum[3] = p->pb.update.updNum[4] = 0;
}

void __AXVPBInit(void) {
    u32 i;
    AXPB * ppb;
    AXPBITDBUFFER * ppbi;
    AXPBU * ppbu;
    AXVPB * pvpb;
    u32 * p;

#ifdef DEBUG
    OSReport("Initializing AXVPB code module\n");
#endif
    __AXMaxDspCycles = OS_BUS_CLOCK / 400;
    __AXRecDspCycles = 0U;
    p = (u32*)__AXPB;
    for(i = sizeof(__AXPB) / sizeof(u32); i != 0; i--) {
        *p = 0; p++;
    }
    p = (u32*)__AXITD;
    for(i = sizeof(__AXITD) / sizeof(u32); i != 0; i--) {
        *p = 0; p++;
    }
    p = (u32*)__AXVPB;
    for(i = sizeof(__AXVPB) / sizeof(u32); i != 0; i--) {
        *p = 0; p++;
    }
    for(i = 0; i < AX_MAX_VOICES; i++) {
        ppb = &__AXPB[i];
        ppbi = &__AXITD[i];
        ppbu = &__AXUpdates[i];
        pvpb = &__AXVPB[i];
        ASSERTLINE(0x2F6, (u32)ppb ^ 0x1F);
        ASSERTLINE(0x2F7, (u32)ppbi ^ 0x1F);
        ASSERTLINE(0x2F8, (u32)ppbu ^ 0x1F);
        pvpb->index = i;
        pvpb->updateWrite = pvpb->updateData;
        pvpb->itdBuffer = ppbi;
        __AXSetPBDefault(pvpb);
        if (i == 0x3F) {
            pvpb->pb.nextHi = pvpb->pb.nextLo = ppb->nextHi = ppb->nextLo = 0;
        } else {
            pvpb->pb.nextHi = (u16)(  (u32)((char*)ppb + 0xC0) >> 16 );
            pvpb->pb.nextLo = (u16)(  (u32)((char*)ppb + 0xC0) );
            ppb->nextHi = (u16)(  (u32)((char*)ppb + 0xC0) >> 16 );
            ppb->nextLo = (u16)(  (u32)((char*)ppb + 0xC0) );
        }
        pvpb->pb.currHi = (u16)(((u32)ppb) >> 16);
        pvpb->pb.currLo = (u16)((u32)ppb);
        ppb->currHi = (u16)(((u32)ppb) >> 16);
        ppb->currLo = (u16)((u32)ppb);
        pvpb->pb.itd.bufferHi = (u16)(((u32)ppbi) >> 16);
        pvpb->pb.itd.bufferLo = (u16)((u32)ppbi);
        ppb->itd.bufferHi = (u16)(((u32)ppbi) >> 16);
        ppb->itd.bufferLo = (u16)((u32)ppbi);
        pvpb->pb.update.dataHi = (u16)(((u32)ppbu) >> 16);
        pvpb->pb.update.dataLo = (u16)((u32)ppbu);
        ppb->update.dataHi = (u16)(((u32)ppbu) >> 16);
        ppb->update.dataLo = (u16)((u32)ppbu);
        pvpb->priority = 1;
        __AXPushFreeStack(pvpb);
    }
    DCFlushRange(__AXPB, sizeof(__AXPB));
}

void AXSetVoiceState(AXVPB * p, u16 state) {
    int old;

    old = OSDisableInterrupts();
    p->pb.state = state;
    p->sync |= AX_SYNC_FLAG_COPYSTATE;
    if (state == 0) {
        p->depop = 1;
    }
    OSRestoreInterrupts(old);
}
