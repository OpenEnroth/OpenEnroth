#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include "Engine/Engine.h"
#include "IndoorCameraD3D.h"
#include "Indoor.h"
#include "Viewport.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"

#include "stru9.h"


// calculates FOV (Field of View) angle in radians for IndoorCamera::Initialize and BLVRenderParams::Reset
float _calc_fov(int viewport_width, int angle_degree)
{
  return viewport_width * 0.5 / tan(angle_degree / 2 * 0.01745329) + 0.5;
}

IndoorCameraD3D *pIndoorCameraD3D = new IndoorCameraD3D;


//----- (004361EF) --------------------------------------------------------
IndoorCameraD3D::IndoorCameraD3D()
{
  this->field_108 = 0.0;
  this->field_138 = 0.0;
  this->field_168 = 0.0;
  this->field_198 = 0.0;
  this->field_1C8 = 0.0;
  this->field_1F8 = 0.0;
  this->field_228 = 0.0;
  this->field_258 = 0.0;
  this->field_288 = 0.0;
  this->field_2B8 = 0.0;
  this->field_2E8 = 0.0;
  this->field_2BC = 0.0;
  this->field_2C0 = 0.0;
  this->field_2C4 = 0.0;
  this->field_318 = 0.0;
  this->field_2EC = 0.0;
  this->field_2F0 = 0.0;
  this->field_2F4 = 0.0;
  this->field_348 = 0.0;
  this->field_31C = 0.0;
  this->field_320 = 0.0;
  this->field_324 = 0.0;
  this->field_378 = 0.0;
  this->field_34C = 0.0;
  this->field_350 = 0.0;
  this->field_354 = 0.0;
  for (uint i = 0; i < 16384; ++i)
  {
    list_0037C[i].field_0 = 0;
    list_0037C[i].flt_30 = 0.0f;
  }
  list_0037C_size = 0;
  for (uint i = 0; i < 256; ++i)
    list_E0380[i].mm7__vector_000004_size = 0;
  list_E0380_size = 0;
}


//----- (0043643E) --------------------------------------------------------
float IndoorCameraD3D::GetPickDepth()
{
  if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
    return pODMRenderParams->uPickDepth;
  else
    return 16192.0;
}

//----- (00436427) --------------------------------------------------------
float IndoorCameraD3D::GetShadingDistMist()
{
  if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
    return (double)pODMRenderParams->shading_dist_mist;
  else
    return 16192.0;
}

//----- (004364C5) --------------------------------------------------------
void IndoorCameraD3D::ViewTransfrom_OffsetUV(RenderVertexSoft *pVertices, unsigned int uNumVertices, RenderVertexSoft *pOutVertices, LightsData *a5)
{
  for (uint i = 0; i < uNumVertices; ++i)
  {
    pOutVertices[i].vWorldPosition.x = pVertices[i].vWorldPosition.x;
    pOutVertices[i].vWorldPosition.y = pVertices[i].vWorldPosition.y;
    pOutVertices[i].vWorldPosition.z = pVertices[i].vWorldPosition.z;

    pOutVertices[i].u = pVertices[i].u + a5->pDeltaUV[0];
    pOutVertices[i].v = pVertices[i].v + a5->pDeltaUV[1];
  }
  ViewTransform(pOutVertices, uNumVertices);
}

//----- (0043669D) --------------------------------------------------------
bool IndoorCameraD3D::ApplyViewTransform_TrueIfStillVisible_BLV(int x, int y, int z, signed int *pOutX, int *pOutZ, int *pOutY, char bDoNotShow)
{
  int to_z; // esi@2
  int v9; // ecx@3
  //signed int *v10; // esi@5
  //int pOutY_; // ecx@5
  //signed int v12; // esi@7
  int v14; // [sp+8h] [bp-4h]@3
  int to_x; // [sp+14h] [bp+8h]@1
  int to_y; // [sp+18h] [bp+Ch]@1
//  int a2b; // [sp+18h] [bp+Ch]@5
  int a3a; // [sp+1Ch] [bp+10h]@5

  to_x = x - pIndoorCameraD3D->vPartyPos.x;
  to_y = y - pIndoorCameraD3D->vPartyPos.y;
  if (pIndoorCameraD3D->sRotationX)
  {
    to_z = (z - pIndoorCameraD3D->vPartyPos.z) << 16;
    //if ( pRenderer->pRenderD3D )
    //{
      v14 = (unsigned __int64)(to_x * (signed __int64)pIndoorCameraD3D->int_cosine_y)
          + (unsigned __int64)(to_y * (signed __int64)pIndoorCameraD3D->int_sine_y);
      v9 = (unsigned __int64)(to_x * (signed __int64)pIndoorCameraD3D->int_sine_y)
         - (unsigned __int64)(to_y * (signed __int64)pIndoorCameraD3D->int_cosine_y);
    //}
    a3a = (z - pIndoorCameraD3D->vPartyPos.z) << 16;
    *pOutX = fixpoint_mul(v14, pIndoorCameraD3D->int_cosine_x) - fixpoint_mul(to_z, pIndoorCameraD3D->int_sine_x);
    *pOutZ = v9;
    *pOutY = fixpoint_mul(v14, pIndoorCameraD3D->int_sine_x) + fixpoint_mul(a3a, pIndoorCameraD3D->int_cosine_x);
  }
  else
  {
    *pOutY = (z - pIndoorCameraD3D->vPartyPos.z) << 16;
    //if ( pRenderer->pRenderD3D )
    //{
      //v10 = pOutX;
      *pOutX = (unsigned __int64)(to_x * (signed __int64)pIndoorCameraD3D->int_cosine_y)
             + (unsigned __int64)(to_y * (signed __int64)pIndoorCameraD3D->int_sine_y);
      *pOutZ = (unsigned __int64)(to_x * (signed __int64)pIndoorCameraD3D->int_sine_y)
             - (unsigned __int64)(to_y * (signed __int64)pIndoorCameraD3D->int_cosine_y);
    //}
  }
  if (!bDoNotShow)
    return false;

  return *pOutX >= fixpoint_from_int(4, 0) &&
         *pOutX <= fixpoint_from_int(8000, 0);
}

//----- (00436455) --------------------------------------------------------
bool IndoorCameraD3D::IsCulled(BLVFace *pFace)
{
  RenderVertexSoft v; // [sp+8h] [bp-30h]@1

  //----- (0043648F) --------------------------------------------------------
  auto Vec3_short__to_RenderVertexSoft = [](RenderVertexSoft *_this, Vec3_short_ *a2) -> void
  {
    _this->flt_2C = 0.0;

    _this->vWorldPosition.x = a2->x;
    _this->vWorldPosition.y = a2->y;
    _this->vWorldPosition.z = a2->z;
  };


  Vec3_short__to_RenderVertexSoft(&v, &pIndoor->pVertices[*pFace->pVertexIDs]);
  return is_face_faced_to_camera(pFace, &v);
}


