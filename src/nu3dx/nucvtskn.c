#include "nucvtskn.h"
#include "../nucore/numem.h"

void NuPs2CreateSkinNorm(struct nugobj_s *gobj);
static s32 SortPrimdefs(struct primdef_s* pd, s32 cnt);


struct primdef_s* primdefs_sorted;
struct primdef_s* primdefs;
s32 stritot;
s32 totaldupes;
s32 totalpts;
s32 tritot;
s32 stats[15];
static char mtxused[256];

//MATCH GCN
void InitSkinning(s32 buffsize) {  //makes primitive
  primdefs = (struct primdef_s *)NuMemAlloc(0xc7ce0); // 0xC7CE0 bytes can fit 0x3548 NuPrims perfectly
  primdefs_sorted = (struct primdef_s *)NuMemAlloc(0xc7ce0);
  return;
}

//MATCH GCN
void CloseSkinning(void) {
  NuMemFree(primdefs);
  NuMemFree(primdefs_sorted);
  primdefs = NULL;
  primdefs_sorted = NULL;
  return;
}

//MATCH GCN
void NuPs2CreateSkin(struct nugobj_s* gobj) {
    if ((gobj->geom != NULL) && (gobj->geom->prim->type == NUPT_NDXTRI)) {
        NuPs2CreateSkinNorm(gobj);
    }
return;
}

//MATCH GCN
static void CreateSkinGeom(struct nugeom_s* geom, struct primdef_s* pd, s32 pdcnt) {

    s32 amount_prim;
    struct nuprim_s* nextprim;
    struct nuprim_s* currentprim;
    struct nuprim_s* startprim;
    struct nuvtx_sk3tc1_s* newvertexbuff;
    struct primdef_s* currpd;
    s32 i;
    s32 count;
    u16* indexbuffer;
    s32 primsize[300];
    s32 currentbaseid;
    s32 iVar6;
    s32 j;
    s32 k;
    s32 l;
    s32 q;

    if (geom->vtxtype != NUVT_TC1) {
        NuErrorProlog("C:/source/crashwoc/code/nu3dx/nucvtskn.c", 0x69)("CreateSkinGeom : Unknown vertex type!");
    }

    currentbaseid = pd[0].baseid;
    count = 1;

    memset(primsize, 0, sizeof(primsize));

    for (i = 0; i < pdcnt; i++) {
        if (pd[i].baseid != currentbaseid) {
            currentbaseid = pd[i].baseid;
            count++;
        }
        primsize[count] += 3;
    }
    
    startprim = currentprim = NuPrimCreate(primsize[1], NUPT_NDXTRI);;
    for (i = 1; i < count; i++) {
        currentprim->next = nextprim = NuPrimCreate(primsize[i + 1], NUPT_NDXTRI);
        currentprim = currentprim->next;
    }
    
    currentprim = startprim;
    newvertexbuff = (struct nuvtx_sk3tc1_s*)GS_CreateBuffer(geom->vtxcnt * sizeof(struct nuvtx_sk3tc1_s), 1);
    currpd = pd;
    
    for (i = 0; i < count; i++) {
        for (j = 0; j < 15; j++) {
            currentprim->skinmtxlookup[j] = currpd->mtxid[j];
        }
        indexbuffer = (u16*)currentprim->idxbuff;
        for (amount_prim = 0; amount_prim < currentprim->cnt; amount_prim += 3) {
            indexbuffer[amount_prim] = currpd->vid[0];
            indexbuffer[amount_prim + 1] = currpd->vid[1];
            indexbuffer[amount_prim + 2] = currpd->vid[2];
            for (k = 0; k < 3; k++) {
                newvertexbuff[currpd->vid[k]].pnt = currpd->vrts[k].pnt;
                newvertexbuff[currpd->vid[k]].nrm = currpd->vrts[k].nrm;
                newvertexbuff[currpd->vid[k]].diffuse = currpd->vrts[k].diffuse;
                newvertexbuff[currpd->vid[k]].tc[0] = currpd->vrts[k].tc[0];
                newvertexbuff[currpd->vid[k]].tc[1] = currpd->vrts[k].tc[1];
                for (j = 0; j < 3; j++) {
                    newvertexbuff[currpd->vid[k]].weights[j] = 0.0f;
                    newvertexbuff[currpd->vid[k]].indexes[j] = 0.0f;
                }
                for (j = 0, iVar6 = 0; j < 15 && iVar6 < 3; j++) {
                    if (currpd->weights[k][j] != 0.0f) {
                        newvertexbuff[currpd->vid[k]].indexes[iVar6] = (float)j;
                        if (iVar6 < 2) {
                            newvertexbuff[currpd->vid[k]].weights[iVar6] = currpd->weights[k][j];
                        }
                        iVar6++;
                    }
                }
            }
            currpd++;
        }
        currentprim = currentprim->next;
    }
    GS_DeleteBuffer((u8*)geom->hVB);
    for (currentprim = geom->prim; currentprim != NULL;) {
        nextprim = currentprim->next;
        NuPrimDestroy(currentprim);
        currentprim = nextprim;
    }
    geom->hVB = (s32)newvertexbuff;
    geom->prim = startprim;
    geom->vtxtype = NUVT_SK3TC1;
    return;
}

