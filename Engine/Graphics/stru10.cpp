#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include "Engine/Engine.h"
#include "stru10.h"
#include "Indoor.h"
#include "Engine/Party.h"



int _49CE9E_sub0_x(RenderVertexSoft *pVertices, unsigned int uNumVertices, float test_val)
{
  float max_val = FLT_MAX;
  int   idx = -1;

  float temp_val;
  for (uint i = 0; i < uNumVertices; ++i)
  {
    if (pVertices[i].vWorldPosition.x <= test_val)
      temp_val = test_val - pVertices[i].vWorldPosition.x;
    else
      temp_val = pVertices[i].vWorldPosition.x - test_val;

    if (temp_val < max_val)
    {
      max_val = temp_val;
      idx = i;
    }
  }
  return idx;
}


int _49CE9E_sub0_y(RenderVertexSoft *pVertices, unsigned int uNumVertices, float test_val)
{
  float max_val = FLT_MAX;
  int   idx = -1;

  float temp_val;
  for (uint i = 0; i < uNumVertices; ++i)
  {
    if (pVertices[i].vWorldPosition.y <= test_val)
      temp_val = test_val - pVertices[i].vWorldPosition.y;
    else
      temp_val = pVertices[i].vWorldPosition.y - test_val;

    if (temp_val < max_val)
    {
      max_val = temp_val;
      idx = i;
    }
  }
  return idx;
}


int _49CE9E_sub0_z(RenderVertexSoft *pVertices, unsigned int uNumVertices, float test_val)
{
  float max_val = FLT_MAX;
  int   idx = -1;

  float temp_val;
  for (uint i = 0; i < uNumVertices; ++i)
  {
    if (pVertices[i].vWorldPosition.z <= test_val)
      temp_val = test_val - pVertices[i].vWorldPosition.z;
    else
      temp_val = pVertices[i].vWorldPosition.z - test_val;

    if (temp_val < max_val)
    {
      max_val = temp_val;
      idx = i;
    }
  }
  return idx;
}

//----- (0049CE9E) --------------------------------------------------------
void stru10::_49CE9E(BLVFace *pFace, RenderVertexSoft *pVertices, unsigned int uNumVertices, RenderVertexSoft *pOutLimits)
{
  Assert(sizeof(RenderVertexSoft) == 0x30);
  
  RenderVertexSoft pLimits[64];
  stru10::CalcPolygonLimits(pFace, pLimits);

  if (pFace->uAttributes & FACE_XY_PLANE)
  {
    memcpy(&pOutLimits[0], &pVertices[_49CE9E_sub0_x(pVertices, uNumVertices, pLimits[0].vWorldPosition.x)], 0x30);
    memcpy(&pOutLimits[2], &pVertices[_49CE9E_sub0_x(pVertices, uNumVertices, pLimits[2].vWorldPosition.x)], 0x30);
    memcpy(&pOutLimits[1], &pVertices[_49CE9E_sub0_y(pVertices, uNumVertices, pLimits[1].vWorldPosition.y)], 0x30);
    memcpy(&pOutLimits[3], &pVertices[_49CE9E_sub0_y(pVertices, uNumVertices, pLimits[3].vWorldPosition.y)], 0x30);
  }
  else if (pFace->uAttributes & FACE_XZ_PLANE)
  {    
    memcpy(&pOutLimits[0], &pVertices[_49CE9E_sub0_x(pVertices, uNumVertices, pLimits[0].vWorldPosition.x)], 0x30);
    memcpy(&pOutLimits[2], &pVertices[_49CE9E_sub0_x(pVertices, uNumVertices, pLimits[2].vWorldPosition.x)], 0x30);
    memcpy(&pOutLimits[1], &pVertices[_49CE9E_sub0_z(pVertices, uNumVertices, pLimits[1].vWorldPosition.z)], 0x30);
    memcpy(&pOutLimits[3], &pVertices[_49CE9E_sub0_z(pVertices, uNumVertices, pLimits[3].vWorldPosition.z)], 0x30);
  }
  else if (pFace->uAttributes & FACE_YZ_PLANE)
  {
    memcpy(&pOutLimits[0], &pVertices[_49CE9E_sub0_y(pVertices, uNumVertices, pLimits[0].vWorldPosition.y)], 0x30);
    memcpy(&pOutLimits[2], &pVertices[_49CE9E_sub0_y(pVertices, uNumVertices, pLimits[2].vWorldPosition.y)], 0x30);
    memcpy(&pOutLimits[1], &pVertices[_49CE9E_sub0_z(pVertices, uNumVertices, pLimits[1].vWorldPosition.z)], 0x30);
    memcpy(&pOutLimits[3], &pVertices[_49CE9E_sub0_z(pVertices, uNumVertices, pLimits[3].vWorldPosition.z)], 0x30);
  }
}

