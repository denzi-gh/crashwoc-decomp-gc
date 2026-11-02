#include "dolphin/gx.h"
#include "dolphin/os.h"
#include "macros.h"

#include "__gx.h"

#if DEBUG
#define GX_WRITE_SOME_REG5(a, b) \
do { \
    GX_WRITE_U8(a); \
    GX_WRITE_U32(b); \
    __gxVerif->rasRegs[(b >> 24) & 0xFF] = b; \
} while (0)
#else
#define GX_WRITE_SOME_REG5(a, b) \
do { \
    GX_WRITE_U8(a); \
    GX_WRITE_U32(b); \
} while (0)
#endif

void GXSetTevIndirect(GXTevStageID tev_stage, GXIndTexStageID ind_stage, GXIndTexFormat format, GXIndTexBiasSel bias_sel, GXIndTexMtxID matrix_sel, GXIndTexWrap wrap_s, GXIndTexWrap wrap_t, GXBool add_prev, GXBool utc_lod, GXIndTexAlphaSel alpha_sel)
{
    u32 reg;

    CHECK_GXBEGIN(0x7F, "GXInitIndTexture");
    reg = 0;
    SET_REG_FIELD(0x81, reg, 2, 0, ind_stage);
    SET_REG_FIELD(0x82, reg, 2, 2, format);
    SET_REG_FIELD(0x83, reg, 3, 4, bias_sel);
    SET_REG_FIELD(0x84, reg, 2, 7, alpha_sel);
    SET_REG_FIELD(0x85, reg, 4, 9, matrix_sel);
    SET_REG_FIELD(0x86, reg, 3, 13, wrap_s);
    SET_REG_FIELD(0x87, reg, 3, 16, wrap_t);
    SET_REG_FIELD(0x88, reg, 1, 19, utc_lod);
    SET_REG_FIELD(0x89, reg, 1, 20, add_prev);
    SET_REG_FIELD(0x8A, reg, 8, 24, tev_stage + 16);
    GX_WRITE_SOME_REG5(0x61, reg);
    gx->bpSentNot = 0;
}

void GXSetIndTexCoordScale(GXIndTexStageID ind_state, GXIndTexScale scale_s, GXIndTexScale scale_t)
{
    CHECK_GXBEGIN(0xE6, "GXSetIndTexScale");

    switch (ind_state) {
    case GX_INDTEXSTAGE0:
        SET_REG_FIELD(0xEA, gx->IndTexScale0, 4, 0, scale_s);
        SET_REG_FIELD(0xEB, gx->IndTexScale0, 4, 4, scale_t);
        SET_REG_FIELD(0xEC, gx->IndTexScale0, 8, 24, 0x25);
        GX_WRITE_SOME_REG5(0x61, gx->IndTexScale0);
        break;
    case GX_INDTEXSTAGE1:
        SET_REG_FIELD(0xF0, gx->IndTexScale0, 4, 8, scale_s);
        SET_REG_FIELD(0xF1, gx->IndTexScale0, 4, 12, scale_t);
        SET_REG_FIELD(0xF2, gx->IndTexScale0, 8, 24, 0x25);
        GX_WRITE_SOME_REG5(0x61, gx->IndTexScale0);
        break;
    case GX_INDTEXSTAGE2:
        SET_REG_FIELD(0xF6, gx->IndTexScale1, 4, 0, scale_s);
        SET_REG_FIELD(0xF7, gx->IndTexScale1, 4, 4, scale_t);
        SET_REG_FIELD(0xF8, gx->IndTexScale1, 8, 24, 0x26);
        GX_WRITE_SOME_REG5(0x61, gx->IndTexScale1);
        break;
    case GX_INDTEXSTAGE3:
        SET_REG_FIELD(0xFC, gx->IndTexScale1, 4, 8, scale_s);
        SET_REG_FIELD(0xFD, gx->IndTexScale1, 4, 12, scale_t);
        SET_REG_FIELD(0xFE, gx->IndTexScale1, 8, 24, 0x26);
        GX_WRITE_SOME_REG5(0x61, gx->IndTexScale1);
        break;
    default:
        ASSERTMSGLINE(0x102, 0, "GXSetIndTexCoordScale: Invalid Indirect Stage Id");
        break;
    }
    gx->bpSentNot = 0;
}

void GXSetNumIndStages(u8 nIndStages)
{
    CHECK_GXBEGIN(0x144, "GXSetNumIndStages");
    ASSERTMSGLINE(0x146, nIndStages <= 4, "GXSetNumIndStages: Exceeds max. number of indirect texture stages");
    SET_REG_FIELD(0x147, gx->genMode, 3, 16, nIndStages);
    gx->dirtyState |= 6;
}

void GXSetTevDirect(GXTevStageID tev_stage)
{
    CHECK_GXBEGIN(0x158, "GXSetTevDirect");
    GXSetTevIndirect(tev_stage, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_OFF, GX_ITW_OFF, GX_ITW_OFF, 0U, 0, 0);
}

void __GXUpdateBPMask(void)
{
    u32 nIndStages;
    u32 i;
    u32 tmap;
    u32 new_imask;
    u32 nStages;
    u32 new_dmask;

    new_imask = 0;
    new_dmask = 0;
    nIndStages = GET_REG_FIELD(gx->genMode, 3, 16);
    for (i = 0; i < nIndStages; i++) {
        switch (i) {
        case 0: tmap = GET_REG_FIELD(gx->iref, 3, 0); break;
        case 1: tmap = GET_REG_FIELD(gx->iref, 3, 6); break;
        case 2: tmap = GET_REG_FIELD(gx->iref, 3, 12); break;
        case 3: tmap = GET_REG_FIELD(gx->iref, 3, 18); break;
        }
        new_imask |= 1 << tmap;
    }

#if DEBUG
    nStages = GET_REG_FIELD(gx->genMode, 4, 10) + 1;
    for (i = 0; i < nStages; i++) {
        tmap = gx->texmapId[i] & 0xFFFFFEFF;
        if (tmap != 0xFF) {
            new_dmask |= 1 << tmap;
        }
    }
    ASSERTMSGLINE(0x269, !(new_imask & new_dmask), "GXSetTevOrder/GXSetIndTexOrder: Same texture map cannot be specified in both");
#endif

    if ((u8)gx->bpMask != new_imask) {
        SET_REG_FIELD(0x26E, gx->bpMask, 8, 0, new_imask);
        GX_WRITE_SOME_REG5(0x61, gx->bpMask);
        gx->bpSentNot = 0;
    }
}

void __GXFlushTextureState(void)
{
    GX_WRITE_SOME_REG5(0x61, gx->bpMask);
    gx->bpSentNot = 0;
}
