#include "edobj.h"
#include "../nu.h"

extern struct nuanimdata_s *ObjectAnim[64];
extern u8 object_switches[128];
extern const struct numtx_s numtx_identity;

s32 PlatImpactInfo(struct nuvec_s *norm, s32 *info, s32 *extra);
s32 LookupDebrisEffect(char *name);
void AddVariableShotDebrisEffectMtx2(s32 type, struct nuvec_s *pos, s32 numdeb, struct numtx_s *emitrotmtx, struct numtx_s *rotmtx);
void edbitsSoundPlay(struct nuvec_s *pos, s32 sid);
s32 edbitsLookupSoundFX(char *name);
struct nuanimdata_s *NuAnimDataCreate(s32 nchunks);
struct nuanimdatachunk_s *NuAnimDataChunkCreate(s32 numnodes);
struct nuanimcurveset_s *NuAnimCurveSetCreate(s32 ncurves);
struct nuanimcurve_s *NuAnimCurveCreate(s32 numkeys);
void NuAnimCurveDestroy(struct nuanimcurve_s *curve);
s32 NewPlatInst(struct numtx_s *mat, s32 instance);
void UpdatePlatinst(s32 platid, struct numtx_s *mat);
void PlatInstRotate(s32 platid, s32 state);
void PlatInstBounce(s32 platid, float plrgrav, float tension, float damp);
void edobjSoundDestroy(s32 obj, s32 sound);
void edobjParticleDestroy(s32 obj, s32 ptl);
void NuAnimDataCalcTime(struct nuanimdata_s *animdata, float time, struct nuanimtime_s *atime);
void NuAnimCurveSetApplyToMatrix(struct nuanimcurveset_s *animcurveset, struct nuanimtime_s *atime, struct numtx_s *T);

//NGC MATCH
float edobjPlayerObjectDistance(s32 objid) {
  if (edmainQueryLocVec() != NULL) {
    return NuVecDist(&ObjectInstance[objid].mtx._30,edmainQueryLocVec(),NULL);
  }
  else {
    return 0.0f;
  }
}

static f32 localframecount_156_N1;