//----- (0049D379) --------------------------------------------------------
void stru10::CalcPolygonLimits(BLVFace *pFace, RenderVertexSoft *pOutVertices)
{
  struct
  {
    float x;
    float y;
    int c;
  } v46[40]; //[sp+0C];

  if (pFace->uAttributes & FACE_XY_PLANE)
  {
    for (uint i = 0; i < pFace->uNumVertices; ++i)
    {
      v46[i].x = pIndoor->pVertices[pFace->pVertexIDs[i]].x + pFace->pXInterceptDisplacements[i];
      v46[i].y = pIndoor->pVertices[pFace->pVertexIDs[i]].y + pFace->pYInterceptDisplacements[i];
      v46[i].c = i;
    }
  }
  if (pFace->uAttributes & FACE_XZ_PLANE)
  {
    for (uint i = 0; i < pFace->uNumVertices; ++i)
    {
      v46[i].x = pIndoor->pVertices[pFace->pVertexIDs[i]].x + pFace->pXInterceptDisplacements[i];
      v46[i].y = pIndoor->pVertices[pFace->pVertexIDs[i]].z + pFace->pZInterceptDisplacements[i];
      v46[i].c = i;
    }
  }
  if (pFace->uAttributes & FACE_YZ_PLANE)
  {
    for (uint i = 0; i < pFace->uNumVertices; ++i)
    {
      v46[i].x = pIndoor->pVertices[pFace->pVertexIDs[i]].y + pFace->pYInterceptDisplacements[i];
      v46[i].y = pIndoor->pVertices[pFace->pVertexIDs[i]].z + pFace->pZInterceptDisplacements[i];
      v46[i].c = i;
    }
  }

  float x_min = v46[0].x;
  uint  x_min_idx = 0;

  float x_max = v46[0].x;
  uint  x_max_idx = 0;

  float y_min = v46[0].y;
  uint  y_min_idx = 0;

  float y_max = v46[0].y;
  uint  y_max_idx = 0;

  for (uint i = 0; i < pFace->uNumVertices; ++i)
  {
    if (v46[i].x < x_min)
    {
      x_min = v46[i].x;
      x_min_idx = v46[i].c;
    }
    if (v46[i].x > x_max)
    {
      x_max = v46[i].x;
      x_max_idx = v46[i].c;
    }

    if (v46[i].y < y_min)
    {
      y_min = v46[i].y;
      y_min_idx = v46[i].c;
    }
    if (v46[i].y > y_max)
    {
      y_max = v46[i].y;
      y_max_idx = v46[i].c;
    }
  }

  RenderVertexSoft v1; // [sp+30Ch] [bp-54h]@24
  v1.vWorldPosition.x = (float)pIndoor->pVertices[pFace->pVertexIDs[x_min_idx]].x;
  v1.vWorldPosition.y = (float)pIndoor->pVertices[pFace->pVertexIDs[x_min_idx]].y;
  v1.vWorldPosition.z = (float)pIndoor->pVertices[pFace->pVertexIDs[x_min_idx]].z;
  memcpy(&pOutVertices[0], &v1, sizeof(RenderVertexSoft));

  RenderVertexSoft v2; // [sp+30Ch] [bp-54h]@24
  v2.vWorldPosition.x = (float)pIndoor->pVertices[pFace->pVertexIDs[y_min_idx]].x;
  v2.vWorldPosition.y = (float)pIndoor->pVertices[pFace->pVertexIDs[y_min_idx]].y;
  v2.vWorldPosition.z = (float)pIndoor->pVertices[pFace->pVertexIDs[y_min_idx]].z;
  memcpy(&pOutVertices[1], &v2, sizeof(RenderVertexSoft));

  RenderVertexSoft v3; // [sp+30Ch] [bp-54h]@24
  v3.vWorldPosition.x = (float)pIndoor->pVertices[pFace->pVertexIDs[x_max_idx]].x;
  v3.vWorldPosition.y = (float)pIndoor->pVertices[pFace->pVertexIDs[x_max_idx]].y;
  v3.vWorldPosition.z = (float)pIndoor->pVertices[pFace->pVertexIDs[x_max_idx]].z;
  memcpy(&pOutVertices[2], &v3, sizeof(RenderVertexSoft));

  RenderVertexSoft v4; // [sp+30Ch] [bp-54h]@24
  v4.vWorldPosition.x = (double)pIndoor->pVertices[pFace->pVertexIDs[y_max_idx]].x;
  v4.vWorldPosition.y = (double)pIndoor->pVertices[pFace->pVertexIDs[y_max_idx]].y;
  v4.vWorldPosition.z = (double)pIndoor->pVertices[pFace->pVertexIDs[y_max_idx]].z;
  memcpy(&pOutVertices[3], &v4, sizeof(RenderVertexSoft));
}


