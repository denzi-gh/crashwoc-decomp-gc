#include "nutex.h"
#include "../system.h"
#include "../nuraster/dxframe.h"
#include <string.h>

struct nusystex_s tinfo[0x400];

//static s32 initialised;
static s32 palette_128[256];
static char strBuf[128];
static s32 ntex;
struct nutex_s tex;

//MATCH NGC
void NuTexInit() {
	if (initialised != 0) {
		NuTexClose();
	}
	GS_TexReInit();
	memset(tinfo, 0, sizeof(tinfo));
	ntex = 0;
	tpid = 0;
	initialised = 1;
}

//MATCH NGC
void NuTexClose(void)
{
    s32 i;

    NudxFw_DestroyBackBufferCopy();   //empty function
    for (i = 0; i < 0x400; i++) {
        if (tinfo[i].tex.bits != NULL) {
            NuMemFree(tinfo[i].tex.bits);
            tinfo[i].tex.bits = 0;
        }
    }
    initialised = ntex = tpid = 0;
	return;
}

s32 GetTPID()
{
	return tpid;
}

//MATCH NGC
s32 NuTexCreate(struct nutex_s *nutex) {
    //replace struct D3DTexture with SDL_Surface *surface;

    if ((u32)nutex->type > NUTEX_BB) {
        tinfo[tpid].dds = NudxTx_Create(nutex, 1);
    } else {
        tinfo[tpid].dds = NudxTx_Create(nutex, 0);
    }
  tinfo[tpid].tex.bits = NULL;
  tinfo[tpid].tex.type = nutex->type;
  tinfo[tpid].tex.decal = nutex->decal;
  tinfo[tpid].tex.linear = nutex->linear;
  tinfo[tpid].tex.width = nutex->width;
  tinfo[tpid].tex.height = nutex->height;
  if (force_decal != 0) {
    tinfo[tpid].tex.decal = 1;
  }
  tpid++;
  return tpid;
}

//MATCH NGC
s32 NuTexGetDecalInfo(s32 tid) {
  return (s32)tinfo[tid + -1].tex.decal;
}

//MATCH NGC
s32 NuTexCreateFromSurface(struct nutex_s *tex, struct D3DTexture *surface) {
  tinfo[tpid].tex = *tex;

  tinfo[tpid].dds = surface;
  tinfo[tpid].tex.bits = NULL;
  tpid++;
  return tpid;
}

void NuTexDestroy(s32 id)
{
	id--;
	if (id > -1)
	{
		if (tinfo[id].tex.bits != NULL)
		{
			NuMemFree(tinfo[id].tex.bits);
			tinfo[id].tex.bits = NULL;
		}
	}
}


u32 NuTexUnRef(s32 id)
{
  int new_var;
  new_var = 0;
  id--;
  if (id < new_var)
  {
    if (new_var || id)
    {
      return (void *) new_var;
    }
    else
    {
      return (void *) new_var;
    }
  }
  return --tinfo[id].ref;
}

s32 NuTexPixelSize(enum nutextype_e type) {
  switch (type) {
    case NUTEX_RGB16:
      return 0x10;
    case NUTEX_RGBA16:
      return 0x10;
    case NUTEX_RGB24:
      return 0x18;
    case NUTEX_RGBA32:
      return 0x20;
    case NUTEX_PAL8:
      return 8;
    case NUTEX_PAL4:
      return 4;
    default:
      return 0;
  }
}

s32 NuTexImgSize(enum nutextype_e type, s32 width, s32 height)
{
	s32 size = width * height * NuTexPixelSize(type);
	if (size < 0)
	{
		size += 7;
	}
	return size >> 3;
}

s32 NuTexPalSize(enum nutextype_e type)

{
  switch (type) {
    case NUTEX_PAL8:
      return 0x400;
    case NUTEX_PAL4:
      return 0x40;
    default:
      return 0;
  }
}

//NGC MATCH
void NuTexSetTextureStates(struct numtl_s *mtl) {

  if (mtl->tid == 0) {
    NuTexSetTexture(0,0);
    NudxFw_SetTextureState(0,D3DTSS_COLOROP,1);
    NudxFw_SetTextureState(0,D3DTSS_ALPHAOP,1);
  }
  else {
    NudxFw_SetTextureState(0,D3DTSS_COLOROP,4);
    NudxFw_SetTextureState(0,D3DTSS_COLORARG1,2);
    NudxFw_SetTextureState(0,D3DTSS_COLORARG2,0);
    NudxFw_SetTextureState(0,D3DTSS_ALPHAARG1,2);
    NudxFw_SetTextureState(0,D3DTSS_ALPHAARG2,0);
    NudxFw_SetTextureState(0,D3DTSS_ALPHAOP,4);
    NudxFw_SetTextureState(0,D3DTSS_TEXCOORDINDEX,0);
    NudxFw_SetTextureState(0,D3DTSS_TEXTURETRANSFORMFLAGS,0);
        if (mtl->attrib.utc == 0) {
            GS_TexSetWrapModes(0,GX_REPEAT);
        }
        else if (mtl->attrib.utc == 2) {
            GS_TexSetWrapModes(0,GX_MIRROR);
        }
        else if (mtl->attrib.utc == 3) {
            GS_TexSetWrapModes(0,GX_CLAMP);
        }
        else {
            GS_TexSetWrapModes(0,GX_CLAMP);
        }

        if (mtl->attrib.vtc == 0) {
            GS_TexSetWrapModet(0,GX_REPEAT);
        }
        else if (mtl->attrib.vtc == 2) {
            GS_TexSetWrapModet(0,GX_MIRROR);
        }
        else if (mtl->attrib.vtc == 3) {
            GS_TexSetWrapModet(0,GX_CLAMP);
        }
        else {
            GS_TexSetWrapModet(0,GX_CLAMP);
        }
    NudxFw_SetTextureState(0,D3DTSS_MAGFILTER,2);
    NudxFw_SetTextureState(0,D3DTSS_MINFILTER,2);
    NudxFw_SetTextureState(0,D3DTSS_MIPFILTER,2);
  }
  return;
}