//NGC MATCH
void edobjUpdateObjects(float dt) {
    s32 i;
    s32 j;
    s32 k;
    struct nuinstance_s* instance;
    struct nuvec_s norm;
    s32 info;
    s32 extra;
    s32 impactid;
    float ltime;
    struct nuinstanim_s* instanim;
    s32 sid;

    impactid = PlatImpactInfo(&norm, &info, &extra);
    for (i = 0; i < 0x40; i++) {
        ltime = 1.0f;
        instance = &ObjectInstance[i];
        if (instance->objid != -1) {
            instanim = instance->anim;
            NuMtxSetIdentity(&instance->mtx);
            instance->mtx._30 = ObjectPath[i].waypoint[0].x;
            instance->mtx._31 = ObjectPath[i].waypoint[0].y;
            instance->mtx._32 = ObjectPath[i].waypoint[0].z;
            if ((instanim != 0) && (ObjectAnim[instanim->anim_ix] != 0)) {
                switch (ObjectPath[i].trigger_type) {
                    case 0:
                        instanim->playing = 1;
                        break;

                    case 1:
                        instanim->playing = 0;
                        if ((ObjectPath[i].trigger_id != -1) && (object_switches[ObjectPath[i].trigger_id] != 0)) {
                            instanim->playing = 1;
                        }
                        break;
                        ltime = dt;

                    case 2: {
                        instanim->repeating = 0;
                        if (((ObjectPath[i].trigger_id != -1) && (object_switches[ObjectPath[i].trigger_id] != 0))
                            && (instanim->playing == 0))
                        {
                            instanim->ltime = 1.0f;
                            instanim->playing = 1;
                            instanim->backwards = 0;
                            instanim->waiting = 0;
                        }
                        break;
                    }

                    case 3:
                        instanim->repeating = 1;
                        if ((ObjectPath[i].trigger_id != -1) && (object_switches[ObjectPath[i].trigger_id] != 0)) {
                            instanim->playing = 1;
                        }
                        break;

                    case 4:
                        if (edobjPlayerObjectDistance(i) < ObjectPath[i].trigger_var) {
                            instanim->playing = 1;
                        } else {
                            instanim->playing = 0;
                        }
                        break;

                    case 5: {
                        instanim->repeating = 0;
                        if ((edobjPlayerObjectDistance(i) < ObjectPath[i].trigger_var)
                            && (instanim->playing == 0))
                        {
                            instanim->ltime = 1.0f;
                            instanim->playing = 1;
                            instanim->backwards = 0;
                            instanim->waiting = 0;
                        }
                        break;
                    }

                    case 6:
                        instanim->repeating = 1;
                        if (edobjPlayerObjectDistance(i) < ObjectPath[i].trigger_var) {
                            instanim->playing = 1;
                        }
                        break;

                    case 7:
                        if (ObjectPath[i].terrplatid == impactid) {
                            ObjectPath[i].trigger_var = 10.0f;
                        }
                        if (ObjectPath[i].trigger_var > 0.0f) {
                            ObjectPath[i].trigger_var -= dt;
                            instanim->playing = 1;
                        } else {
                            instanim->playing = 0;
                        }
                        break;

                    case 8: {
                        instanim->repeating = 0;
                        if ((ObjectPath[i].terrplatid == impactid) && (instanim->playing == 0)) {
                            instanim->playing = 1;
                            instanim->ltime = ltime;
                            instanim->waiting = 0;
                            if (ObjectPath[i].trigger_wait > 0.0f) {
                                instanim->tfirst = ObjectPath[i].trigger_wait + ltime;
                                instanim->waiting = 1;
                            }
                            instanim->backwards = 0;
                        }
                        break;
                    }

                    case 9:
                        instanim->repeating = 1;
                        if (ObjectPath[i].terrplatid == impactid) {
                            instanim->playing = 1;
                        }
                        break;
                }

                {
                    struct nuanimdata_s* animdata;

                    animdata = ObjectAnim[instanim->anim_ix];
                    if ((animdata != 0) && (instanim->playing != 0)) {
                        instanim->ltime += (dt * instanim->tfactor);
                        if (instanim->waiting != 0) {
                            if (instanim->ltime >= instanim->tfirst) {
                                instanim->waiting = 0;
                                instanim->ltime -= (instanim->tfirst - 1.0f);
                            }
                        }
                        if (instanim->waiting == 0) {
                            if (instanim->ltime >= (animdata->time + instanim->tinterval)) {
                                if (instanim->repeating != 0) {
                                    if (instanim->oscillate != 0) {
                                        instanim->backwards = (instanim->backwards == 0) ? 1 : 0;
                                    }
                                    instanim->ltime = 1.0f;
                                } else if ((instanim->backwards != 0) || (instanim->oscillate == 0)) {
                                    instanim->playing = 0;
                                    if (ObjectPath[i].trigger_id != -1) {
                                        object_switches[ObjectPath[i].trigger_id] = 0;
                                    }
                                } else {
                                    instanim->ltime = 1.0f;
                                    instanim->backwards = (instanim->backwards == 0) ? 1 : 0;
                                }
                            }
                            if (instanim->ltime > animdata->time) {
                                ltime = animdata->time;
                            } else {
                                ltime = instanim->ltime;
                            }
                        }
                    } else {
                        ltime = instanim->ltime;
                    }
                    if (instanim->backwards != 0) {
                        ltime = (animdata->time + 1.0f) - ltime;
                    }

                    {
                        struct nuanimtime_s atime;
                        struct numtx_s animtx;
                        NuAnimDataCalcTime(animdata, ltime, &atime);
                        NuAnimCurveSetApplyToMatrix(*animdata->chunks[atime.chunk]->animcurvesets, &atime, &animtx);
                        instanim->mtx = animtx;
                        NuMtxTranslate(&instanim->mtx, (struct nuvec_s*)(&instance->mtx._30));
                    }
                }
            }
                for (j = 0; j < ((s32)dt); j++) {
                    k = 0;
                    while (k < ObjectPath[i].usedpart) {
                        if (ObjectPath[i].particle_type[k] != -1) {
                            if ((ObjectPath[i].particle_switch[k] == 0)
                                || (((instanim != 0) && (instanim->playing != 0))
                                    && !(instanim->ltime >= ObjectAnim[instanim->anim_ix]->time)))
                            {
                                struct numtx_s* whichmtx;
                                struct nuvec_s emit;
                                struct numtx_s tmtx;
                                struct numtx_s orient;
                                s32 absrate;
                                s32 frame;

                                if (instanim != 0) {
                                    whichmtx = (struct numtx_s*)instanim;
                                } else {
                                   whichmtx = (struct numtx_s*)instance;
                                }
                                if (((edobj_particle_mode != 0) || (edobj_sound_mode != 0)) && (edobj_nearest == i)) {
                                    whichmtx = (struct numtx_s*)instance;
                                }
                                NuVecMtxTransform(&emit, &ObjectPath[i].particle_offset[k], whichmtx);
                                orient = numtx_identity;
                                NuMtxRotateZ(&orient, ObjectPath[i].particle_emitrotz[k]);
                                NuMtxRotateY(&orient, ObjectPath[i].particle_emitroty[k]);
                                NuMtxMul(&tmtx, &orient, whichmtx);
                                if (ObjectPath[i].particle_rate[k] > 0) {
                                    AddVariableShotDebrisEffectMtx2(
                                        ObjectPath[i].particle_type[k], &emit, ObjectPath[i].particle_rate[k], &tmtx, &numtx_identity
                                    );
                                } else if (ObjectPath[i].particle_rate[k] < 0) {
                                    absrate = (ObjectPath[i].particle_rate[k] >= 0) ? (ObjectPath[i].particle_rate[k]) : (-ObjectPath[i].particle_rate[k]);
                                    frame = (s32)(localframecount_156_N1 + ((float)j));
                                    if (frame == ((frame / absrate) * absrate)) {
                                        AddVariableShotDebrisEffectMtx2(
                                            ObjectPath[i].particle_type[k], &emit, 1, &tmtx, &numtx_identity
                                        );
                                    }
                                }
                            }
                        } else if (ObjectPath[i].particle_name[k][0] != 0) {
                                ObjectPath[i].particle_type[k] = LookupDebrisEffect(ObjectPath[i].particle_name[k]);
                                if (ObjectPath[i].particle_type[k] == -1) {
                                    edobjParticleDestroy(i, k);
                                    k--;
                                }
                        }
                        k++;
                    }
                }
                for (j = 0; j < ObjectPath[i].usedsound; j++) {
                    if (ObjectPath[i].sound_id[j] != -1) {
                        sid = -1;
                        if (ObjectPath[i].sound_type[j] == 1) {
                            if (((s32)(localframecount_156_N1 + dt) / (s32)ObjectPath[i].sound_time[j]) > (((s32)localframecount_156_N1) / (s32)ObjectPath[i].sound_time[j])) {
                                sid = ObjectPath[i].sound_id[j];
                            }
                        } else {
                            if ((ltime >= ObjectPath[i].sound_time[j])
                                && (ObjectPath[i].sound_time[j] > ObjectPath[i].sound_last_time))
                            {
                                sid = ObjectPath[i].sound_id[j];
                            }
                            if (ObjectPath[i].oscillate != 0) {
                                if (ltime <= ObjectPath[i].sound_time[j]) {
                                    s32 abs_last = (s32)ObjectPath[i].sound_last_time;
                                    if (ObjectPath[i].sound_time[j]
                                        < ((float)((abs_last >= 0) ? (abs_last) : (-abs_last))))
                                    {
                                        sid = ObjectPath[i].sound_id[j];
                                    }
                                }
                            }
                        }
                        if (sid != -1) {
                            struct numtx_s* whichmtx;
                            struct nuvec_s emit;

                            if (instanim != 0) {
                                whichmtx = (struct numtx_s*)instanim;
                            } else {
                               whichmtx = (struct numtx_s*)instance;
                            }
                            if (((edobj_particle_mode != 0) || (edobj_sound_mode != 0)) && (edobj_nearest == i)) {
                                whichmtx = (struct numtx_s*)instance;
                            }
                            NuVecMtxTransform(&emit, &ObjectPath[i].sound_offset[j], whichmtx);
                            edbitsSoundPlay(&emit, sid);
                        }
                    } else if (ObjectPath[i].sound_name[j][0] != 0) {
                        ObjectPath[i].sound_id[j] = edbitsLookupSoundFX(ObjectPath[i].sound_name[j]);
                        if (ObjectPath[i].sound_id[j] == -1) {
                            edobjSoundDestroy(i, j);
                            j--;
                        }
                    }
                }
            if (ObjectPath[i].oscillate != 0) {
                ObjectPath[i].sound_last_time = -ltime;
            } else {
                ObjectPath[i].sound_last_time = ltime;
            }
        }
    }

    localframecount_156_N1 += dt;
}