//----- (0049C9E3) --------------------------------------------------------
bool stru10::CalcFaceBounding(BLVFace *pFace, RenderVertexSoft *pFaceLimits, unsigned int uNumVertices, RenderVertexSoft *pOutBounding)
{
  //IndoorCameraD3D *v6; // edi@1
  //PolygonType v7; // al@1
  //unsigned int v8; // edx@7
  //char v10; // zf@10
  //float v13; // ST14_4@20
  //stru10 *v15; // ecx@21
  //RenderVertexSoft *v16; // ST0C_4@21
  //bool result; // eax@21
  //float v18; // ST14_4@24
  //stru10 *v19; // edi@29
  //float v20; // ST14_4@30
  //float v21; // ST14_4@30
  //float v22; // ST14_4@30
  //float v23; // ST14_4@30
  //float v24; // ST14_4@31
  //RenderVertexSoft v25; // [sp+10h] [bp-90h]@24
  //RenderVertexSoft v26; // [sp+40h] [bp-60h]@20
  //IndoorCameraD3D *thisa; // [sp+70h] [bp-30h]@1
  //stru10 *v31; // [sp+84h] [bp-1Ch]@1
  //float v32; // [sp+88h] [bp-18h]@8
  Vec3_float_ a1; // [sp+8Ch] [bp-14h]@1
  //float v35; // [sp+9Ch] [bp-4h]@8

  //auto a3 = pFace;
  //auto arg4 = pFaceBounding;

  //_ESI = a3;
  //v31 = this;
  //v6 = pIndoorCameraD3D;
  //v7 = a3->uPolygonType;

  a1.x = 0.0f;
  a1.y = 0.0f;
  a1.z = 0.0f;

  float var_28;
  float var_24;
  switch (pFace->uPolygonType)
  {
    case POLYGON_VerticalWall:
      a1.x = -pFace->pFacePlane.vNormal.y;// направление полигона
      a1.y = pFace->pFacePlane.vNormal.x;
      a1.z = 0.0f;
      a1.Normalize();

      var_28 = 0;
      var_24 = 1;
    break;

    case POLYGON_Floor:
    case POLYGON_Ceiling:
      a1.x = 1;
      a1.y = 0;
      a1.z = 0.0f;

      var_28 = 1;
      var_24 = 0;
    break;

    default:
      Error("Invalid polygon type (%u)", pFace->uPolygonType);
  }


  float face_center_x;
  float face_center_y;
  float face_center_z;
  float a3;
  float var_8;

  if (pFace->uAttributes & FACE_XY_PLANE)
  {
    face_center_x = (pFaceLimits[0].vWorldPosition.x + pFaceLimits[2].vWorldPosition.x) / 2;
    face_center_y = (pFaceLimits[3].vWorldPosition.y + pFaceLimits[1].vWorldPosition.y) / 2;
    face_center_z = (pFaceLimits[0].vWorldPosition.z + pFaceLimits[2].vWorldPosition.z) / 2;

    a3 = face_center_x - pFaceLimits[0].vWorldPosition.x;
    var_8 = face_center_y - pFaceLimits[1].vWorldPosition.y;
  }
  if (pFace->uAttributes & FACE_XZ_PLANE)
  {
    face_center_x = (pFaceLimits[0].vWorldPosition.x + pFaceLimits[2].vWorldPosition.x) / 2;// центр полигона
    face_center_y = (pFaceLimits[0].vWorldPosition.y + pFaceLimits[2].vWorldPosition.y) / 2;
    face_center_z = (pFaceLimits[1].vWorldPosition.z + pFaceLimits[3].vWorldPosition.z) / 2;

    a3 = face_center_x - pFaceLimits[0].vWorldPosition.x;//от центра до верхнего края
    var_8 = face_center_z - pFaceLimits[1].vWorldPosition.z;// высота от центра

    if (pFace->uPolygonType == POLYGON_VerticalWall)
      a3 /= a1.x;
  }
  if (pFace->uAttributes & FACE_YZ_PLANE)
  {
    face_center_x = (pFaceLimits[0].vWorldPosition.x + pFaceLimits[2].vWorldPosition.x) / 2;
    face_center_y = (pFaceLimits[0].vWorldPosition.y + pFaceLimits[2].vWorldPosition.y) / 2;
    face_center_z = (pFaceLimits[1].vWorldPosition.z + pFaceLimits[3].vWorldPosition.z) / 2;

    a3 = face_center_y - pFaceLimits[0].vWorldPosition.y;
    var_8 = face_center_z - pFaceLimits[1].vWorldPosition.z;
                                       // [0.5]
    if (pFace->uPolygonType == POLYGON_VerticalWall)
    {
      if (a1.x != 1.0f)
        a3 /= a1.y;
    }
  }




  //_EBX = arg0;
  //v15 = v31;
  //v16 = arg0;
  //float var_20 = var_8 * var_24;
  //var_8 = a3 * a1.z;
  //float arg_0 = var_8 + var_4;

/*



.text:0049CBB3   fld     [ebp+var_8]			0	var8
.text:0049CBB6 1 fmul    ds:flt_4D84E8			0	var8 * flt_4D84E8

.text:0049CBBC 1 fld     [ebp+var_8]			0	var8
							1	var8 * flt_4D84E8

.text:0049CBBF 2 fmul    [ebp+var_28]			0	var8 * var28
							1	var8 * flt_4D84E8

.text:0049CBC2 2 fld     [ebp+var_8]			0	var8
							1	var8 * var28
							2	var8 * flt_4D84E8

.text:0049CBC5 3 fmul    [ebp+var_24]			0	var8 * var24		768
							1	var8 * var28		0
							2	var8 * flt_4D84E8	0

.text:0049CBD5 3 fst     [ebp+var_20]			0	var8 * var24		768
							1	var8 * var28		0
							2	var8 * flt_4D84E8	0
					[var20]	var8 * var24	768

.text:0049CBD8 3 fld     [ebp+a3]			0	a3			-1984
							1	var8 * var24		768
							2	var8 * var28		0
							3	var8 * flt_4D84E8	0

.text:0049CBDB 4 fmul    [ebp+a1.x]			0	a3 * a1.x		1984
							1	var8 * var24		768
							2	var8 * var28		0
							3	var8 * flt_4D84E8	0

.text:0049CBDE 4 fld     [ebp+a3]			0	a3
							1	a3 * a1.x		1984
							2	var8 * var24		768
							3	var8 * var28		0
							4	var8 * flt_4D84E8	0

.text:0049CBE1 5 fmul    [ebp+a1.y]			0	a3 * a1.y		0
							1	a3 * a1.x		1984
							2	var8 * var24		768
							3	var8 * var28		0
							4	var8 * flt_4D84E8	0
.text:0049CBE4 5 fld     [ebp+a3]
.text:0049CBE7 6 fmul    [ebp+a1.z]			0	a3 * a1.z		0
							1	a3 * a1.y		0
							2	a3 * a1.x		1984
							3	var8 * var24		768
							4	var8 * var28		0
							5	var8 * flt_4D84E8	0

.text:0049CBEC 6 fstp    [ebp+var_8]
					var8 <- a3 * a1.z	0

.text:0049CBEF 5 fld     [ebp+arg4]			0	arg4			-1700
							1	a3 * a1.y		0
							2	a3 * a1.x		1984
							3	var8 * var24		768
							4	var8 * var28		0
							5	var8 * flt_4D84E8	0

.text:0049CBF2 6 fsub    st, st(2)			0	arg4 - a3 * a1.x	-3684
							1	a3 * a1.y		0
							2	a3 * a1.x		1984
							3	var8 * var24		768
							4	var8 * var28		0
							5	var8 * flt_4D84E8	0

.text:0049CBF4 6 fld     st				0	arg4 - a3 * a1.x	-3684
							1	arg4 - a3 * a1.x	-3684
							2	a3 * a1.y		0
							3	a3 * a1.x		1984
							4	var8 * var24		768
							5	var8 * var28		0
							6	var8 * flt_4D84E8	0

.text:0049CBF6 7 fadd    st, st(6)			0	arg4 - a3 * a1.x + var8 * flt_4D84E8	-3684
							1	arg4 - a3 * a1.x			-3684
							2	a3 * a1.y				0
							3	a3 * a1.x				1984
							4	var8 * var24				768
							5	var8 * var28				0
							6	var8 * flt_4D84E8			0
.text:0049CBF8 7 fstp    dword ptr [ebx]
			[0].x <- arg4 - a3 * a1.x + var8 * flt_4D84E8		-3684

.text:0049CBFA 6 fld     [ebp+var_18]			0	var18					1480
							1	arg4 - a3 * a1.x			-3684
							2	a3 * a1.y				0
							3	a3 * a1.x				1984
							4	var8 * var24				768
							5	var8 * var28				0
							6	var8 * flt_4D84E8			0

.text:0049CBFD 7 fsub    st, st(2)			0	var18 - a3 * a1.y			1480
							1	arg4 - a3 * a1.x			-3684
							2	a3 * a1.y				0
							3	a3 * a1.x				1984
							4	var8 * var24				768
							5	var8 * var28				0
							6	var8 * flt_4D84E8			0

.text:0049CBFF 7 fld     st				0	var18 - a3 * a1.y			1480
							1	var18 - a3 * a1.y			1480
							2	arg4 - a3 * a1.x			-3684
							3	a3 * a1.y				0
							4	a3 * a1.x				1984
							5	var8 * var24				768
							6	var8 * var28				0
							7	var8 * flt_4D84E8			0

.text:0049CC01 8 fadd    st, st(6)			0	var18 - a3 * a1.y + var8 * var28	1480
							1	var18 - a3 * a1.y			1480
							2	arg4 - a3 * a1.x			-3684
							3	a3 * a1.y				0
							4	a3 * a1.x				1984
							5	var8 * var24				768
							6	var8 * var28				0
							7	var8 * flt_4D84E8			0

.text:0049CC03 8 fstp    dword ptr [ebx+4]
			[0].y <- var18 - a3 * a1.y + var8 * var28		1480

.text:0049CC06 7 fld     [ebp+var_4]
.text:0049CC09 8 fsub    [ebp+var_8]			0	var4 - a3 * a1.z			768
							1	var18 - a3 * a1.y			1480
							2	arg4 - a3 * a1.x			-3684
							3	a3 * a1.y				0
							4	a3 * a1.x				1984
							5	var8 * var24				768
							6	var8 * var28				0
							7	var8 * flt_4D84E8			0
.text:0049CC0C 8 fst     [ebp+a3]
			a3 <- var4 - a3 * a1.z	768

.text:0049CC0F 8 fadd    st, st(5)
.text:0049CC11 8 fstp    dword ptr [ebx+8]
			[0].z <- var4 - a3 * a1.z + var8 * var24	1536

							0	var18 - a3 * a1.y			1480
							1	arg4 - a3 * a1.x			-3684
							2	a3 * a1.y				0
							3	a3 * a1.x				1984
							4	var8 * var24				768
							5	var8 * var28				0
							6	var8 * flt_4D84E8			0




						[var20]	var8 * var24		768
						[var8]	a3 * a1.z
						[a3]	var4 - a3 * a1.z	768


.text:0049CC14 7 fld     st(1)
.text:0049CC16 8 fsub    st, st(7)			0	arg4 - a3 * a1.x - var8 * flt_4D84E8	-3684
							1	var18 - a3 * a1.y			1480
							2	arg4 - a3 * a1.x			-3684
							3	a3 * a1.y				0
							4	a3 * a1.x				1984
							5	var8 * var24				768
							6	var8 * var28				0
							7	var8 * flt_4D84E8			0
.text:0049CC18 8 fstp    dword ptr [ebx+30h]
				[1].x <- arg4 - a3 * a1.x - var8 * flt_4D84E8		-3684

							0	var18 - a3 * a1.y			1480
							1	arg4 - a3 * a1.x			-3684
							2	a3 * a1.y				0
							3	a3 * a1.x				1984
							4	var8 * var24				768
							5	var8 * var28				0
							6	var8 * flt_4D84E8			0
.text:0049CC1B 7 fsub    st, st(5)
.text:0049CC1D 7 fstp    dword ptr [ebx+34h]
				[1].y <- var18 - a3 * a1.y - var8 * var28		1480
.text:0049CC20 6 fstp    st
							0	a3 * a1.y				0
							1	a3 * a1.x				1984
							2	var8 * var24				768
							3	var8 * var28				0
							4	var8 * flt_4D84E8			0


						[var20]	var8 * var24		768
						[var8]	a3 * a1.z
						[a3]	var4 - a3 * a1.z	768

.text:0049CC22 5 fld     [ebp+a3]			0	var4 - a3 * a1.z			768
							1	a3 * a1.y				0
							2	a3 * a1.x				1984
							3	var8 * var24				768
							4	var8 * var28				0
							5	var8 * flt_4D84E8			0
.text:0049CC25 6 fsub    st, st(3)
.text:0049CC27 6 fstp    dword ptr [ebx+38h]
			[1].z <- var4 - a3 * a1.z - var8 * var24	768 - 768 = 0

							0	a3 * a1.y				0
							1	a3 * a1.x				1984
							2	var8 * var24				768
							3	var8 * var28				0
							4	var8 * flt_4D84E8			0


.text:0049CC2A   fld     st(1)
.text:0049CC2C 6 fadd    [ebp+arg4]			0	arg4 + a3 * a1.x			284
							1	a3 * a1.y				0
							2	a3 * a1.x				1984
							3	var8 * var24				768
							4	var8 * var28				0
							5	var8 * flt_4D84E8			0
.text:0049CC2F 6 fst     [ebp+a3]

						[var20]	var8 * var24		768
						[var8]	a3 * a1.z
						[a3]	arg4 + a3 * a1.x	284
.text:0049CC32 6 fsub    st, st(5)
.text:0049CC34 6 fstp    dword ptr [ebx+60h]
			[2].x <- arg4 + a3 * a1.x - var8 * flt_4D84E8		284

							0	a3 * a1.y				0
							1	a3 * a1.x				1984
							2	var8 * var24				768
							3	var8 * var28				0
							4	var8 * flt_4D84E8			0

.text:0049CC37 5 fadd    [ebp+var_18]			0	var18 + a3 * a1.y			1480
							1	a3 * a1.x				1984
							2	var8 * var24				768
							3	var8 * var28				0
							4	var8 * flt_4D84E8			0

.text:0049CC3A 5 fstp    st(2)				0	a3 * a1.x				1984
							1	var18 + a3 * a1.y			1480
							2	var8 * var28				0
							3	var8 * flt_4D84E8			0

.text:0049CC3C 4 fstp    st				0	var18 + a3 * a1.y			1480
							1	var8 * var28				0
							2	var8 * flt_4D84E8			0

.text:0049CC3E 3 fld     st				0	var18 + a3 * a1.y			1480
							1	var18 + a3 * a1.y			1480
							2	var8 * var28				0
							3	var8 * flt_4D84E8			0
.text:0049CC40 4 fsub    st, st(2)
.text:0049CC42 4 fstp    dword ptr [ebx+64h]
			[2].y <- var18 + a3 * a1.y - var8 * var28	1480

							0	var18 + a3 * a1.y			1480
							1	var8 * var28				0
							2	var8 * flt_4D84E8			0

						[var20]	var8 * var24		768
						[var8]	a3 * a1.z
						[a3]	arg4 + a3 * a1.x	284
.text:0049CC45 3 fld     [ebp+var_8]
.text:0049CC48 4 fadd    [ebp+var_4]			0	var4 + a3 * a1.z			768
							1	var18 + a3 * a1.y			1480
							2	var8 * var28				0
							3	var8 * flt_4D84E8			0

.text:0049CC4B 4 fst     [ebp+arg0]
						[var20]	var8 * var24		768
						[arg0]	var4 + a3 * a1.z	768
						[var8]	a3 * a1.z		0
						[a3]	arg4 + a3 * a1.x	284

.text:0049CC4E 4 fsub    [ebp+var_20]
.text:0049CC51 4 fstp    dword ptr [ebx+68h]
			[2].z <- var4 + a3 * a1.z - var8 * var24		0



						[var20]	var8 * var24		768
						[arg0]	var4 + a3 * a1.z	768
						[var8]	a3 * a1.z		0
						[a3]	arg4 + a3 * a1.x	284


							0	var18 + a3 * a1.y			1480
							1	var8 * var28				0
							2	var8 * flt_4D84E8			0

.text:0049CC54 3 fld     [ebp+a3]
.text:0049CC57 4 fadd    st, st(3)
.text:0049CC59 4 fstp    dword ptr [ebx+90h]
			[3].x <- arg4 + a3 * a1.x + var8 * flt_4D84E8		284

.text:0049CC5F 3 fadd    st, st(1)
.text:0049CC61 3 fstp    dword ptr [ebx+94h]
			[3].y <- var18 + a3 * a1.y + var8 * var28		1480

.text:0049CC67 2 fstp    st
.text:0049CC69 1 fstp    st
							0	empty
.text:0049CC6B 0 fld     [ebp+arg0]
.text:0049CC6E 1 fadd    [ebp+var_20]
.text:0049CC71 1 fstp    dword ptr [ebx+98h]
			[3].z <- var4 + a3 * a1.z + var8 * var24	1536

							0 empty
*/

  pOutBounding[0].vWorldPosition.x = face_center_x - a3 * a1.x + var_8 * flt_4D84E8;
  pOutBounding[0].vWorldPosition.y = face_center_y - a3 * a1.y + var_8 * var_28;
  pOutBounding[0].vWorldPosition.z = face_center_z - a3 * a1.z + var_8 * var_24;

  pOutBounding[1].vWorldPosition.x = face_center_x - a3 * a1.x - var_8 * flt_4D84E8;
  pOutBounding[1].vWorldPosition.y = face_center_y - a3 * a1.y - var_8 * var_28;
  pOutBounding[1].vWorldPosition.z = face_center_z - a3 * a1.z - var_8 * var_24;

  pOutBounding[2].vWorldPosition.x = face_center_x + a3 * a1.x - var_8 * flt_4D84E8;
  pOutBounding[2].vWorldPosition.y = face_center_y + a3 * a1.y - var_8 * var_28;
  pOutBounding[2].vWorldPosition.z = face_center_z + a3 * a1.z - var_8 * var_24;

  pOutBounding[3].vWorldPosition.x = face_center_x + a3 * a1.x + var_8 * flt_4D84E8;
  pOutBounding[3].vWorldPosition.y = face_center_y + a3 * a1.y + var_8 * var_28;
  pOutBounding[3].vWorldPosition.z = face_center_z + a3 * a1.z + var_8 * var_24;

  a1.x = 0.0f;
  a1.y = 0.0f;
  a1.z = 0.0f;
  a3 = face_center_x + a3 * a1.x;

  if (!FindFacePlane(pOutBounding, &a1, &a3))
    return false;



  RenderVertexSoft v25; // [sp+10h] [bp-90h]@20
  memcpy(&v25, pOutBounding, sizeof(RenderVertexSoft));

  float _dp = (v25.vWorldPosition.x - pIndoorCameraD3D->vPartyPos.x) * a1.x +
              (v25.vWorldPosition.y - pIndoorCameraD3D->vPartyPos.y) * a1.y +
              (v25.vWorldPosition.z - pIndoorCameraD3D->vPartyPos.z) * a1.z;
  if (fabs(_dp) < 1e-6f)
  {
    memcpy(&v25, &pOutBounding[1], sizeof(RenderVertexSoft));
    memcpy(&pOutBounding[1], &pOutBounding[3], sizeof(RenderVertexSoft));
    memcpy(&pOutBounding[3], &v25, sizeof(RenderVertexSoft));
  }

    //if ( byte_4D864C && pEngine->uFlags & GAME_FLAGS_1_DRAW_BLV_DEBUGS)
    //{
      RenderVertexSoft v26; // [sp+40h] [bp-60h]@20
      if ( draw_portals_loops )
      {
        if (!bDoNotDrawPortalFrustum)
        {
          v26.vWorldPosition.x = pParty->vPosition.x;
          v26.vWorldPosition.y = pParty->vPosition.y;
          v26.vWorldPosition.z = pParty->vPosition.z + pParty->sEyelevel;             // frustum

          pIndoorCameraD3D->do_draw_debug_line_sw(&v26, 0xFF0000u, &pOutBounding[0], 0xFF0000u, 0, 0);
          pIndoorCameraD3D->do_draw_debug_line_sw(&v26, 0xFF00u, &pOutBounding[1], 0xFF00u, 0, 0);
          pIndoorCameraD3D->do_draw_debug_line_sw(&v26, 0xFFu, &pOutBounding[2], 0xFFu, 0, 0);
          pIndoorCameraD3D->do_draw_debug_line_sw(&v26, 0xFFFFFFu, &pOutBounding[3], 0xFFFFFFu, 0, 0);
          bDoNotDrawPortalFrustum = true;
        }
        pIndoorCameraD3D->debug_outline_sw(pOutBounding, uNumVertices, 0x1EFF1Eu, 0.00019999999);    // bounding
      }
    //}

    //pIndoorCameraD3D->debug_outline_sw(pFaceLimits,  4, 0xFFF14040, 0.000099999997);     // limits

  /*if ( byte_4D864C && pEngine->uFlags & GAME_FLAGS_1_DRAW_BLV_DEBUGS)
  {
    RenderVertexSoft v26; // [sp+40h] [bp-60h]@20
    v26.vWorldPosition.x = face_center_x;                                    // corner to center
    v26.vWorldPosition.y = face_center_y;
    v26.vWorldPosition.z = face_center_z;

    pIndoorCameraD3D->do_draw_debug_line_sw(pFaceLimits, 0xFF00u, &v26, 0xFF0000u, 0, 0);  
  }*/


        /*if ( byte_4D864C )
  {
      if ( pEngine->uFlags & GAME_FLAGS_1_DRAW_BLV_DEBUGS)*/
      {
        RenderVertexSoft v25; // [sp+10h] [bp-90h]@20
        RenderVertexSoft v26; // [sp+40h] [bp-60h]@20

        v25.vWorldPosition.x = face_center_x;                 // portal normal
        v25.vWorldPosition.y = face_center_y;
        v25.vWorldPosition.z = face_center_z;

        v26.vWorldPosition.x = face_center_x + a1.x * 400.0f;
        v26.vWorldPosition.y = face_center_y + a1.y * 400.0f;
        v26.vWorldPosition.z = face_center_z + a1.z * 400.0f;

        if ( draw_portals_loops )
          pIndoorCameraD3D->do_draw_debug_line_sw(&v25, -1, &v26, 0xFFFF00u, 0, 0);
      }
  //}

  return true;
}



