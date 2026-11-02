#ifndef _DOLPHIN_AX_INTERNAL_H_
#define _DOLPHIN_AX_INTERNAL_H_

#include "dolphin/ax.h"

// AXAlloc.c
AXVPB * __AXGetStackHead(u32 priority);
void __AXServiceCallbackStack(void);
void __AXAllocInit(void);
void __AXPushFreeStack(AXVPB * p);
void __AXPushCallbackStack(AXVPB * p);
AXVPB * __AXPopCallbackStack(void);
void __AXRemoveFromStack(AXVPB * p);

// AXAux.c
void __AXAuxInit(void);
void __AXGetAuxAInput(u32 * p);
void __AXGetAuxAOutput(u32 * p);
void __AXGetAuxBInput(u32 * p);
void __AXGetAuxBOutput(u32 * p);
void __AXProcessAux(void);

// AXCL.c
extern u32 __AXClMode;

u32 __AXGetCommandListCycles(void);
u32 __AXGetCommandListAddress(void);
void __AXNextFrame(void * sbuffer, void * buffer);
void __AXClInit(void);

// AXOut.c
void __AXOutNewFrame(u32 lessDspCycles);
void __AXOutAiCallback(void);
void __AXOutInitDSP(void);
void __AXOutInit(void);

// AXProf.c
AXPROFILE * __AXGetCurrentProfile(void);

// AXSPB.c
extern long __AXSpbAL;
extern long __AXSpbAR;
extern long __AXSpbAS;
extern long __AXSpbAAL;
extern long __AXSpbAAR;
extern long __AXSpbAAS;
extern long __AXSpbABL;
extern long __AXSpbABR;
extern long __AXSpbABS;

u32 __AXGetStudio(void);
void __AXPrintStudio(void);
void __AXSPBInit(void);

// AXVPB.c
void __AXDepopVoice(AXPB * p);
u32 __AXGetNumVoices(void);
void __AXServiceVPB(AXVPB *pvpb);
void __AXSyncPBs(u32 lessDspCycles);
AXPB * __AXGetPBs(void);
void __AXSetPBDefault(AXVPB * p);
void __AXVPBInit(void);

#endif // _DOLPHIN_AX_INTERNAL_H_
