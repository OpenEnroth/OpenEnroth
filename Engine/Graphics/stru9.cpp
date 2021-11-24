#include "Engine/Graphics/stru9.h"

#include "Engine/Engine.h"

#include "IndoorCameraD3D.h"

//----- (00498377) --------------------------------------------------------
bool stru9::ClipVertsToPortal(struct RenderVertexSoft *pPortalBounding,  // test skipping this
                    unsigned int uNumVertices,
                    IndoorCameraD3D_Vec4 *pVertices,
                    RenderVertexSoft *pVertices2,
                    unsigned int *pOutNumVertices) {
    // portal corners
    // 4
    // node
    // faceverts
    // num face verts

    int result;             // eax@7
    RenderVertexSoft *v9;   // ecx@9
    RenderVertexSoft *v19;  // [sp+14h] [bp-14h]@0
    int v21;         // [sp+1Ch] [bp-Ch]@7

    // __debugbreak();
    // thisa = this;


    return true;  // testing bypass



    static RenderVertexSoft static_AE3FB4;
    /*static bool __init_flag1 = false;
    if (!__init_flag1) {
      __init_flag1 = true;

      static_AE3FB4.flt_2C = 0.0;
    }*/

    static stru312 static_AE3FA4;  // idb
    /*static bool __init_flag2 = false;
    if (!__init_flag2) {
      __init_flag2 = true;

      //stru312::stru312(&static_AE3FA4);
    }*/

    static VertexBuffer static_AE33A0;  // idb
    /*static bool __init_flag3 = false;
    if (!__init_flag3) {
      __init_flag3 = true;

      //VertexBuffer::VertexBuffer(&static_AE33A0);
    }*/

    result = false;
    // v7 = pOutNumVertices;
    v21 = 0;
    if ((signed int)uNumVertices <= 0) {
        *pOutNumVertices = 0;
        return true;
    }
    // a2 = a1;
    // v8 = (char *)&a3->y;
    // v18 = (char *)&a3->y;
    // do
    for (uint j = 0; j < uNumVertices; j++) {
        // v17 = result + 1;
        result = j;
        v9 = &pPortalBounding[(j + 1) % (signed int)uNumVertices];
        if (pPortalBounding->vWorldPosition.x != v9->vWorldPosition.x ||
            pPortalBounding->vWorldPosition.y != v9->vWorldPosition.y ||
            pPortalBounding->vWorldPosition.z != v9->vWorldPosition.z) {
            // v10 = a3->z;
            // v11 = a3->y;
            ++v21;
            // result = 0;
            static_AE33A0.uNumVertices = 0;
            // a3a = 0;
            static_AE3FA4.x = pVertices->x;
            static_AE3FA4.y = pVertices->y;
            static_AE3FA4.z = pVertices->z;
            if ((signed int)*pOutNumVertices <= 0) {
                *pOutNumVertices = 0;
                return true;
            }
            // do
            for (int i = 0; i < (signed int)*pOutNumVertices; i++) {
                // v12 = &pVertices[result];
                if (result) {
                    if (DoDecalVertsNeedClipping(&pVertices2[i], &pVertices2[i], pPortalBounding,
                                &static_AE3FA4) &&
                        AdjustVertToClipEdge(&pVertices2[i], &pVertices2[i], pPortalBounding,
                                &static_AE3FA4, &static_AE3FB4))
                        AddVertex(&static_AE33A0, &static_AE3FB4);
                } else {
                    v19 = &pVertices2[i];
                }
                // v20 = v12;
                if (AreVectorsCollinear(&pVertices2[i], pPortalBounding,
                                        &static_AE3FA4))
                    AddVertex(&static_AE33A0, &pVertices2[i]);
                // pOutNumVertices = pOutNumVertices;
                // a3a++;
            }
            // while ( a3a < (signed int)*pOutNumVertices );
            if (!static_AE33A0.uNumVertices) {
                *pOutNumVertices = 0;
                return true;
            }
            if (DoDecalVertsNeedClipping(&pVertices2[result], v19, pPortalBounding,
                        &static_AE3FA4) &&
                AdjustVertToClipEdge(&pVertices2[result], v19, pPortalBounding,
                        &static_AE3FA4, &static_AE3FB4))
                AddVertex(&static_AE33A0, &static_AE3FB4);

            for (uint i = 0; i < static_AE33A0.uNumVertices; ++i) {
                pVertices2[i].vWorldPosition.x =
                    static_AE33A0.pVertices[i].vWorldPosition.x;
                pVertices2[i].vWorldPosition.y =
                    static_AE33A0.pVertices[i].vWorldPosition.y;
                pVertices2[i].vWorldPosition.z =
                    static_AE33A0.pVertices[i].vWorldPosition.z;
                pVertices2[i].u = static_AE33A0.pVertices[i].u;
                pVertices2[i].v = static_AE33A0.pVertices[i].v;
            }

            // v13 = static_AE33A0.uNumVertices;
            /*a3b = 0;
            if ( static_AE33A0.uNumVertices > 0 )
            {
              v14 = &static_AE33A0;
              v15 = (char *)&pVertices->vWorldPosition.y;
              for ( i = (char *)&static_AE33A0.pVertices[0].vWorldPosition.y -
            (char *)pVertices;
                    ;
                    i = (char *)&static_AE33A0.pVertices[0].vWorldPosition.y -
            (char *)pVertices )
              {
                ++a3b;
                pVertices->vWorldPosition.x =
            v14->pVertices[0].vWorldPosition.x; v14 = (VertexBuffer *)((char
            *)v14 + 48); pVertices->vWorldPosition.y = *(float *)&v15[(char
            *)&static_AE33A0 - (char *)pVertices]; pVertices->vWorldPosition.z =
            *(float *)&v15[i]; pVertices->u = *(float *)&v15[(char
            *)&static_AE33A0.pVertices[0]._rhw - (char *)pVertices];
                pVertices->v = *(float *)&v15[(char
            *)&static_AE33A0.pVertices[0].u - (char *)pVertices];
                //static_AE33A0.uNumVertices = static_AE33A0.uNumVertices;
                v15 += 48;
                if ( a3b >= static_AE33A0.uNumVertices )
                  break;
              }
            }*/

            // v7 = pOutNumVertices;
            // a3->y = a3->y;
            *pOutNumVertices = static_AE33A0.uNumVertices;
        }
        // result = v17;
        ++pPortalBounding;
        pVertices++;
        // v18 = (char *)&a3->y;
    }
    // while ( v17 < (signed int)uNumVertices );
    if (v21 < 3) *pOutNumVertices = 0;
    return true;
}