//----- (0049C5B0) --------------------------------------------------------
stru10::stru10()
{
  this->bDoNotDrawPortalFrustum = false;
}

//----- (0049C5BD) --------------------------------------------------------
stru10::~stru10()
{
}

//----- (0049C5DA) --------------------------------------------------------
char stru10::_49C5DA(BLVFace *pFace, RenderVertexSoft *pVertices, unsigned int *pNumVertices, IndoorCameraD3D_Vec4 *a5, RenderVertexSoft *pOutBounding)
{
  RenderVertexSoft pLimits[4]; // [sp+Ch] [bp-C0h]@1

  _49CE9E(pFace, pVertices, *pNumVertices, pLimits);

  //if ( byte_4D864C && pEngine->uFlags & GAME_FLAGS_1_DRAW_BLV_DEBUGS)
  //  pIndoorCameraD3D->debug_outline_sw(a4a, 4u, 0xFF1E1Eu, 0.000099999997);
  if (CalcFaceBounding(pFace, pLimits, 4, pOutBounding))
    return _49C720(pOutBounding, a5);
  return false;
}
// 4D864C: using guessed type char byte_4D864C;

//----- (0049C681) --------------------------------------------------------
bool stru10::CalcPortalShape(BLVFace *pFace, IndoorCameraD3D_Vec4 *pPortalDataFrustum, RenderVertexSoft *pOutBounding)
{
  RenderVertexSoft pLimits[4]; // [sp+Ch] [bp-C0h]@1

  CalcPolygonLimits(pFace, pLimits);//определение границ портала
  //if ( byte_4D864C && pEngine->uFlags & GAME_FLAGS_1_DRAW_BLV_DEBUGS)
 //   pIndoorCameraD3D->debug_outline_sw(pLimits, 4, 0xFF1E1E, 0.000099999997);
  if (CalcFaceBounding(pFace, pLimits, 4, pOutBounding))
    return _49C720(pOutBounding, pPortalDataFrustum) != 0;
  return false;
}