//----- (00436523) --------------------------------------------------------
void IndoorCameraD3D::ViewTransform(RenderVertexSoft *a1a, unsigned int uNumVertices)
{
  if (byte_4D864C && pEngine->uFlags & 0x80 ||
      uCurrentlyLoadedLevelType == LEVEL_Indoor)
  {
    float sin_x = fRotationXSine,
          cos_x = fRotationXCosine;
    float sin_y = fRotationYSine,
          cos_y = fRotationYCosine;

    //v4 = uNumVertices;
    //v7 = pIndoorCamera->fRotationXSine;
    if (pIndoorCameraD3D->sRotationX)
    {

        //_EAX = a1a;
    for (uint i = 0; i < uNumVertices; ++i)
    {
          float st0, st1, st2;
          //if ( pRenderer->pRenderD3D )
          {
            /*__asm
            {
              fld     [ebp+uNumVertices] // [(a1a[i].y - pIndoorCamera->pos.y)]   [(a1a[i].z - pIndoorCamera->pos.z)]   [pIndoorCamera->fRotationXCosine]   [pIndoorCamera->fRotationYSine]   [pIndoorCamera->fRotationYCosine]
              fmul    st, st(3)          // [pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]   [(a1a[i].z - pIndoorCamera->pos.z)]   [pIndoorCamera->fRotationXCosine]   [pIndoorCamera->fRotationYSine]   [pIndoorCamera->fRotationYCosine]
              fld     [ebp+a1]           // [(a1a[i].x - pIndoorCamera->pos.x)]   [pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]   [(a1a[i].z - pIndoorCamera->pos.z)]   [pIndoorCamera->fRotationXCosine]   [pIndoorCamera->fRotationYSine]   [pIndoorCamera->fRotationYCosine]
              fmul    st, st(5)          // 0[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x)]
                                         // 1[pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 2[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 3[pIndoorCamera->fRotationXCosine]
                                         // 4[pIndoorCamera->fRotationYSine]
                                         // 5[pIndoorCamera->fRotationYCosine]
              faddp   st(1), st          // [pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) + pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]   [(a1a[i].z - pIndoorCamera->pos.z)]   [pIndoorCamera->fRotationXCosine]   [pIndoorCamera->fRotationYSine]   [pIndoorCamera->fRotationYCosine]
              fld     [ebp+a1]           // [(a1a[i].x - pIndoorCamera->pos.x)]   [pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) + pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]   [(a1a[i].z - pIndoorCamera->pos.z)]   [pIndoorCamera->fRotationXCosine]   [pIndoorCamera->fRotationYSine]   [pIndoorCamera->fRotationYCosine]
              fmul    st, st(4)          // [pIndoorCamera->fRotationYSine * (a1a[i].x - pIndoorCamera->pos.x)]   [pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) + pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]   [(a1a[i].z - pIndoorCamera->pos.z)]   [pIndoorCamera->fRotationXCosine]   [pIndoorCamera->fRotationYSine]   [pIndoorCamera->fRotationYCosine]

              fld     [ebp+uNumVertices] // 0[a1a[i].y - pIndoorCamera->pos.y]
                                         // 1[pIndoorCamera->fRotationYSine * (a1a[i].x - pIndoorCamera->pos.x)]
                                         // 2[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) + pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 3[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 4[pIndoorCamera->fRotationXCosine]
                                         // 5[pIndoorCamera->fRotationYSine]
                                         // 6[pIndoorCamera->fRotationYCosine]

              fmul    st, st(6)          // 0[pIndoorCamera->fRotationYCosine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 1[pIndoorCamera->fRotationYSine * (a1a[i].x - pIndoorCamera->pos.x)]
                                         // 2[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) + pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 3[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 4[pIndoorCamera->fRotationXCosine]
                                         // 5[pIndoorCamera->fRotationYSine]
                                         // 6[pIndoorCamera->fRotationYCosine]

              fsubp   st(1), st          // 0[pIndoorCamera->fRotationYSine * (a1a[i].x - pIndoorCamera->pos.x) - pIndoorCamera->fRotationYCosine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 1[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) + pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 2[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 3[pIndoorCamera->fRotationXCosine]
                                         // 4[pIndoorCamera->fRotationYSine]
                                         // 5[pIndoorCamera->fRotationYCosine]
            }*/
            st0 = sin_y * (a1a[i].vWorldPosition.x - pIndoorCameraD3D->vPartyPos.x) - cos_y * (a1a[i].vWorldPosition.y - pIndoorCameraD3D->vPartyPos.y);
            st1 = cos_y * (a1a[i].vWorldPosition.x - pIndoorCameraD3D->vPartyPos.x) + sin_y * (a1a[i].vWorldPosition.y - pIndoorCameraD3D->vPartyPos.y);
            st2 = (a1a[i].vWorldPosition.z - pIndoorCameraD3D->vPartyPos.z);
          }
          if (false)//else
          {
            /*__asm
            {
              fld     [ebp+a1]           // 0[(a1a[i].x - pIndoorCamera->pos.x)]
                                         // 1[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 2[pIndoorCamera->fRotationXCosine]
                                         // 3[pIndoorCamera->fRotationYSine]
                                         // 4[pIndoorCamera->fRotationYCosine]
              fmul    st, st(4)          // 0[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x)]
                                         // 1[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 2[pIndoorCamera->fRotationXCosine]
                                         // 3[pIndoorCamera->fRotationYSine]
                                         // 4[pIndoorCamera->fRotationYCosine]
              fld     [ebp+uNumVertices] // 0[(a1a[i].y - pIndoorCamera->pos.y)]
                                         // 1[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x)]
                                         // 2[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 3[pIndoorCamera->fRotationXCosine]
                                         // 4[pIndoorCamera->fRotationYSine]
                                         // 5[pIndoorCamera->fRotationYCosine]
              fmul    st, st(4)          // 0[pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 1[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x)]
                                         // 2[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 3[pIndoorCamera->fRotationXCosine]
                                         // 4[pIndoorCamera->fRotationYSine]
                                         // 5[pIndoorCamera->fRotationYCosine]
              fsubp   st(1), st          // 0[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) - pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 1[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 2[pIndoorCamera->fRotationXCosine]
                                         // 3[pIndoorCamera->fRotationYSine]
                                         // 4[pIndoorCamera->fRotationYCosine]
              fld     [ebp+a1]           // 0[(a1a[i].x - pIndoorCamera->pos.x)]
                                         // 1[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) - pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 2[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 3[pIndoorCamera->fRotationXCosine]
                                         // 4[pIndoorCamera->fRotationYSine]
                                         // 5[pIndoorCamera->fRotationYCosine]
              fmul    st, st(4)          // 0[pIndoorCamera->fRotationYSine * (a1a[i].x - pIndoorCamera->pos.x)]
                                         // 1[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) - pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 2[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 3[pIndoorCamera->fRotationXCosine]
                                         // 4[pIndoorCamera->fRotationYSine]
                                         // 5[pIndoorCamera->fRotationYCosine]
              fld     [ebp+uNumVertices] // 0[(a1a[i].y - pIndoorCamera->pos.y)]
                                         // 1[pIndoorCamera->fRotationYSine * (a1a[i].x - pIndoorCamera->pos.x)]
                                         // 2[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) - pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 3[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 4[pIndoorCamera->fRotationXCosine]
                                         // 5[pIndoorCamera->fRotationYSine]
                                         // 6[pIndoorCamera->fRotationYCosine]
              fmul    st, st(6)          // 0[pIndoorCamera->fRotationYCosine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 1[pIndoorCamera->fRotationYSine * (a1a[i].x - pIndoorCamera->pos.x)]
                                         // 2[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) - pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 3[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 4[pIndoorCamera->fRotationXCosine]
                                         // 5[pIndoorCamera->fRotationYSine]
                                         // 6[pIndoorCamera->fRotationYCosine]
              faddp   st(1), st          // 0[pIndoorCamera->fRotationYCosine * (a1a[i].y - pIndoorCamera->pos.y) + pIndoorCamera->fRotationYSine * (a1a[i].x - pIndoorCamera->pos.x)]
                                         // 1[pIndoorCamera->fRotationYCosine * (a1a[i].x - pIndoorCamera->pos.x) - pIndoorCamera->fRotationYSine * (a1a[i].y - pIndoorCamera->pos.y)]
                                         // 2[(a1a[i].z - pIndoorCamera->pos.z)]
                                         // 3[pIndoorCamera->fRotationXCosine]
                                         // 4[pIndoorCamera->fRotationYSine]
                                         // 5[pIndoorCamera->fRotationYCosine]
            }*/
            st0 = cos_y * (a1a[i].vWorldPosition.y - pIndoorCameraD3D->vPartyPos.y) + sin_y * (a1a[i].vWorldPosition.x - pIndoorCameraD3D->vPartyPos.x);
            st1 = cos_y * (a1a[i].vWorldPosition.x - pIndoorCameraD3D->vPartyPos.x) - sin_y * (a1a[i].vWorldPosition.y - pIndoorCameraD3D->vPartyPos.y);
            st2 = (a1a[i].vWorldPosition.z - pIndoorCameraD3D->vPartyPos.z);
          }

          a1a[i].vWorldViewPosition.x = st1*cos_x - st2*sin_x;
          a1a[i].vWorldViewPosition.y = st0;
          a1a[i].vWorldViewPosition.z = st2*cos_x + st1*sin_x;
      }
    }
    else
    {
      for (uint i = 0; i < uNumVertices; ++i)
      {
          //if ( pRenderer->pRenderD3D )
          {
            /*__asm
            {
              fld     [ebp+uNumVertices]   // 0[(a1a[i].y - pIndoorCamera->pos.y)]
                                           // 1[(a1a[i].z - pIndoorCamera->pos.z)]
                                           // 2[sin_y]
                                           // 3[cos_y]
              fmul    st, st(2)            // 0[sin_y * (a1a[i].y - pIndoorCamera->pos.y)]
                                           // 1[(a1a[i].z - pIndoorCamera->pos.z)]
                                           // 2[sin_y]
                                           // 3[cos_y]
              fld     [ebp+a1]             // 0[(a1a[i].x - pIndoorCamera->pos.x)]
                                           // 1[sin_y * (a1a[i].y - pIndoorCamera->pos.y)]
                                           // 2[(a1a[i].z - pIndoorCamera->pos.z)]
                                           // 3[sin_y]
                                           // 4[cos_y]
              fmul    st, st(4)            // 0[cos_y * (a1a[i].x - pIndoorCamera->pos.x)]
                                           // 1[sin_y * (a1a[i].y - pIndoorCamera->pos.y)]
                                           // 2[(a1a[i].z - pIndoorCamera->pos.z)]
                                           // 3[sin_y]
                                           // 4[cos_y]
              faddp   st(1), st            // 0[cos_y * (a1a[i].x - pIndoorCamera->pos.x) + sin_y * (a1a[i].y - pIndoorCamera->pos.y)]
                                           // 1[(a1a[i].z - pIndoorCamera->pos.z)]
                                           // 2[sin_y]
                                           // 3[cos_y]
              fstp    dword ptr [eax+0Ch]
              fld     [ebp+a1]             // 0[(a1a[i].x - pIndoorCamera->pos.x)]
                                           // 1[(a1a[i].z - pIndoorCamera->pos.z)]
                                           // 2[sin_y]
                                           // 3[cos_y]
              fmul    st, st(2)            // 0[sin_y * (a1a[i].x - pIndoorCamera->pos.x)]
                                           // 1[(a1a[i].z - pIndoorCamera->pos.z)]
                                           // 2[sin_y]
                                           // 3[cos_y]
              fld     [ebp+uNumVertices]   // 0[(a1a[i].y - pIndoorCamera->pos.y)]
                                           // 1[sin_y * (a1a[i].x - pIndoorCamera->pos.x)]
                                           // 2[(a1a[i].z - pIndoorCamera->pos.z)]
                                           // 3[sin_y]
                                           // 4[cos_y]
              fmul    st, st(4)            // 0[cos_y * (a1a[i].y - pIndoorCamera->pos.y)]
                                           // 1[sin_y * (a1a[i].x - pIndoorCamera->pos.x)]
                                           // 2[(a1a[i].z - pIndoorCamera->pos.z)]
                                           // 3[sin_y]
                                           // 4[cos_y]
              fsubp   st(1), st            // 0[sin_y * (a1a[i].x - pIndoorCamera->pos.x) - cos_y * (a1a[i].y - pIndoorCamera->pos.y)]
                                           // 1[(a1a[i].z - pIndoorCamera->pos.z)]
                                           // 2[sin_y]
                                           // 3[cos_y]
              fstp    dword ptr [eax+10h]
              fstp    dword ptr [eax+14h]
            }*/
            a1a[i].vWorldViewPosition.x = cos_y * (a1a[i].vWorldPosition.x - pIndoorCameraD3D->vPartyPos.x) + sin_y * (a1a[i].vWorldPosition.y - pIndoorCameraD3D->vPartyPos.y);
            a1a[i].vWorldViewPosition.y = sin_y * (a1a[i].vWorldPosition.x - pIndoorCameraD3D->vPartyPos.x) - cos_y * (a1a[i].vWorldPosition.y - pIndoorCameraD3D->vPartyPos.y);
            a1a[i].vWorldViewPosition.z = (a1a[i].vWorldPosition.z - pIndoorCameraD3D->vPartyPos.z);
          }
          if (false)//else
          {
            __debugbreak();
            /*__asm
            {
              fld     [ebp+a1]
              fmul    st, st(3)
              fld     [ebp+uNumVertices]
              fmul    st, st(3)
              fsubp   st(1), st
              fstp    dword ptr [eax+0Ch]
              fld     [ebp+a1]
              fmul    st, st(2)
              fld     [ebp+uNumVertices]
              fmul    st, st(4)
              faddp   st(1), st

              fstp    dword ptr [eax+10h]
              fstp    dword ptr [eax+14h]
            }*/
          }
      }
    }
  }
  else for (uint i = 0; i < uNumVertices; ++i)
    //pIndoorCamera->ViewTransform_ODM(a1a + i);
  {
    //ViewTransform_ODM(a1a + i);

    RenderVertexSoft* a1 = &a1a[i];
    //----- (00481CCE) --------------------------------------------------------
    //void ViewTransform_ODM(RenderVertexSoft *a1)
    {
  float result; // eax@1
  double vCamToVertexZ; // st7@1
  double v3; // st6@1
  double v4; // st5@1
  double v5; // st4@1
  float v6; // ST04_4@3
  float v7; // [sp+0h] [bp-14h]@1
  float v8; // [sp+8h] [bp-Ch]@1
  float vCamToVertexX; // [sp+Ch] [bp-8h]@1
  float vCamToVertexY; // [sp+10h] [bp-4h]@1

  v8 = fRotationXCosine;
  result = fRotationXSine;
  v7 = fRotationXSine;
  vCamToVertexX = a1->vWorldPosition.x - (double)pIndoorCameraD3D->vPartyPos.x;
  vCamToVertexY = a1->vWorldPosition.y - (double)pIndoorCameraD3D->vPartyPos.y;
  vCamToVertexZ = a1->vWorldPosition.z - (double)pIndoorCameraD3D->vPartyPos.z;
  v3 = fRotationYCosine;
  v4 = fRotationYSine;
  v5 = vCamToVertexY * fRotationYSine + fRotationYCosine * vCamToVertexX;
  if (pIndoorCameraD3D->sRotationX)
  {
    v6 = vCamToVertexY * fRotationYSine + fRotationYCosine * vCamToVertexX;
    a1->vWorldViewPosition.x = v5 * fRotationXCosine + fRotationXSine * vCamToVertexZ;
    a1->vWorldViewPosition.y = v3 * vCamToVertexY - v4 * vCamToVertexX;
    a1->vWorldViewPosition.z = v8 * vCamToVertexZ - v6 * v7;
  }
  else
  {
    a1->vWorldViewPosition.x = vCamToVertexY * fRotationYSine + fRotationYCosine * vCamToVertexX;
    a1->vWorldViewPosition.y = v3 * vCamToVertexY - v4 * vCamToVertexX;
    a1->vWorldViewPosition.z = vCamToVertexZ;
  }
    }
  }
}