//NGC MATCH
s32 edobjRenderCutoffTest(struct nuvec_s *pos) {
  float x;
  float y;
  float z;
  
  x = pos->x - global_camera.mtx._30;
  y = pos->y - global_camera.mtx._31;
  z = pos->z - global_camera.mtx._32;
  if (NuFsqrt(x * x + y * y + z * z) > 80.0f) {
      return 0;
  }
  return 1;
}

//NGC MATCH
void edobjRenderObjects(struct nugscn_s *scn) {
    struct nuinstance_s *instance;
    s32 i;
    struct nuinstanim_s *instanim;

    instance = ObjectInstance;
    
    for (i = 0; i < 0x40; i++) {
        instance = &ObjectInstance[i];
        if (instance->objid != -1) {
            instanim = instance->anim;
            if (((edobj_particle_mode != 0) || (edobj_sound_mode != 0)) && (edobj_nearest == i)) {
                instance->flags.onscreen = NuRndrGScnObj(scn->gobjs[instance->objid],&instance->mtx);
            }
            else if (instanim != NULL) {
                if (edobjRenderCutoffTest((struct nuvec_s *)&(instanim->mtx)._30) != 0) {
                    instance->flags.onscreen = NuRndrGScnObj(scn->gobjs[instance->objid],&instanim->mtx);
                }
            }
            else {
                if (edobjRenderCutoffTest((struct nuvec_s *)&ObjectInstance[i].mtx._30) != 0) {
                    instance->flags.onscreen = NuRndrGScnObj(scn->gobjs[instance->objid], &instance->mtx);
                }
            }
        }
    }
}

//NGC MATCH
void edobjResetAnimsToZero(void) {
  s32 i;
  struct nuinstance_s *instance;
  struct nuinstanim_s *instanim;
  
  instance = ObjectInstance;
  for(i = 0; i < 0x40; i++) {
    ObjectPath[i].particle_type[0] = -1;
    if (instance[i].objid != -1) {
    instanim = instance[i].anim;
        if(instanim != NULL) {
          instanim->tfirst = 0.0f;
          instanim->waiting = 1;
          instanim->ltime = ObjectPath[i].start_offset;
          instanim->playing = 0;
          instanim->backwards = 0;
        }
    }
  }
  return;
}

//NGC MATCH
s32 edobjLookupInstanceIndex(s32 specialid) {
  s32 i;
  
  if (edobj_base_scene == NULL) {
    return -1;
  }
    for(i = 0; i < edobj_base_scene->numinstance; i++) {
      if (&edobj_base_scene->instances[i] == edobj_base_scene->specials[specialid].instance) {
        return i;
      }
    }
  return -1;
}

//NGC MATCH
void edobjRegisterBaseScene(struct nugscn_s *scn) {
  edobj_base_scene = scn;
  return;
}

//NGC MATCH
void edobjDetermineNearestObject(float ndist) {
  s32 i;
  float dist;
  struct nuvec_s distv;
  
    if ((edobj_nearest != -1)) {
        NuVecSub(&distv,&edobj_cam_pos,(struct nuvec_s *)&ObjectInstance[edobj_nearest].mtx._30);
        dist = (distv.x * distv.x + distv.y * distv.y + distv.z * distv.z);
        if (dist == 0.0) return;
    }
    edobj_nearest = -1;
    for(i = 0; i < 0x40; i++) {
      if (ObjectInstance[i].objid != -1) {
        NuVecSub(&distv,&edobj_cam_pos,(struct nuvec_s *)&ObjectInstance[i].mtx._30);
        dist = distv.x * distv.x + distv.y * distv.y + distv.z * distv.z;
        if ((ndist < 0.0f) || (dist < ndist)) {
          ndist = dist;
          edobj_nearest = i;
        }
      }
    }
  return;
}