// 4D864C: using guessed type char byte_4D864C;

//----- (0049C720) --------------------------------------------------------
char stru10::_49C720(RenderVertexSoft *pFaceBounding, IndoorCameraD3D_Vec4 *pPortalDataFrustum)
{
  Vec3_float_ pRayStart; // [sp+4h] [bp-34h]@1
  pRayStart.x = (double)pIndoorCameraD3D->vPartyPos.x;
  pRayStart.y = (double)pIndoorCameraD3D->vPartyPos.y;
  pRayStart.z = (double)pIndoorCameraD3D->vPartyPos.z;

  if (FindFaceNormal(&pFaceBounding[0], &pFaceBounding[1], &pRayStart, &pPortalDataFrustum[0]) &&
      FindFaceNormal(&pFaceBounding[1], &pFaceBounding[2], &pRayStart, &pPortalDataFrustum[1]) &&
      FindFaceNormal(&pFaceBounding[2], &pFaceBounding[3], &pRayStart, &pPortalDataFrustum[2]) &&
      FindFaceNormal(&pFaceBounding[3], &pFaceBounding[0], &pRayStart, &pPortalDataFrustum[3]))
    return true;
  return false;
}

//----- (0049C7C5) --------------------------------------------------------
bool stru10::FindFaceNormal(RenderVertexSoft *pFaceBounding1, RenderVertexSoft *pFaceBounding2, Vec3_float_ *pRayStart, IndoorCameraD3D_Vec4 *pPortalDataFrustum)
{
  Vec3_float_ ray_dir; // [sp+4h] [bp-48h]@1
  Vec3_float_ pRay2; // [sp+10h] [bp-3Ch]@1

  ray_dir.x = pFaceBounding1->vWorldPosition.x - pRayStart->x;//get ray for cmera to bounding1
  ray_dir.y = pFaceBounding1->vWorldPosition.y - pRayStart->y;
  ray_dir.z = pFaceBounding1->vWorldPosition.z - pRayStart->z;
  Vec3_float_::Cross(&ray_dir, &pRay2, pFaceBounding2->vWorldPosition.x - pFaceBounding1->vWorldPosition.x,
                               pFaceBounding2->vWorldPosition.y - pFaceBounding1->vWorldPosition.y,
                               pFaceBounding2->vWorldPosition.z - pFaceBounding1->vWorldPosition.z);

  float sqr_mag = pRay2.x * pRay2.x + pRay2.y * pRay2.y + pRay2.z * pRay2.z;
  if (fabsf(sqr_mag) > 1e-6f)
  {
    float inv_mag = 1.0f / sqrtf(sqr_mag);
    pRay2.x *= inv_mag;
    pRay2.y *= inv_mag;
    pRay2.z *= inv_mag;
    pRay2.Normalize();

    pPortalDataFrustum->x = pRay2.x;
    pPortalDataFrustum->y = pRay2.y;
    pPortalDataFrustum->z = pRay2.z;
    pPortalDataFrustum->dot = pRayStart->z * pRay2.z + pRayStart->y * pRay2.y + pRayStart->x * pRay2.x;
    return true;
  }
  return false;
}

