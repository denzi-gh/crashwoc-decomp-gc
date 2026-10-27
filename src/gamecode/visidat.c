#include "main.h"
#include <stddef.h>
#include <string.h>

//NGC MATCH
void visiSetSplineKnot(struct visidata_s* vd, s32 spix, s32 knotix) {
    s32 n;
    s32 i;
    s32 ix;
    s32 len;
    unsigned long long mask; // r7

    if (vd == NULL) {
        return;
    }
    if ((spix == vd->curspline) && (knotix == vd->curknot)) {
        return;
    }
    if (spix < 0) {
        return;
    }
    if (spix >= vd->sc->numsplines) {
        return;
    }
    if (vd->vspline[spix] == NULL) {
        return;
    }
    
    len = vd->sc->splines[spix].len;
    
    knotix = (knotix < len - 1) ? knotix : len - 1;
    
    if (knotix < 0) {
        return;
    } else if (knotix >= len) {
        return;
    }
    vd->curspline = spix;
    vd->curknot = knotix;

    for (knotix; knotix >= 0; knotix--) {
        if (vd->vspline[spix][knotix] != NULL) {
            i = 0;
            for (ix = 0; ix < vd->binfosize; ix++) {
                mask = ((u64 ***)vd->vspline)[spix][knotix][ix];
                for (n = 0; n < 64; n++) {
                    vd->sc->instances[i++].flags.visitest = mask & 1;
                    mask >>= 1;
                    if (i >= vd->sc->numinstance)
                        break;
                }
                if (i >= vd->sc->numinstance) {
                    return;
                }
            }
            return;
        }
    }
}

//NGC MATCH
static s32 PtInsideFBound(struct nuvec_s *wpos,s32 spcnt,struct nuvec_s *pts) {
  struct nuvec_s* curr;
  struct nuvec_s* prev;
  struct nuvec_s ipnt;
  s32 icnt;
  s32 n;
  struct nuplane_s p;

  prev = &pts[spcnt - 1];
  curr = pts;
  p.a = 0.0f;
  p.b = 0.0f;
  p.c = 1.0f;
  p.d = -((wpos->x * p.a) + (wpos->y * p.b) + (wpos->z * p.c));
  for (icnt = 0, n = 0; icnt < spcnt; icnt++, curr++) {
      if ((NuPlnLine(&p,prev,curr,&ipnt) != 0) && (ipnt.x >= wpos->x)) {
        n++;
      }
      prev = curr;
 }
  return n & 1;
}

struct visbndassoc_s {
    s32 visspline;
    s32 bndspline;
    s32 visstart_ix;
    s32 visend_ix;
};

//NGC MATCH
struct visidata_s *visiLoadData(char *fname, struct nugscn_s *sc, union variptr_u *buff) {
    struct visidata_s *vd;
    union variptr_u fdatabase;
    union variptr_u fdata;
    int numassoc;
    int numbnd;
    int splen;
    struct nugobj_s *g;
    struct nuvec_s wpos;
    int gcnt;
    int n;
    int m;
    int o;
    int p;
    int binfix;
    int tmpsize;
    int spline_used;
    struct visbndassoc_s *assocs;
    u64 *binfo;
    u64 *tmpalloc;
    u64 *tmpdelta;
    int binfosize;
    int ver;

