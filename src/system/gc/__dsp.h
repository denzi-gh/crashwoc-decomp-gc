#ifndef _DOLPHIN_DSP_INTERNAL_H_
#define _DOLPHIN_DSP_INTERNAL_H_

#include "dolphin/os.h"
#include "dolphin/dsp.h"

extern DSPTaskInfo *__DSP_curr_task;
extern DSPTaskInfo *__DSP_last_task;
extern DSPTaskInfo *__DSP_first_task;
extern DSPTaskInfo *__DSP_tmp_task;
extern DSPTaskInfo *__DSP_rude_task;
extern int __DSP_rude_task_pending;

void __DSPHandler(__OSInterrupt, OSContext *);
void __DSP_exec_task(DSPTaskInfo *, DSPTaskInfo *);
void __DSP_boot_task(DSPTaskInfo *);
void __DSP_insert_task(DSPTaskInfo *);
void __DSP_remove_task(DSPTaskInfo *task);
void __DSP_debug_printf(const char *fmt, ...);

#endif // _DOLPHIN_DSP_INTERNAL_H_