//----- (0049C8DC) --------------------------------------------------------
bool stru10::FindFacePlane(RenderVertexSoft *arg0, Vec3_float_ *a2, float *a3)
{
  Vec3_float_ v1; // [sp+8h] [bp-3Ch]@1
  Vec3_float_ v2; // [sp+14h] [bp-30h]@1

  v1.x = arg0[1].vWorldPosition.x - arg0[0].vWorldPosition.x;
  v1.y = arg0[1].vWorldPosition.y - arg0[0].vWorldPosition.y;
  v1.z = arg0[1].vWorldPosition.z - arg0[0].vWorldPosition.z;

  Vec3_float_::Cross(&v1, &v2, arg0[2].vWorldPosition.x - arg0[1].vWorldPosition.x,
                               arg0[2].vWorldPosition.y - arg0[1].vWorldPosition.y,
                               arg0[2].vWorldPosition.z - arg0[1].vWorldPosition.z);

  float sqr_mag = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
  if (fabsf(sqr_mag) > 1e-6f)
  {
    //v2.Normalize();
    float inv_mag = 1.0f / sqrtf(sqr_mag);
    v2.x *= inv_mag;
    v2.y *= inv_mag;
    v2.z *= inv_mag;

    a2->x = v2.x;
    a2->y = v2.y;
    a2->z = v2.z;
    *a3 = -(arg0[0].vWorldPosition.z * v2.z + arg0[0].vWorldPosition.y * v2.y + arg0[0].vWorldPosition.x * v2.x);
    return true;
  }
  return false;
}