//----- (00436932) --------------------------------------------------------
bool IndoorCameraD3D::GetFacetOrientation(char polyType, Vec3_float_ *a2, Vec3_float_ *a3, Vec3_float_ *a4)
{
  switch ((PolygonType)polyType)
  {
    case POLYGON_VerticalWall:
    {
      a4->x = -a2->y;
      a4->y = a2->x;
      a4->z = 0.0;

      a3->x = 0.0;
      a3->y = 0.0;
      a3->z = 1.0f;
    }
    return true;

    case POLYGON_Floor:
    case POLYGON_Ceiling:
    {
      a4->x = 1.0;
      a4->y = 0.0;
      a4->z = 0.0;

      a3->x = 0.0;
      a3->y = 1.0;
      a3->z = 0.0;
    }
    return true;

    case POLYGON_InBetweenFloorAndWall:
    case POLYGON_InBetweenCeilingAndWall:
    {
      if (fabs(a2->z) < 0.70811361)
      {
        a4->x = -a2->y;
        a4->y = a2->x;
        a4->z = 0.0;
        a4->Normalize();

        a3->x = 0.0;
        a3->y = 0.0;
        a3->z = 1.0;
      }
      else
      {
        a4->x = 1.0;
        a4->y = 0.0;
        a4->z = 0.0;

        a3->x = 0.0;
        a3->y = 1.0;
        a3->z = 0.0;
      }
    }
    return true;

    default:
      return false;
  }
}