//NGC MATCH
u32 reverse_endian_32(u32 arg0) {
    return (arg0 >> 0x18U) | ((arg0 >> 8U) & 0xFF00) | ((arg0 << 8) & 0xFF0000) | (arg0 << 0x18);
}

#define OBJECT_CURVESET(obj) (ObjectAnim[obj]->chunks[0]->animcurvesets[0])

void edobjConvertPathToAnim(s32 sel)
{
    int rotanimflag;
    unsigned int mask;
    struct nuanimkey_s *akey;
    int used;
    int animflag;
    int maskcount;
    int i;
    int j;

    ObjectInstance[sel].anim = 0;
    NuMtxSetIdentity((struct Mtx *)&ObjectInstance[sel].mtx);
    ObjectInstance[sel].mtx._30 = ObjectPath[sel].waypoint[0].x;
    ObjectInstance[sel].mtx._31 = ObjectPath[sel].waypoint[0].y;
    ObjectInstance[sel].mtx._32 = ObjectPath[sel].waypoint[0].z;
    used = ObjectPath[sel].usedway;
    if (ObjectPath[sel].terrplatid == -1) {
        ObjectPath[sel].terrplatid = NewPlatInst(&ObjectInstance[sel].mtx,
                                               edobjLookupInstanceIndex(ObjectPath[sel].objid));
    } else {
        UpdatePlatinst(ObjectPath[sel].terrplatid, &ObjectInstance[sel].mtx);
        PlatInstRotate(ObjectPath[sel].terrplatid, 0);
    }
    PlatInstBounce(ObjectPath[sel].terrplatid, ObjectPath[sel].playergrav,
                   ObjectPath[sel].tension, ObjectPath[sel].damping);
    if (used < 2) {
        return;
    }
    if (ObjectAnim[sel] == 0) {
        ObjectAnim[sel] = NuAnimDataCreate(1);
        ObjectAnim[sel]->chunks[0] = NuAnimDataChunkCreate(1);
        ObjectAnim[sel]->nchunks = 1;
        ObjectAnim[sel]->chunks[0]->animcurvesets[0] = NuAnimCurveSetCreate(9);
        OBJECT_CURVESET(sel)->flags = 1;
    }
    ObjectAnim[sel]->time = 1.0f;
    maskcount = 0;
    mask = 1;
    for (i = 0; i < used - 1; i++) {
        ObjectAnim[sel]->time += ObjectPath[sel].waypoint_time[i];
        maskcount += ObjectPath[sel].waypoint_time[i];
        mask |= 1 << maskcount;
    }

    animflag = 0;
    for (i = 1; i < used; i++) {
        if (ObjectPath[sel].waypoint[i].x != ObjectPath[sel].waypoint[0].x) {
            animflag = 1;
            break;
        }
    }
    if (animflag) {
        OBJECT_CURVESET(sel)->constants[0] = 3.4028235e38f;
        if (OBJECT_CURVESET(sel)->set[0] == 0) {
            OBJECT_CURVESET(sel)->set[0] = NuAnimCurveCreate(used);
        }
        if (OBJECT_CURVESET(sel)->set[0]->numkeys != used) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[0]);
            OBJECT_CURVESET(sel)->set[0] = NuAnimCurveCreate(used);
        }
        OBJECT_CURVESET(sel)->set[0]->mask = reverse_endian_32(mask);
        for (i = 0; i < used; i++) {
            akey = &OBJECT_CURVESET(sel)->set[0]->animkeys[i];
            akey->time = 1.0f;
            for (j = 0; j < i; j++) {
                akey->time += ObjectPath[sel].waypoint_time[j];
            }
            if (i == used - 1) {
                akey->dtime = 0.0f;
            } else {
                akey->dtime = 1.0 / (float)ObjectPath[sel].waypoint_time[i];
            }
            if (i == 0) {
                akey->c = (ObjectPath[sel].waypoint[i + 1].x - ObjectPath[sel].waypoint[i].x) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else if (i == used - 1) {
                akey->c = (ObjectPath[sel].waypoint[i].x - ObjectPath[sel].waypoint[i - 1].x) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else {
                akey->c = (ObjectPath[sel].waypoint[i + 1].x - ObjectPath[sel].waypoint[i - 1].x)
                       * ObjectPath[sel].waypoint_speed[i];
            }
            akey->d = ObjectPath[sel].waypoint[i].x - ObjectPath[sel].waypoint[0].x;
        }
    } else {
        OBJECT_CURVESET(sel)->constants[0] = 0.0f;
        if (OBJECT_CURVESET(sel)->set[0] != 0) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[0]);
            OBJECT_CURVESET(sel)->set[0] = 0;
        }
    }

    animflag = 0;
    for (i = 1; i < used; i++) {
        if (ObjectPath[sel].waypoint[i].y != ObjectPath[sel].waypoint[0].y) {
            animflag = 1;
            break;
        }
    }
    if (animflag) {
        OBJECT_CURVESET(sel)->constants[1] = 3.4028235e38f;
        if (OBJECT_CURVESET(sel)->set[1] == 0) {
            OBJECT_CURVESET(sel)->set[1] = NuAnimCurveCreate(used);
        }
        if (OBJECT_CURVESET(sel)->set[1]->numkeys != used) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[1]);
            OBJECT_CURVESET(sel)->set[1] = NuAnimCurveCreate(used);
        }
        OBJECT_CURVESET(sel)->set[1]->mask = reverse_endian_32(mask);
        for (i = 0; i < used; i++) {
            akey = &OBJECT_CURVESET(sel)->set[1]->animkeys[i];
            akey->time = 1.0f;
            for (j = 0; j < i; j++) {
                akey->time += ObjectPath[sel].waypoint_time[j];
            }
            if (i == used - 1) {
                akey->dtime = 0.0f;
            } else {
                akey->dtime = 1.0 / (float)ObjectPath[sel].waypoint_time[i];
            }
            if (i == 0) {
                akey->c = (ObjectPath[sel].waypoint[i + 1].y - ObjectPath[sel].waypoint[i].y) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else if (i == used - 1) {
                akey->c = (ObjectPath[sel].waypoint[i].y - ObjectPath[sel].waypoint[i - 1].y) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else {
                akey->c = (ObjectPath[sel].waypoint[i + 1].y - ObjectPath[sel].waypoint[i - 1].y)
                       * ObjectPath[sel].waypoint_speed[i];
            }
            akey->d = ObjectPath[sel].waypoint[i].y - ObjectPath[sel].waypoint[0].y;
        }
    } else {
        OBJECT_CURVESET(sel)->constants[1] = 0.0f;
        if (OBJECT_CURVESET(sel)->set[1] != 0) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[1]);
            OBJECT_CURVESET(sel)->set[1] = 0;
        }
    }

    animflag = 0;
    for (i = 1; i < used; i++) {
        if (ObjectPath[sel].waypoint[i].z != ObjectPath[sel].waypoint[0].z) {
            animflag = 1;
            break;
        }
    }
    if (animflag) {
        OBJECT_CURVESET(sel)->constants[2] = 3.4028235e38f;
        if (OBJECT_CURVESET(sel)->set[2] == 0) {
            OBJECT_CURVESET(sel)->set[2] = NuAnimCurveCreate(used);
        }
        if (OBJECT_CURVESET(sel)->set[2]->numkeys != used) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[2]);
            OBJECT_CURVESET(sel)->set[2] = NuAnimCurveCreate(used);
        }
        OBJECT_CURVESET(sel)->set[2]->mask = reverse_endian_32(mask);
        for (i = 0; i < used; i++) {
            akey = &OBJECT_CURVESET(sel)->set[2]->animkeys[i];
            akey->time = 1.0f;
            for (j = 0; j < i; j++) {
                akey->time += ObjectPath[sel].waypoint_time[j];
            }
            if (i == used - 1) {
                akey->dtime = 0.0f;
            } else {
                akey->dtime = 1.0 / (float)ObjectPath[sel].waypoint_time[i];
            }
            if (i == 0) {
                akey->c = (ObjectPath[sel].waypoint[i + 1].z - ObjectPath[sel].waypoint[i].z) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else if (i == used - 1) {
                akey->c = (ObjectPath[sel].waypoint[i].z - ObjectPath[sel].waypoint[i - 1].z) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else {
                akey->c = -(ObjectPath[sel].waypoint[i + 1].z - ObjectPath[sel].waypoint[i - 1].z)
                       * ObjectPath[sel].waypoint_speed[i];
            }
            akey->d = -(ObjectPath[sel].waypoint[i].z - ObjectPath[sel].waypoint[0].z);
        }
    } else {
        OBJECT_CURVESET(sel)->constants[2] = 0.0f;
        if (OBJECT_CURVESET(sel)->set[2] != 0) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[2]);
            OBJECT_CURVESET(sel)->set[2] = 0;
        }
    }

    rotanimflag = 0;
    animflag = 0;
    for (i = 1; i < used; i++) {
        if (ObjectPath[sel].waypoint_rot[i].x != ObjectPath[sel].waypoint_rot[0].x) {
            animflag = 1;
            rotanimflag = 1;
            break;
        }
    }
    if (animflag) {
        OBJECT_CURVESET(sel)->constants[3] = 3.4028235e38f;
        if (OBJECT_CURVESET(sel)->set[3] == 0) {
            OBJECT_CURVESET(sel)->set[3] = NuAnimCurveCreate(used);
        }
        if (OBJECT_CURVESET(sel)->set[3]->numkeys != used) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[3]);
            OBJECT_CURVESET(sel)->set[3] = NuAnimCurveCreate(used);
        }
        OBJECT_CURVESET(sel)->set[3]->mask = reverse_endian_32(mask);
        for (i = 0; i < used; i++) {
            akey = &OBJECT_CURVESET(sel)->set[3]->animkeys[i];
            akey->time = 1.0f;
            for (j = 0; j < i; j++) {
                akey->time += ObjectPath[sel].waypoint_time[j];
            }
            if (i == used - 1) {
                akey->dtime = 0.0f;
            } else {
                akey->dtime = 1.0 / (float)ObjectPath[sel].waypoint_time[i];
            }
            if (i == 0) {
                akey->c = (ObjectPath[sel].waypoint_rot[i + 1].x - ObjectPath[sel].waypoint_rot[i].x) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else if (i == used - 1) {
                akey->c = (ObjectPath[sel].waypoint_rot[i].x - ObjectPath[sel].waypoint_rot[i - 1].x) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else {
                akey->c = (ObjectPath[sel].waypoint_rot[i + 1].x - ObjectPath[sel].waypoint_rot[i - 1].x)
                       * ObjectPath[sel].waypoint_speed[i];
            }
            akey->d = ObjectPath[sel].waypoint_rot[i].x;
        }
    } else {
        OBJECT_CURVESET(sel)->constants[3] = ObjectPath[sel].waypoint_rot[0].x;
        if (OBJECT_CURVESET(sel)->set[3] != 0) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[3]);
            OBJECT_CURVESET(sel)->set[3] = 0;
        }
    }

    animflag = 0;
    for (i = 1; i < used; i++) {
        if (ObjectPath[sel].waypoint_rot[i].y != ObjectPath[sel].waypoint_rot[0].y) {
            animflag = 1;
            rotanimflag = 1;
            break;
        }
    }
    if (animflag) {
        OBJECT_CURVESET(sel)->constants[4] = 3.4028235e38f;
        if (OBJECT_CURVESET(sel)->set[4] == 0) {
            OBJECT_CURVESET(sel)->set[4] = NuAnimCurveCreate(used);
        }
        if (OBJECT_CURVESET(sel)->set[4]->numkeys != used) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[4]);
            OBJECT_CURVESET(sel)->set[4] = NuAnimCurveCreate(used);
        }
        OBJECT_CURVESET(sel)->set[4]->mask = reverse_endian_32(mask);
        for (i = 0; i < used; i++) {
            akey = &OBJECT_CURVESET(sel)->set[4]->animkeys[i];
            akey->time = 1.0f;
            for (j = 0; j < i; j++) {
                akey->time += ObjectPath[sel].waypoint_time[j];
            }
            if (i == used - 1) {
                akey->dtime = 0.0f;
            } else {
                akey->dtime = 1.0 / (float)ObjectPath[sel].waypoint_time[i];
            }
            if (i == 0) {
                akey->c = (ObjectPath[sel].waypoint_rot[i + 1].y - ObjectPath[sel].waypoint_rot[i].y) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else if (i == used - 1) {
                akey->c = (ObjectPath[sel].waypoint_rot[i].y - ObjectPath[sel].waypoint_rot[i - 1].y) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else {
                akey->c = (ObjectPath[sel].waypoint_rot[i + 1].y - ObjectPath[sel].waypoint_rot[i - 1].y)
                       * ObjectPath[sel].waypoint_speed[i];
            }
            akey->d = ObjectPath[sel].waypoint_rot[i].y;
        }
    } else {
        OBJECT_CURVESET(sel)->constants[4] = ObjectPath[sel].waypoint_rot[0].y;
        if (OBJECT_CURVESET(sel)->set[4] != 0) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[4]);
            OBJECT_CURVESET(sel)->set[4] = 0;
        }
    }

    animflag = 0;
    for (i = 1; i < used; i++) {
        if (ObjectPath[sel].waypoint_rot[i].z != ObjectPath[sel].waypoint_rot[0].z) {
            animflag = 1;
            rotanimflag = 1;
            break;
        }
    }
    if (animflag) {
        OBJECT_CURVESET(sel)->constants[5] = 3.4028235e38f;
        if (OBJECT_CURVESET(sel)->set[5] == 0) {
            OBJECT_CURVESET(sel)->set[5] = NuAnimCurveCreate(used);
        }
        if (OBJECT_CURVESET(sel)->set[5]->numkeys != used) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[5]);
            OBJECT_CURVESET(sel)->set[5] = NuAnimCurveCreate(used);
        }
        OBJECT_CURVESET(sel)->set[5]->mask = reverse_endian_32(mask);
        for (i = 0; i < used; i++) {
            akey = &OBJECT_CURVESET(sel)->set[5]->animkeys[i];
            akey->time = 1.0f;
            for (j = 0; j < i; j++) {
                akey->time += ObjectPath[sel].waypoint_time[j];
            }
            if (i == used - 1) {
                akey->dtime = 0.0f;
            } else {
                akey->dtime = 1.0 / (float)ObjectPath[sel].waypoint_time[i];
            }
            if (i == 0) {
                akey->c = (ObjectPath[sel].waypoint_rot[i + 1].z - ObjectPath[sel].waypoint_rot[i].z) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else if (i == used - 1) {
                akey->c = (ObjectPath[sel].waypoint_rot[i].z - ObjectPath[sel].waypoint_rot[i - 1].z) * 0.5
                       * ObjectPath[sel].waypoint_speed[i];
            } else {
                akey->c = (ObjectPath[sel].waypoint_rot[i + 1].z - ObjectPath[sel].waypoint_rot[i - 1].z)
                       * ObjectPath[sel].waypoint_speed[i];
            }
            akey->d = ObjectPath[sel].waypoint_rot[i].z;
        }
    } else {
        OBJECT_CURVESET(sel)->constants[5] = ObjectPath[sel].waypoint_rot[0].z;
        if (OBJECT_CURVESET(sel)->set[5] != 0) {
            NuAnimCurveDestroy(OBJECT_CURVESET(sel)->set[5]);
            OBJECT_CURVESET(sel)->set[5] = 0;
        }
    }

    if (ObjectPath[sel].waypoint_rot[0].x != 0.0f || ObjectPath[sel].waypoint_rot[0].y != 0.0f
        || ObjectPath[sel].waypoint_rot[0].z != 0.0f) {
        rotanimflag = 1;
    }
    ObjectInstance[sel].anim = &ObjectInstAnim[sel];
    if (ObjectPath[sel].terrplatid != -1) {
        UpdatePlatinst(ObjectPath[sel].terrplatid, &ObjectInstAnim[sel].mtx);
        if (rotanimflag) {
            PlatInstRotate(ObjectPath[sel].terrplatid, 1);
        }
    }
    ObjectInstAnim[sel].tfactor = ObjectPath[sel].speed;
    ObjectInstAnim[sel].playing = 0;
    ObjectInstAnim[sel].repeating = ObjectPath[sel].repeat;
    ObjectInstAnim[sel].oscillate = ObjectPath[sel].oscillate;
    if (!ObjectPath[sel].oscillate) {
        ObjectInstAnim[sel].backwards = 0;
    }
    ObjectInstAnim[sel].anim_ix = sel;
    ObjectInstAnim[sel].tinterval = ObjectPath[sel].pause;
}