//MATCH GCN
static s32 AddMtxToPrimDef(struct primdef_s *primdef,s32 mtxid)
{
  s32 n;
  s32 i;


     for(i = 0; i < primdef->nummtx; i++) {

          n = primdef->mtxid[i];
        if (n == mtxid) {
            return i;
        }
    }
    if (i >= 15) {
        return 0;
    }
    primdef->mtxid[i] = mtxid;
    primdef->nummtx++;
    return i;
}

//NGC MATCH
static void SetVtxSkinData (struct primdef_s * pd, s32 pdix, struct nuvtx_tc1_s * vb, s32 vid, struct nugeom_s * ge)
{
  s32 n;
  s32 offset;
  float *weights;
  s32 ix;
  struct nuskin_s *sk;

      //memcpy(&vb[vid], &pd->vrts[pdix], sizeof (struct nuvtx_tc1_s));
      pd->vrts[pdix] = vb[vid];
      pd->vid[pdix] = vid;

      if (pdix < 3) {
        for (sk = ge->skin; sk != NULL; sk = sk->next) {
          offset = vid - sk->vtxoffset;
          if ((-1 < offset) && (offset < sk->vtxcnt)) {
            weights = &sk->weights[sk->mtxcnt * offset];
            for (ix = 0, n = 0; ix < sk->mtxcnt; ix++)
            {
              n = AddMtxToPrimDef(pd,sk->mtxid[ix]);
              pd->weights[pdix][n] = weights[ix];
            }
            return;
          }
        }
      }
      return;
}

//MATCH GCN
static void SetVtxSkinData2(struct primdef_s *pd,s32 pdix,struct nuvtx_tc1_s *vertexbuf,s32 vid,struct nugeom_s *currgeom)
{
    struct NUVTXSKININFO_s *skinfo;
    s32 i;
    s32 ix;


    pd->vrts[pdix] = vertexbuf[vid];
    pd->vid[pdix] = vid;
    skinfo = currgeom->vtxskininfo;


    for (i = 0; i != 3 && skinfo[vid].wts[i] != 0.0f; i++) {
        ix = AddMtxToPrimDef(pd, skinfo[vid].joint_ixs[i]);
        pd->weights[pdix][ix] = skinfo[vid].wts[i];
    }

    return;
}

//MATCH GCN
// This function creates a skinned geometry. It does this by creating an array of
// primitives, setting vertex skin data, sorting the primitives and then creating a
// new geometry from the sorted primitive array. A better name for this function
void NuPs2CreateSkinNorm(struct nugobj_s *gobj) {
    struct nugeom_s* ge;
    s32 n;
    struct nuprim_s* p;
    struct nuvtx_tc1_s* srcvb;
    struct primdef_s* pd;
    u16* vid;
    s32 peak;
    s32 mode;
    s32 pdcnt;
    s32 totmtx;

    memset(stats, NULL, sizeof(stats));


    totaldupes = 0;
    totalpts = 0;
    tritot = 0;
    stritot = 0;

    for (ge = gobj->geom; ge != 0; ge = ge->next)
    {
        if (ge->skin == NULL && !ge->vtxskininfo) {
            continue;
        }
        pd = primdefs;
        p = ge->prim;
        pdcnt = 0;
        vid = (u16*)p->idxbuff;
        srcvb = ge->hVB;
        // Maximum amount of faces is 2200 (0x898)
        if ((p->cnt / 3) > 2200)
        {
            NuErrorProlog("C:/source/crashwoc/code/nu3dx/nucvtskn.c", 0x35e)("NuPs2CreateSkinNorm: TOO MANY PRIMS!");
        }

        for (n = 0; n < p->cnt; n += 3)
        {
            memset(pd, 0, sizeof(struct primdef_s));
            memset(pd->mtxid, -1, 0x3c);

            if (ge->vtxskininfo)
            {
                SetVtxSkinData2(pd, 0, srcvb, vid[n], ge);
                SetVtxSkinData2(pd, 1, srcvb, vid[n + 1], ge);
                SetVtxSkinData2(pd, 2, srcvb, vid[n + 2], ge);
            }
            else
            {
                SetVtxSkinData(pd, 0, srcvb, vid[n], ge);
                SetVtxSkinData(pd, 1, srcvb, vid[n + 1], ge);
                SetVtxSkinData(pd, 2, srcvb, vid[n + 2], ge);
            }

            if (((pd->vid[0] == pd->vid[1]) || (pd->vid[1] == pd->vid[2])) || (pd->vid[0] == pd->vid[2]))
            {
                pd -= 1;
                pdcnt -= 1;
            }
            totmtx = pd->nummtx;
            pd++;
            pdcnt++;
            stats[totmtx]++;
        }
        SortPrimdefs(primdefs,pdcnt);
        mode = 0;
        peak = 0;
        for(n = 0; n < 15; n++) {
            if ((stats[n] != 0) && (n > peak)) {
                peak = n;
            }
            if (stats[n] >= stats[mode]) {
                mode = n;
            }
        }
        CreateSkinGeom(ge,primdefs,pdcnt);
    }

	return;
}