//----- (00438258) --------------------------------------------------------
bool IndoorCameraD3D::is_face_faced_to_camera(BLVFace *pFace, RenderVertexSoft *a2)
{
  if (pFace->Portal())
    return false;

  //really strange cull; dot(to_cam, normal) < 0 means we see the BACK face, not font %_%
  if ( (a2->vWorldPosition.z - (double)pIndoorCameraD3D->vPartyPos.z) * (double)pFace->pFacePlane_old.vNormal.z
     + (a2->vWorldPosition.y - (double)pIndoorCameraD3D->vPartyPos.y) * (double)pFace->pFacePlane_old.vNormal.y
     + (a2->vWorldPosition.x - (double)pIndoorCameraD3D->vPartyPos.x) * (double)pFace->pFacePlane_old.vNormal.x < 0.0)
    return false;

  return true;
}


//----- (00437AB5) --------------------------------------------------------
void IndoorCameraD3D::do_draw_debug_line_sw(RenderVertexSoft *pLineBegin, signed int sStartDiffuse, RenderVertexSoft *pLineEnd, signed int sEndDiffuse, unsigned int uOutNumVertices, float z_stuff)
{
  RenderVertexSoft a1[20]; // [sp+8h] [bp-7C4h]@6
  RenderVertexSoft pVertices[20]; // [sp+3C8h] [bp-404h]@2
  RenderVertexD3D3 v24[2]; // [sp+788h] [bp-44h]@11

  //if ( pRenderer->pRenderD3D )
  //{
    for ( uint i = 0; i < 20; i++ )
      pVertices[i].flt_2C = 0.0;
    if ( (char)uOutNumVertices )
    {
      pVertices[0].vWorldViewProjX = pLineBegin->vWorldViewProjX;
      pVertices[0].vWorldViewProjY = pLineBegin->vWorldViewProjY;

      pVertices[1].vWorldViewProjX = pLineEnd->vWorldViewProjX;
      pVertices[1].vWorldViewProjY = pLineEnd->vWorldViewProjY;
      v24[0].specular = 0;
      v24[0].pos.x = pVertices[0].vWorldViewProjX;
      v24[0].pos.y = pVertices[0].vWorldViewProjY;
      v24[0].pos.z = 0.001 - z_stuff;
      v24[0].diffuse = sStartDiffuse;
      v24[0].rhw = 0.001;
      v24[0].texcoord.x = 0.0;
      v24[0].texcoord.y = 0.0;

      v24[1].pos.x = pVertices[1].vWorldViewProjX;
      v24[1].pos.y = pVertices[1].vWorldViewProjY;
      v24[1].diffuse = sEndDiffuse;
      v24[1].pos.z = 0.001 - z_stuff;
      v24[1].specular = 0;
      v24[1].rhw = 0.001;
      v24[1].texcoord.x = 0.0;
      v24[1].texcoord.y = 0.0;
      //v19 = pRenderer->pRenderD3D->pDevice;
      pRenderer->DrawLines(v24, 2);
      return;
    }
    for ( uint i = 0; i < 20; i++ )
      a1[i].flt_2C = 0.0;
    uOutNumVertices = 2;
    a1[0].vWorldPosition.x = pLineBegin->vWorldPosition.x;
    a1[0].vWorldPosition.y = pLineBegin->vWorldPosition.y;
    a1[0].vWorldPosition.z = pLineBegin->vWorldPosition.z;
    a1[1].vWorldPosition.x = pLineEnd->vWorldPosition.x;
    a1[1].vWorldPosition.y = pLineEnd->vWorldPosition.y;
    a1[1].vWorldPosition.z = pLineEnd->vWorldPosition.z;
    if ( CalcPortalShape(a1, &uOutNumVertices, pVertices, this->std__vector_000034_prolly_frustrum, 4, 1, 0) != 1 || (signed int)uOutNumVertices >= 2 )
    {
      ViewTransform(pVertices, 2);
      Project(pVertices, 2, 0);
      v24[0].specular = 0;
      v24[0].pos.x = pVertices[0].vWorldViewProjX;
      v24[0].pos.y = pVertices[0].vWorldViewProjY;
      v24[0].pos.z = 0.001 - z_stuff;
      v24[0].diffuse = sStartDiffuse;
      v24[0].rhw = 0.001;
      v24[0].texcoord.x = 0.0;
      v24[0].texcoord.y = 0.0;

      v24[1].pos.x = pVertices[1].vWorldViewProjX;
      v24[1].pos.y = pVertices[1].vWorldViewProjY;
      v24[1].diffuse = sEndDiffuse;
      v24[1].pos.z = 0.001 - z_stuff;
      v24[1].specular = 0;
      v24[1].rhw = 0.001;
      v24[1].texcoord.x = 0.0;
      v24[1].texcoord.y = 0.0;
      //v19 = pRenderer->pRenderD3D->pDevice;
      pRenderer->DrawLines(v24, 2);
      return;
    }
  //}
}

//----- (00437A55) --------------------------------------------------------
void IndoorCameraD3D::debug_outline_d3d(const RenderVertexD3D3 *pLineVertices, unsigned int uNumLines, int uDiffuse, float z_stuff)
{
  for (int i = 0; i < uNumLines-1; ++i)
    pRenderer->do_draw_debug_line_d3d(&pLineVertices[i], uDiffuse, &pLineVertices[i+1], uDiffuse, z_stuff);
  pRenderer->do_draw_debug_line_d3d(&pLineVertices[uNumLines-1], uDiffuse, pLineVertices, uDiffuse, z_stuff);
}

//----- (004379EE) --------------------------------------------------------
void IndoorCameraD3D::debug_outline_sw(RenderVertexSoft *a2, unsigned int uNumVertices, unsigned int uDiffuse, float a5)
{
  if ( !uNumVertices )
    return;
  if ( (signed int)(uNumVertices - 1) > 0 )
  {
    for ( uint i = 0; i < uNumVertices - 1; i++ )
      do_draw_debug_line_sw(&a2[i], uDiffuse, &a2[i + 1], uDiffuse, 0, a5);
  }
  do_draw_debug_line_sw(&a2[uNumVertices - 1], uDiffuse, a2, uDiffuse, 0, a5);
}


void IndoorCameraD3D::DebugDrawPortal(BLVFace *pFace)
{
  Assert(pFace->uNumVertices <= 32);

  RenderVertexSoft sw[32];
  for (uint i = 0; i < pFace->uNumVertices; ++i)
  {
    sw[i].vWorldPosition.x = pIndoor->pVertices[pFace->pVertexIDs[i]].x;
    sw[i].vWorldPosition.y = pIndoor->pVertices[pFace->pVertexIDs[i]].y;
    sw[i].vWorldPosition.z = pIndoor->pVertices[pFace->pVertexIDs[i]].z;
  }
  ViewTransform(sw, pFace->uNumVertices);
  Project(sw, pFace->uNumVertices, 0);



  RenderVertexD3D3 v[32];
  for (uint i = 0; i < pFace->uNumVertices; ++i)
  {
    v[i].pos.x = sw[i].vWorldViewProjX;
    v[i].pos.y = sw[i].vWorldViewProjY;
    v[i].pos.z = 1.0 - 1.0 / (sw[i].vWorldViewPosition.x * 0.061758894);
    v[i].rhw = 1.0 / sw[i].vWorldViewPosition.x;
    v[i].diffuse = 0x80F020F0;
    v[i].specular = 0;
    //v[i].texcoord.x = pFace->pVertexUIDs[i] / (double)pTex->uTextureWidth;
    //v[i].texcoord.y = pFace->pVertexUIDs[i] / (double)pTex->uTextureHeight;
    v[i].texcoord.x = 0;
    v[i].texcoord.y = 0;
  }

  pRenderer->DrawFansTransparent(v, pFace->uNumVertices);
}

