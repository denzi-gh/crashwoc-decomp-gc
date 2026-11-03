#include "dolphin.h"
#include "dolphin/ax.h"

#include "__ax.h"

void AXInit(void) {
#ifdef DEBUG
    OSReport("Initializing AX\n");
#endif
    __AXAllocInit();
    __AXVPBInit();
    __AXSPBInit();
    __AXAuxInit();
    __AXClInit();
    __AXOutInit();
}
