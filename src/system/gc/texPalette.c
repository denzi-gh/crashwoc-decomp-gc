#include "dolphin/dvd.h"
#include "dolphin/gx.h"
#include "dolphin/os.h"
#include "charPipeline/texPalette.h"
#include "macros.h"

TEXDescriptorPtr TEXGet(TEXPalettePtr pal, u32 id)
{
    ASSERTMSGLINE(0x90, id < pal->numDescriptors, "GetTexture():  Texture Not Found ");
    return &pal->descriptorArray[id];
}