//----- (00437906) --------------------------------------------------------
void IndoorCameraD3D::PrepareAndDrawDebugOutline(BLVFace *pFace, unsigned int uDiffuse)
{
  static RenderVertexSoft static_sub_437906_array_50CDD0[64];
  static bool __init_flag1 = false;
  if (!__init_flag1)
  {
    __init_flag1 = true;

    for (uint i = 0; i < 64; ++i)
      static_sub_437906_array_50CDD0[i].flt_2C = 0.0f;
  }
  if ( pFace->uNumVertices )
  {
    for ( uint i = 0; i < pFace->uNumVertices; i++ )
    {
      static_sub_437906_array_50CDD0[i].vWorldPosition.x = (double)pIndoor->pVertices[pFace->pVertexIDs[i]].x;
      static_sub_437906_array_50CDD0[i].vWorldPosition.y = (double)pIndoor->pVertices[pFace->pVertexIDs[i]].y;
      static_sub_437906_array_50CDD0[i].vWorldPosition.z = (double)pIndoor->pVertices[pFace->pVertexIDs[i]].z;
      static_sub_437906_array_50CDD0[i].u = (double)pFace->pVertexUIDs[i];
      static_sub_437906_array_50CDD0[i].v = (double)pFace->pVertexVIDs[i];
    }
  }
  if ( draw_portals_loops )
    debug_outline_sw(static_sub_437906_array_50CDD0, pFace->uNumVertices, uDiffuse, 0.0);
}
// 50D9D0: using guessed type char static_sub_437906_byte_50D9D0_init_flag;

//----- (004378BA) --------------------------------------------------------
void IndoorCameraD3D::MatrixMultiply(Matrix3x3_float_ *ma, Matrix3x3_float_ *mb, Matrix3x3_float_ *m_out)
{
  float sum;

  for ( uint row = 0; row < 3; row++ )
   {
    for ( uint col = 0; col < 3; col++ )
     {
      sum = 0;
      for ( int index = 0; index < 3; index++ )
        sum += ma->v[row][index]*mb->v[index][col];
      m_out->v[row][col] = sum;
    }
  }
}

//----- (004376E7) --------------------------------------------------------
void IndoorCameraD3D::CreateWorldMatrixAndSomeStuff()
{
  Matrix3x3_float_ m1; // [sp+10h] [bp-B8h]@1
  Matrix3x3_float_ m2; // [sp+34h] [bp-94h]@1
  Matrix3x3_float_ m3; // [sp+58h] [bp-70h]@1
  Matrix3x3_float_ m4; // [sp+7Ch] [bp-4Ch]@1
  Matrix3x3_float_ m5; // [sp+A0h] [bp-28h]@1

 //RotationZ(0)
  m5._11 = cosf(0);         m5._12 = sinf(0);        m5._13 = 0;
  m5._21 = -sinf(0);        m5._22 = cosf(0);        m5._23 = 0;
  m5._31 = 0;               m5._32 = 0;              m5._33 = 1;

  float cos_x1 = fRotationXCosine,
        sin_x1 = fRotationXSine;
 //RotationX(x)
  m4._11 = 1;               m4._12 = 0;              m4._13 = 0;
  m4._21 = 0;               m4._22 = cos_x1;         m4._23 = sin_x1;
  m4._31 = 0;               m4._32 = -sin_x1;        m4._33 = cos_x1;
  
  float cos_y1 = fRotationYCosine,
        sin_y1 = fRotationYSine;
 //RotationY(some_angle)
  m3._11 = cos_y1;          m3._12 = 0;              m3._13 = -sin_y1;
  m3._21 = 0;               m3._22 = 1;              m3._23 = 0;
  m3._31 = sin_y1;          m3._32 = 0;              m3._33 = cos_y1;

  MatrixMultiply(&m5, &m3, &m1);
  MatrixMultiply(&m4, &m1, &m2);

  for (uint i = 0; i < 3; ++i)
  {
    field_4[0].v[i] = m2.v[1][i];
    field_4[1].v[i] = m2.v[0][i];
    field_4[2].v[i] = m2.v[2][i];
  }

  inv_fov = 1.1344639;
  fov_x = (double)pViewport->uScreenWidth * 0.8814736;

  fov_y = 0.8814736 * (double)pViewport->uScreenHeight;
  fov = fov_y;
  if ( fov_x > fov )
    fov = fov_x;
  screenCenterX = (double)pViewport->uScreenCenterX;
  screenCenterY = (double)(pViewport->uScreenCenterY - pViewport->uScreen_TL_Y);
}

//----- (00437691) --------------------------------------------------------
void IndoorCameraD3D::Vec3Transform(const IndoorCameraD3D_Vec3 *pVector, IndoorCameraD3D_Vec3 *pOut)
{
  pOut->y = field_4[1].x * pVector->x + field_4[0].x * pVector->y + field_4[2].x * pVector->z;
  pOut->z = field_4[1].y * pVector->x + field_4[0].y * pVector->y + field_4[2].y * pVector->z;
  pOut->x = field_4[1].z * pVector->x + field_4[0].z * pVector->y + field_4[2].z * pVector->z;
}

//----- (00437607) --------------------------------------------------------
void IndoorCameraD3D::_437607(IndoorCameraD3D_Vec3 *a1, IndoorCameraD3D_Vec4 *a2)
{
  double v4; // st7@1
  IndoorCameraD3D_Vec3 v8; // [sp+8h] [bp-1Ch]@1

  v8.x = (double)pIndoorCameraD3D->vPartyPos.x;
  v8.y = (double)pIndoorCameraD3D->vPartyPos.y;
  v8.z = (double)pIndoorCameraD3D->vPartyPos.z;
  Vec3Transform(a1, a2);

  v4 = v8.x * a2->x + v8.y * a2->y + v8.z * a2->z;
  a2->dot = v4 + 0.000099999997;
}

//----- (004374E8) --------------------------------------------------------
void IndoorCameraD3D::_4374E8_ProllyBuildFrustrum()
{
  //IndoorCameraD3D *v1; // esi@1
  //double v2; // st7@1
  double v3; // st7@1
  //double v4; // st7@1
  double v5; // st7@1
  //double v6; // st7@1
  IndoorCameraD3D_Vec3 v7; // [sp+Ch] [bp-24h]@1
  //float v8; // [sp+10h] [bp-20h]@1
  //float v9; // [sp+14h] [bp-1Ch]@1
  //float v10; // [sp+18h] [bp-18h]@1
  //float v11; // [sp+1Ch] [bp-14h]@1
  //float v12; // [sp+20h] [bp-10h]@1
  //int v13; // [sp+2Ch] [bp-4h]@1

  //v1 = this;
  //IndoorCameraD3D_Vec3::IndoorCameraD3D_Vec3(&v7);
  //v2 = 2.0 / inv_fov;
  //v13 = 0;
  v3 = atan(2.0 / inv_fov * fov / fov_x);
  //v12 = v3;
  //v11 = sin(v3);
  //v4 = cos(v3);
  v7.x = -sin(v3);
  v7.y = 0.0;
  v7.z = cos(v3);
  _437607(&v7, std__vector_000034_prolly_frustrum + 0);
  v7.x = sin(v3);
  _437607(&v7, std__vector_000034_prolly_frustrum + 1);
  v5 = atan(2.0 / inv_fov * fov / (fov_y + 0.5));
  //v12 = v5;
  //v11 = sin(v5);
  //v6 = cos(v5);
  v7.y = sin(v5);
  v7.x = 0.0;
  v7.z = cos(v5);
  _437607(&v7, &std__vector_000034_prolly_frustrum[2]);
  v7.y = -sin(v5);
  _437607(&v7, &std__vector_000034_prolly_frustrum[3]);
  //v13 = -1;
  //IndoorCameraD3D_Vec3::dtor(&v7);
}

