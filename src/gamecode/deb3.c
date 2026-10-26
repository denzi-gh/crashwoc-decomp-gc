#include "main.h"
#include <stddef.h>
#include <string.h>

struct deb3_s;

struct deb3info {
  s16 type;
  s16 classid;
  s16 info;
  float timer;
  float size;
  s16 deb;
  s16 rate;
  void (*impact)(struct deb3_s *);
  void (*end)(struct deb3_s *);
  s32 data;
};

struct deb3_s {
  struct numtx_s mtx;
  struct numtx_s invWorldInertiaTensor;
  struct nuvec_s velocity;
  struct nuvec_s angularvelocity;
  struct nuvec_s angularMomentum;
  struct nuvec_s impact;
  struct nuvec_s norm;
  struct nuvec_s diff;
  float shadow;
  float grav;
  s16 status;
  s16 timer;
  s16 check;
  s16 count;
  struct deb3info *info;
  s32 data;
};

struct rbclass_s {
  struct numtx_s invBodyInertiaTensor;
  float mass;
  float kr;
  float kf;
};

struct firedrop_s {
  struct nuvec_s pos;
  float time;
  s32 type;
};

typedef struct ELEC_s {
  struct nuvec_s start;
  struct nuvec_s end;
  s32 time;
  s32 ang;
} ELEC;

struct gdeb_s {
  s32 i;
  char *name;
  u64 levbits;
};

struct chaseevent_s {
  struct nugspline_s *spl;
  struct nuhspecial_s obj[24];
};

struct chase_s {
  struct nugspline_s *spl_START;
  float time;
  float duration;
  struct nugspline_s *spl_CHASER[6];
  struct nuvec_s pos[6];
  struct RPos_s RPos[6];
  struct anim_s anim[6];
  struct nuhspecial_s obj[6];
  short character[6];
  short action[6];
  u16 xrot[6];
  u16 yrot[6];
  u16 zrot[6];
  struct chaseevent_s event[6][24];
  struct nugspline_s *spl_MISC[6][4];
  struct Nearest_Light_s lights[6];
  float scale[6];
  u8 misc_phase[6];
  char ok[6];
  u8 cuboid;
  char i;
  char i_last;
  char i_next;
  char status;
  char iRAIL;
  short iALONG;
  float fALONG;
};

struct objtab_s {
  struct nuhspecial_s obj;
  struct nugscn_s **scene;
  char visible;
  char font3d_letter;
  char pad1;
  char pad2;
  char *name;
  char unk[4];
  u64 levbits;
};

extern struct gdeb_s GDeb[170];
extern struct objtab_s ObjTab[201];
extern struct nugspline_s *pVIS;
extern s32 iVIS;
extern struct nuvec_s ShadNorm;
extern struct chase_s Chase[3];
extern float ShadRoofY;
extern char *HutList[];
extern s32 rsfxJung[];
extern s32 VEHICLECONTROL;
extern s32 SKELETALCRASH;
extern s32 in_finish_range;
extern s32 exkey;

void FireBossWaterFire(s32 WaterOn);
void NuBridgeOn(s32 flag);
s32 StartHGobjAnim(struct nuhspecial_s *obj);
void DebrisEmitterOrientation(s32 key, short emitrotz, short emitroty);
void LaunchObjects(void);

static float gauntletFarOffset = 80.0f;
static float gauntletWobble = 200.0f;

struct rbclass_s rbclass[5];
volatile ELEC H2OElec[22];

s32 rsfxcount;
s32* rsfxpt;
volatile float HotCoals[13];
volatile float HotRocks[73];
s32 cmask;
struct deb3_s deb3[64];
s32 debpt;
s32 dmask;
s32 dropfire;
s32 exroty;
struct firedrop_s firedrop[16];
s32 firedroppt;
float floor1;
s32 flooron;
s32 hutexplode;
s32 hutframe;
s32 jcrunch;
s32 jonframe1;
s32 lcrunch;
s32 maskoff;
s32 maskrot;
s32 maskx;
s32 masky;
float* rockpt;
float roof1;
s32 roofon;
s32 subprop;
float torndist;
s32 water1;
s32 water2;
s32 wdir;
s32 wtimer;

struct deb3emit_s {
  float yoff;
  float grav;
  float px;
  float py;
  float pz;
  float vx;
  float vy;
  float vz;
  float ang;
};

extern volatile struct deb3emit_s Deb3Emit[7];
extern volatile struct deb3info Deb3Info[27];

//NGC MATCH
struct deb3_s* AddDeb3Ang(struct nuvec_s* pos, s32 db, s32 emit, struct nuangvec_s* angle, s32 rotx, s32 roty) {
    s32 loop;
    s32 flag;
    struct deb3_s* deb;
    float dist;
    float ang2;
    struct nuvec_s vec;
    struct nuvec_s t;
    struct nuangvec_s ang;

    for (loop = 0; loop < 0x40; loop++, debpt++) {
        deb = &deb3[debpt & 0x3f];
        if (deb->timer == 0.0f) {
            pos->x += (Deb3Emit[emit].px * (float)(qrand() - 0x8000 >> 8)) * 0.0078125f;
            pos->y += (Deb3Emit[emit].py * (float)(qrand() - 0x8000 >> 8)) * 0.0078125f;
            pos->z += (Deb3Emit[emit].pz * (float)(qrand() - 0x8000 >> 8)) * 0.0078125f;
            if (angle != NULL) {
                ang = *angle;
            } else {
                ang.x = qrand();
                ang.y = qrand();
                ang.z = qrand();
            }
            NuMtxSetRotateXYZ(&deb->mtx, &ang);
            NuMtxTranslate(&deb->mtx, pos);
            vec.x = (Deb3Emit[emit].vx * (float)(qrand() - 0x8000 >> 8)) / 120.0f;
            vec.y = (Deb3Emit[emit].vy * (float)(qrand() - 0x8000 >> 8) + Deb3Emit[emit].yoff * 128.0f) / 120.0f;
            vec.z = (Deb3Emit[emit].vz * (float)(qrand() - 0x8000 >> 8)) / 120.0f;
            NuVecRotateX(&vec, &vec, rotx);
            NuVecRotateY(&vec, &vec, roty);
            deb->velocity.x = vec.x;
            deb->velocity.y = vec.y;
            deb->velocity.z = vec.z;
            deb->grav = Deb3Emit[emit].grav * 2.0f;
            ang2 = Deb3Emit[emit].ang;
            deb->angularvelocity.x = 0.0f;
            deb->angularvelocity.y = 0.0f;
            deb->angularvelocity.z = 0.0f;
            deb->info = (struct deb3info*)&Deb3Info[db];
            if (ang2 != 0.0f) {
                if (rbclass[deb->info->classid].mass != 0.0f) {
                    ang2 = (ang2 / 2560.0f);
                }
                deb->angularMomentum.x = ((qrand() - 0x8000 >> 8) * ang2);
                deb->angularMomentum.y = ((qrand() - 0x8000 >> 8) * ang2);
                deb->angularMomentum.z = ((qrand() - 0x8000 >> 8) * ang2);
            } else {
                deb->angularMomentum.x = 0.0f;
                deb->angularMomentum.y = 0.0f;
                deb->angularMomentum.z = 0.0f;
            }
            deb->status = 0;
            deb->timer = (deb->info->timer * 60.0f);
            deb->count = 0;
            deb->diff.x = 0.0f;
            deb->diff.y = 0.0f;
            deb->diff.z = 0.0f;
            deb->data = deb->info->data;
            t.x = deb->mtx._30;
            t.y = deb->mtx._31;
            t.z = deb->mtx._32;
            vec.x = ((deb->velocity.x * 8.0f) / 60.0f);
            vec.y = ((deb->velocity.y * 8.0f) / 60.0f);
            vec.z = ((deb->velocity.z * 8.0f) / 60.0f);
            dist = (vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
            if ((deb->info->info & 8U) != 0) {
                flag = 0;
            } else {
                flag = NewRayCast(&t, &vec, deb->info->size);
            }
            if (flag == 0) {
                deb->status |= 2;
            }
            if ((flag == 2 || flag == 1 || flag == 3 || flag == 4 || flag == 5 || flag == 6 || flag == 7 || flag == 8
                 || flag == 9 || flag == 10 || flag == 11 || flag == 12 || flag == 13 || flag == 14 || flag == 15)
                || (flag > 0xf && ((deb->status & 2U) != 0)))
            {
                deb->impact.x = t.x + vec.x;
                deb->impact.y = t.y + vec.y;
                deb->impact.z = t.z + vec.z;
                vec.x = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
                deb->norm = ShadNorm;
                vec.x = NuFsqrt(vec.x);
                dist = NuFsqrt(dist);
                if (dist == 0.0f) {
                    deb->check = 0;
                } else {
                    deb->check = ((vec.x * 8.0f) / dist);
                }
                if (deb->check == 0) {
                    deb->mtx._30 = deb->impact.x;
                    deb->mtx._31 = deb->impact.y;
                    deb->mtx._32 = deb->impact.z;
                }
            } else {
                deb->check = 8;
                deb->norm.y = 100.0f;
            }
            return deb;
        }
    }
    debpt &= 0x3f;
    return NULL;
}

//NGC MATCH
struct deb3_s* AddDeb3(struct nuvec_s* pos, s32 db, s32 emit, struct nuangvec_s* angle) {
    s32 loop;
    s32 flag;
    struct deb3_s* deb;
    float dist;
    float ang2;
    struct nuvec_s vec;
    struct nuvec_s t;
    struct nuangvec_s ang;