//MATCH GCN
s32 NuTexReadBitmapMM(char *filename, s32 mmlevel, struct nutex_s *tex)
{
    s32 fh;
    struct tagBITMAPFILEHEADER bmh;
    struct tagBITMAPINFOHEADER bmi;
    s32 palsize;
    void *tmpbuff;
    union variptr_u dst;
    s32 rowsize;
    s32 imgsize;
    s32 n;
    s32 m;
    char *top;
    char *bottom;
    s32 w;
    s32 h;
    s32 i;
    union {
        u32 u32;
        u8 u8[4];
    } colour;
    u8 swap;

    if (filename == NULL) {
        NuErrorProlog("C:/source/crashwoc/code/nu3dx/nutex.c", 0x1fa)("assert");
    }
    fh = NuFileOpen(filename, NUFILE_READ);
    if (fh != 0) {
        if (tex != NULL) {
            memset(tex, 0, sizeof(struct nutex_s));
            NuFilePos(fh);
            NuFileRead(fh, &bmh.bfType, sizeof(bmh) - 2);
            NuFileRead(fh, &bmi, sizeof(bmi));
            switch (bmi.biBitCount) {
            case 32:
                tex->type = NUTEX_RGBA32;
                palsize = 0;
                break;
            case 24:
                tex->type = NUTEX_RGB24;
                palsize = 0;
                break;
            case 4:
                tex->type = NUTEX_PAL4;
                palsize = 0x40;
                NuFileRead(fh, palette_128, 0x40);
                for (i = 0; i < 16; i++) {
                    colour.u32 = palette_128[i];
                    swap = colour.u8[0];
                    colour.u8[0] = colour.u8[2];
                    colour.u8[2] = swap;
                    palette_128[i] = colour.u32;
                }
                break;
            case 8:
                tex->type = NUTEX_PAL8;
                NuFileRead(fh, palette_128, 0x400);
                palsize = 0x400;
                for (i = 0; i < 256; i++) {
                    colour.u32 = palette_128[i];
                    swap = colour.u8[0];
                    colour.u8[0] = colour.u8[2];
                    colour.u8[2] = swap;
                    palette_128[i] = colour.u32;
                }
                break;
            default:
                NuErrorProlog("C:/source/crashwoc/code/nu3dx/nutex.c", 0x22d)
                    ("NuTexLoadBitmap:Bad BitCount <%d> on loading bitmap <%s>", bmi.biBitCount, filename);
                break;
            }
            n = mmlevel + 1;
            tex->mmcnt = mmlevel;
            tex->width = bmi.biWidth;
            tex->height = bmi.biHeight;
            rowsize = (tex->width * NuTexPixelSize(tex->type) + 7) / 8;
            m = n;
            h = tex->height;
            w = tex->width;
            imgsize = 0;
            do {
                imgsize += NuTexImgSize(tex->type, w, h);
                w >>= 1;
                h >>= 1;
            } while (--m);
            tex->bits = NuMemAlloc(imgsize);
            if (palsize != 0) {
                tex->pal = NuMemAlloc(palsize);
            } else {
                tex->pal = NULL;
            }
            tmpbuff = tex->bits;
            NuFileRead(fh, tmpbuff, NuTexImgSize(tex->type, tex->width, tex->height));
            dst.voidptr = malloc_x(rowsize);
            top = tex->bits;
            bottom = top + NuTexImgSize(tex->type, tex->width, tex->height) - rowsize;
            while (bottom > top) {
                memcpy(dst.voidptr, top, rowsize);
                memcpy(top, bottom, rowsize);
                top += rowsize;
                memcpy(bottom, dst.voidptr, rowsize);
                bottom -= rowsize;
            }
            free_x(dst.voidptr);
            NuTexImgSize(tex->type, tex->width, tex->height);
            if (palsize != 0) {
                memcpy(tex->pal, palette_128, palsize);
            }
        }
        NuFileClose(fh);
    } else {
        sprintf(strBuf, "Cannot open file: %s", filename);
        return 0;
    }
    return 1;
}

//MATCH GCN
void NuTexSetTexture(u32 stage,s32 tid) {
  void GS_TexSelect();

  if (tid > 0) {
    GS_TexSelect(stage,tid);
  }
  else {
    GS_TexSelect(stage,0);
  }
  return;
}

//MATCH GCN
struct nutex_s * NuTexReadBitmap(char* fileName)
{
	struct nutex_s* ret = NuMemAlloc(sizeof(struct nutex_s));
	if (!NuTexReadBitmapMM(fileName, 0, ret))
	{
		NuMemFree(ret);
		ret = NULL;
	}
	return ret;
}

struct D3DTexture* NuTexLoadTextureFromDDSFile(char* fileName)
{
	return NULL;
}