//----- (00437376) --------------------------------------------------------
char IndoorCameraD3D::_437376(stru154 *thisa, RenderVertexSoft *a2, unsigned int *pOutNumVertices)
{
  double v6; // st7@3
  int previous; // esi@6
  int current; // ebx@8
  int next; // eax@8
  int v13; // eax@15
  signed int v14; // ebx@17
  RenderVertexSoft v18; // [sp+Ch] [bp-34h]@2
  signed int thisb; // [sp+48h] [bp+8h]@6
  bool result; // [sp+4Fh] [bp+Fh]@5

  memcpy(&v18, a2, sizeof(v18));
  result = false;
  memcpy(&a2[*pOutNumVertices], a2, sizeof(a2[*pOutNumVertices]));
  memcpy(&a2[*pOutNumVertices + 1], &a2[1], sizeof(a2[*pOutNumVertices + 1]));

  if ( (signed int)*pOutNumVertices <= 3
    || (((v18.vWorldPosition.z - (double)pIndoorCameraD3D->vPartyPos.z) * thisa->face_plane.vNormal.z
       + (v18.vWorldPosition.y - (double)pIndoorCameraD3D->vPartyPos.y) * thisa->face_plane.vNormal.y
       + (v18.vWorldPosition.x - (double)pIndoorCameraD3D->vPartyPos.x) * thisa->face_plane.vNormal.x < 0.0) ? (v6 = 1.0) : (v6 = -1.0),
        (signed int)*pOutNumVertices <= 0) )
    return 0;

  for ( int i = 1; i - 1 < (signed int)*pOutNumVertices; i++ )
  {
	current = i;
    next = i + 1;

    previous = i - 1;
    if ( previous >= (signed int)*pOutNumVertices )
      previous -= *pOutNumVertices;

    if ( current >= (signed int)*pOutNumVertices )
      current -= *pOutNumVertices;

    if ( next >= (signed int)*pOutNumVertices )
      next -= *pOutNumVertices;

    if ( -0.009999999776482582 < ((a2[current].vWorldViewProjX - a2[previous].vWorldViewProjX)
                                * (a2[next].vWorldViewProjY - a2[previous].vWorldViewProjY)
                                - (a2[current].vWorldViewProjY - a2[previous].vWorldViewProjY)
                                * (a2[next].vWorldViewProjX - a2[previous].vWorldViewProjX))
                               * v6 )
    {
      v13 = next;
      if ( next >= (signed int)*pOutNumVertices )
        v13 = next - *pOutNumVertices;

      if ( v13 < (signed int)*pOutNumVertices )
      {
        for ( v14 = v13; v14 < (signed int)*pOutNumVertices; ++v14 )
          memcpy(&a2[v14], &a2[v14 + 1], sizeof(a2[v14]));
      }
      result = true;
      --*pOutNumVertices;
    }
  }
  if ( result )
    return true;
  else
    return false;
}

//----- (00437285) --------------------------------------------------------
bool IndoorCameraD3D::CalcPortalShape(RenderVertexSoft *a1, unsigned int *pOutNumVertices, RenderVertexSoft *pVertices, IndoorCameraD3D_Vec4 *a4, signed int uNumVertices, char a6, int _unused)
{
//  char *v8; // eax@2
//  signed int v9; // ecx@2
  //bool result; // eax@5
  int v11; // ecx@5
  //signed int v12; // ecx@6
  //char *v13; // esi@6
  RenderVertexSoft *v14; // eax@8
  RenderVertexSoft *v15; // edx@8
  Vec3_float_ a5; // [sp+18h] [bp-3Ch]@12
  //float v17; // [sp+44h] [bp-10h]@1
  //int v18; // [sp+48h] [bp-Ch]@5
  //stru9 *thisa; // [sp+4Ch] [bp-8h]@1
  int a7a; // [sp+53h] [bp-1h]@5
  //bool a6a; // [sp+70h] [bp+1Ch]@5

  //v17 = 0.0;
  //thisa = pEngine->pStru9Instance;
  
  static RenderVertexSoft sr_vertices_50D9D8[64];

  //result = 0;
  a7a = 0;
  v11 = 2 * (a6 == 0) + 1;
  //a6a = 0;
  //v18 = v11;
  if (uNumVertices <= 0)
    return false;

    //v12 = *pOutNumVertices;
    //v13 = (char *)&a4->y;
  
  //while ( 1 )
  for ( uint i = 0; i < uNumVertices; ++i )
  {
    if (i % 2)
    {
      v14 = a1;
      v15 = sr_vertices_50D9D8;
    }
    else
    {
      v15 = a1;
      v14 = sr_vertices_50D9D8;
    }
    if (i == uNumVertices - 1)
      v14 = pVertices;
    a5.x = a4[i].x;
    a5.y = a4[i].y;
    a5.z = a4[i].z;
    pEngine->pStru9Instance->CalcPortalShape(v15, *pOutNumVertices, v14, pOutNumVertices, &a5, a4[i].dot, (char *)&a7a, _unused);
    //v12 = *pOutNumVertices;
    if (*pOutNumVertices < v11)
    {
      *pOutNumVertices = 0;
      return true;
    }
    //result = a6a;
    //v13 += 24;
    //if (++i >= uNumVertices)
      //
  }
  return a7a;
}

//----- (004371C3) --------------------------------------------------------
bool IndoorCameraD3D::_4371C3(RenderVertexSoft *pVertices, unsigned int *pOutNumVertices, int _unused)
{
//  char *v4; // eax@2
//  signed int v5; // ecx@2
  RenderVertexSoft *v6; // esi@5
  unsigned int *v7; // edi@5
  char *v8; // ecx@6
  int v9; // eax@6
  IndoorCameraD3D *thisa; // [sp+0h] [bp-Ch]@1
  signed int v12; // [sp+4h] [bp-8h]@5
  unsigned int pVerticesa; // [sp+14h] [bp+8h]@6
  unsigned int pOutNumVerticesa; // [sp+18h] [bp+Ch]@6

  thisa = this;

  
  static RenderVertexSoft static_4371C3_array_50E5E0[64];
  static bool __init_flag1 = false;
  if (!__init_flag1)
  {
    __init_flag1 = true;

    for (uint i = 0; i < 64; ++i)
      static_4371C3_array_50E5E0[i].flt_2C = 0.0f;
  }

  v12 = 0;
  v6 = pVertices;
  v7 = pOutNumVertices;
  if ( (signed int)*pOutNumVertices > 0 )
  {
    pOutNumVerticesa = (char *)static_4371C3_array_50E5E0 - (char *)pVertices;
    pVerticesa = (char *)&static_4371C3_array_50E5E0[0].vWorldViewProjY - (char *)pVertices;
    v8 = (char *)&static_4371C3_array_50E5E0[0].vWorldPosition.y;
    v9 = (int)&v6->vWorldPosition.z;
    do
    {
      ++v12;
      *((int *)v8 - 1) = *(int *)(v9 - 8);
      *(int *)v8 = *(int *)(v9 - 4);
      v8 += 48;
      *(float *)(pOutNumVerticesa + v9) = *(float *)v9;
      *(float *)(pVerticesa + v9) = *(float *)(v9 + 28);
      *(float *)((char *)&static_4371C3_array_50E5E0[0]._rhw - (char *)v6 + v9) = *(float *)(v9 + 32);
      v9 += 48;
    }
    while ( v12 < (signed int)*v7 );
  }
  return CalcPortalShape(
           static_4371C3_array_50E5E0,
           v7,
           v6,
           thisa->std__vector_000034_prolly_frustrum,
           4,
           0,
           _unused);
}
// 50F1E0: using guessed type char static_sub_4371C3_byte_50F1E0_init_flags;

//----- (00437143) --------------------------------------------------------
void IndoorCameraD3D::_437143(unsigned int uNumInVertices, RenderVertexSoft *pOutVertices, RenderVertexSoft *pInVertices, signed int *pOutNumVertices)
{
  double v9; // st7@3

  uint i = 0;

  for ( i; i < uNumInVertices; ++i )
  {
    pInVertices[i]._rhw = 1.0 / (pInVertices[i].vWorldViewPosition.x + 0.0000001);
    memcpy(&pOutVertices[i], &pInVertices[i], sizeof(pOutVertices[i]));
    v9 = (double)pODMRenderParams->int_fov_rad * pInVertices[i]._rhw;
    pOutVertices[i].vWorldViewProjX = (double)pViewport->uScreenCenterX - v9 * pInVertices[i].vWorldViewPosition.y;
    pOutVertices[i].vWorldViewProjY = (double)pViewport->uScreenCenterY - v9 * pInVertices[i].vWorldViewPosition.z;
  }
  *pOutNumVertices = i;
  return;
}

