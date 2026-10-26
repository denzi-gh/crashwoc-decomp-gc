#include "system/crashlib.h"



//MATCH GCN
unsigned long timeGetTime(void) {
	return 0;
    //return OSGetTick(); //SDK GCN
}

//MATCH GCN
void DBTimerStart(int index) {
    unsigned long time;

    time = timeGetTime();
    DBTimers[index].start = time;
    return;
}

//MATCH GCN
void DBTimerEnd(int index) {
    unsigned long avg;
    unsigned long time;
    unsigned long elapsed;

    time = timeGetTime();
    avg = DBTimers[index].average;
    elapsed = DBTimers[index].elapsed + (time - DBTimers[index].start);
    DBTimers[index].stop = time;
    DBTimers[index].elapsed = elapsed;
    DBTimers[index].average = avg + elapsed >> 1;
    DBTimers[0].elapsed = 0xa4cb8;
    return;
}

//MATCH GCN
void DBTimerReset(void) {
    s32 i;

    for (i = 0; i < 0x28; i++) {
        DBTimers[i].elapsed = 0;
        DBTimers[i].stop = 0;
        DBTimers[i].start = 0;
    }
    return;
}

//99.63% WIP: only an r7/r10 swap between s and the loop-generated s+1 copy in the row loops
struct D3DTexture* NudxTx_Create(struct nutex_s* texture, s32 rendertargetflag) {
    long data;
    long data2;
    unsigned char* inbits24;
    unsigned short* inbits16;
    long* inbits32;
    long* outbits;
    long* outline;
    unsigned char* inbits8;
    int s;
    int t;
    unsigned char* b1;
    int imagesize;
    int palsize;
    int mapix;
    int ix;
    int format;
    int solid;
    int blended;
    int transparent;
    enum nutextype_e type;
    int width;
    int height;
    void* bits;
    int* pal;
    float alphatest;

    format = iss3cmp;
    type = texture->type;
    width = texture->width;
    height = texture->height;
    bits = texture->bits;
    pal = (int*)texture->pal;

    if (format != 0) {
        GS_TexCreateNU(type, width, height, bits, texture->mmcnt, rendertargetflag, GetTPID());
        return NULL;
    }
    if (type == 0x82) {
        GS_TexCreateNU(0x82, width, height, bits, texture->mmcnt, rendertargetflag, GetTPID());
        return NULL;
    }
    if (type == 0x81) {
        GS_TexCreateNU(0x81, width, height, bits, texture->mmcnt, rendertargetflag, GetTPID());
        return NULL;
    }
    if (type == 0x80) {
        GS_TexCreateNU(0x80, width, height, bits, texture->mmcnt, rendertargetflag, GetTPID());
        return NULL;
    }

    imagesize = width * height;
    inbits16 = (unsigned short*)bits;
    inbits24 = (unsigned char*)bits;
    b1 = (unsigned char*)malloc_x(imagesize * 4);
    texture->linear = format;
    texture->decal = format;
    outline = (long*)b1;
    solid = 0;
    blended = 0;
    transparent = 0;

    switch (type) {
    case NUTEX_RGB16:
    case NUTEX_RGBA16:
        sprintf(DebugText, "RGB16/RGBA16");
        for (s = 0; s < height; s++) {
            outbits = outline;
            for (t = 0; t < width; t++) {
                data = *inbits16;
                data2 = (data & 0x1f) << 3;
                data2 |= (data & 0x3e0) << 6;
                data2 |= (data & 0x7c00) << 9;
                if (data & 1) {
                    data2 |= 0xff000000;
                }
                *outbits = data2;
                inbits16++;
                inbits8 = (unsigned char*)outbits;
                ix = inbits8[1];
                data2 = inbits8[3];
                inbits8[3] = ix;
                inbits8[1] = data2;
                outbits++;
            }
            outline += width;
        }
        break;

    case NUTEX_RGB24:
        sprintf(DebugText, "RGB24");
        for (s = 0; s < height; s++) {
            outbits = outline;
            for (t = 0; t < width; t++) {
                solid++;
                data = *inbits24++;
                data += *inbits24++ << 8;
                data += *inbits24++ << 16;
                data += 0xff000000;
                *outbits = data;
                inbits8 = (unsigned char*)outbits;
                ix = inbits8[1];
                data2 = inbits8[3];
                inbits8[3] = ix;
                inbits8[1] = data2;
                outbits++;
            }
            outline += width;
        }
        break;

    case NUTEX_RGBA32:
        sprintf(DebugText, "RGB32");
        inbits32 = (long*)bits;
        for (s = 0; s < height; s++) {
            outbits = outline;
            for (t = 0; t < width; t++) {
                data = *inbits32;
                *outbits = data;
                alphatest = ((s8*)outbits)[3] / 255.0f;
                if (alphatest < 0.9f) {
                    transparent++;
                } else if (alphatest > 0.063f) {
                    solid++;
                } else {
                    blended++;
                }
                inbits32++;
                outbits++;
            }
            outline += width;
        }
        break;

    case NUTEX_PAL4:
    case NUTEX_PAL8:
        if (type == NUTEX_PAL4) {
            sprintf(DebugText, "PAL4");
        } else if (type == NUTEX_PAL8) {
            sprintf(DebugText, "PAL8");
        }
        palsize = width * height;
        outbits = (long*)b1;
        for (s = 0; s < palsize; s++) {
            if (type == NUTEX_PAL4) {
                if (s & 1) {
                    mapix = ((unsigned char*)bits)[s / 2] >> 4;
                } else {
                    mapix = ((unsigned char*)bits)[s / 2] & 0xf;
                }
                data = pal[mapix];
            } else if (type == NUTEX_PAL8) {
                mapix = ((unsigned char*)bits)[s];
                data = pal[mapix];
            }
            *outbits = data;
            alphatest = ((s8*)outbits)[0] / 255.0f;
            if (alphatest < 0.9f) {
                transparent++;
            } else if (alphatest > 0.063f) {
                solid++;
            } else {
                blended++;
            }
            outbits++;
        }
        break;

    default:
        NuErrorProlog("C:/source/crashwoc/code/system/crashlib.c", 0x187)("NudxTx_Create:\tUnknown texture type!");
        break;
    }

    if (solid != 0 && blended != 0 && transparent != 0) {
        if ((float)blended / (float)solid < 0.6f) {
            texture->decal = 1;
        }
    } else {
        texture->decal = 0;
    }

    GS_TexCreateNU(type, texture->width, texture->height, b1, texture->mmcnt, rendertargetflag, GetTPID());
    free_x(b1);
    DebugText[0] = 0;
    return NULL;
}