bool stru9::ClipVertsToFrustumPlane(RenderVertexSoft *pInVertices, signed int pInNumVertices,
                            RenderVertexSoft *pOutVertices,
                            unsigned int *pOutNumVertices,
                            struct Vec3_float_ *CamFrustumNormal, float CamDotDistance, char *VertsAdjusted,
                            int unused) {
    // this cycles through adjust vertice posisiton to supplied frustum plane
    // points are inside frstum plane when point dot product is greater than camera dot distance

    RenderVertexSoft *pLineEnd;  // ecx@9
    double pLinelength2;         // st7@9
    double t;                    // st6@12

    RenderVertexSoft *pLineStart = &pInVertices[0];
    // pVertices = a3;
    double pLinelength1 = CamFrustumNormal->x * pInVertices[0].vWorldPosition.x +
                    pInVertices[0].vWorldPosition.y * CamFrustumNormal->y +
                    pInVertices[0].vWorldPosition.z * CamFrustumNormal->z;
    // v20 = v13;
    bool Vert1Inside = pLinelength1 >= CamDotDistance;

    *pOutNumVertices = 0;
    if (pInNumVertices <= 0) return false;

    int VertCounter = 1;

    while (true) {
        if (Vert1Inside) {
            // ++pVertices;
            memcpy(pOutVertices, pLineStart, sizeof(RenderVertexSoft));
            ++*pOutNumVertices;
            // v10 = a5;
            pOutVertices++;
            // v9 = a1;
        }
        bool Vert2Inside = false;
        pLineEnd = &pInVertices[VertCounter % pInNumVertices];
        pLinelength2 = CamFrustumNormal->x * pLineEnd->vWorldPosition.x +
                       pLineEnd->vWorldPosition.y * CamFrustumNormal->y +
                       pLineEnd->vWorldPosition.z * CamFrustumNormal->z;
        if (pLinelength2 >= CamDotDistance) Vert2Inside = true;

        if (Vert1Inside != Vert2Inside) {
            t = (CamDotDistance - pLinelength1) / (pLinelength2 - pLinelength1);
            pOutVertices->vWorldPosition.x =
                pLineStart->vWorldPosition.x +
                (pLineEnd->vWorldPosition.x - pLineStart->vWorldPosition.x) * t;
            pOutVertices->vWorldPosition.y =
                pLineStart->vWorldPosition.y +
                (pLineEnd->vWorldPosition.y - pLineStart->vWorldPosition.y) * t;
            pOutVertices->vWorldPosition.z =
                pLineStart->vWorldPosition.z +
                (pLineEnd->vWorldPosition.z - pLineStart->vWorldPosition.z) * t;
            pOutVertices->u = pLineStart->u + (pLineEnd->u - pLineStart->u) * t;
            pOutVertices->v = pLineStart->v + (pLineEnd->v - pLineStart->v) * t;
            ++pOutVertices;
            // a3 = pVertices;
            ++*pOutNumVertices;
            *VertsAdjusted = true;
        }

        pLineStart++;
        Vert1Inside = Vert2Inside;
        pLinelength1 = pLinelength2;
        if (VertCounter >= pInNumVertices) break;
        // v9 = a1;
        VertCounter++;
    }

    return *pOutNumVertices >= 3;
}