//----- (00436F09) --------------------------------------------------------
void IndoorCameraD3D::LightmapFarClip(RenderVertexSoft *pInVertices, int uNumInVertices, RenderVertexSoft *pOutVertices, unsigned int *pOutNumVertices)
{
  bool current_vertices_flag;
  bool next_vertices_flag;
  double t;
  signed int depth_num_vertices;

  memcpy(&pInVertices[uNumInVertices], &pInVertices[0], sizeof(pInVertices[uNumInVertices]));
  depth_num_vertices = 0;
  current_vertices_flag = false;
  if ( pInVertices[0].vWorldViewPosition.x >= pODMRenderParams->shading_dist_mist )
    current_vertices_flag = true;//настоящая вершина больше границы видимости
  if ( (signed int)uNumInVertices <= 0 )
    return;
  //check for far clip plane(проверка по дальней границе)
  //   
  // v3.__________________. v0
  //   |                  |
  //   |                  |
  //   |                  |
  //  ----------------------- 8192.0(far_clip - 0x2000)
  //   |                  |
  //   .__________________.
  //  v2                     v1

  for ( uint i = 0; i < uNumInVertices; ++i )
  {
    next_vertices_flag = pInVertices[i + 1].vWorldViewPosition.x >= pODMRenderParams->shading_dist_mist;
    if ( current_vertices_flag ^ next_vertices_flag )//одна из граней за границей видимости
    {
      if ( next_vertices_flag )//следующая вершина больше границы видимости(настоящая вершина меньше границы видимости) - v3
      {
        //t = far_clip - v2.x / v3.x - v2.x (формула получения точки пересечения отрезка с плоскостью)
        t = (pODMRenderParams->shading_dist_mist - pInVertices[i].vWorldViewPosition.x) / (pInVertices[i].vWorldViewPosition.x - pInVertices[i + 1].vWorldViewPosition.x);
        pOutVertices[depth_num_vertices].vWorldViewPosition.x = pODMRenderParams->shading_dist_mist;
        //New_y = v2.y + (v3.y - v2.y)*t
        pOutVertices[depth_num_vertices].vWorldViewPosition.y = pInVertices[i].vWorldViewPosition.y + (pInVertices[i].vWorldViewPosition.y - pInVertices[i + 1].vWorldViewPosition.y) * t;
        //New_z = v2.z + (v3.z - v2.z)*t
        pOutVertices[depth_num_vertices].vWorldViewPosition.z = pInVertices[i].vWorldViewPosition.z + (pInVertices[i].vWorldViewPosition.z - pInVertices[i + 1].vWorldViewPosition.z) * t;
        pOutVertices[depth_num_vertices].u = pInVertices[i].u + (pInVertices[i].u - pInVertices[i + 1].u) * t;
        pOutVertices[depth_num_vertices].v = pInVertices[i].v + (pInVertices[i].v - pInVertices[i + 1].v) * t;
        pOutVertices[depth_num_vertices]._rhw = 1.0 / pODMRenderParams->shading_dist_mist;
      }
      else//настоящая вершина больше границы видимости(следующая вершина меньше границы видимости) - v0
      {
        //t = far_clip - v1.x / v0.x - v1.x
        t = (pODMRenderParams->shading_dist_mist - pInVertices[i].vWorldViewPosition.x) / (pInVertices[i + 1].vWorldViewPosition.x - pInVertices[i].vWorldViewPosition.x);
        pOutVertices[depth_num_vertices].vWorldViewPosition.x = pODMRenderParams->shading_dist_mist;
        //New_y = (v0.y - v1.y)*t + v1.y
        pOutVertices[depth_num_vertices].vWorldViewPosition.y = pInVertices[i].vWorldViewPosition.y + (pInVertices[i + 1].vWorldViewPosition.y - pInVertices[i].vWorldViewPosition.y) * t;
        //New_z = (v0.z - v1.z)*t + v1.z
        pOutVertices[depth_num_vertices].vWorldViewPosition.z = pInVertices[i].vWorldViewPosition.z + (pInVertices[i + 1].vWorldViewPosition.z - pInVertices[i].vWorldViewPosition.z) * t;
        pOutVertices[depth_num_vertices].u = pInVertices[i].u + (pInVertices[i + 1].u - pInVertices[i].u) * t;
        pOutVertices[depth_num_vertices].v = pInVertices[i].v + (pInVertices[i + 1].v - pInVertices[i].v) * t;
        pOutVertices[depth_num_vertices]._rhw = 1.0 / pODMRenderParams->shading_dist_mist;
      }
      ++depth_num_vertices;
    }
    if ( !next_vertices_flag )//оба в границе видимости
    {
      memcpy(&pOutVertices[depth_num_vertices], &pInVertices[i + 1], sizeof(pOutVertices[depth_num_vertices]));
      depth_num_vertices++;
    }
    current_vertices_flag = next_vertices_flag;
  }
  if(depth_num_vertices >= 3)
	*pOutNumVertices = depth_num_vertices;
  else
	*pOutNumVertices = 0;
}

//----- (00436CDC) --------------------------------------------------------
void IndoorCameraD3D::LightmapNeerClip(RenderVertexSoft *pInVertices, int uNumInVertices, RenderVertexSoft *pOutVertices, unsigned int *pOutNumVertices)
{
  double t; // st6@11
  bool current_vertices_flag; // esi@2
  bool next_vertices_flag; // [sp+Ch] [bp+8h]@7

  //check for near clip plane(проверка по ближней границе)
  //   
  // v3.__________________. v0
  //   |                  |
  //   |                  |
  //   |                  |
  //  ----------------------- 8.0(near_clip - 8.0)
  //   |                  |
  //   .__________________.
  //  v2                     v1

  int out_num_vertices = 0;

  if ( uNumInVertices )
  {
	  memcpy(&pInVertices[uNumInVertices], &pInVertices[0], sizeof(pInVertices[0]));
	  next_vertices_flag = false;
	  current_vertices_flag = false;
	  if ( pInVertices[0].vWorldViewPosition.x <= 8.0 )
		current_vertices_flag = true;
	  for (uint i = 0; i < uNumInVertices; ++i)
	  {
		next_vertices_flag = pInVertices[i + 1].vWorldViewPosition.x <= 8.0;//
		if ( current_vertices_flag ^ next_vertices_flag )
		{
		  if ( next_vertices_flag )//следующая вершина за ближней границей
		  {
			//t = near_clip - v0.x / v1.x - v0.x    (формула получения точки пересечения отрезка с плоскостью)
			t = (8.0 - pInVertices[i].vWorldViewPosition.x) / (pInVertices[i + 1].vWorldViewPosition.x - pInVertices[i].vWorldViewPosition.x);
			pOutVertices[out_num_vertices].vWorldViewPosition.x = 8.0;
			pOutVertices[out_num_vertices].vWorldViewPosition.y = pInVertices[i].vWorldViewPosition.y + (pInVertices[i + 1].vWorldViewPosition.y - pInVertices[i].vWorldViewPosition.y) * t;
			pOutVertices[out_num_vertices].vWorldViewPosition.z = pInVertices[i].vWorldViewPosition.z + (pInVertices[i + 1].vWorldViewPosition.z - pInVertices[i].vWorldViewPosition.z) * t;
			pOutVertices[out_num_vertices].u = pInVertices[i].u + (pInVertices[i + 1].u - pInVertices[i].u) * t;
			pOutVertices[out_num_vertices].v = pInVertices[i].v + (pInVertices[i + 1].v - pInVertices[i].v) * t;
			pOutVertices[out_num_vertices]._rhw = 1.0 / 8.0;
			//pOutVertices[*pOutNumVertices]._rhw = 0.125;
		  }
		  else// текущая вершина за ближней границей
		  {
			t = (8.0 - pInVertices[i].vWorldViewPosition.x) / (pInVertices[i].vWorldViewPosition.x - pInVertices[i + 1].vWorldViewPosition.x);
			pOutVertices[out_num_vertices].vWorldViewPosition.x = 8.0;
			pOutVertices[out_num_vertices].vWorldViewPosition.y = pInVertices[i].vWorldViewPosition.y + (pInVertices[i].vWorldViewPosition.y - pInVertices[i + 1].vWorldViewPosition.y) * t;
			pOutVertices[out_num_vertices].vWorldViewPosition.z = pInVertices[i].vWorldViewPosition.z + (pInVertices[i].vWorldViewPosition.z - pInVertices[i + 1].vWorldViewPosition.z) * t;
			pOutVertices[out_num_vertices].u = pInVertices[i].u + (pInVertices[i].u - pInVertices[i + 1].u) * t;
			pOutVertices[out_num_vertices].v = pInVertices[i].v + (pInVertices[i].v - pInVertices[i + 1].v) * t;
			pOutVertices[out_num_vertices]._rhw = 1.0 / 8.0;
		  }
		  ++out_num_vertices;
		}
		if ( !next_vertices_flag )
		{
		  memcpy(&pOutVertices[out_num_vertices], &pInVertices[i + 1], sizeof(pInVertices[i + 1]));
		  //pOutVertices[out_num_vertices]._rhw = 1.0 / (pInVertices[i].vWorldViewPosition.x + 0.0000001);
		  out_num_vertices++;
		}
		current_vertices_flag = next_vertices_flag;
	  }
	  if(out_num_vertices >=3 )
		*pOutNumVertices = out_num_vertices;
	  else
		*pOutNumVertices = 0;
	}
}