//NGC MATCH
void edobjSoundDestroy(s32 obj,s32 sound) {
  s32 i;

    for(i = sound; i < (s32)ObjectPath[obj].usedsound + -1; i++) {
      ObjectPath[obj].sound_id[i] = ObjectPath[obj].sound_id[i + 1];
      ObjectPath[obj].sound_time[i] = ObjectPath[obj].sound_time[i + 1];
      ObjectPath[obj].sound_offset[i] = ObjectPath[obj].sound_offset[i + 1];
      strcpy(ObjectPath[obj].sound_name[i],ObjectPath[obj].sound_name[i + 1]);
    }
  ObjectPath[obj].usedsound--;
  return;
}

//NGC MATCH
void edobjParticleDestroy(s32 obj,s32 ptl) {
  s32 i;
  
    for(i = ptl; i < ObjectPath[obj].usedpart - 1; i++) {
      ObjectPath[obj].particle_type[i] = ObjectPath[obj].particle_type[i + 1];
      ObjectPath[obj].particle_rate[i] = ObjectPath[obj].particle_rate[i + 1];
      ObjectPath[obj].particle_switch[i] = ObjectPath[obj].particle_switch[i + 1];
      ObjectPath[obj].particle_offset[i] = ObjectPath[obj].particle_offset[i + 1];
      strcpy(ObjectPath[obj].particle_name[i],ObjectPath[obj].particle_name[i + 1]);
    }
  ObjectPath[obj].usedpart--;
  return;
}