    vd = 0;
    fdatabase.voidptr = NuFileLoad(fname);
    if (fdatabase.voidptr != 0) {
        fdata.s32 = fdatabase.s32 + 1;
        vd = buff->voidptr;
        buff->intaddr += sizeof(struct visidata_s);
        memset(vd, 0, sizeof(struct visidata_s));
        vd->sc = sc;
        vd->curspline = -1;
        vd->curknot = -1;
        vd->vspline = buff->voidptr;
        buff->intaddr += sc->numsplines * 4;

        ver = fdatabase.s32[0];
        if (ver < 0) {
            numbnd = fdatabase.s32[1];
            fdata.s32++;
        } else {
            numbnd = ver;
        }

        binfosize = (vd->sc->numinstance + 63) / 64;
        vd->binfosize = binfosize;
        tmpalloc = NuMemAlloc(binfosize * numbnd * 8);
        if (tmpalloc == 0) {
            NuErrorProlog("C:/source/crashwoc/code/gamecode/visidat.c", 0xAD)("unable to alloc %d bytes", binfosize * numbnd * 8);
        }

        for (o = 0; o < numbnd; o++) {
            u64 mask;

            splen = *fdata.s32++;
            binfix = o * binfosize;
            tmpalloc[binfix] = 0;
            mask = 1;
            for (n = 0; n < vd->sc->numinstance; n++) {
                g = vd->sc->gobjs[vd->sc->instances[n].objid];
                gcnt = g->ngobjs;
                for (m = 0; m < gcnt; m++) {
                    NuVecMtxTransform(&wpos, &g->bounding_box_center, (struct Mtx*)&vd->sc->instances[n].mtx);
                    if (PtInsideFBound(&wpos, splen, fdata.vec3)) {
                        tmpalloc[binfix] |= mask;
                    }
                    g++;
                }
                mask <<= 1;
                if (mask == 0) {
                    binfix++;
                    mask = 1;
                }
            }
            fdata.vec3 += splen;
        }

        numassoc = *fdata.s32++;
        assocs = fdata.voidptr;

        if (ver < 0) {
            union variptr_u splused;
            int *spix;
            char *spname;

            splused.voidptr = &assocs[numassoc];
            if ((*splused.u32) != 0) {
                spix = NuMemAlloc((*splused.u32) * 4);
                spname = splused.charptr + 4;
                for (o = 0; o < (*splused.u32); o++) {
                    spix[o] = -1;
                    for (n = 0; n < sc->numsplines; n++) {
                        if (strcmp(sc->splines[n].name, spname) == 0) {
                            spix[o] = n;
                            break;
                        }
                    }
                    while (*spname != 0) {
                        spname++;
                    }
                    spname++;
                }
                for (o = 0; o < numassoc; o++) {
                    assocs[o].visspline = spix[assocs[o].visspline];
                    if (assocs[o].visspline < 0) {
                        assocs[o].visspline = 0;
                        assocs[o].visstart_ix = 0;
                        assocs[o].visend_ix = 1;
                    }
                }
                NuMemFree(spix);
            }
        }

        binfo = NuMemAlloc(binfosize * 8);
        tmpsize = 0;
        tmpdelta = 0;
        if (binfo == 0) {
            NuErrorProlog("C:/source/crashwoc/code/gamecode/visidat.c", 0xFB)("unable to alloc %d bytes", binfosize * 8);
        }

        for (o = 0; o < vd->sc->numsplines; o++) {
            spline_used = 0;
            for (n = 0; n < numassoc; n++) {
                if (assocs[n].visspline != o) {
                    continue;
                }
                if (spline_used == 0) {
                    if (vd->sc->splines[o].len * binfosize > tmpsize) {
                        if (tmpdelta != 0) {
                            NuMemFree(tmpdelta);
                        }
                        tmpsize = vd->sc->splines[o].len * binfosize + 1;
                        tmpdelta = NuMemAlloc(tmpsize * 8);
                        if (tmpdelta == 0) {
                            NuErrorProlog("C:/source/crashwoc/code/gamecode/visidat.c", 0x10D)("unable to alloc %d bytes", tmpsize * 8);
                        }
                    }
                    memset(tmpdelta, 0, tmpsize * 8);
                }
                spline_used = 1;
                assocs[n].visstart_ix = (assocs[n].visstart_ix < vd->sc->splines[o].len)
                                            ? assocs[n].visstart_ix : vd->sc->splines[o].len;
                assocs[n].visend_ix = (assocs[n].visend_ix < vd->sc->splines[o].len)
                                          ? assocs[n].visend_ix : vd->sc->splines[o].len;
                for (m = assocs[n].visstart_ix; m <= assocs[n].visend_ix; m++) {
                    for (p = 0; p < binfosize; p++) {
                        tmpdelta[m * binfosize + p] |= tmpalloc[assocs[n].bndspline * binfosize + p];
                    }
                }
            }

            if (spline_used != 0) {
                vd->vspline[o] = buff->voidptr;
                buff->intaddr += vd->sc->splines[o].len * 8;
                memset(vd->vspline[o], 0, vd->sc->splines[o].len * 8);
                buff->intaddr = (buff->intaddr + 7) & ~7;
                for (n = 0; n < vd->sc->splines[o].len; n++) {
                    u64 deltatot = 0;
                    for (m = 0; m < binfosize; m++) {
                        if (n == 0) {
                            binfo[m] = tmpdelta[m];
                        } else {
                            binfo[m] = tmpdelta[(n - 1) * binfosize + m] ^ tmpdelta[n * binfosize + m];
                        }
                        deltatot |= binfo[m];
                    }
                    if (deltatot != 0) {
                        vd->vspline[o][n] = buff->u64;
                        for (m = 0; m < binfosize; m++) {
                            *buff->u64++ = tmpdelta[n * binfosize + m];
                        }
                    }
                }
            }
        }

        if (tmpdelta != 0) {
            NuMemFree(tmpdelta);
        }
        if (binfo != 0) {
            NuMemFree(binfo);
        }
        if (tmpalloc != 0) {
            NuMemFree(tmpalloc);
        }
        NuMemFree(fdatabase.voidptr);
        buff->intaddr = (buff->intaddr + 15) & ~15;
    }
    return vd;
}