//----- (00481D77) --------------------------------------------------------
void _outdoor_project(RenderVertexSoft *v)
{
	double v1; // st7@1
	double v2; // st7@1

	v1 = 1.0 / (v->vWorldViewPosition.x + 0.0000001);
	v->_rhw = v1;
	v2 = v1 * (double)pODMRenderParams->int_fov_rad;
	v->vWorldViewProjX = (double)pViewport->uScreenCenterX - v2 * v->vWorldViewPosition.y;
	v->vWorldViewProjY = (double)pViewport->uScreenCenterY - v2 * v->vWorldViewPosition.z;
}

//----- (00436BB7) --------------------------------------------------------
void IndoorCameraD3D::Project(RenderVertexSoft *pVertices, unsigned int uNumVertices, char a4)
{
  double v7; // st7@7
  double v8; // st7@9
  double v9; // st6@10
  double v10; // st5@12
  double v11; // st7@16
  double v12; // st6@17
  double v13; // st5@19
  float uNumVerticesa; // [sp+14h] [bp+Ch]@13
  float uNumVerticesb; // [sp+14h] [bp+Ch]@20

  for (uint i = 0; i < uNumVertices; ++i)
  {
    if (byte_4D864C && pEngine->uFlags & 0x80 || uCurrentlyLoadedLevelType == LEVEL_Indoor)
    {
      v7 = 1.0 / pVertices[i].vWorldViewPosition.x;

      pVertices[i].vWorldViewProjX = pVertices[i].vWorldViewPosition.y * fov * v7 + screenCenterX;
      pVertices[i].vWorldViewProjY = (signed int)pViewport->uViewportBR_Y - (pVertices[i].vWorldViewPosition.z * fov * v7 + screenCenterY);
    }
    else
    {
      extern void _outdoor_project(RenderVertexSoft *v);
      _outdoor_project(pVertices + i);
    }

      if ( a4 )
      {
//        __debugbreak();
        v8 = (double)(signed int)pViewport->uViewportBR_X;
        if ( v8 >= pVertices[i].vWorldViewProjX )
          v9 = pVertices[i].vWorldViewProjX;
        else
          v9 = v8;
        v10 = (double)(signed int)pViewport->uViewportTL_X;
        if ( v10 <= v9 )
        {
          if ( v8 >= pVertices[i].vWorldViewProjX)
            v8 = pVertices[i].vWorldViewProjX;
        }
        else
        {
          uNumVerticesa = v10;
          v8 = uNumVerticesa;
        }
        pVertices[i].vWorldViewProjX = v8;
        v11 = (double)(signed int)pViewport->uViewportBR_Y;
        if ( v11 >= pVertices[i].vWorldViewProjY)
          v12 = pVertices[i].vWorldViewProjY;
        else
          v12 = v11;
        v13 = (double)(signed int)pViewport->uViewportTL_Y;
        if ( v13 <= v12 )
        {
          if ( v11 >= pVertices[i].vWorldViewProjY)
            v11 = pVertices[i].vWorldViewProjY;
        }
        else
        {
          uNumVerticesb = v13;
          v11 = uNumVerticesb;
        }
        pVertices[i].vWorldViewProjY = v11;
      }
  }
}

//----- (00436A9A) --------------------------------------------------------
void IndoorCameraD3D::Project(signed int x, signed int y, signed int z, int *a5, int *a6)
{
  double v6; // ST00_8@2
  //double v7; // ST08_8@2
  //double v8; // ST00_8@2
//  signed __int64 v9; // qtt@3
//  int v10; // ST04_4@3
  float a2a; // [sp+18h] [bp+8h]@2
  float a2b; // [sp+18h] [bp+8h]@2

  //if ( pRenderer->pRenderD3D )
  {
    v6 = 1.0 / (double)x;
    a2a = (double)y * fov * v6 + screenCenterX;
    //v7 = a2a + 6.7553994e15;
    *a5 = floorf(a2a + 0.5f);
    a2b = (double)z * fov * v6 + screenCenterY;
    //v8 = a2b + 6.7553994e15;
    *a6 = pViewport->uViewportBR_Y - floorf(a2b + 0.5f);
  }
  /*else
  {
    LODWORD(v9) = pBLVRenderParams->fov_rad_fixpoint << 16;
    HIDWORD(v9) = pBLVRenderParams->fov_rad_fixpoint >> 16;
    v10 = v9 / x;
    LODWORD(v9) = pBLVRenderParams->fov_rad_fixpoint << 16;
    HIDWORD(v9) = pBLVRenderParams->fov_rad_fixpoint >> 16;
    *a5 = pBLVRenderParams->uViewportCenterX
        - ((signed int)(((unsigned __int64)(v10 * (signed __int64)y) >> 16) + 32768) >> 16);
    *a6 = pBLVRenderParams->uViewportCenterY - ((signed int)(((unsigned __int64)(v9 / x * z) >> 16) + 32768) >> 16);
  }*/
}

//----- (00436A6D) --------------------------------------------------------
double IndoorCameraD3D::GetPolygonMinZ(RenderVertexSoft *pVertices, unsigned int uStripType)
{
  double result; // st7@1

  result = FLT_MAX;
  for ( uint i = 0; i < uStripType; i++ )
  {
    if ( pVertices[i].vWorldPosition.z < FLT_MAX )
      result = pVertices[i].vWorldPosition.z;
  }
  return result;
}

//----- (00436A24) --------------------------------------------------------
struct IDirect3DTexture2 *IndoorCameraD3D::LoadTextureAndGetHardwarePtr(char *Str1)
{
  return pBitmaps_LOD->pHardwareTextures[pBitmaps_LOD->LoadTexture(Str1)];
}

//----- (00436A40) --------------------------------------------------------
double IndoorCameraD3D::GetPolygonMaxZ(RenderVertexSoft *pVertex, unsigned int uStripType)
{
  double result; // st7@1

  result = 1.1754944e-38;
  for ( uint i = 0; i < uStripType; i++ )
  {
    if ( pVertex[i].vWorldPosition.z > result )
      result = pVertex[i].vWorldPosition.z;
  }
  return result;
}

// -- new
// merged from IndoorCamera::Initialize2
//         and ODMRenderParams::RotationToInts
//         and BLVRenderParams::Reset
void IndoorCameraD3D::CalculateRotations(int camera_rot_x, int camera_rot_y)
{
  sRotationX = camera_rot_x;
  sRotationY = camera_rot_y;

  fRotationYSine = sin((pi_double + pi_double) * (double)sRotationY / 2048.0);
  fRotationYCosine = cos((pi_double + pi_double) * (double)sRotationY / 2048.0);
  if (byte_4D864C && pEngine->uFlags & 0x80 || uCurrentlyLoadedLevelType == LEVEL_Indoor)
  {
    fRotationXSine = sin((pi_double + pi_double) * (double)-sRotationX / 2048.0);
    fRotationXCosine = cos((pi_double + pi_double) * (double)-sRotationX / 2048.0);

    int_sine_y   = stru_5C6E00->Sin( pIndoorCameraD3D->sRotationY);
    int_cosine_y = stru_5C6E00->Cos( pIndoorCameraD3D->sRotationY);
    int_sine_x   = stru_5C6E00->Sin(-pIndoorCameraD3D->sRotationX);
    int_cosine_x = stru_5C6E00->Cos(-pIndoorCameraD3D->sRotationX);
  }
  else
  {
    fRotationXSine = sin((pi_double + pi_double) * (double)sRotationX / 2048.0);
    fRotationXCosine = cos((pi_double + pi_double) * (double)sRotationX / 2048.0);

    int_sine_y   = stru_5C6E00->Sin(pIndoorCameraD3D->sRotationY);
    int_cosine_y = stru_5C6E00->Cos(pIndoorCameraD3D->sRotationY);
    int_sine_x   = stru_5C6E00->Sin(pIndoorCameraD3D->sRotationX);
    int_cosine_x = stru_5C6E00->Cos(pIndoorCameraD3D->sRotationX);
  }
}