//NGC MATCH
s32 edobjFileLoadObjects(char *file) {
  s32 i;
  s32 j;
  s32 version;
  s32 numobjects;
  char tname[20];
  s32 dummyreads;
  struct nuvec_s dummyvec;
  char dummyname[16];
  
  if (EdFileOpen(file,NUFILE_READ) != 0) {
    version = EdFileReadInt();
    if (version > 0xd) {
        EdFileClose();
        return 0;
    } 
  } else {
      return 0;
  }
    numobjects = EdFileReadInt();
    for(i = 0; i < numobjects; i++) {
          if (version == 1) {
            ObjectPath[i].objid = EdFileReadInt();
          }
          else {
            EdFileRead(tname,0x14);
            ObjectPath[i].objid = edobjLookupInstance(tname);
          }
          j = i * 0x3ec;
          if (ObjectPath[i].objid != -1) {
            ObjectInstance[i].objid = (edobj_base_scene->specials[ObjectPath[i].objid].instance)->objid;
          }
          else {
            ObjectInstance[i].objid = -1;
          }
          ObjectPath[i].speed = EdFileReadFloat();
          ObjectPath[i].oscillate = EdFileReadInt();
          if (version > 5) {
            ObjectPath[i].repeat = EdFileReadInt();
          }
          else {
            ObjectPath[i].repeat = 1;
          }
          ObjectPath[i].pause = EdFileReadFloat();
          ObjectPath[i].usedway = EdFileReadInt();
          if (version > 8) {
            ObjectPath[i].usedpart = EdFileReadInt();
          }
          else {
            ObjectPath[i].usedpart = 0;
          }
          if (version > 9) {
            ObjectPath[i].usedsound = EdFileReadInt();
          }
          else {
            ObjectPath[i].usedsound = 0;
          }
          ObjectPath[i].terrplatid = -1;
          if (version > 5) {
            ObjectPath[i].trigger_type = EdFileReadInt();
            ObjectPath[i].trigger_id = EdFileReadInt();
            ObjectPath[i].trigger_var = EdFileReadFloat();
          }
          else {
            ObjectPath[i].trigger_type = 0;
            ObjectPath[i].trigger_id = -1;
            ObjectPath[i].trigger_var = 0.0f;
          }
          if (version > 7) {
            ObjectPath[i].trigger_wait = EdFileReadFloat();
          }
          else {
            ObjectPath[i].trigger_wait = 0.0f;
          }
          if (version > 8) {
                dummyreads = 0;
                if (8 < ObjectPath[i].usedpart) {
                  dummyreads = 8 - ObjectPath[i].usedpart;
                  ObjectPath[i].usedpart = 8;
                }
                for(j = 0; j < ObjectPath[i].usedpart; j++) {
                    EdFileRead(ObjectPath[i].particle_name[j],0x10);
                    ObjectPath[i].particle_type[j] = -1;
                    ObjectPath[i].particle_rate[j] = EdFileReadInt();
                    ObjectPath[i].particle_switch[j] = EdFileReadInt();
                    EdFileRead(&ObjectPath[i].particle_offset[j],0xc);
                    if (version > 0xc) {
                      ObjectPath[i].particle_emitrotz[j] = EdFileReadShort();
                      ObjectPath[i].particle_emitroty[j] = EdFileReadShort();
                    }
                    else {
                      ObjectPath[i].particle_emitrotz[j] = 0;
                      ObjectPath[i].particle_emitroty[j] = 0;
                    }
                }
                for(j = 0; j < dummyreads; j++) {
                    EdFileRead(dummyname,0x10);
                    EdFileReadInt();
                    EdFileReadInt();
                    EdFileRead(&dummyvec,0xc);
                    if (version > 0xc) {
                      EdFileReadShort();
                      EdFileReadShort();
                    }
                }
          }
          else if (version > 6) {
              EdFileRead(ObjectPath[i].particle_name,0x10);
              ObjectPath[i].particle_type[0] = -1;
              ObjectPath[i].particle_rate[0] = EdFileReadInt();
              ObjectPath[i].particle_switch[0] = EdFileReadInt();
              if (ObjectPath[i].particle_name[0][0] != '\0') {
                ObjectPath[i].usedpart = 1;
              }
          }
          if (version > 4) {
            ObjectPath[i].start_offset = EdFileReadInt();
          }
          else {
            ObjectPath[i].start_offset = 0;
          }
          dummyreads = 0;
          if (8 < ObjectPath[i].usedway) {
            dummyreads = 8 - ObjectPath[i].usedway;
            ObjectPath[i].usedway = 8;
          }
          ObjectPath[i].usedtime = 0;
          for(j = 0; j < ObjectPath[i].usedway; j++) {
              EdFileRead(&ObjectPath[i].waypoint[j],0xc);
              ObjectPath[i].waypoint_speed[j] = EdFileReadFloat();
              if (version > 2) {
                EdFileRead(&ObjectPath[i].waypoint_rot[j],0xc);
              }
              else {
                memset(&ObjectPath[i].waypoint_rot[j],0,0xc);
              }
              if (version > 3) {
                ObjectPath[i].waypoint_time[j] = EdFileReadInt();
              }
              else {
                if (j < ObjectPath[i].usedway - 1) {
                  ObjectPath[i].waypoint_time[j] = 2;
                }
                else {
                  ObjectPath[i].waypoint_time[j] = 0;
                }
              }
              ObjectPath[i].usedtime += ObjectPath[i].waypoint_time[j];
          }
          for(j = 0; j < dummyreads; j++) {
              EdFileRead(&dummyvec,0xc);
              EdFileReadFloat();
              if (2 < version) {
                EdFileRead(&dummyvec,0xc);
              }
              if (3 < version) {
                EdFileReadInt();
              }
          }
          dummyreads = 0;
          ObjectPath[i].sound_last_time = 0.99f;
          if (8 < ObjectPath[i].usedsound) {
            dummyreads = 8 - ObjectPath[i].usedsound;
            ObjectPath[i].usedsound = 8;
          }
          if (version > 10) {
            for(j = 0; j < ObjectPath[i].usedsound; j++) {
                EdFileRead(ObjectPath[i].sound_name[j],0x10);
                ObjectPath[i].sound_id[j] = -1;
                ObjectPath[i].sound_type[j] = EdFileReadInt();
                ObjectPath[i].sound_time[j] = EdFileReadFloat();
                EdFileRead(&ObjectPath[i].sound_offset[j],0xc);
            }
            for(j = 0; j < dummyreads; j++) {
                EdFileRead(dummyname,0x10);
                EdFileReadInt();
                EdFileReadFloat();
                EdFileRead(&dummyvec,0xc);
            }
          }
          else if (version == 10) {
            for(j = 0; j < ObjectPath[i].usedsound; j++) {
                ObjectPath[i].sound_id[j] = EdFileReadFloat();
                ObjectPath[i].sound_type[j] = 1;
                ObjectPath[i].sound_time[j] = EdFileReadFloat();
                EdFileRead(&ObjectPath[i].sound_offset[j],0xc);
            }
            for(j = 0; j < dummyreads; j++) {
                EdFileReadFloat();
                EdFileReadFloat();
                EdFileRead(&dummyvec,0xc);
            }
          }
          if (version > 0xb) {
            ObjectPath[i].playergrav = EdFileReadFloat();
            ObjectPath[i].tension = EdFileReadFloat();
            ObjectPath[i].damping = EdFileReadFloat();
          }
          else {
            ObjectPath[i].playergrav = 0.0f;
            ObjectPath[i].tension = 0.0f;
            ObjectPath[i].damping = 0.0f;
          }
      }
      EdFileClose();
      for(i = 0; i < numobjects; i++) {
          if (ObjectPath[i].objid != -1) {
            edobjConvertPathToAnim(i);
          }
      }
      edobjResetAnimsToZero();
      edobj_next_instance = numobjects;
      edobj_waypoint_mode = 0;
      edobj_copy_mode = 0;
      edobj_particle_mode = 0;
      edobj_sound_mode = 0;
      edobj_nearest = -1;
      edobjDetermineNearestObject(1.0f);
      return 1;
}

//NGC MATCH
void edobjObjectReset(void) {
  s32 i;

  for(i = 0; i < 64; i++) {
    ObjectInstance[i].objid = -1;
    ObjectPath[i].objid = -1;
  }
  edobj_next_instance = 0.0f;
  return;
}

//NGC MATCH
s32 edobjLookupInstance(char *name) {
  s32 i;
  
  if (edobj_base_scene != NULL) {
    for(i = 0; i < edobj_base_scene->numspecial; i++) {
        if (strncmp(edobj_base_scene->specials[i].name,name,0x13) == 0) {
            return i;
        }
    }
  }
  return -1;
}