    for (loop = 0; loop < 0x40; loop++, debpt++) {
        deb = &deb3[debpt & 0x3f];
        if (deb->timer == 0.0f) {
            pos->x += (Deb3Emit[emit].px * (float)((qrand() - 0x8000) >> 8)) * 0.0078125f;
            pos->y += (Deb3Emit[emit].py * (float)((qrand() - 0x8000) >> 8)) * 0.0078125f;
            pos->z += (Deb3Emit[emit].pz * (float)((qrand() - 0x8000) >> 8)) * 0.0078125f;
            if (angle != NULL) {
                ang = *angle;
            } else {
                ang.x = qrand();
                ang.y = qrand();
                ang.z = qrand();
            }
            NuMtxSetRotateXYZ(&deb->mtx, &ang);
            NuMtxTranslate(&deb->mtx, pos);
            deb->velocity.y =
                (Deb3Emit[emit].vy * (float)((qrand() - 0x8000) >> 8) + Deb3Emit[emit].yoff * 128.0f) / 120.0f;
            if (Deb3Emit[emit].vx >= 0.0f) {
                deb->velocity.x = (Deb3Emit[emit].vx * (float)((qrand() - 0x8000) >> 8)) / 120.0f;
                deb->velocity.z = (Deb3Emit[emit].vz * (float)((qrand() - 0x8000) >> 8)) / 120.0f;
            } else {
                deb->velocity.x = Deb3Emit[emit].vz * NuTrigTable[(u16)angle->y];
                deb->velocity.z = Deb3Emit[emit].vz * NuTrigTable[(angle->y + 0x4000) & 0xffff];
            }
            deb->grav = Deb3Emit[emit].grav * 2.0f;
            ang2 = Deb3Emit[emit].ang;
            deb->info = (struct deb3info*)&Deb3Info[db];
            deb->angularvelocity.x = 0.0f;
            deb->angularvelocity.y = 0.0f;
            deb->angularvelocity.z = 0.0f;
            if (ang2 != 0.0f) {
                if (rbclass[Deb3Info[db].classid].mass != 0.0f) {
                    ang2 = (ang2 / 2560.0f);
                }
                deb->angularMomentum.x = (float)((qrand() - 0x8000) >> 8) * ang2;
                deb->angularMomentum.y = (float)((qrand() - 0x8000) >> 8) * ang2;
                deb->angularMomentum.z = (float)((qrand() - 0x8000) >> 8) * ang2;
            } else {
                deb->angularMomentum.x = 0.0f;
                deb->angularMomentum.y = 0.0f;
                deb->angularMomentum.z = 0.0f;
            }
            deb->status = 0;
            deb->timer = (short)(deb->info->timer * 60.0f);
            deb->count = 0;
            deb->diff.x = 0.0f;
            deb->diff.y = 0.0f;
            deb->diff.z = 0.0f;
            deb->data = deb->info->data;
            t.x = deb->mtx._30;
            t.y = deb->mtx._31;
            t.z = deb->mtx._32;
            vec.x = (deb->velocity.x * 8.0f) / 60.0f;
            vec.y = (deb->velocity.y * 8.0f) / 60.0f;
            vec.z = (deb->velocity.z * 8.0f) / 60.0f;
            dist = (vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
            if ((deb->info->info & 8U) != 0) {
                flag = 0;
            } else {
                flag = NewRayCast(&t, &vec, deb->info->size);
            }
            if (flag == 0) {
                deb->status = deb->status | 2;
            }
            if ((flag == 2 || flag == 1 || flag == 3 || flag == 4 || flag == 5 || flag == 6 || flag == 7 || flag == 8
                 || flag == 9 || flag == 10 || flag == 11 || flag == 12 || flag == 13 || flag == 14 || flag == 15)
                || ((flag > 0xf && ((deb->status & 2U) != 0))))
            {
                deb->impact.x = t.x + vec.x;
                deb->impact.y = t.y + vec.y;
                deb->impact.z = t.z + vec.z;
                vec.x = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
                deb->norm = ShadNorm;
                vec.x = NuFsqrt(vec.x);
                ang2 = NuFsqrt(dist);
                if (ang2 == 0.0f) {
                    deb->check = 0;
                } else {
                    deb->check = ((vec.x * 8.0f) / ang2);
                }
                if (deb->check == 0) {
                    deb->mtx._30 = deb->impact.x;
                    deb->mtx._31 = deb->impact.y;
                    deb->mtx._32 = deb->impact.z;
                }
            } else {
                deb->check = 8;
                deb->norm.y = 100.0f;
            }
            return deb;
        }
    }
    debpt = debpt & 0x3f;
    return NULL;
}

//NGC MATCH
void InitRandSFX(void) {
  rsfxcount = 0;
  rsfxpt = NULL;
  return;
}

//NGC MATCH
void PlayRandSFX(void) {
  struct nuvec_s vec;
  s32 iVar1;
  
  if (rsfxpt != NULL) {
    rsfxcount = rsfxcount + -0x3c;
    if (rsfxcount < 1) {
      vec.x = ((s32)(qrand() - 0x8000U) * 0.00024414062f) + global_camera.mtx._30;
      vec.y = ((s32)(qrand() - 0x8000U) * 0.00024414062f) + global_camera.mtx._31;
      vec.z = ((s32)(qrand() - 0x8000U) * 0.00024414062f) + global_camera.mtx._32;
      iVar1 = qrand() * *rsfxpt;
      if (iVar1 < 0) {
        iVar1 += 0xffff;
      }
      rsfxcount = ((iVar1 >> 0x10) + rsfxpt[1]) * 0x3c;
      iVar1 = qrand() * rsfxpt[2];
      if (iVar1 < 0) {
        iVar1 += 0xffff;
      }
      GameSfx(rsfxpt[(iVar1 >> 0x10) + 3],&vec);
    }
  }
  return;
}

//NGC MATCH
void RBodySetSphereClass(int classid,float r,float mass,float kr,float kf) {
  struct rbclass_s *rbc;
  
  rbc = &rbclass[classid];
  rbc->invBodyInertiaTensor = numtx_identity;
  rbc->invBodyInertiaTensor._00 = (mass * 0.4f) * r * r;
  rbc->invBodyInertiaTensor._11 = (mass * 0.4f) * r * r;
  rbc->invBodyInertiaTensor._22 = (mass * 0.4f) * r * r;
  rbc->mass = mass;
  rbc->kr = kr;
  rbc->kf = kf;
  return;
}

//NGC MATCH
void RBodySetFakeClass(s32 classid,float r,float kr) {
  struct rbclass_s* rbc;

  rbc = &rbclass[classid];    
  rbc->mass = 0.0f;
  rbc->kr = kr;
  rbc->kf = r;
  return;
}

//NGC MATCH
static void RBodySetCubiodClass(int classid,float dx,float dy,float dz,float mass,float kr,float kf) {
  struct rbclass_s* rbc; 
  float dz2;
  float dx2;
  float dy2;

  dx2 = (dx * 0.5f);
  dy2 = (dy * 0.5f);
  dz2 = (dz * 0.5f);
  rbc = &rbclass[classid];
  rbc->invBodyInertiaTensor = numtx_identity;
  rbc->invBodyInertiaTensor._00 = 3.0f / (mass * ((dy2 * dy2) + (dz2 * dz2)));
  rbc->invBodyInertiaTensor._11 = 3.0f / (mass * ((dx2 * dx2) + (dz2 * dz2)));
  rbc->invBodyInertiaTensor._22 = 3.0f / (mass * ((dx2 * dx2) + (dy2 * dy2)));
  rbc->mass = mass;
  rbc->kr = kr;
  rbc->kf = kf;
  return;
}

//NGC MATCH
void RBodyInitClasses(void) {
  RBodySetSphereClass(0,1.0f,1.0f,0.5f,0.1f);
  RBodySetCubiodClass(1,1.0f,1.0f,1.0f,0.1f,0.9f,0.2f);
  RBodySetCubiodClass(2,1.0f,2.0f,1.0f,0.1f,0.75f,0.2f);
  RBodySetFakeClass(3,0.0f,0.9f);
  RBodySetFakeClass(4,10.0f,0.75f);
  return;
}

//NGC MATCH
void RBodyMove(struct deb3_s *deb,float dt) {
  struct rbclass_s * rbc;
  struct numtx_s tm;
  struct numtx_s tmtx;
  struct nuvec_s v;
  
  rbc = &rbclass[deb->info->classid];
  if (rbc->mass != 0.0f) {
    tmtx = deb->mtx;
    NuVecScale(dt,&v,&deb->angularvelocity);
    NuMtxSkewSymmetric(&tmtx,&v);
    NuMtxMulR(&tmtx,&deb->mtx,&tmtx);
    NuMtxAddR(&tmtx,&tmtx,&deb->mtx);
    tmtx._30 = ((deb->velocity).x * dt + (deb->mtx)._30);
    tmtx._31 = ((deb->velocity).y * dt + (deb->mtx)._31);
    tmtx._32 = ((deb->velocity).z * dt + (deb->mtx)._32);
    (deb->velocity).y = (deb->grav * dt + (deb->velocity).y);
    NuMtxOrth(&tmtx);
    NuMtxInvR(&tm,&tmtx);
    NuMtxMulR(&deb->invWorldInertiaTensor,&tm,&rbc->invBodyInertiaTensor);
    NuMtxMulR(&deb->invWorldInertiaTensor,&deb->invWorldInertiaTensor,&tmtx);
    NuVecInvMtxRotate(&deb->angularvelocity,&deb->angularMomentum,&deb->invWorldInertiaTensor);
    deb->mtx = tmtx;
  }
  else {
    if (rbc->kf != 0.0f) {
      NuMtxPreRotateX(&deb->mtx,(deb->angularMomentum).x);
      NuMtxPreRotateY(&deb->mtx,(deb->angularMomentum).y);
      NuMtxPreRotateZ(&deb->mtx,(deb->angularMomentum).z);
    }
    (deb->mtx)._30 = ((deb->velocity).x * dt + (deb->mtx)._30);
    (deb->mtx)._31 = ((deb->velocity).y * dt + (deb->mtx)._31);
    (deb->mtx)._32 = ((deb->velocity).z * dt + (deb->mtx)._32);
    (deb->velocity).y = (deb->grav * dt + (deb->velocity).y);
      
  }
  return;
}

//NGC MATCH
void RBodyImpact(struct deb3_s *deb,struct nuvec_s *pnt,struct nuvec_s *nrm) {
  struct rbclass_s* rbc;
  struct nuvec_s R;
  struct nuvec_s velocity;
  struct nuvec_s v;
  struct nuvec_s impulse;
  float impulseD;
  float impulseND;

  rbc = &rbclass[deb->info->classid];
  NuVecSub(&R,pnt,(struct nuvec_s *)&(deb->mtx)._30);
  NuVecCross(&velocity,&R,&deb->angularvelocity);
  NuVecAdd(&velocity,&velocity,&deb->velocity);
  impulseD = NuVecDot(&velocity,nrm);
  impulseND = (impulseD * -(rbc->kr + 1.0f));
  NuVecCross(&v,&R,nrm);
  NuVecInvMtxRotate(&v,&v,&deb->invWorldInertiaTensor);
  NuVecCross(&v,&v,&R);
  impulseD = NuVecDot(&v,nrm);
  impulseD = (impulseND / ((1.0f / rbc->mass) + impulseD));
  impulse.x = impulseD * nrm->x;
  impulse.y = impulseD * nrm->y;
  impulse.z = impulseD * nrm->z;
  (deb->velocity).x = (deb->velocity).x + (impulse.x / rbc->mass );
  (deb->velocity).y = (deb->velocity).y + impulse.y / rbc->mass;
  (deb->velocity).z = (deb->velocity).z + impulse.z / rbc->mass;
  NuVecCross(&v,&impulse,&R);
  (deb->angularMomentum).x = (deb->angularMomentum).x + v.x;
  (deb->angularMomentum).y = (deb->angularMomentum).y + v.y;
  (deb->angularMomentum).z = (deb->angularMomentum).z + v.z;
  NuVecInvMtxRotate(&v,&deb->angularMomentum,&deb->invWorldInertiaTensor);
  (deb->angularvelocity).x = (deb->angularvelocity).x + v.x;
  (deb->angularvelocity).y = (deb->angularvelocity).y + v.y;
  (deb->angularvelocity).z = (deb->angularvelocity).z + v.z;
  return;
}

//NGC MATCH
void InitDeb3(void) {
  InitRandSFX();
  memset(firedrop,0,0x140);
  torndist = 99999.0f;
  debpt = 0;
  firedroppt = 0;
  dropfire = 0;
  hutexplode = 0;
  hutframe = 0;
  exroty = 0;
  jonframe1 = 0;
  subprop = 0;
  water1 = 0;
  water2 = 0;
  wdir = 0;
  wtimer = 0;
  jcrunch = 0;
  lcrunch = 0;
  cmask = 0;
  dmask = 0;
  maskx = 0;
  masky = 0;
  maskoff = 0;
  maskrot = 0;
  memset(deb3,0,0x3800);
  RBodyInitClasses();
  rockpt = NULL;
  if (Level == 1) {
    rockpt = HotRocks;
  }
  if (Level == 0x11) {
    rockpt = HotCoals;
  }
  if (Level == 0x14) {
    floor1 = -0.725f;
    roof1 = 2.225f;
    flooron = 0;
    roofon = 0;
  }
  return;
}

//NGC MATCH
s32 JonMaskFPS(s32 val,s32 add) {
    return add * 0x3c + val <= 0x3c0000 ? val + add * 0x3c : (add * 0x3c + val) - 0x3c0000;
}

//NGC MATCH
void JonExtraDraw(void) {
    ELEC* pt;
    s32 i;
    s32 d;
    s32 col;
    struct nuvec_s vec;

    if (Level == 0x19) {
        switch (cmask) {
            default:
                i = -1;
                d = 0x7e;
                break;
            case 1:
                i = CRemap[85];
                d = 0x7e;
                break;
            case 2:
                i = CRemap[87];
                d = 0x7f;
                break;
            case 3:
                i = CRemap[86];
                d = 0x80;
                break;
            case 4:
                i = CRemap[88];
                d = 0x81;
                break;
        }
        vec.x = 0.0f;
        vec.y = 5.0f - NuTrigTable[(u16)(maskoff / 0x3c)] * 3.0f;
        vec.z = 2.87f;
        vec.x = NuTrigTable[(u16)(maskx / 0x3c)] * 1.5f + vec.x;
        vec.y += (NuTrigTable[(u16)(masky / 0x3c)] * 0.75f);
        vec.z = NuTrigTable[(u16)((maskx / 0x3c) + 0x4000)] * 3.0f + vec.z;
        if (i != -1) {
            Draw3DCharacter(&vec, 0, (u16)(NuTrigTable[(u16)(maskrot / 0x3c)] * 8192.0f), 0, &CModel[i], -1, 1.0f, 1.0f, 0);
            vec.y += 0.2f;
            if (Paused == 0) {
                AddVariableShotDebrisEffect(GDeb[d].i, &vec, 1, 0, 0);
            }
        }
    }
    if (Level != 2) {
        return;
    }
    pt = (ELEC*)&H2OElec[0];
    if (Paused != 0) {
        return;
    }
        for (pt; (pt->start).x != 99999.0f; pt++) {
            vec.x = (pt->start).x - (player->obj).pos.x;
            vec.z = pt->start.z - (player->obj).pos.z;
            if ((vec.x * vec.x + vec.z * vec.z) < 625.0f) {
                pt->time = pt->time - 0x3c;
                if (pt->time < 0) {
                    pt->time = ((qrand() & 0xff) + 0x3c) * 0x3c;
                    pt->ang = qrand() & 0xffff;
                }
                if (pt->time < 0x960) {
                    if (qrand() < 0x4000) {
                        GameSfx(0x89, &pt->start);
                    }
                    if (pt->time > 0x5a0) {
                        col = ((0x28 - pt->time) << 4) / 0x3c;
                    } else if (pt->time < 0x3c0) {
                        col = (pt->time << 4) / 0x3c;
                    } else {
                        col = 0x80;
                    }
                    col = col << 0x18;
                    vec.x = NuTrigTable[pt->ang & 0xffff] * 0.47999999f;
                    vec.y = 0.0f;
                    vec.z = NuTrigTable[(pt->ang + 0x4000U) & 0x3fffc / 4] * 0.47999999f;
                    NuLgtArcLaser( 0, &pt->start, &pt->end, &vec, 0.05f, 0.1f, 0.01f, 0.1f, col + 0x00FF7F00);
                    NuLgtArcLaser(0, &pt->start, &pt->end, &vec, 0.4f, 0.3f, 0.001f, 0.1f, col | 0x800000);
                    AddVariableShotDebrisEffect(GDeb[143].i, &pt->end, 1, 0, 0);
                }
            }
        }
    return;
}

//NGC MATCH
static void DrawDeb3() {
  struct nuvec_s pos;
  struct deb3_s *deb;
  float r;
  s32 i;
  s32 j;
  
  JonExtraDraw();
  deb = deb3;
  for(i = 0; i < 0x40; i++, deb++) {
    if (deb->timer != 0) {
      if (ObjTab[deb->info->type].obj.special != NULL) {
        NuSpecialDrawAt(&ObjTab[deb->info->type].obj,&deb->mtx);
        if ((deb->info->info & 0x10U) != 0) {
          if (deb->shadow != 2000000.0f) {
            pos.x = deb->mtx._30;
            pos.y = deb->shadow;
            pos.z = deb->mtx._32;
            r = (deb->info->size * 0.75f);
            if (0.0f < r) {
              NuRndrAddShadow(&pos,r,0x7f,0,0,0);
            }
          }
        }
      }
    }
  }
  if (rockpt == HotRocks) {
    i = (s32)(pVIS - world_scene[0]->splines);
    if ((i == 1) && (iVIS < 0x8d)) {
      if (ObjTab[i + 0x3e].obj.special != NULL) {
        NuSpecialDrawAt(&ObjTab[i + 0x3e].obj,&(ObjTab[i + 0x3e].obj.special)->mtx);
      }
    }
    else if (((i == 6 || i == 7) || (i == 9))) {
        i = 64;
        if ((ObjTab[i].obj.special != NULL))  {
            NuSpecialDrawAt(&ObjTab[i].obj,&(ObjTab[i].obj.special)->mtx);
        }
    }
  }
}

//NGC MATCH PART OF DRAWDEB3
void RockSpark(struct deb3_s *deb) {
  s32 key;
  struct nuvec_s vec;
  
  vec.x = (deb->mtx)._30;
  vec.y = (deb->mtx)._31;
  vec.z = (deb->mtx)._32;
  key = -1;
  deb->data--;
  if (deb->data < 0) {
    deb->timer = 1;
  }
  else {
    AddFiniteShotDebrisEffect(&key,GDeb[33].i,&vec,1);
    GameSfx(0x5f,&vec);
  }
  return;
}

//NGC MATCH PART OF DRAWDEB3
void CoalSpark(struct deb3_s *deb) {
  s32 key;
  struct nuvec_s vec;
  
  vec.x = (deb->mtx)._30;
  vec.y = (deb->mtx)._31;
  vec.z = (deb->mtx)._32;
  key = -1;
  deb->data--;
  if (deb->data < 0) {
    deb->timer = 1;
  }
  else {
    AddFiniteShotDebrisEffect(&key,GDeb[35].i,&vec,1);
  }
  return;
}

//NGC MATCH PART OF DRAWDEB3
void RockBreak(struct deb3_s *deb) {
  s32 key;
  struct nuvec_s vec;
  
  vec.x = (deb->mtx)._30;
  vec.y = (deb->mtx)._31;
  vec.z = (deb->mtx)._32;
  key = -1;
  AddFiniteShotDebrisEffect(&key,GDeb[33].i,&vec,1);
  GameSfx(0x5f,&vec);
  return;
}

//NGC MATCH PART OF DRAWDEB3
void CoalBreak(struct deb3_s *deb) {
  s32 key;
  struct nuvec_s vec;
  
  vec.x = (deb->mtx)._30;
  vec.y = (deb->mtx)._31;
  vec.z = (deb->mtx)._32;
  key = -1;
  AddFiniteShotDebrisEffect(&key,GDeb[36].i,&vec,1);
  return;
}

//NGC MATCH PART OF DRAWDEB3
void DynaBreak(struct deb3_s *deb) {
  struct nuvec_s pos;
  
  pos.x = (deb->mtx)._30;
  pos.y = (deb->mtx)._31;
  pos.z = (deb->mtx)._32;
  AddGameDebris(0x44,&pos);
  GameSfx(0x3b,&pos);
  return;
}

//99.79% NGC (level dispatch/scheduling/regswap)
void LaunchObjects(void) {
    float* pt;
    float temp;
    float dx;
    float dy;
    float dz;
    s32 loop;
    s32 key;
    s32 count;
    s32 ang;
    struct nuvec_s vec;
    struct nuvec_s vec2;
    struct nuhspecial_s obj;
    s32 r;
    s32 g;
    s32 b;
    s32 hazea;

    jonframe1++;
    r = 0x80;
    g = 0x80;
    b = 0x80;
    hazea = 0;
    if (Level == 0x19) {
        if (jcrunch == 1) {
            b = 1;
            g = 0x40;
        }
        if (jcrunch == 2) {
            r = 0x40;
            g = 0x60;
            b = 0x7E;
        }
        if (jcrunch == 3) {
            r = 0x80;
            g = 0x40;
            b = 3;
            hazea = 1;
        }
        if (jcrunch == 4) {
            g = 0;
            b = 0x40;
            hazea = 0;
            r = (s32)(NuTrigTable[(u16)(water1 / 60)] * 64.0f) + 0x40;
        }
        if (jcrunch == 0) {
            if (water2 < 0) {
                water2 = -water2;
            }
            water2 -= 480;
            if (water2 < 0) {
                water2 = 0;
            }
            if (LDATA->hazea != 0) {
                LDATA->hazea -= 8;
                if ((s8)LDATA->hazea < 0) {
                    LDATA->hazea = 0;
                }
            }
        } else if (water2 >= 0) {
            if (LDATA->hazea != 0) {
                LDATA->hazea -= 8;
                if ((s8)LDATA->hazea < 0) {
                    LDATA->hazea = 0;
                }
            }
            water2 -= 480;
            if (water2 < 0) {
                LDATA->fogr = r;
                LDATA->fogg = g;
                LDATA->fogb = b;
                LDATA->hazea = hazea;
            }
        } else {
            vec.x = 0.0f;
            vec.y = 2.13f;
            vec.z = 4.69f;
            switch (LDATA->fogb & 3) {
                case 1:
                    AddVariableShotDebrisEffect(GDeb[122].i, &vec, 1, 0, 0);
                    break;
                case 2:
                    AddVariableShotDebrisEffect(GDeb[123].i, &vec, 1, 0, 0);
                    break;
                case 3:
                    vec.y += 3.43f;
                    AddVariableShotDebrisEffect(GDeb[125].i, &vec, 2, -0x8000, 0);
                    break;
                case 0:
                    AddVariableShotDebrisEffect(GDeb[124].i, &vec, 1, 0, 0);
                    break;
            }
            water2 = water2 - 0x1e0;
            if (water2 < -0x1e00) {
                water2 = -0x1e00;
            }
            if ((LDATA->hazea != 0) && (LDATA->hazea += 8, (LDATA->hazea & 0x80) != 0)) {
                LDATA->hazea = 0x7f;
            }
            if (LDATA->fogb == 0x40) {
                water1 = JonMaskFPS(water1, 800);
                LDATA->fogr = (char)(NuTrigTable[(water1 / 0x3c) & 0xffff] * 64.0f) + 0x40;
            }
        }
        LDATA->foga = ((water2 / 0x78) >= 0 ? (water2 / 0x78) : -(water2 / 0x78));
        dmask = jcrunch;
        if ((cmask == jcrunch) && (cmask != 0)) {
            maskoff += 0x5a00;
            if (maskoff > 0xf0000) {
                maskoff = 0xf0000;
            }
        } else {
            maskoff -= 0x5a00;
            if (maskoff < 0) {
                maskoff = 0;
                cmask = jcrunch;
            }
        }
        maskx = JonMaskFPS(maskx, 0x1c2);
        masky = JonMaskFPS(masky, 800);
        maskrot = JonMaskFPS(maskrot, 600);
    }
    if ((Level == 0x21) && ((jonframe1 & 1U) == 0)) {
        vec.x = global_camera.mtx._30;
        vec.y = global_camera.mtx._31;
        vec.z = global_camera.mtx._32;
        vec2.x = player->obj.pos.x - global_camera.mtx._30;
        vec2.z = player->obj.pos.z - global_camera.mtx._32;
        ang = NuAtan2D(vec2.x, vec2.z);
        vec.x = NuTrigTable[ang & 0xffff] * 5.0f + vec.x;
        vec.z = NuTrigTable[(ang + 0x4000) & 0xffff] * 5.0f + vec.z;
        AddVariableShotDebrisEffect(GDeb[115].i, &vec, 1, 0, 0);
    }
    if ((Level == 0x1f) && ((jonframe1 & 1U) == 0)) {
        vec.x = global_camera.mtx._30;
        vec.y = global_camera.mtx._31;
        vec.z = global_camera.mtx._32;
        vec2.x = player->obj.pos.x - global_camera.mtx._30;
        vec2.z = player->obj.pos.z - global_camera.mtx._32;
        ang = NuAtan2D(vec2.x, vec2.z);
        vec.x = NuTrigTable[ang & 0xffff] * 5.0f + vec.x;
        vec.z = NuTrigTable[(ang + 0x4000) & 0xffff] * 5.0f + vec.z;
        AddVariableShotDebrisEffect(GDeb[115].i, &vec, 1, 0, 0);
    }
    if (Level == 0x14) {
        if ((jonframe1 & 1U) == 0) {
            vec.x = global_camera.mtx._30;
            vec.y = global_camera.mtx._31;
            vec.z = global_camera.mtx._32;
            vec2.x = player->obj.pos.x - global_camera.mtx._30;
            vec2.z = player->obj.pos.z - global_camera.mtx._32;
            ang = NuAtan2D(vec2.x, vec2.z);
            vec.x = NuTrigTable[ang & 0xffff] * 5.0f + vec.x;
            vec.z = NuTrigTable[(ang + 0x4000) & 0xffff] * 5.0f + vec.z;
            AddVariableShotDebrisEffect(GDeb[115].i, &vec, 1, 0, 0);
        }
        if ((flooron != 0) || (roofon != 0)) {
            for (loop = 0; loop < 4; loop++) {
                vec.x = (float)(qrand() - 0x8000) * 0.00024414062f + player->obj.pos.x;
                vec.y = player->obj.pos.y + 0.5f;
                vec.z = (float)(qrand() - 0x8000) * 0.00024414062f + player->obj.pos.z;
                if ((vec.z > 58.0f) && (vec.z < 66.0f)) {
                    vec.y = NewShadow(&vec, 0.0f);
                    if ((flooron != 0)
                        && ((((vec.y != 2000000.0f && (vec.x > 18.0f)) && (vec.x < 50.0f)) && (vec.y > (floor1 - 0.2f)))
                        ))
                    {
                        vec.y = (vec.y - 0.1f);
                        key = -1;
                        AddFiniteShotDebrisEffect(&key, GDeb[109].i, &vec, 1);
                    }
                    if (roofon != 0) {
                        vec.y = ShadRoofY;
                        if (((ShadRoofY != 2000000.0f) && (ShadRoofY < (roof1 + 0.2f)))
                            && ((15.0f < vec.x && (vec.x < 55.0f))))
                        {
                            vec.y = (ShadRoofY + 0.1f);
                            key = -1;
                            AddFiniteShotDebrisEffect(&key, GDeb[109].i, &vec, 1);
                        }
                    }
                }
            }
            if (roofon != 0) {
                roofon = roofon - 0x3c;
                if (roofon < 0) {
                    roofon = 0;
                }
                if (roofon < 0x1068) {
                    if (ObjTab[150].obj.special != NULL) {
                        ((ObjTab[150].obj.special)->instance->mtx)._31 = floor1 - 100.0f;
                    }
                    if (ObjTab[151].obj.special != NULL) {
                        ((ObjTab[151].obj.special)->instance->mtx)._31 = roof1;
                    }
                }
                goto LAB_8002512c;
            }
        }
        if (flooron != 0) {
            flooron -= 0x3c;
            if (flooron < 0) {
                flooron = 0;
            }
            if (flooron < 0x1068) {
                if (ObjTab[150].obj.special != NULL) {
                    ((ObjTab[150].obj.special)->instance->mtx)._31 = floor1;
                }
                if (ObjTab[151].obj.special != NULL) {
                    ((ObjTab[151].obj.special)->instance->mtx)._31 = roof1 - 100.0f;
                }
            }
        } else {
            if (ObjTab[150].obj.special != NULL) {
                ((ObjTab[150].obj.special)->instance->mtx)._31 = floor1 - 100.0f;
            }
            if (ObjTab[151].obj.special != NULL) {
                ((ObjTab[151].obj.special)->instance->mtx)._31 = roof1 - 100.0f;
            }
            water1 = water1 - 0x3c;
            if (water1 < 0) {
                water1 = ((qrand() & 0x7f) + 0x1e) * 0x3c;
                water2 = 1 - water2;
                if (water2 != 0) {
                    flooron = 0x1c20;
                } else {
                    roofon = 0x1c20;
                }
            }
        }
    }
LAB_8002512c:
    if (((Level == 0x16) && (VEHICLECONTROL == 1)) && ((player->obj.anim.newaction == 0x68 && (Pad[0] != NULL)))) {
        FireBossWaterFire(Pad[0]->paddata & 0x80);
        vec.x = player->mtxLOCATOR[8][0]._30;
        vec.y = player->mtxLOCATOR[8][0]._31;
        vec.z = player->mtxLOCATOR[8][0]._32;
        if ((Pad[0]->paddata & 0x80) != 0) {
            AddVariableShotDebrisEffect(GDeb[108].i, &vec, 4, 0x3d54, player->obj.hdg + 0x4000);
        }
    }
    if ((Level == 0xe) && (Chase[0].status == 2)) {
        vec.x = Chase[0].pos[0].x;
        vec.y = Chase[0].pos[0].y;
        vec.z = Chase[0].pos[0].z;
        vec.z -= 2.5f;
        vec.y += 0.2f;
        AddVariableShotDebrisEffect(GDeb[100].i, &vec, 2, 0, 0);
        vec.y = Chase[0].pos[0].y + 0.6f;
        AddVariableShotDebrisEffect(GDeb[101].i, &vec, 1, 0, 0);
        vec.y = Chase[0].pos[0].y + 3.0f;
        AddVariableShotDebrisEffect(GDeb[103].i, &vec, 2, -0x218, 0);
        vec.y = Chase[0].pos[0].y + 3.6f;
        AddVariableShotDebrisEffect(GDeb[102].i, &vec, 4, -0x600, 0);
    }
    if (Level == 0x13) {
        dx = global_camera.mtx._30 - -0.0f;
        dz = global_camera.mtx._32 - -0.0f;
        dy = dx * dx + dz * dz;
        dx = NuFsqrt(dy);
        dx = 50.0f - dx * 0.125f;
        if (dx < 0.0f) {
            dx = 0.0f;
        }
        LDATA->fogg = dx;
    }
    if (((Level == 0x22) || (Level == 2)) || (Level == 6)) {
        LDATA->fogg = (char)(NuTrigTable[(water1 / 0x3c) & 0xffff] * 30.0f) + 0x28;
        LDATA->fogb = (char)(NuTrigTable[(water2 / 0x3c) & 0xffff] * 30.0f) + 0x32;
        water1 = JonMaskFPS(water1, 0x6e);
        water2 = JonMaskFPS(water2, 0x69);
        if (((jonframe1 & 1U) == 0) && (VEHICLECONTROL != 0)) {
            vec.x = global_camera.mtx._30;
            vec.y = global_camera.mtx._31;
            vec.z = global_camera.mtx._32;
            vec2.x = player->obj.pos.x - global_camera.mtx._30;
            vec2.z = player->obj.pos.z - global_camera.mtx._32;
            ang = NuAtan2D(vec2.x, vec2.z);
            vec.x = NuTrigTable[ang & 0xffff] * 5.0f + vec.x;
            vec.z = NuTrigTable[(ang + 0x4000) & 0xffff] * 5.0f + vec.z;
            AddVariableShotDebrisEffect(GDeb[151].i, &vec, 1, 0, 0);
        }
    }
    if ((Level == 0xf) && (player->obj.dead == 0)) {
        key = (u32)LDATA->foga * 0x3c;
        if (SKELETALCRASH != 0) {
            if (key < 0x1e00) {
                key = 0x1e00;
            }
            if (key <= 0x3a1f) {
                key += 0x1e0;
            } else {
                key = 0x3bc4;
            }
        } else {
            if (0x1e00 < key) {
                key = 0x1e00;
            }
            if (0xf00 < key) {
                key -= 0x1e0;
            } else {
                key = 0xf00;
            }
        }
        LDATA->foga = (s8)((float)(key / 0x3c) * 0.5f);
        key = (u32)LDATA->hazea * 0x3c;
        if (VEHICLECONTROL != 1) {
            key -= 300;
            if (key < 0) {
                key = 0;
            }
        } else {
            key += 300;
            if (key > 0x2b98) {
                key = 0x2b98;
            }
            vec.x = NuTrigTable[(player->obj.hdg + 0x1000) & 0xffff] * -0.3f + player->obj.pos.x;
            vec.y = player->obj.pos.y + 0.55f;
            vec.z = NuTrigTable[(player->obj.hdg + 0x5000) & 0xffff] * -0.3f + player->obj.pos.z;
            AddVariableShotDebrisEffect(GDeb[104].i, &vec, 1, 0x4000, player->obj.hdg + 0xc000);
            vec.x = NuTrigTable[(player->obj.hdg - 0x1000) & 0xffff] * -0.3f + player->obj.pos.x;
            vec.z = NuTrigTable[(player->obj.hdg + 0x3000) & 0xffff] * -0.3f + player->obj.pos.z;
            AddVariableShotDebrisEffect(GDeb[104].i, &vec, 1, 0x4000, player->obj.hdg + 0xc000);
        }
        LDATA->hazea = (u8)((s32)key / 0x3c);
    }
    if (((Level == 0x1d) && (in_finish_range < 0x3c)) && (player->obj.dead == 0)) {
        vec.x = NuTrigTable[(player->obj.hdg + 0x1000) & 0xffff] * -0.3f + player->obj.pos.x;
        vec.y = player->obj.pos.y + 0.55f;
        vec.z = NuTrigTable[(player->obj.hdg + 0x5000) & 0xffff] * -0.3f + player->obj.pos.z;
        AddVariableShotDebrisEffect(GDeb[104].i, &vec, 1, 0x4000, player->obj.hdg + 0xc000);
        vec.x = NuTrigTable[(player->obj.hdg - 0x1000) & 0xffff] * -0.3f + player->obj.pos.x;
        vec.z = NuTrigTable[(player->obj.hdg + 0x3000) & 0xffff] * -0.3f + player->obj.pos.z;
        AddVariableShotDebrisEffect(GDeb[104].i, &vec, 1, 0x4000, player->obj.hdg + 0xc000);
    }
    if (Level == 10) {
        dx = global_camera.mtx._30 - -0.46f;
        dz = global_camera.mtx._32 - -77.3f;
        dy = dx * dx + dz * dz;
        dx = NuFsqrt(dy) - 8.0f;
        if (dx < 1.0f) {
            dx = 1.0f;
        }
        if (dx > 20.0f) {
            LDATA->fogr = 0;
            LDATA->fogg = 0;
            LDATA->fogb = 0;
            LDATA->foga = 0;
            LDATA->hazea = 0;
            LDATA->fognear = 20.0f;
            LDATA->fogfar = gauntletFarOffset;
        } else {
            float fog_scale = 1.0f - dx / 20.0f;
            float haze_height;
            float haze_scale;
            float distance_scale;

            LDATA->fogr = (s32)(fog_scale * 80.0f);
            LDATA->fogg = (s32)(fog_scale * 15.0f);
            LDATA->fogb = 0;
            LDATA->foga = 0x40;
            haze_height = 3.0f - dx;
            haze_scale = gauntletWobble / 3.0f;
            LDATA->hazea = (u8)(s32)((haze_height < 0.0f) ? (haze_scale * 0.0f) : (haze_height * haze_scale));
            distance_scale = gauntletFarOffset / 20.0f;
            LDATA->fognear = dx;
            LDATA->fogfar = -(20.0f - LDATA->fognear) * distance_scale + gauntletFarOffset + 20.0f;
        }
    }
    if ((VEHICLECONTROL == 1) && (player->obj.vehicle == 0x20)) {
        vec.y = NuTrigTable[(u16)(subprop / 0x3c)] * 0.15f + player->obj.pos.y;
        dx = NuTrigTable[((u16)(subprop / 0x3c) + 0x4000) & 0xffff] * 0.15f + 0.8f;
        vec.x = NuTrigTable[(player->obj.hdg + 0x5000) & 0xffff] * dx + player->obj.pos.x;
        vec.z = NuTrigTable[(player->obj.hdg - 0x7000) & 0xffff] * dx + player->obj.pos.z;
        AddVariableShotDebrisEffect(GDeb[22].i, &vec, 1, 0x4000, player->obj.hdg + 0xc000);
        vec.x = NuTrigTable[(player->obj.hdg - 0x5000) & 0xffff] * dx + player->obj.pos.x;
        vec.z = NuTrigTable[(player->obj.hdg - 0x1000) & 0xffff] * dx + player->obj.pos.z;
        AddVariableShotDebrisEffect(GDeb[22].i, &vec, 1, 0x4000, player->obj.hdg + 0xc000);
        subprop = JonMaskFPS(subprop, 35000);
    }
    if ((((Level == 0x22) || (Level == 6)) && ((player->tap != 0 || (player->spin != 0)))) && (VEHICLECONTROL == 2)) {
        AddVariableShotDebrisEffect(GDeb[20].i, &player->obj.pos, 1, 0, 0);
    }
    if (Level == 0xb) {
        vec.x = global_camera.mtx._30;
        vec.y = global_camera.mtx._31 - 9.0f;
        vec.z = global_camera.mtx._32;
    }
    if ((Level - 0xbU < 2) && ((jonframe1 & 1U) == 0)) {
        vec.x = global_camera.mtx._30;
        vec.y = global_camera.mtx._31;
        vec.z = global_camera.mtx._32;
        vec2.x = player->obj.pos.x - global_camera.mtx._30;
        vec2.z = player->obj.pos.z - global_camera.mtx._32;
        ang = NuAtan2D(vec2.x, vec2.z);
        vec.x = NuTrigTable[ang & 0xffff] * 5.0f + vec.x;
        vec.z = NuTrigTable[(ang + 0x4000) & 0xffff] * 5.0f + vec.z;
        AddVariableShotDebrisEffect(GDeb[98].i, &vec, 1, 0, 0);
    }
    if (Level == 0x1a) {
        dx = 10.0f;
        for (loop = 0; loop < 4; loop++) {
            vec.x = global_camera.mtx._30;
            vec.y = global_camera.mtx._31;
            vec.z = global_camera.mtx._32;
            vec2.x = player->obj.pos.x - global_camera.mtx._30;
            vec2.z = player->obj.pos.z - global_camera.mtx._32;
            ang = NuAtan2D(vec2.x, vec2.z);
            vec.x = NuTrigTable[ang & 0xffff] * dx + vec.x;
            vec.z = NuTrigTable[(ang + 0x4000) & 0xffff] * dx + vec.z;
            AddVariableShotDebrisEffect(GDeb[113].i, &vec, 1, 0, 0);
        }
    }
    if (Level == 0xd) {
        dx = NuFsqrt(torndist) * 0.25f;
        if (dx < 30.0f) {
            if (dx < 10.0f) {
                key = 1;
                count = (s32)((10.0f - dx) + 1.0f);
                if (count > 5) {
                    count = 5;
                }
            } else {
                count = 1;
                key = (s32)(dx / 10.0f);
            }
        } else {
            count = 1;
            key = 3;
        }
        if (((jonframe1 == (jonframe1 / (s32)key) * key) || (count != 1)) && (0 < count)) {
            dx = 10.0f;
            for (loop = count; loop != 0; loop--) {
                vec.x = global_camera.mtx._30;
                vec.y = global_camera.mtx._31;
                vec.z = global_camera.mtx._32;
                vec2.x = player->obj.pos.x - global_camera.mtx._30;
                vec2.z = player->obj.pos.z - global_camera.mtx._32;
                ang = NuAtan2D(vec2.x, vec2.z);
                vec.x = (NuTrigTable[ang & 0xffff] * dx) + vec.x;
                vec.z = (NuTrigTable[(ang + 0x4000) & 0xffff] * dx) + vec.z;
                if ((qrand() & 7) != 0) {
                    AddVariableShotDebrisEffect(GDeb[110].i, &vec, 1, 0, 0);
                } else if ((jonframe1 & 1U) != 0) {
                    AddVariableShotDebrisEffect(GDeb[111].i, &vec, 1, 0, 0);
                } else {
                    AddVariableShotDebrisEffect(GDeb[112].i, &vec, 1, 0, 0);
                }
            }
        }
    }
    switch (Level) {
      case 8:
      case 0xe:
      case 0x17:
        if (Level != 8) {
            water1 = water1 - 0x3c;
            if (water1 < 0) {
                if (qrand() < 0x8000) {
                    water1 = ((qrand() & 0xff) + 0x3c) * 0x3c;
                } else {
                    water1 = ((qrand() & 0xf) + 10) * 0x3c;
                }
            }
            if (water1 < 0xb4) {
                if (0xb3 < water1 + 0x3c) {
                    if (Level == 0xe) {
                        GameSfx(0xbf, NULL);
                    } else {
                        GameSfx(0xbb, NULL);
                    }
                }
                LDATA->fogr = 0x7f;
                LDATA->fogg = 0x7f;
                LDATA->fogb = 0x7f;
                LDATA->foga = 0x4c;
                LDATA->fognear = 0.1f;
                LDATA->fogfar = 10.0f;
            } else {
                LDATA->fogr -= 8;
                if ((LDATA->fogr & 0x80) != 0) {
                    LDATA->fogr = 0;
                }
                LDATA->fogg -= 8;
                if ((char)LDATA->fogg < 6) {
                    LDATA->fogg = 6;
                }
                LDATA->fogb -= 8;
                if ((char)LDATA->fogb < 0x21) {
                    LDATA->fogb = 0x21;
                }
                LDATA->foga = 0x4c;
                LDATA->fognear += 2.0f;
                if (LDATA->fognear > 4.0f) {
                    LDATA->fognear = 4.0f;
                }
                LDATA->fogfar += 10.0f;
                if (LDATA->fogfar > 100.0f) {
                    LDATA->fogfar = 100.0f;
                }
            }
        } else {
            if (VEHICLECONTROL != 0) {
                NuBridgeOn(0);
            } else {
                NuBridgeOn(1);
            }
        }
        vec.x = global_camera.mtx._30;
        vec.y = global_camera.mtx._31 + 3.0f;
        vec.z = global_camera.mtx._32;
        vec2.x = player->obj.pos.x - global_camera.mtx._30;
        vec2.z = player->obj.pos.z - global_camera.mtx._32;
        ang = NuAtan2D(vec2.x, vec2.z);
        vec.x = NuTrigTable[ang & 0xffff] * 5.0f + vec.x;
        vec.z = NuTrigTable[(ang + 0x4000) & 0xffff] * 5.0f + vec.z;
        vec.y = vec.y + 100.0f;
        dy = NewShadow(&vec, 0.0f);
        vec.y -= 100.0f;
        if (vec.y > dy) {
            AddVariableShotDebrisEffect(GDeb[95].i, &vec, 0x10, -0x8000, 0);
        }
        if ((Level == 0x17) || ((Level == 8 && (VEHICLECONTROL != 0)))) {
            g = 1;
            temp = 8192.0f;
        } else {
            if (Level == 0x17) {
                g = 4;
                temp = 4096.0f;
            } else if (Level == 0xe) {
                g = 2;
                temp = 4096.0f;
            } else if (Level == 8) {
                if ((((float)qrand()) * 0.000015259022f) > 0.6) {
                    g = 1;
                } else {
                    g = 0;
                }
                temp = 4096.0f;
            }
        }
        if (g != 0) {
            for (loop = g; loop != 0; loop--) {
                vec.x = player->obj.pos.x + (float)(qrand() - 0x8000) / temp;
                vec.y = player->obj.pos.y + 10.0f;
                vec.z = player->obj.pos.z + (float)(qrand() - 0x8000) / temp;
                vec.y = NewShadow(&vec, 0.0f);
                if (vec.y != 2000000.0f) {
                    vec.y -= 0.2f;
                    key = -1;
                    AddFiniteShotDebrisEffect(&key, GDeb[96].i, &vec, 1);
                }
            }
        }
        break;
    }
    pt = rockpt;
    if (pt != NULL) {
        if (*pt != 9999.0f) {
            do {
                temp = pt[3];
                pt[3] = (temp - 1.0f);
                if (pt[3] <= 0.0f) {
                    pt[3] = (pt[3] + ((pt[4] * 60.0f) / 60.0f));
                }
                vec.x = *pt;
                vec.y = pt[1];
                vec.z = pt[2];
                dx = player->obj.pos.x - vec.x;
                dy = player->obj.pos.y - vec.y;
                dz = player->obj.pos.z - vec.z;
                if ((pt[0xb] < 0.0f) || (dx * dx + dy * dy + dz * dz < (pt[0xb] * pt[0xb]))) {
                    for (loop = 1; (float)loop <= pt[5]; loop++) {
                        if ((temp > pt[6] * (float)loop) && (pt[3] <= pt[6] * (float)loop)) {
                            AddDeb3Ang(&vec, (s32)pt[7], (s32)pt[8], NULL, (s32)pt[9], (s32)pt[10]);
                            if (rockpt == (float*)HotRocks) {
                                GameSfx(0x85, &vec);
                            }
                            break;
                        }
                    }
                }
                pt = pt + 0xc;
            } while (*pt != 9999.0f);
        }
        if (rockpt == (float*)HotRocks) {
            loop = pVIS - world_scene[0]->splines;
            if (((loop == 1) && (0x6b < iVIS))
                && (hutexplode == 0))
            {
                hutexplode = 1;
                for (loop = 0; HutList[loop] != NULL; loop++) {
                    if (NuSpecialFind(world_scene[0], &obj, HutList[loop]) != 0) {
                        StartHGobjAnim(&obj);
                    }
                }
            }
            if (hutexplode > 0) {
                key = hutexplode;
                hutexplode += 0x3c;
                if (hutexplode >= 0x4b0) {
                    vec.x = 11.94f;
                    vec.y = 2.12f;
                    vec.z = -42.61f;
                    if (key <= 0x4af) {
                        exkey = -1;
                        AddFiniteShotDebrisEffect(&exkey, GDeb[43].i, &vec, 1);
                        exroty = 0xf0000;
                        GameSfx(0xbd, NULL);
                        NewRumble(&player->rumble, 0xff);
                        NewBuzz(&player->rumble, 0xc);
                    }
                    hutexplode = 0x708;
                    hutframe += 0x3c;
                    if (hutframe > 0xf0) {
                        hutframe -= 0xf0;
                        AddVariableShotDebrisEffect(GDeb[42].i, &vec, 1, 0, 0);
                    }
                    if (exroty > 0) {
                        DebrisEmitterOrientation(exkey, 0x1000, (short)(exroty / 0x3c));
                        exroty = exroty - 0x1e000;
                    }
                }
            }
            if ((iVIS - 0xcaU < 0x1b) && (pVIS != NULL)) {
                dropfire = 600;
            }
            dropfire -= 0x3c;
            if (dropfire < 1) {
                vec.x = (s32)(qrand() - 0x8000U) * 0.00012207031f + player->obj.pos.x;
                vec.y = player->obj.pos.y + 10.0f;
                vec.z = ((s32)(qrand() - 0x8000U) * 0.00012207031f + player->obj.pos.z);
                vec.y = NewShadow(&vec, 0.0f);
                if (vec.y != 2000000.0f) {
                    if (vec.y < 0.0f) {
                        firedrop[firedroppt].type = 0x29;
                        vec.y = 0.0f;
                    } else {
                        firedrop[firedroppt].type = 0x28;
                    }
                    firedrop[firedroppt].time = 3600.0f;
                    firedrop[firedroppt].pos = vec;
                    firedroppt = firedroppt + 1 & 0xf;
                    vec.y += 10.0f;
                    AddVariableShotDebrisEffect(GDeb[39].i, &vec, 1, -0x8000, 0);
                    dropfire = 600;
                } else {
                    dropfire = 0x3c;
                }
            }
            for (loop = 0; loop < 0x10; loop++) {
                if ((firedrop[loop].time != 0.0f) && (firedrop[loop].time -= 60.0f, firedrop[loop].time <= 0.0f)) {
                    firedrop[loop].time = 0.0f;
                    key = -1;
                    AddFiniteShotDebrisEffect(&key, GDeb[firedrop[loop].type].i, &firedrop[loop].pos, 1);
                    if (NuCameraClipTestPoints(&firedrop[loop].pos, 1, NULL) == 0) {
                        if (firedrop[loop].type == 0x28) {
                            GameSfx(0x9c, &firedrop[loop].pos);
                        } else {
                            GameSfx(0x85, &firedrop[loop].pos);
                        }
                    }
                }
            }
        }
    }
    if (Level == 8) {
        rsfxpt = (s32*)rsfxJung;
    }
    PlayRandSFX();
}

//NGC MATCH
void ProcDeb3(void) {
  s32 loop;
  s32 flag;
  struct deb3_s *deb;
  struct nuvec_s vec;
  struct nuvec_s t;
  struct numtx_s mat;
  float dist;
  float dx;
  float dy;
  float dz;
  float radius;
  float r;
  
  LaunchObjects();
  deb = deb3;
  NuMtxSetIdentity(&mat);
  radius = ((player->obj.max.y - player->obj.min.y) * player->obj.SCALE * 0.5f);
  for(loop = 0; loop < 0x40; loop++, deb++) {
    if (deb->timer != 0) {
      deb->diff.x *= 0.5f;
      deb->diff.y *= 0.5f;
      deb->diff.z *= 0.5f;
      if ((deb->status & 1) == 0) {
        mat = deb->mtx;
        RBodyMove(deb,0.01666667f);
        if ((deb->info->info & 6U) != 0) {
          dx = deb->mtx._30 - player->obj.pos.x;
          dy = deb->mtx._31 - (((player->obj.bot + player->obj.top) * player->obj.SCALE) * 0.5f + player->obj.pos.y);
          dz = deb->mtx._32 - player->obj.pos.z;
          r = (deb->info->size * 0.5f + radius);
          if (dx * dx + dy * dy + dz * dz < r * r) {
            if (((deb->info->info & 2U) != 0) && (deb->info->type == 0x93)) {
              player->freeze = 0x78;
              player->spin = 0;
              GameSfx(0x54,&player->obj.pos);
            }
            deb->timer = 1;
            deb->status |= 4;
          }
        }
        deb->check--;
        if (deb->check < 1) {
          if (deb->norm.y != 100.0f) {
            deb->diff.x += (deb->mtx._30 - deb->impact.x);
            deb->diff.y += (deb->mtx._31 - deb->impact.y);
            deb->diff.z += (deb->mtx._32 - deb->impact.z);
            deb->mtx._30 = deb->impact.x;
            deb->mtx._31 = deb->impact.y;
            deb->mtx._32 = deb->impact.z;
            if (deb->info->impact != NULL) {
              (*deb->info->impact)(deb);
            }
            if (rbclass[deb->info->classid].mass != 0.0f) {
              t = deb->impact;
              CubeImpact(&mat,&deb->mtx,&deb->norm,(deb->info->size * 0.5f),&t);
              deb->norm.x = -deb->norm.x;
              deb->norm.y = -deb->norm.y;
              deb->norm.z = -deb->norm.z;
              RBodyImpact(deb,&t,&deb->norm);
            }
            else {
              FullReflect(&deb->norm,&deb->velocity,&deb->velocity);
              deb->velocity.x *= rbclass[deb->info->classid].kr;
              deb->velocity.y *= rbclass[deb->info->classid].kr;
              deb->velocity.z *= rbclass[deb->info->classid].kr;
              if (rbclass[deb->info->classid].kf != 0.0f) {
                deb->angularMomentum.x = (qrand() + -0x8000 >> 8) * rbclass[deb->info->classid].kf;
                deb->angularMomentum.y = (qrand() + -0x8000 >> 8) * rbclass[deb->info->classid].kf;
                deb->angularMomentum.z = (qrand() + -0x8000 >> 8) * rbclass[deb->info->classid].kf;
              }
            }
            if (deb->info->type == 0x93) {
              GameSfx(0x44,(struct nuvec_s *)&deb->mtx._30);
            }
          }
          vec.x = ((deb->velocity.x * 8.0f) / 60.0f);
          vec.y = ((deb->velocity.y * 8.0f) / 60.0f);
          vec.z = ((deb->velocity.z * 8.0f) / 60.0f);
          dist = (vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
          t.x = deb->mtx._30;
          t.y = deb->mtx._31;
          t.z = deb->mtx._32;
          if ((deb->info->info & 8U) != 0) {
            flag = 0;
          }
          else {
            flag = NewRayCast(&t,&vec,deb->info->size);
          }
          if (flag == 0) {
            deb->status |= 2;
          }
          if (((flag == 1) || (flag == 2) || (flag == 3) || (flag == 4) || (flag == 5) || (flag == 6)
              || (flag == 7) || (flag == 8) || (flag == 9) || (flag == 10) || (flag == 11) || (flag == 12)
              || (flag == 13) || (flag == 14) || (flag == 15)) 
              || (flag > 0xf) && ((deb->status & 2U) != 0)) {
                deb->impact.x = t.x + vec.x;
                deb->impact.y = t.y + vec.y;
                deb->impact.z = t.z + vec.z;
                deb->norm = ShadNorm;
                vec.x = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
                vec.x = NuFsqrt(vec.x);
                dist = NuFsqrt(dist);
                if (dist == 0.0f) {
                  deb->check = 0;
                }
                else {
                  deb->check = (vec.x * 8.0f) / dist;
                }
                if (deb->check == 0) {
                  deb->diff.x += (deb->mtx._30 - deb->impact.x);
                  deb->diff.y += (deb->mtx._31 - deb->impact.y);
                  deb->diff.z += (deb->mtx._32 - deb->impact.z);
                  deb->mtx._30 = deb->impact.x;
                  deb->mtx._31 = deb->impact.y;
                  deb->mtx._32 = deb->impact.z;
                  deb->count++;
                  if (deb->count > 4) {
                    if (0.7f < ShadNorm.y) {
                      if ((deb->info->info & 1U) != 0) {
                        deb->timer = 1;
                      }
                      else {
                        deb->status |= 1;
                      }
                    }
                  }
                } else {
                    deb->count = 0;
                }
            } else {
                    deb->check = 8;
                    deb->norm.y = 100.0f;
                    deb->count = 0;
            }
        }
      }
      deb->mtx._30 += deb->diff.x;
      deb->mtx._31 += deb->diff.y;
      deb->mtx._32 += deb->diff.z;
      vec.x = deb->mtx._30;
      vec.y = deb->mtx._31;
      vec.z = deb->mtx._32;
      if ((deb->info->deb != 0) && ((deb->status & 1U) == 0)) {
        if (deb->info->rate > 0) {
          AddVariableShotDebrisEffect(GDeb[deb->info->deb].i,&vec,deb->info->rate,0,0);
        }
        else {
          if ((s32)deb->timer == ((s32)deb->timer / -deb->info->rate) * -deb->info->rate) {
            AddVariableShotDebrisEffect(GDeb[deb->info->deb].i,&vec,1,0,0);
          }
        }
      }
      deb->mtx._30 -= deb->diff.x;
      deb->mtx._31 -= deb->diff.y;
      deb->mtx._32 -= deb->diff.z;
      deb->timer--;
      if (deb->timer < 1) {
        if (deb->info->end != NULL) {
          (*deb->info->end)(deb);
        }
        if (deb->info->type == 0x93) {
          GameSfx(0x70,(struct nuvec_s *)&deb->mtx._30);
        }
      }
      else if ((deb->info->info & 0x10U) != 0) {
        deb->shadow = NewShadow((struct nuvec_s *)&deb->mtx._30,0.0f);
      }
    }
  }
}