//----- (00498737) --------------------------------------------------------
void stru9::AddVertex(struct VertexBuffer *pVertexBuffer,
                      struct RenderVertexSoft *pVertex) {
    pVertexBuffer->pVertices[pVertexBuffer->uNumVertices].vWorldPosition.x =
        pVertex->vWorldPosition.x;
    pVertexBuffer->pVertices[pVertexBuffer->uNumVertices].vWorldPosition.y =
        pVertex->vWorldPosition.y;
    pVertexBuffer->pVertices[pVertexBuffer->uNumVertices].vWorldPosition.z =
        pVertex->vWorldPosition.z;
    pVertexBuffer->pVertices[pVertexBuffer->uNumVertices].u = pVertex->u;
    pVertexBuffer->pVertices[pVertexBuffer->uNumVertices].v = pVertex->v;
    ++pVertexBuffer->uNumVertices;
}


// this adjusts the vert to the face
//----- (00498774) --------------------------------------------------------
bool stru9::AdjustVertToClipEdge(struct RenderVertexSoft *a1, struct RenderVertexSoft *a2,
                    struct RenderVertexSoft *a3, struct stru312 *a4,
                    struct RenderVertexSoft *a5) {
    // this looks like it is meant to clip the light map / decal verts to the face plane verts

    // a1 - decal vert1
    // a2 - decal vert2
    // a3 - face vertex to clip to
    // a4 - vec along plane perp to decal verts
    // a5 - verts out


    RenderVertexSoft *v6;  // ecx@5
    int result;            // eax@5
    double v8;             // st7@5
                           //  __int16 v9; // fps@5
    double v10;            // st7@6
    double v11;            // st6@8
    double v12;            // st6@18
    char v13;              // c0@24
    char v14;              // c2@24
    char v15;              // c3@24
    float a1a;             // [sp+10h] [bp+8h]@5

    // __debugbreak();

    static stru312 static_AE3388;
    static stru312 static_AE3378;

    v6 = a1;
    static_AE3378.x = a1->vWorldPosition.x - a3->vWorldPosition.x;
    static_AE3378.y = a1->vWorldPosition.y - a3->vWorldPosition.y;
    HEXRAYS_HIWORD(result) = HEXRAYS_HIWORD(a4);
    static_AE3378.z = a1->vWorldPosition.z - a3->vWorldPosition.z;
    static_AE3388.x = a2->vWorldPosition.x - a1->vWorldPosition.x;
    static_AE3388.y = a2->vWorldPosition.y - a1->vWorldPosition.y;
    static_AE3388.z = a2->vWorldPosition.z - a1->vWorldPosition.z;
    a1a = static_AE3388.x * a4->x + static_AE3388.z * a4->z +
          static_AE3388.y * a4->y;
    v8 = static_AE3378.x * a4->x + static_AE3378.z * a4->z +
         static_AE3378.y * a4->y;
    // UNDEF(v9);
    if (a1a != 0.0) {
        v10 = -(v8 / a1a);
        if (a1a <= 0.0) {
            if (v10 <= 0.0)
                v12 = 0.0;
            else
                v12 = v10;
            if (v12 <= 1.0) {
                if (v10 <= 0.0) v10 = (float)0.0;
            } else {
                v10 = 1.0;
            }
        } else {
            if (v10 >= 1.0)
                v11 = 1.0;
            else
                v11 = v10;
            if (v11 >= 0.0) {
                if (v10 >= 1.0) v10 = (float)1.0;
            } else {
                v10 = 0.0;
            }
        }
        result = (bool)a5;
        a5->vWorldPosition.x =
            (a2->vWorldPosition.x - v6->vWorldPosition.x) * v10 +
            v6->vWorldPosition.x;
        a5->vWorldPosition.y =
            (a2->vWorldPosition.y - v6->vWorldPosition.y) * v10 +
            v6->vWorldPosition.y;
        a5->vWorldPosition.z =
            (a2->vWorldPosition.z - v6->vWorldPosition.z) * v10 +
            v6->vWorldPosition.z;
        a5->u = (a2->u - v6->u) * v10 + v6->u;
        a5->v = (a2->v - v6->v) * v10 + v6->v;
        return 1;
    }
    v13 = v8 < 0.0;
    v14 = 0;
    v15 = v8 == 0.0;
    // BYTE1(result) = HIBYTE(v9);
    if (v8 < 0.0) return 1;
    return 0;
}
// AE3398: using guessed type char static_init_flag__AE3388_bit1__AE3378_bit2;