//MATCH GCN
static s32 FillFreeMatrixSlots(struct primdef_s* pd, s32 cnt, s32 start) {
    struct primdef_s* primdef_batch; //
    int numbatchmtx; //
    int n; //
    int m; //
    int maxmtxperprim; //

    maxmtxperprim = 0xC;

    primdef_batch = &pd[start];
    numbatchmtx = primdef_batch->nummtx;

    if (start >= cnt) {
        return numbatchmtx;
    }

    if (numbatchmtx >= maxmtxperprim) {
        return numbatchmtx;
    }

    for(n = start + 1; n < cnt; n++) {
        if (pd[n].sorted != 1) {
            for(m = 0; m < maxmtxperprim && pd[n].mtxid[m] != -1; m++) {
                if (mtxused[pd[n].mtxid[m]] == 0) {
                    primdef_batch->mtxid[numbatchmtx] = pd[n].mtxid[m];
                    mtxused[pd[n].mtxid[m]] = 1;

                    if (numbatchmtx++ == 11) {
                        return maxmtxperprim;
                    }
                }
            }
        }
    }
    return numbatchmtx;
}


//MATCH GCN
static s32 SortPrimdefs(struct primdef_s* pd, s32 cnt) {
    s32 scnt;
    s32 tot;
    s32 baseid;
    s32 totsmtx;
    s32 n;
    s32 m;
    s32 o;
    s32 p;
    s32 s;
    s32 t;
    struct primdef_s tpd;
    s32 batchmatrices[200][15]; //dwarf original: [15][200]
    s32 batchmatrices_sorted[200][15]; //dwarf original: [15][200]
    s32 batchcount;
    s32 matrixusecount[150];
    s32 matrixindex;
    s32 matrixcount;
    s32 maxmatrix;
    struct matchingslot_s matchingslots[200];
    s32 nummatches;
    s32 freeslotcount;
    s32 matrixslot;
    s32 batchnum;
    s32 totalmtx;

    batchcount = 0;
    scnt = cnt;

    for (s = 0; s < scnt; s++) {
        for (t = s + 1; t < scnt; t++) {
            if (pd[t].nummtx > pd[s].nummtx) {
                tpd = pd[s];
                pd[s] = pd[t];
                pd[t] = tpd;
            }
        }
    }

    tot = 0;
    for (s = 0; s < scnt; s++) {
        if (pd[s].sorted == 0) {
            memset(mtxused, 0, sizeof(mtxused));
            for (t = 0; t < pd[s].nummtx; t++) {
                mtxused[pd[s].mtxid[t]] = 1;
            }
            pd[s].nummtx = FillFreeMatrixSlots(pd, scnt, s);
            baseid = tot;
            pd[s].baseid = baseid;
            primdefs_sorted[tot] = pd[s];
            tot++;

            for (t = s + 1; t < scnt; t++) {
                if (pd[t].sorted == 0 && pd[s].nummtx >= pd[t].nummtx) {
                    totsmtx = 0;
                    for (p = 0; p < pd[t].nummtx; p++) {
                        totsmtx += abs(mtxused[pd[t].mtxid[p]] - 1);
                    }
                    if (totsmtx == 0) {
                        pd[t].baseid = baseid;
                        primdefs_sorted[tot] = pd[t];
                        pd[t].sorted = 1;
                        for (n = 0; n < 15; n++) {
                            primdefs_sorted[tot].mtxid[n] = -1;
                            primdefs_sorted[tot].weights[0][n] = 0.0f;
                            primdefs_sorted[tot].weights[1][n] = 0.0f;
                            primdefs_sorted[tot].weights[2][n] = 0.0f;
                        }
                        for (m = 0; m < pd[s].nummtx; m++) {
                            for (o = 0; o < pd[t].nummtx; o++) {
                                if (pd[s].mtxid[m] == pd[t].mtxid[o]) {
                                    primdefs_sorted[tot].weights[0][m] = pd[t].weights[0][o];
                                    primdefs_sorted[tot].weights[1][m] = pd[t].weights[1][o];
                                    primdefs_sorted[tot].weights[2][m] = pd[t].weights[2][o];
                                    primdefs_sorted[tot].mtxid[m] = pd[s].mtxid[m];
                                }
                            }
                        }
                        tot++;
                    }
                }
            }

            for (n = 0; n < 15; n++) {
                batchmatrices[batchcount][n] = primdefs_sorted[baseid].mtxid[n];
            }
            batchcount++;
        }
    }

    memset(matrixusecount, 0, sizeof(matrixusecount));
    maxmatrix = -1;
    for (s = 0; s < batchcount; s++) {
        for (t = 0; t < 15; t++) {
            if (batchmatrices[s][t] != -1) {
                matrixusecount[batchmatrices[s][t]]++;
                maxmatrix = (batchmatrices[s][t] < maxmatrix) ? maxmatrix : batchmatrices[s][t];
            }
        }
    }

    memset(batchmatrices_sorted, -1, sizeof(batchmatrices_sorted));

    do {
        matrixindex = 0;
        matrixcount = matrixusecount[matrixindex];
        for (s = 0; s <= maxmatrix; s++) {
            if (matrixusecount[s] > matrixusecount[matrixindex]) {
                matrixcount = matrixusecount[s];
                matrixindex = s;
            }
        }

        if (matrixcount != 0) {
            nummatches = 0;
            for (s = 0; s < batchcount; s++) {
                for (t = 0; t < 15; t++) {
                    if (batchmatrices[s][t] == matrixindex) {
                        matchingslots[nummatches].batch = s;
                        matchingslots[nummatches].slot = t;
                        nummatches++;
                    }
                }
            }

            matrixslot = 0;
            freeslotcount = 0;
            while (freeslotcount < nummatches && matrixslot < 15) {
                freeslotcount = 0;
                for (t = 0; t < nummatches; t++) {
                    if (batchmatrices_sorted[matchingslots[t].batch][matrixslot] == -1) {
                        freeslotcount++;
                    }
                }
                matrixslot++;
            }

            if (freeslotcount < nummatches) {
                NuErrorProlog("C:/source/crashwoc/code/nu3dx/nucvtskn.c", 0x232)("SortPrimDefs: Unable to find a matching free slot in all batches!!");
            } else {
                matrixslot--;
                for (s = 0; s < nummatches; s++) {
                    batchmatrices_sorted[matchingslots[s].batch][matrixslot] = matrixindex;
                    matrixusecount[matrixindex] = 0;
                }
            }
        }
    } while (matrixcount != 0);

    batchnum = 0;
    baseid = 0;
    for (s = 0; s < scnt; s++) {
        if (primdefs_sorted[s].baseid != baseid) {
            baseid = primdefs_sorted[s].baseid;
            batchnum++;
        }
        pd[s] = primdefs_sorted[s];
        for (n = 0; n < 15; n++) {
            pd[s].mtxid[n] = -1;
            pd[s].weights[0][n] = 0.0f;
            pd[s].weights[1][n] = 0.0f;
            pd[s].weights[2][n] = 0.0f;
        }
        pd[s].nummtx = primdefs_sorted[baseid].nummtx;
        pd[s].baseid = baseid;
        for (n = 0; n < 15; n++) {
            for (t = 0; t < 15; t++) {
                if (batchmatrices_sorted[batchnum][n] == primdefs_sorted[s].mtxid[t]) {
                    pd[s].weights[0][n] = primdefs_sorted[s].weights[0][t];
                    pd[s].weights[1][n] = primdefs_sorted[s].weights[1][t];
                    pd[s].weights[2][n] = primdefs_sorted[s].weights[2][t];
                    pd[s].mtxid[n] = primdefs_sorted[s].mtxid[t];
                }
            }
        }
    }

    totalmtx = 0;
    baseid = -1;
    for (s = 0; s < tot; s++) {
        if (primdefs_sorted[s].baseid != baseid) {
            totalmtx += primdefs_sorted[s].nummtx;
            baseid = primdefs_sorted[s].baseid;
        }
    }
    return totalmtx;
}