//----- (0049895A) --------------------------------------------------------
bool stru9::AreVectorsCollinear(struct RenderVertexSoft *a1,
                                struct RenderVertexSoft *a2,
                                struct stru312 *a3) {
    // is vector a2->a1 in same semiplane as a3

    /*static */stru312 static_F942A0;

    static_F942A0.x = a1->vWorldPosition.x - a2->vWorldPosition.x;
    static_F942A0.y = a1->vWorldPosition.y - a2->vWorldPosition.y;
    static_F942A0.z = a1->vWorldPosition.z - a2->vWorldPosition.z;

    /*static*/ float flt_F942B4 = static_F942A0.z * a3->z +
                              static_F942A0.y * a3->y + static_F942A0.x * a3->x;
    if (flt_F942B4 >= 0) return true;
    return false;
}

//----- (004989E1) --------------------------------------------------------
bool stru9::DoDecalVertsNeedClipping(struct RenderVertexSoft *a1, struct RenderVertexSoft *a2,
                    struct RenderVertexSoft *a3, struct stru312 *a4) {
    // lightmap/decal first two verts
    // 3rd is face vert
    // 4th is vec along face

    bool r1;
    bool r2;

    // checks if vector a3->a1 is in the same semiplane as vec a4

    r1 = AreVectorsCollinear(a1, a3, a4);
    r2 = AreVectorsCollinear(a2, a3, a4);

    // if one vert is in and one is out, we need clipping

    bool result = ((!r1) && (r2 == 1)) || ((r1 == 1) && (!r2));

    return result;
}

// for clipping lightmaps and decals to face
//----- (004980B9) --------------------------------------------------------
bool stru9::ClipVertsToFace(RenderVertexSoft *a1, unsigned int uNumVertices,
                    float pNormalX, float pNormalY, float pNormalZ,
                    RenderVertexSoft *pOutVertices,
                    signed int *pOutNumVertices) {
    // first two are for the face
    // last two are light map / decal


    RenderVertexSoft *v12;  // ecx@9
    double v13;             // st7@12
    double v14;             // st6@12
    double v15;             // st5@12
    signed int v25;         // [sp+18h] [bp-Ch]@7

    static RenderVertexSoft stru_AE4BFC;
    static stru312 static_sub_4980B9_stru_AE4BEC;       // idb
    static VertexBuffer static_sub_4980B9_stru_AE3FE8;  // idb

    v25 = 0;
    if (uNumVertices <= 0) {
        *pOutNumVertices = 0;
        return true;
    }

    for (uint i = 0; i < uNumVertices; ++i) {
        v12 = &a1[(i + 1) % uNumVertices];
        if (a1[i].vWorldPosition.x != v12->vWorldPosition.x ||
            a1[i].vWorldPosition.y != v12->vWorldPosition.y ||
            a1[i].vWorldPosition.z != v12->vWorldPosition.z) {
            v13 = v12->vWorldPosition.x - a1[i].vWorldPosition.x;
            v14 = v12->vWorldPosition.y - a1[i].vWorldPosition.y;
            v15 = v12->vWorldPosition.z - a1[i].vWorldPosition.z;
            ++v25;

            static_sub_4980B9_stru_AE3FE8.uNumVertices = 0;
            // cross product of face normals and vec ab gives vec running along face plane
            static_sub_4980B9_stru_AE4BEC.x = pNormalY * v15 - v14 * pNormalZ;
            static_sub_4980B9_stru_AE4BEC.y = v13 * pNormalZ - v15 * pNormalX;
            static_sub_4980B9_stru_AE4BEC.z = v14 * pNormalX - v13 * pNormalY;
            if (*pOutNumVertices == 0) return true;

            for (uint j = 0; j < *pOutNumVertices; ++j) {
                if (j) {
                    if (DoDecalVertsNeedClipping(&pOutVertices[j - 1], &pOutVertices[j], &a1[i],
                                &static_sub_4980B9_stru_AE4BEC) &&
                        AdjustVertToClipEdge(&pOutVertices[j - 1], &pOutVertices[j], &a1[i],
                                &static_sub_4980B9_stru_AE4BEC, &stru_AE4BFC))
                        AddVertex(&static_sub_4980B9_stru_AE3FE8, &stru_AE4BFC);
                }

                if (AreVectorsCollinear(&pOutVertices[j], &a1[i],
                                        &static_sub_4980B9_stru_AE4BEC))
                    AddVertex(&static_sub_4980B9_stru_AE3FE8, &pOutVertices[j]);
            }

            if (!static_sub_4980B9_stru_AE3FE8.uNumVertices) {
                *pOutNumVertices = 0;
                return true;
            }
            if (DoDecalVertsNeedClipping(&pOutVertices[*pOutNumVertices - 1], &pOutVertices[0],
                        &a1[i], &static_sub_4980B9_stru_AE4BEC) &&
                AdjustVertToClipEdge(&pOutVertices[*pOutNumVertices - 1], &pOutVertices[0],
                        &a1[i], &static_sub_4980B9_stru_AE4BEC, &stru_AE4BFC))
                AddVertex(&static_sub_4980B9_stru_AE3FE8, &stru_AE4BFC);

            for (uint j = 0; j < static_sub_4980B9_stru_AE3FE8.uNumVertices;
                 ++j) {
                pOutVertices[j].vWorldPosition.x =
                    static_sub_4980B9_stru_AE3FE8.pVertices[j].vWorldPosition.x;
                pOutVertices[j].vWorldPosition.y =
                    static_sub_4980B9_stru_AE3FE8.pVertices[j].vWorldPosition.y;
                pOutVertices[j].vWorldPosition.z =
                    static_sub_4980B9_stru_AE3FE8.pVertices[j].vWorldPosition.z;
                pOutVertices[j].u =
                    static_sub_4980B9_stru_AE3FE8.pVertices[j].u;
                pOutVertices[j].v =
                    static_sub_4980B9_stru_AE3FE8.pVertices[j].v;
            }
            *pOutNumVertices = static_sub_4980B9_stru_AE3FE8.uNumVertices;
        }
    }

    if (v25 < 3) *pOutNumVertices = 0;

    return true;
}
// AE4C2C: using guessed type char static_sub_4980B9_byte_AE4C2C_init_flag;
