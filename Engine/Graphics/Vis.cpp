#include "Engine/Graphics/Vis.h"

#include <cstdlib>

#include "Engine/Engine.h"
#include "Engine/IocContainer.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"

#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Viewport.h"

#include "Engine/Objects/Actor.h"

using EngineIoc = Engine_::IocContainer;

static Vis_SelectionList Vis_static_sub_4C1944_stru_F8BDE8;

Vis_SelectionFilter vis_sprite_filter_1 = {
    VisObjectType_Sprite, OBJECT_Decoration, 0, 0, ExcludeType};  // 00F93E1C
Vis_SelectionFilter vis_sprite_filter_2 = {
    VisObjectType_Sprite, OBJECT_Decoration, 0, 0, ExcludeType};  // 00F93E30
Vis_SelectionFilter vis_face_filter = {
    VisObjectType_Face, OBJECT_Any, -1, 0, None};  // 00F93E44
Vis_SelectionFilter vis_door_filter = {
    VisObjectType_Face, OBJECT_BLVDoor, -1, FACE_HAS_EVENT, None };  // 00F93E58
Vis_SelectionFilter vis_sprite_filter_3 = {
    VisObjectType_Sprite, OBJECT_Decoration, -1, 0, ExclusionIfNoEvent};  // 00F93E6C
Vis_SelectionFilter vis_sprite_filter_4 = {
    VisObjectType_Any, OBJECT_Item, -1, 0, None };  // static to sub_44EEA7

//----- (004C1026) --------------------------------------------------------
Vis_ObjectInfo *Vis::DetermineFacetIntersection(BLVFace *face, unsigned int pid,
                                                float pick_depth) {
    //  char *v4; // eax@4
    //  signed int v5; // ecx@4
    RenderVertexSoft pRay[2];  // [sp+20h] [bp-70h]@17
                               //  int v20; // [sp+84h] [bp-Ch]@10

    static Vis_SelectionList SelectedPointersList;  // stru_F8FE00
    SelectedPointersList.uNumPointers = 0;

    static bool _init_flag = false;
    static RenderVertexSoft static_DetermineFacetIntersection_array_F8F200[64];
    if (!_init_flag) {
        _init_flag = true;
        for (uint i = 0; i < 64; ++i)
            static_DetermineFacetIntersection_array_F8F200[i].flt_2C = 0.0f;
    }

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        if ((signed int)face->uNumVertices > 0) {
            for (int i = 0; i < face->uNumVertices; i++) {
                static_DetermineFacetIntersection_array_F8F200[i]
                    .vWorldPosition.x =
                    (float)pIndoor->pVertices[face->pVertexIDs[i]].x;
                static_DetermineFacetIntersection_array_F8F200[i]
                    .vWorldPosition.y =
                    (float)pIndoor->pVertices[face->pVertexIDs[i]].y;
                static_DetermineFacetIntersection_array_F8F200[i]
                    .vWorldPosition.z =
                    (float)pIndoor->pVertices[face->pVertexIDs[i]].z;
            }
        }
    } else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        uint bmodel_id = pid >> 9;
        Vec3_int_ *v =
            (Vec3_int_ *)pOutdoor->pBModels[bmodel_id].pVertices.pVertices;
        for (uint i = 0; i < face->uNumVertices; ++i) {
            static_DetermineFacetIntersection_array_F8F200[i].vWorldPosition.x =
                (float)v[face->pVertexIDs[i]].x;
            static_DetermineFacetIntersection_array_F8F200[i].vWorldPosition.y =
                (float)v[face->pVertexIDs[i]].y;
            static_DetermineFacetIntersection_array_F8F200[i].vWorldPosition.z =
                (float)v[face->pVertexIDs[i]].z;
        }
    } else {
        assert(false);
    }

    pIndoorCameraD3D->ViewTransform(
        static_DetermineFacetIntersection_array_F8F200, face->uNumVertices);
    pIndoorCameraD3D->Project(static_DetermineFacetIntersection_array_F8F200,
                              face->uNumVertices, 1);

    SortVectors_x(static_DetermineFacetIntersection_array_F8F200, 0,
                  face->uNumVertices - 1);
    if (static_DetermineFacetIntersection_array_F8F200[0].vWorldViewPosition.x >
        pick_depth)
        return nullptr;

    float screenspace_center_x, screenspace_center_y;
    GetPolygonScreenSpaceCenter(static_DetermineFacetIntersection_array_F8F200,
                                face->uNumVertices, &screenspace_center_x,
                                &screenspace_center_y);
    if (IsPolygonOccludedByBillboard(
            static_DetermineFacetIntersection_array_F8F200, face->uNumVertices,
            screenspace_center_x, screenspace_center_y))
        return nullptr;

    CastPickRay(pRay, screenspace_center_x, screenspace_center_y, pick_depth);

    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        PickOutdoorFaces_Mouse(pick_depth, pRay, &SelectedPointersList,
                               &vis_face_filter, true);
    else if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        PickIndoorFaces_Mouse(pick_depth, pRay, &SelectedPointersList,
                              &vis_face_filter);
    else
        assert(false);

    SelectedPointersList.create_object_pointers();
    sort_object_pointers(SelectedPointersList.object_pointers, 0,
                         SelectedPointersList.uNumPointers - 1);
    if (!SelectedPointersList.uNumPointers) return nullptr;

    if (!SelectedPointersList.SelectionPointers(VisObjectType_Face, pid))
        return nullptr;

    if (SelectedPointersList.uNumPointers)
        return SelectedPointersList.object_pointers[0];
    else
        return nullptr;
}
// F91E08: using guessed type char
// static_DetermineFacetIntersection_byte_F91E08__init_flags;

//----- (004C12C3) --------------------------------------------------------
bool Vis::IsPolygonOccludedByBillboard(RenderVertexSoft *vertices,
                                       int num_vertices, float x, float y) {
    int v13 = -1;
    // v5 = 0;

    // v6 = render->pBillboardRenderListD3D;
    for (uint i = 0; i < render->uNumBillboardsToDraw; ++i) {
        RenderBillboardD3D *billboard = &render->pBillboardRenderListD3D[i];
        if (IsPointInsideD3DBillboard(billboard, x, y)) {
            if (v13 == -1)
                v13 = i;
            else if (pBillboardRenderList[billboard->sParentBillboardID]
                         .screen_space_z <
                     pBillboardRenderList[render->pBillboardRenderListD3D[v13]
                                              .sParentBillboardID]
                         .screen_space_z)
                v13 = i;
        }
    }

    if (v13 == -1) return false;

    // //Bounding rectangle(Ограничивающий
    // прямоугольник)-------------------------
    // v7 = 3.4028235e38;
    float min_x = FLT_MAX;
    // a4a = 3.4028235e38;
    float min_y = FLT_MAX;
    // a3a = -3.4028235e38;
    float max_x = -FLT_MAX;
    // thisb = -3.4028235e38;
    float max_y = -FLT_MAX;
    for (int i = 0; i < num_vertices; ++i) {
        RenderVertexSoft *v = &vertices[i];

        if (v->vWorldViewProjX < min_x) min_x = v->vWorldViewProjX;
        if (v->vWorldViewProjX > max_x) max_x = v->vWorldViewProjX;

        if (v->vWorldViewProjY < min_y) min_y = v->vWorldViewProjY;
        if (v->vWorldViewProjY > max_y) max_y = v->vWorldViewProjY;
    }
    // //--------------------------------

    if (min_x < render->pBillboardRenderListD3D[v13].pQuads[0].pos.x ||
        render->pBillboardRenderListD3D[v13].pQuads[0].pos.y > min_y ||
        render->pBillboardRenderListD3D[v13].pQuads[3].pos.x < max_x ||
        render->pBillboardRenderListD3D[v13].pQuads[1].pos.y < max_y)
        return false;

    return true;
}

//----- (004C1417) --------------------------------------------------------
void Vis::GetPolygonCenter(RenderVertexD3D3 *pVertices,
                           unsigned int uNumVertices, float *pCenterX,
                           float *pCenterY) {
    static RenderVertexD3D3 unk_F8EA00[64];

    memcpy(unk_F8EA00, pVertices, 32 * uNumVertices);

    SortVerticesByX(unk_F8EA00, 0, uNumVertices - 1);
    *pCenterX =
        (unk_F8EA00[uNumVertices - 1].pos.x - unk_F8EA00[0].pos.x) * 0.5 +
        unk_F8EA00[0].pos.x;

    SortVerticesByY(unk_F8EA00, 0, uNumVertices - 1);
    *pCenterY =
        (unk_F8EA00[uNumVertices - 1].pos.y - unk_F8EA00[0].pos.y) * 0.5 +
        unk_F8EA00[0].pos.y;
}

//----- (004C1495) --------------------------------------------------------
void Vis::GetPolygonScreenSpaceCenter(RenderVertexSoft *vertices,
                                      int num_vertices, float *out_center_x,
                                      float *out_center_y) {
    //  char *v5; // eax@2
    //  signed int v6; // ecx@2
    //  float *result; // eax@5

    static RenderVertexSoft static_sub_4C1495_array_F8DDF8[64];

    memcpy(static_sub_4C1495_array_F8DDF8, vertices, 48 * num_vertices);

    SortByScreenSpaceX(static_sub_4C1495_array_F8DDF8, 0, num_vertices - 1);
    *out_center_x =
        (static_sub_4C1495_array_F8DDF8[num_vertices - 1].vWorldViewProjX -
         static_sub_4C1495_array_F8DDF8[0].vWorldViewProjX) *
            0.5 +
        static_sub_4C1495_array_F8DDF8[0].vWorldViewProjX;

    SortByScreenSpaceY(static_sub_4C1495_array_F8DDF8, 0, num_vertices - 1);
    *out_center_y =
        (static_sub_4C1495_array_F8DDF8[num_vertices - 1].vWorldViewProjY -
         static_sub_4C1495_array_F8DDF8[0].vWorldViewProjY) *
            0.5 +
        static_sub_4C1495_array_F8DDF8[0].vWorldViewProjY;
}

//----- (004C1542) --------------------------------------------------------
void Vis::PickBillboards_Mouse(float fPickDepth, float fX, float fY,
                               Vis_SelectionList *list,
                               Vis_SelectionFilter *filter) {
    for (uint i = 0; i < render->uNumBillboardsToDraw; ++i) {
        RenderBillboardD3D *d3d_billboard = &render->pBillboardRenderListD3D[i];
        if (is_part_of_selection((void *)i, filter) &&
            IsPointInsideD3DBillboard(d3d_billboard, fX, fY)) {
            if (DoesRayIntersectBillboard(fPickDepth, i)) {
                RenderBillboard *billboard =
                    &pBillboardRenderList[d3d_billboard->sParentBillboardID];

                list->AddObject((void *)d3d_billboard->sParentBillboardID,
                                VisObjectType_Sprite, billboard->screen_space_z,
                                billboard->object_pid);
            }
        }
    }
}

//----- (004C1607) --------------------------------------------------------
bool Vis::IsPointInsideD3DBillboard(RenderBillboardD3D *a1, float x, float y) {
    /*Not the original implementation.
    This function is redone to use Grayface's mouse pick implementation to take
    only the visible
    parts of billboards into account - I don't really have too much of an idea
    how it actually works*/

    if (a1->sParentBillboardID == -1) return false;

    float drX = a1->pQuads[0].pos.x;
    float drW = a1->pQuads[3].pos.x - drX;
    float drY = a1->pQuads[0].pos.y;
    float drH = a1->pQuads[1].pos.y - drY;



    // for small items dont bother with the per pixel checks
    if (abs(drH) < 5 || abs(drW) < 5) {
        if (drW < 0) {  // sprite reversed
            drX = a1->pQuads[3].pos.x;
            drW = a1->pQuads[0].pos.x - drX;
        }
        if (x >= drX && x < (drW + drX) && y >= drY && y < (drH + drY)) {  // simple bounds check
            return 1;
        } else {
            return 0;
        }
    }


    Sprite *ownerSprite = nullptr;
    for (int i = 0; i < pSprites_LOD->uNumLoadedSprites; ++i) {
        if ((void *)pSprites_LOD->pHardwareSprites[i].texture == a1->texture) {
            ownerSprite = &pSprites_LOD->pHardwareSprites[i];
            break;
        }
    }

    if (ownerSprite == nullptr) return false;

    int sx =
        ownerSprite->uAreaX + int(ownerSprite->uAreaWidth * (x - drX) / drW);
    int sy =
        ownerSprite->uAreaY + int(ownerSprite->uAreaHeight * (y - drY) / drH);

    LODSprite *spriteHeader = ownerSprite->sprite_header;

    if (sy < 0 || sy >= spriteHeader->uHeight) return false;
    if (sx < 0 || sx >= spriteHeader->uWidth) return false;

    return spriteHeader->bitmap[sy * spriteHeader->uWidth + sx] != 0;
}

//----- (004C16B4) --------------------------------------------------------
void Vis::PickIndoorFaces_Mouse(float fDepth, RenderVertexSoft *pRay,
                                Vis_SelectionList *list,
                                Vis_SelectionFilter *filter) {
    int v5;              // eax@1
    // signed int pFaceID;  // edi@2
    // int v9; // eax@7
    unsigned int *pNumPointers;  // eax@7
    Vis_ObjectInfo *v12;         // edi@7
    RenderVertexSoft a1;         // [sp+Ch] [bp-44h]@1
    // void *v15; // [sp+40h] [bp-10h]@7
    int v17;  // [sp+48h] [bp-8h]@1

    v5 = 0;
    v17 = 0;

    // for (a1.flt_2C = 0.0; v17 < (signed int)pBspRenderer->num_faces; ++v17) {
    //     pFaceID = pBspRenderer->faces[v5].uFaceID;
    //     if (pFaceID >= 0) {
    //        if (pFaceID < (signed int)pIndoor->uNumFaces) {

    for (a1.flt_2C = 0.0; v17 < (signed int)pIndoor->uNumFaces; ++v17) {
        BLVFace *face = &pIndoor->pFaces[/*pFaceID*/v17];
        if (is_part_of_selection(face, filter)) {
            if (!pIndoorCameraD3D->IsCulled(face)) {
                if (Intersect_Ray_Face(pRay, pRay + 1, &fDepth, &a1,
                                        face, 0xFFFFFFFFu)) {
                    pIndoorCameraD3D->ViewTransform(&a1, 1);
                    // v9 = fixpoint_from_float(/*v8,
                    // */a1.vWorldViewPosition.x); HEXRAYS_LOWORD(v9) =
                    // 0; v15 = (void *)((PID(OBJECT_BModel,pFaceID)) +
                    // v9);
                    pNumPointers = &list->uNumPointers;
                    v12 = &list->object_pool[list->uNumPointers];
                    v12->object = &pIndoor->pFaces[/*pFaceID*/v17];
                    v12->depth = a1.vWorldViewPosition.x;
                    v12->object_pid = PID(OBJECT_BModel, /*pFaceID*/v17);
                    v12->object_type = VisObjectType_Face;
                    ++*pNumPointers;
                    // logger->Info("raypass");
                } else {
                    // __debugbreak();
                    // logger->Info("rayfaile");
                }
            }
        }

        if (face->uAttributes & FACE_PICKED)
            face->uAttributes |= FACE_OUTLINED;
        else
            face->uAttributes &= ~FACE_OUTLINED;
        face->uAttributes &= ~FACE_PICKED;
           // }
       // }
        v5 = v17 + 1;
    }
}

void Vis::PickOutdoorFaces_Mouse(float fDepth, RenderVertexSoft *pRay,
                                 Vis_SelectionList *list,
                                 Vis_SelectionFilter *filter,
                                 bool only_reachable) {
    if (!pOutdoor) return;

    for (BSPModel &model : pOutdoor->pBModels) {
        int reachable;
        if (!IsBModelVisible(&model, &reachable)) {
            continue;
        }
        if (!reachable && only_reachable) {
            continue;
        }

        for (ODMFace &face : model.pFaces) {
            if (is_part_of_selection(&face, filter)) {
                BLVFace blv_face;
                blv_face.FromODM(&face);

                RenderVertexSoft intersection;
                if (Intersect_Ray_Face(pRay, pRay + 1, &fDepth, &intersection,
                                       &blv_face, model.index)) {
                    pIndoorCameraD3D->ViewTransform(&intersection, 1);
                    // int v13 = fixpoint_from_float(/*v12,
                    // */intersection.vWorldViewPosition.x); v13 &= 0xFFFF0000;
                    // v13 += PID(OBJECT_BModel, j | (i << 6));
                    uint32_t pid =
                        PID(OBJECT_BModel, face.index | (model.index << 6));
                    list->AddObject(&face, VisObjectType_Face,
                                    intersection.vWorldViewPosition.x, pid);
                }

                if (blv_face.uAttributes & FACE_PICKED)
                    face.uAttributes |= FACE_OUTLINED;
                else
                    face.uAttributes &= ~FACE_OUTLINED;
                blv_face.uAttributes &= ~FACE_PICKED;
            }
        }
    }
}

//----- (004C1944) --------------------------------------------------------
unsigned short Vis::PickClosestActor(int object_id, unsigned int pick_depth,
                                     VisSelectFlags select_flags, int not_at_ai_state, int at_ai_state) {
    Vis_SelectionFilter selectionFilter;  // [sp+18h] [bp-20h]@3

    static Vis_SelectionList Vis_static_sub_4C1944_stru_F8BDE8;

    selectionFilter.vis_object_type = VisObjectType_Sprite;
    selectionFilter.object_type = object_id;
    selectionFilter.at_ai_state = at_ai_state;
    selectionFilter.no_at_ai_state = not_at_ai_state;
    selectionFilter.select_flags = select_flags;
    Vis_static_sub_4C1944_stru_F8BDE8.uNumPointers = 0;
    PickBillboards_Keyboard(pick_depth, &Vis_static_sub_4C1944_stru_F8BDE8,
                            &selectionFilter);
    Vis_static_sub_4C1944_stru_F8BDE8.create_object_pointers(
        Vis_SelectionList::Unique);
    sort_object_pointers(Vis_static_sub_4C1944_stru_F8BDE8.object_pointers, 0,
                         Vis_static_sub_4C1944_stru_F8BDE8.uNumPointers - 1);

    if (!Vis_static_sub_4C1944_stru_F8BDE8.uNumPointers) return -1;
    return Vis_static_sub_4C1944_stru_F8BDE8.object_pointers[0]->object_pid;
}

//----- (004C1A02) --------------------------------------------------------
void Vis::_4C1A02() {
    RenderVertexSoft v1;  // [sp+8h] [bp-C0h]@1
    RenderVertexSoft v2;  // [sp+38h] [bp-90h]@1
    RenderVertexSoft v3;  // [sp+68h] [bp-60h]@1
    RenderVertexSoft v4;  // [sp+98h] [bp-30h]@1

    v2.flt_2C = 0.0;
    v2.vWorldPosition.x = 0.0;
    v2.vWorldPosition.y = 65536.0;
    v2.vWorldPosition.z = 0.0;

    v1.flt_2C = 0.0;
    v1.vWorldPosition.x = 65536.0;
    v1.vWorldPosition.y = 0.0;
    v1.vWorldPosition.z = 0.0;

    v3.flt_2C = 0.0;
    v3.vWorldPosition.x = 0.0;
    v3.vWorldPosition.y = 65536.0;
    v3.vWorldPosition.z = 0.0;

    v4.flt_2C = 0.0;
    v4.vWorldPosition.x = 65536.0;
    v4.vWorldPosition.y = 0.0;
    v4.vWorldPosition.z = 0.0;

    memcpy(&this->stru_200C, &v1, 0x60u);
    memcpy(&this->stru_206C, &v4, 0x60u);
}

//----- (004C1ABA) --------------------------------------------------------
void Vis::SortVectors_x(RenderVertexSoft *pArray, int start, int end) {
    int left_sort_index;          // ebx@2
    int right_sort_index;         // ecx@2
    RenderVertexSoft temp_array;  // [sp+4h] [bp-6Ch]@8
    RenderVertexSoft max_array;   // [sp+34h] [bp-3Ch]@2

    if (end > start) {
        left_sort_index = start - 1;
        right_sort_index = end;
        memcpy(&max_array, &pArray[end], sizeof(max_array));
        while (1) {
            do {
                ++left_sort_index;
            } while (pArray[left_sort_index].vWorldViewPosition.x <
                     (double)max_array.vWorldViewPosition.x);
            do {
                --right_sort_index;
            } while (pArray[right_sort_index].vWorldViewPosition.x >
                     (double)max_array.vWorldViewPosition.x);
            if (left_sort_index >= right_sort_index) break;
            memcpy(&temp_array, &pArray[left_sort_index], sizeof(temp_array));
            memcpy(&pArray[left_sort_index], &pArray[right_sort_index],
                   sizeof(pArray[left_sort_index]));
            memcpy(&pArray[right_sort_index], &temp_array,
                   sizeof(pArray[right_sort_index]));
        }
        memcpy(&temp_array, &pArray[left_sort_index], sizeof(temp_array));
        memcpy(&pArray[left_sort_index], &pArray[end],
               sizeof(pArray[left_sort_index]));
        memcpy(&pArray[end], &temp_array, sizeof(pArray[end]));
        SortVectors_x(pArray, start, left_sort_index - 1);
        SortVectors_x(pArray, left_sort_index + 1, end);
    }
}

Vis_PIDAndDepth InvalidPIDAndDepth() {
    Vis_PIDAndDepth result;
    result.depth = 0;
    result.object_pid = PID_INVALID;
    return result;
}

//----- (004C1BAA) --------------------------------------------------------
Vis_PIDAndDepth Vis::get_object_zbuf_val(Vis_ObjectInfo *info) {
    switch (info->object_type) {
        case VisObjectType_Sprite:
        case VisObjectType_Face: {
            Vis_PIDAndDepth result;
            result.depth = info->depth;
            result.object_pid = info->object_pid;
            return result;
        }

        default:
            log->Warning(
                "Undefined type requested for: CVis::get_object_zbuf_val()");
            return InvalidPIDAndDepth();
    }
}

//----- (004C1BF1) --------------------------------------------------------
Vis_PIDAndDepth Vis::get_picked_object_zbuf_val() {
    if (!default_list.uNumPointers) return InvalidPIDAndDepth();

    return get_object_zbuf_val(default_list.object_pointers[0]);
}

//----- (004C1C0C) --------------------------------------------------------
bool Vis::Intersect_Ray_Face(RenderVertexSoft *pRayStart,
                             RenderVertexSoft *pRayEnd, float *pDepth,
                             RenderVertexSoft *Intersection, BLVFace *pFace,
                             signed int pBModelID) {
    float c1;                    // st5@6
    float c2;                    // st7@11
    Vec3_short_ IntersectPoint;  // ST04_6@11

    if (pFace->Portal() || pFace->Invisible()) return false;

    int ray_dir_x = pRayEnd->vWorldPosition.x -
                    pRayStart->vWorldPosition
                        .x,  // calculate the direction vector of the
                             // line(вычислим вектор направления линий)
        ray_dir_y = pRayEnd->vWorldPosition.y - pRayStart->vWorldPosition.y,
        ray_dir_z = pRayEnd->vWorldPosition.z - pRayStart->vWorldPosition.z;

    // c1 = -d-(n*p0)
    c1 = -pFace->pFacePlane.dist -
         (pFace->pFacePlane.vNormal.x * pRayStart->vWorldPosition.x +
          pFace->pFacePlane.vNormal.y * pRayStart->vWorldPosition.y +
          pFace->pFacePlane.vNormal.z * pRayStart->vWorldPosition.z);
    if (c1 > 0) return false;
#define EPSILON 1e-6
    // c2 = n*u
    c2 = pFace->pFacePlane.vNormal.x *
             ray_dir_y  // get length of the line(Это дает нам длину линии)
         + pFace->pFacePlane.vNormal.y * ray_dir_x +
         pFace->pFacePlane.vNormal.z * ray_dir_z;
    if (c2 > -EPSILON &&
        c2 < EPSILON)  // ray faces face's normal ( > 0) or parallel ( == 0)
        return false;

    // t = -d-(n*p0)/n*u
    float t = c1 / c2;  // How far is crossing the line in percent for 0 to
                        // 1(Как далеко пересечение линии в процентах от 0 до 1 )

    if (t < 0 || t > 1) return false;

    // p(t) = p0 + tu;
    Intersection->vWorldPosition.x =
        pRayStart->vWorldPosition.x +
        t * ray_dir_y;  // add the interest to the start line(прибавляем процент
                        // линии к линии старта)
    Intersection->vWorldPosition.y =
        pRayStart->vWorldPosition.y + t * ray_dir_x;
    Intersection->vWorldPosition.z =
        pRayStart->vWorldPosition.z + t * ray_dir_z;

    IntersectPoint.x = Intersection->vWorldPosition.x;
    IntersectPoint.y = Intersection->vWorldPosition.y;
    IntersectPoint.z = Intersection->vWorldPosition.z;

    if (!CheckIntersectBModel(pFace, IntersectPoint, pBModelID)) return false;

    *pDepth = t;  // Record the distance from the origin of the ray (Записываем
                  // дистанцию от начала луча)
    return true;
}

//----- (004C1D2B) --------------------------------------------------------
bool Vis::CheckIntersectBModel(BLVFace *pFace, Vec3_short_ IntersectPoint, signed int sModelID) {
    int v5;          // esi@10
    bool v6;         // edi@10
    signed int v10;  // ebx@14
                     //  int v15; // [sp+10h] [bp-Ch]@10
    signed int v16;  // [sp+18h] [bp-4h]@10

    int a = 0, b = 0;

    if (IntersectPoint.x < pFace->pBounding.x1 ||
        IntersectPoint.x > pFace->pBounding.x2 ||
        IntersectPoint.y < pFace->pBounding.y1 ||
        IntersectPoint.y > pFace->pBounding.y2 ||
        IntersectPoint.z < pFace->pBounding.z1 ||
        IntersectPoint.z > pFace->pBounding.z2)
        return false;

    if (sModelID != -1)
        ODM_CreateIntersectFacesVertexCoordList(
            &a, &b, intersect_face_vertex_coords_list_a.data(),
            intersect_face_vertex_coords_list_b.data(), &IntersectPoint, pFace,
            sModelID);
    else
        BLV_CreateIntersectFacesVertexCoordList(
            &a, &b, intersect_face_vertex_coords_list_a.data(),
            intersect_face_vertex_coords_list_b.data(), &IntersectPoint, pFace);

    v5 = 2 * pFace->uNumVertices;
    v16 = 0;
    intersect_face_vertex_coords_list_a[v5] =
        intersect_face_vertex_coords_list_a[0];
    intersect_face_vertex_coords_list_b[v5] =
        intersect_face_vertex_coords_list_b[0];
    v6 = intersect_face_vertex_coords_list_b[0] >= b;
    if (v5 <= 0) return false;
    for (int i = 0; i < v5; ++i) {
        if (v16 >= 2) break;
        if (v6 ^ (intersect_face_vertex_coords_list_b[i + 1] >= b)) {
            if (intersect_face_vertex_coords_list_a[i + 1] >= a)
                v10 = 0;
            else
                v10 = 2;
            v10 |= intersect_face_vertex_coords_list_a[i] < a ? 1 : 0;
            if (v10 != 3) {
                if (!v10) {
                    ++v16;
                } else {
                    float _v1 = ((intersect_face_vertex_coords_list_a[i + 1] - intersect_face_vertex_coords_list_a[i])) / (intersect_face_vertex_coords_list_b[i + 1] - intersect_face_vertex_coords_list_b[i]);
                    int _v2 = (b - intersect_face_vertex_coords_list_b[i]) * _v1 + 0.5;

                    if (intersect_face_vertex_coords_list_a[i] + (_v2) >= a)
                        ++v16;
                }
            }
        }
        v6 = intersect_face_vertex_coords_list_b[i + 1] >= b;
    }

    if (v16 != 1) return false;

    if (engine->config->show_picked_face)
        pFace->uAttributes |= FACE_PICKED;


    return true;
    /*
      int v5; // esi@10
      bool v6; // edi@10
      signed int v10; // ebx@14
      int v11; // edi@16
      signed int v12; // ST28_4@18
      signed __int64 v13; // qtt@18
      signed int result; // eax@21
      int v15; // [sp+10h] [bp-Ch]@10
      signed int v16; // [sp+18h] [bp-4h]@10

      int a = 0, b = 0;

      if (IntersectPoint.x < pFace->pBounding.x1 || IntersectPoint.x >
    pFace->pBounding.x2 || IntersectPoint.y < pFace->pBounding.y1 ||
    IntersectPoint.y > pFace->pBounding.y2 || IntersectPoint.z <
    pFace->pBounding.z1 || IntersectPoint.z > pFace->pBounding.z2 ) return
    false;

      pFace->uAttributes |= 0x80000000;

      if (uModelID != -1)
        ODM_CreateIntersectFacesVertexCoordList(&a, &b,
    intersect_face_vertex_coords_list_a, intersect_face_vertex_coords_list_b,
                                    &IntersectPoint, pFace, uModelID);
      else
        BLV_CreateIntersectFacesVertexCoordList(&a, &b,
    intersect_face_vertex_coords_list_a, intersect_face_vertex_coords_list_b,
                                      &IntersectPoint, pFace);
      v5 = 2 * pFace->uNumVertices;
      v16 = 0;
      intersect_face_vertex_coords_list_a[v5] =
    intersect_face_vertex_coords_list_a[0];
      intersect_face_vertex_coords_list_b[v5] =
    intersect_face_vertex_coords_list_b[0]; v6 =
    intersect_face_vertex_coords_list_b[0] >= b; if (v5 <= 0) return false; for
    ( uint i = 0; i < v5; ++i )
      {
        if ( v16 >= 2 )
          break;
        if ( v6 ^ intersect_face_vertex_coords_list_b[i + 1] >= b )
        {
          if ( intersect_face_vertex_coords_list_a[i + 1] >= a )
            v10 = 0;
          else
            v10 = 2;
          v11 = v10 | intersect_face_vertex_coords_list_a[i] < a;
          if ( v11 != 3 )
          {
            if ( !v11
              || (v12 = intersect_face_vertex_coords_list_a[i + 1] -
    intersect_face_vertex_coords_list_a[i], LODWORD(v13) = v12 << 16,
                  HIDWORD(v13) = v12 >> 16,
                  intersect_face_vertex_coords_list_a[i]
                + ((signed int)(((unsigned __int64)(v13 /
    (intersect_face_vertex_coords_list_b[i + 1] -
    intersect_face_vertex_coords_list_b[i])
                                                  * (signed int)((b -
    intersect_face_vertex_coords_list_b[i]) << 16)) >> 16) + 32768) >> 16) >= a)
    )
              ++v16;
          }
        }
        v6 = intersect_face_vertex_coords_list_b[i + 1] >= b;
      }
      result = true;
      if ( v16 != 1 )
        result = false;
      return result;
    }*/
}

//----- (004C1EE5) --------------------------------------------------------
void Vis::BLV_CreateIntersectFacesVertexCoordList(
    int *a, int *b, __int16 *intersect_face_vertex_coords_list_a,
    __int16 *intersect_face_vertex_coords_list_b, Vec3_short_ *IntersectPoint,
    BLVFace *pFace) {
    if (pFace->uAttributes & FACE_XY_PLANE) {
        *a = IntersectPoint->x;
        *b = IntersectPoint->y;

        for (uint i = 0; i < pFace->uNumVertices; ++i) {
            intersect_face_vertex_coords_list_a[2 * i] =
                pFace->pXInterceptDisplacements[i] +
                pIndoor->pVertices[pFace->pVertexIDs[i]].x;
            intersect_face_vertex_coords_list_a[2 * i + 1] =
                pFace->pXInterceptDisplacements[i + 1] +
                pIndoor->pVertices[pFace->pVertexIDs[i + 1]].x;

            intersect_face_vertex_coords_list_b[2 * i] =
                pFace->pYInterceptDisplacements[i] +
                pIndoor->pVertices[pFace->pVertexIDs[i]].y;
            intersect_face_vertex_coords_list_b[2 * i + 1] =
                pFace->pYInterceptDisplacements[i + 1] +
                pIndoor->pVertices[pFace->pVertexIDs[i + 1]].y;
        }
    } else if (pFace->uAttributes & FACE_XZ_PLANE) {
        *a = IntersectPoint->x;
        *b = IntersectPoint->z;

        for (uint i = 0; i < pFace->uNumVertices; ++i) {
            intersect_face_vertex_coords_list_a[2 * i] =
                pFace->pXInterceptDisplacements[i] +
                pIndoor->pVertices[pFace->pVertexIDs[i]].x;
            intersect_face_vertex_coords_list_a[2 * i + 1] =
                pFace->pXInterceptDisplacements[i + 1] +
                pIndoor->pVertices[pFace->pVertexIDs[i + 1]].x;

            intersect_face_vertex_coords_list_b[2 * i] =
                pFace->pZInterceptDisplacements[i] +
                pIndoor->pVertices[pFace->pVertexIDs[i]].z;
            intersect_face_vertex_coords_list_b[2 * i + 1] =
                pFace->pZInterceptDisplacements[i + 1] +
                pIndoor->pVertices[pFace->pVertexIDs[i + 1]].z;
        }
    } else if (pFace->uAttributes & FACE_YZ_PLANE) {
        *a = IntersectPoint->y;
        *b = IntersectPoint->z;

        for (uint i = 0; i < pFace->uNumVertices; ++i) {
            intersect_face_vertex_coords_list_a[2 * i] =
                pFace->pYInterceptDisplacements[i] +
                pIndoor->pVertices[pFace->pVertexIDs[i]].y;
            intersect_face_vertex_coords_list_a[2 * i + 1] =
                pFace->pYInterceptDisplacements[i + 1] +
                pIndoor->pVertices[pFace->pVertexIDs[i + 1]].y;

            intersect_face_vertex_coords_list_b[2 * i] =
                pFace->pZInterceptDisplacements[i] +
                pIndoor->pVertices[pFace->pVertexIDs[i]].z;
            intersect_face_vertex_coords_list_b[2 * i + 1] =
                pFace->pZInterceptDisplacements[i + 1] +
                pIndoor->pVertices[pFace->pVertexIDs[i + 1]].z;
        }
    } else {
        assert(false);
    }
}

//----- (004C2186) --------------------------------------------------------
void Vis::ODM_CreateIntersectFacesVertexCoordList(
    int *a, int *b, int16_t *intersect_face_vertex_coords_list_a,
    __int16 *intersect_face_vertex_coords_list_b, Vec3_short_ *IntersectPoint,
    BLVFace *pFace, unsigned int uModelID) {
    if (pFace->uAttributes & FACE_XY_PLANE) {
        *a = IntersectPoint->x;
        *b = IntersectPoint->y;

        for (int i = 0; i < pFace->uNumVertices; ++i) {
            intersect_face_vertex_coords_list_a[2 * i] =
                pFace->pXInterceptDisplacements[i] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i]]
                    .x;
            intersect_face_vertex_coords_list_a[i * 2 + 1] =
                pFace->pXInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i + 1]]
                    .x;

            intersect_face_vertex_coords_list_b[2 * i] =
                pFace->pYInterceptDisplacements[i] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i]]
                    .y;
            intersect_face_vertex_coords_list_b[i * 2 + 1] =
                pFace->pYInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i + 1]]
                    .y;
        }
    } else if (pFace->uAttributes & FACE_XZ_PLANE) {
        *a = IntersectPoint->x;
        *b = IntersectPoint->z;

        for (int i = 0; i < pFace->uNumVertices; ++i) {
            intersect_face_vertex_coords_list_a[2 * i] =
                pFace->pXInterceptDisplacements[i] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i]]
                    .x;
            intersect_face_vertex_coords_list_a[i * 2 + 1] =
                pFace->pXInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i + 1]]
                    .x;

            intersect_face_vertex_coords_list_b[2 * i] =
                pFace->pZInterceptDisplacements[i] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i]]
                    .z;
            intersect_face_vertex_coords_list_b[i * 2 + 1] =
                pFace->pZInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i + 1]]
                    .z;
        }
    } else if (pFace->uAttributes & FACE_YZ_PLANE) {
        *a = IntersectPoint->y;
        *b = IntersectPoint->z;

        for (int i = 0; i < pFace->uNumVertices; ++i) {
            intersect_face_vertex_coords_list_a[2 * i] =
                pFace->pYInterceptDisplacements[i] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i]]
                    .y;
            intersect_face_vertex_coords_list_a[i * 2 + 1] =
                pFace->pYInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i + 1]]
                    .y;

            intersect_face_vertex_coords_list_b[2 * i] =
                pFace->pZInterceptDisplacements[i] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i]]
                    .z;
            intersect_face_vertex_coords_list_b[i * 2 + 1] =
                pFace->pZInterceptDisplacements[i + 1] +
                pOutdoor->pBModels[uModelID]
                    .pVertices.pVertices[pFace->pVertexIDs[i + 1]]
                    .z;
        }
    } else {
        assert(false);
    }
}

//----- (0046A0A1) --------------------------------------------------------
int UnprojectX(int x) {
    int v3;  // [sp-4h] [bp-8h]@5

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        // if ( render->pRenderD3D )
        v3 = pIndoorCameraD3D->fov;
        // else
        //  v3 = pIndoorCamera->fov_rad;
    } else {
        v3 = pODMRenderParams->int_fov_rad;
    }
    return TrigLUT->Atan2(x - pViewport->uScreenCenterX, v3) -
           TrigLUT->uIntegerHalfPi;
}

//----- (0046A0F6) --------------------------------------------------------
int UnprojectY(int y) {
    int v3;  // [sp-4h] [bp-8h]@5

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        // if ( render->pRenderD3D )
        v3 = pIndoorCameraD3D->fov;
        // else
        //  v3 = pIndoorCamera->fov_rad;
    } else {
        v3 = pODMRenderParams->int_fov_rad;
    }
    return TrigLUT->Atan2(y - pViewport->uScreenCenterY, v3) -
           TrigLUT->uIntegerHalfPi;
}

//----- (004C248E) --------------------------------------------------------
void Vis::CastPickRay(RenderVertexSoft *pRay, float fMouseX, float fMouseY, float fPickDepth) {
    int pRotY;                // esi@1
    Vec3_int_ pStartR;        // ST08_12@1
    int pRotX;                // ST04_4@1
    RenderVertexSoft v11[2];  // [sp+2Ch] [bp-74h]@1
    int outx;
    int outz;  // [sp+94h] [bp-Ch]@1
    int outy;  // [sp+98h] [bp-8h]@1

    pRotY = pIndoorCameraD3D->sRotationZ + UnprojectX(fMouseX);
    pRotX = -pIndoorCameraD3D->sRotationX + UnprojectY(fMouseY);

    // log->Info("Roty: %d, Rotx: %d", pRotY, pRotX);

    pStartR.z = pIndoorCameraD3D->vPartyPos.z;
    pStartR.x = pIndoorCameraD3D->vPartyPos.x;
    pStartR.y = pIndoorCameraD3D->vPartyPos.y;

    v11[1].vWorldPosition.x = (double)pIndoorCameraD3D->vPartyPos.x;
    v11[1].vWorldPosition.y = (double)pIndoorCameraD3D->vPartyPos.y;
    v11[1].vWorldPosition.z = (double)pIndoorCameraD3D->vPartyPos.z;

    int depth = /*fixpoint_from_float*/(fPickDepth);
    Vec3_int_::Rotate(depth, pRotY, pRotX, pStartR, &outx, &outy, &outz);

    v11[0].vWorldPosition.x = (double)outx;
    v11[0].vWorldPosition.y = (double)outy;
    v11[0].vWorldPosition.z = (double)outz;

    memcpy(pRay + 0, &v11[1], sizeof(RenderVertexSoft));
    memcpy(pRay + 1, &v11[0], sizeof(RenderVertexSoft));
}

//----- (004C2551) --------------------------------------------------------
Vis_ObjectInfo *Vis_SelectionList::SelectionPointers(int pVisObjectType,
                                                     int pid) {
    // unsigned int v3; // esi@1
    // signed int v4; // edx@1
    // char *v5; // eax@2
    // Vis_ObjectInfo *result; // eax@6

    // v3 = this->uNumPointers;
    if (this->uNumPointers > 0) {
        for (uint i = 0; i < this->uNumPointers; ++i) {
            if (this->object_pool[i].object_type == pVisObjectType &&
                this->object_pool[i].object_pid == pid)
                return &this->object_pool[i];
        }
    }
    return nullptr;
}

//----- (004C2591) --------------------------------------------------------
void Vis_SelectionList::create_object_pointers(PointerCreationType type) {
    switch (type) {
        case All: {
            for (uint i = 0; i < uNumPointers; ++i)
                object_pointers[i] = &object_pool[i];
        } break;

        case Unique:  // seems quite retarted; the inner if condition will never
                      // trigger, since we compare pointers, not values.
                      // pointers will always be unique
        {             // but it may be decompilation error thou
            bool create = true;

            for (uint i = 0; i < uNumPointers; ++i) {
                for (uint j = 0; j < i; ++j) {
                    if (object_pointers[j] == &object_pool[i]) {
                        create = false;
                        break;
                    }
                }

                if (create) object_pointers[i] = &object_pool[i];
            }
        } break;

        default:
            logger->Warning(
                "Unknown pointer creation flag passed to "
                "::create_object_pointers()");
    }
}

//----- (004C264A) --------------------------------------------------------
void Vis::sort_object_pointers(Vis_ObjectInfo **pPointers, int start,
                               int end) {  // сортировка
    int sort_start;                  // edx@1
    int forward_sort_index;          // esi@2
    signed int backward_sort_index;  // ecx@2
    unsigned int last_z_val;         // eax@3
    unsigned int more_lz_val;        // ebx@4
    unsigned int less_lz_val;        // ebx@6
    Vis_ObjectInfo *temp_pointer;    // eax@7
    //  Vis_ObjectInfo *a3a; // [sp+14h] [bp+Ch]@2

    sort_start = start;

    if (end > start) {
        do {
            forward_sort_index = sort_start - 1;
            backward_sort_index = end;
            do {
                last_z_val = pPointers[end]->depth;
                do {
                    ++forward_sort_index;
                    more_lz_val = pPointers[forward_sort_index]->depth;
                } while (more_lz_val < last_z_val);

                do {
                    if (backward_sort_index < 1) break;
                    --backward_sort_index;
                    less_lz_val = pPointers[backward_sort_index]->depth;
                } while (less_lz_val > last_z_val);

                temp_pointer = pPointers[forward_sort_index];
                if (forward_sort_index >= backward_sort_index) {
                    pPointers[forward_sort_index] = pPointers[end];
                    pPointers[end] = temp_pointer;
                } else {
                    pPointers[forward_sort_index] =
                        pPointers[backward_sort_index];
                    pPointers[backward_sort_index] = temp_pointer;
                }
            } while (forward_sort_index < backward_sort_index);

            sort_object_pointers(pPointers, sort_start, forward_sort_index - 1);
            sort_start = forward_sort_index + 1;
        } while (end > forward_sort_index + 1);
    }
}

//----- (004C26D0) --------------------------------------------------------
void Vis::SortVerticesByX(RenderVertexD3D3 *pArray, unsigned int uStart,
                          unsigned int uEnd) {
    unsigned int left_sort_index;   // ebx@2
    RenderVertexD3D3 temp_array;    // [sp+4h] [bp-4Ch]@8
    RenderVertexD3D3 max_array;     // [sp+24h] [bp-2Ch]@2
    unsigned int right_sort_index;  // [sp+4Ch] [bp-4h]@2

    if ((signed int)uEnd > (signed int)uStart) {
        left_sort_index = uStart - 1;
        right_sort_index = uEnd;
        while (1) {
            memcpy(&max_array, &pArray[uEnd], sizeof(max_array));
            do {
                ++left_sort_index;
            } while (pArray[left_sort_index].pos.x < (double)max_array.pos.x);
            do {
                --right_sort_index;
            } while (pArray[right_sort_index].pos.x > (double)max_array.pos.x);
            if ((signed int)left_sort_index >= (signed int)right_sort_index)
                break;
            memcpy(&temp_array, &pArray[left_sort_index], sizeof(temp_array));
            memcpy(&pArray[left_sort_index], &pArray[right_sort_index],
                   sizeof(pArray[left_sort_index]));
            memcpy(&pArray[right_sort_index], &temp_array,
                   sizeof(pArray[right_sort_index]));
        }
        memcpy(&temp_array, &pArray[left_sort_index], sizeof(temp_array));
        memcpy(&pArray[left_sort_index], &pArray[uEnd],
               sizeof(pArray[left_sort_index]));
        memcpy(&pArray[uEnd], &temp_array, sizeof(pArray[uEnd]));
        SortVerticesByX(pArray, uStart, left_sort_index - 1);
        SortVerticesByX(pArray, left_sort_index + 1, uEnd);
    }
}

//----- (004C27AD) --------------------------------------------------------
void Vis::SortVerticesByY(RenderVertexD3D3 *pArray, unsigned int uStart,
                          unsigned int uEnd) {
    unsigned int left_sort_index;   // ebx@2
    RenderVertexD3D3 temp_array;    // [sp+4h] [bp-4Ch]@8
    RenderVertexD3D3 max_array;     // [sp+24h] [bp-2Ch]@2
    unsigned int right_sort_index;  // [sp+4Ch] [bp-4h]@2

    if ((signed int)uEnd > (signed int)uStart) {
        left_sort_index = uStart - 1;
        right_sort_index = uEnd;
        while (1) {
            memcpy(&max_array, &pArray[uEnd], sizeof(max_array));
            do {
                ++left_sort_index;
            } while (pArray[left_sort_index].pos.y < (double)max_array.pos.y);
            do {
                --right_sort_index;
            } while (pArray[right_sort_index].pos.y > (double)max_array.pos.y);
            if ((signed int)left_sort_index >= (signed int)right_sort_index)
                break;
            memcpy(&temp_array, &pArray[left_sort_index], sizeof(temp_array));
            memcpy(&pArray[left_sort_index], &pArray[right_sort_index],
                   sizeof(pArray[left_sort_index]));
            memcpy(&pArray[right_sort_index], &temp_array,
                   sizeof(pArray[right_sort_index]));
        }
        memcpy(&temp_array, &pArray[left_sort_index], sizeof(temp_array));
        memcpy(&pArray[left_sort_index], &pArray[uEnd],
               sizeof(pArray[left_sort_index]));
        memcpy(&pArray[uEnd], &temp_array, sizeof(pArray[uEnd]));
        SortVerticesByY(pArray, uStart, left_sort_index - 1);
        SortVerticesByY(pArray, left_sort_index + 1, uEnd);
    }
}

//----- (004C288E) --------------------------------------------------------
void Vis::SortByScreenSpaceX(
    RenderVertexSoft *pArray, int start,
    int end) {  // сортировка по возрастанию экранных координат х
    int left_sort_index;          // ebx@2
    int right_sort_index;         // ecx@2
    RenderVertexSoft temp_array;  // [sp+4h] [bp-6Ch]@8
    RenderVertexSoft max_array;   // [sp+34h] [bp-3Ch]@2

    if (end > start) {
        left_sort_index = start - 1;
        right_sort_index = end;
        memcpy(&max_array, &pArray[end], sizeof(max_array));
        while (1) {
            do {
                ++left_sort_index;
            } while (pArray[left_sort_index].vWorldViewProjX <
                     (double)max_array.vWorldViewProjX);
            do {
                --right_sort_index;
            } while (pArray[right_sort_index].vWorldViewProjX >
                     (double)max_array.vWorldViewProjX);
            if (left_sort_index >= right_sort_index) break;
            memcpy(&temp_array, &pArray[left_sort_index], sizeof(temp_array));
            memcpy(&pArray[left_sort_index], &pArray[right_sort_index],
                   sizeof(pArray[left_sort_index]));
            memcpy(&pArray[right_sort_index], &temp_array,
                   sizeof(pArray[right_sort_index]));
        }
        memcpy(&temp_array, &pArray[left_sort_index], sizeof(temp_array));
        memcpy(&pArray[left_sort_index], &pArray[end],
               sizeof(pArray[left_sort_index]));
        memcpy(&pArray[end], &temp_array, sizeof(pArray[end]));
        Vis::SortByScreenSpaceX(pArray, start, left_sort_index - 1);
        Vis::SortByScreenSpaceX(pArray, left_sort_index + 1, end);
    }
}

//----- (004C297E) --------------------------------------------------------
void Vis::SortByScreenSpaceY(RenderVertexSoft *pArray, int start, int end) {
    int left_sort_index;          // ebx@2
    int right_sort_index;         // ecx@2
    RenderVertexSoft temp_array;  // [sp+4h] [bp-6Ch]@8
    RenderVertexSoft max_array;   // [sp+34h] [bp-3Ch]@2

    if (end > start) {
        left_sort_index = start - 1;
        right_sort_index = end;
        memcpy(&max_array, &pArray[end], sizeof(max_array));
        while (1) {
            do {
                ++left_sort_index;
            } while (pArray[left_sort_index].vWorldViewProjY <
                     (double)max_array.vWorldViewProjY);
            do {
                --right_sort_index;
            } while (pArray[right_sort_index].vWorldViewProjY >
                     (double)max_array.vWorldViewProjY);
            if (left_sort_index >= right_sort_index) break;
            memcpy(&temp_array, &pArray[left_sort_index], sizeof(temp_array));
            memcpy(&pArray[left_sort_index], &pArray[right_sort_index],
                   sizeof(pArray[left_sort_index]));
            memcpy(&pArray[right_sort_index], &temp_array,
                   sizeof(pArray[right_sort_index]));
        }
        memcpy(&temp_array, &pArray[left_sort_index], sizeof(temp_array));
        memcpy(&pArray[left_sort_index], &pArray[end],
               sizeof(pArray[left_sort_index]));
        memcpy(&pArray[end], &temp_array, sizeof(pArray[end]));
        Vis::SortByScreenSpaceY(pArray, start, left_sort_index - 1);
        Vis::SortByScreenSpaceY(pArray, left_sort_index + 1, end);
    }
}

//----- (004C04AF) --------------------------------------------------------
Vis::Vis() {
    this->log = EngineIoc::ResolveLogger();


    RenderVertexSoft v3;  // [sp+Ch] [bp-60h]@1
    RenderVertexSoft v4;  // [sp+3Ch] [bp-30h]@1

    v3.flt_2C = 0.0;
    v3.vWorldPosition.x = 0.0;
    v3.vWorldPosition.y = 65536.0;
    v3.vWorldPosition.z = 0.0;
    v4.flt_2C = 0.0;
    v4.vWorldPosition.x = 65536.0;
    v4.vWorldPosition.y = 0.0;
    v4.vWorldPosition.z = 0.0;
    memcpy(&stru_200C, &v4, sizeof(stru_200C));

    v4.flt_2C = 0.0;
    v4.vWorldPosition.x = 0.0;
    v4.vWorldPosition.y = 65536.0;
    v4.vWorldPosition.z = 0.0;
    memcpy(&stru_203C, &v3, sizeof(stru_203C));

    v3.flt_2C = 0.0;
    v3.vWorldPosition.x = 65536.0;
    v3.vWorldPosition.y = 0.0;
    v3.vWorldPosition.z = 0.0;
    memcpy(&stru_206C, &v3, sizeof(stru_206C));
    memcpy(&stru_209C, &v4, sizeof(stru_209C));

    keyboard_pick_depth = 512;
}

//----- (004C055C) --------------------------------------------------------
Vis_SelectionList::Vis_SelectionList() {
    for (uint i = 0; i < 512; ++i) {
        object_pool[i].object = nullptr;
        object_pool[i].depth = -1;
        object_pool[i].object_pid = PID_INVALID;
        object_pool[i].object_type = VisObjectType_Any;
    }
    uNumPointers = 0;
}

//----- (004C05CC) --------------------------------------------------------
bool Vis::PickKeyboard(Vis_SelectionList *list,
                       Vis_SelectionFilter *sprite_filter,
                       Vis_SelectionFilter *face_filter) {
    if (!list) list = &default_list;
    list->uNumPointers = 0;

    PickBillboards_Keyboard(keyboard_pick_depth, list, sprite_filter);
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        PickIndoorFaces_Keyboard(keyboard_pick_depth, list, face_filter);
    else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        PickOutdoorFaces_Keyboard(keyboard_pick_depth, list, face_filter);
    else
        assert(false);

    list->create_object_pointers(Vis_SelectionList::Unique);
    sort_object_pointers(list->object_pointers, 0, list->uNumPointers - 1);

    return true;
}

//----- (004C0646) --------------------------------------------------------
bool Vis::PickMouse(float fDepth, float fMouseX, float fMouseY,
                    Vis_SelectionFilter *sprite_filter,
                    Vis_SelectionFilter *face_filter) {
    RenderVertexSoft pMouseRay[2];  // [sp+1Ch] [bp-60h]@1

    default_list.uNumPointers = 0;
    CastPickRay(pMouseRay, fMouseX, fMouseY, fDepth);

    // log->Info("Sx: %f, Sy: %f, Sz: %f \n Fx: %f, Fy: %f, Fz: %f", pMouseRay->vWorldPosition.x, pMouseRay->vWorldPosition.y, pMouseRay->vWorldPosition.z,
    //     (pMouseRay+1)->vWorldPosition.x, (pMouseRay + 1)->vWorldPosition.y, (pMouseRay + 1)->vWorldPosition.z);

    PickBillboards_Mouse(fDepth, fMouseX, fMouseY, &default_list, sprite_filter);

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        PickIndoorFaces_Mouse(fDepth, pMouseRay, &default_list, face_filter);
    } else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        PickOutdoorFaces_Mouse(fDepth, pMouseRay, &default_list, face_filter,
            false);
    } else {
        log->Warning(
            "Picking mouse in undefined level");  // picking in main menu is
                                                  // default (buggy) game
                                                  // behaviour. should've
                                                  // returned false in
                                                  // Game::PickMouse
        return false;
    }
    default_list.create_object_pointers(Vis_SelectionList::All);
    sort_object_pointers(default_list.object_pointers, 0,
                         default_list.uNumPointers - 1);

    return true;
}

//----- (004C06F8) --------------------------------------------------------
void Vis::PickBillboards_Keyboard(float pick_depth, Vis_SelectionList *list,
                                  Vis_SelectionFilter *filter) {
    for (uint i = 0; i < render->uNumBillboardsToDraw; ++i) {
        RenderBillboardD3D *d3d_billboard = &render->pBillboardRenderListD3D[i];

        if (is_part_of_selection((void *)i, filter)) {
            if (DoesRayIntersectBillboard(pick_depth, i)) {
                RenderBillboard *billboard =
                    &pBillboardRenderList[d3d_billboard->sParentBillboardID];

                list->AddObject((void *)d3d_billboard->sParentBillboardID,
                                VisObjectType_Sprite, billboard->screen_space_z,
                                billboard->object_pid);
            }
        }
    }
}

// tests the object against selection filter to determine whether it can be
// picked or not
//----- (004C0791) --------------------------------------------------------
bool Vis::is_part_of_selection(void *uD3DBillboardIdx_or_pBLVFace_or_pODMFace,
                               Vis_SelectionFilter *filter) {
    switch (filter->vis_object_type) {
        case VisObjectType_Any:
            return true;

        case VisObjectType_Sprite: {
            // v5 = filter->select_flags;
            int object_idx = PID_ID(
                pBillboardRenderList
                    [render
                         ->pBillboardRenderListD3D[(
                             int64_t)uD3DBillboardIdx_or_pBLVFace_or_pODMFace]
                         .sParentBillboardID]
                        .object_pid);
            int object_type = PID_TYPE(
                pBillboardRenderList
                    [render
                         ->pBillboardRenderListD3D[(
                             int64_t)uD3DBillboardIdx_or_pBLVFace_or_pODMFace]
                         .sParentBillboardID]
                        .object_pid);
            if (filter->select_flags & ExcludeType) {
                return object_type != filter->object_type;
            }
            if (filter->select_flags & ExclusionIfNoEvent) {
                if (object_type != filter->object_type) return true;
                if (filter->object_type != OBJECT_Decoration) {
                    log->Warning(
                        "Unsupported \"exclusion if no event\" type in "
                        "CVis::is_part_of_selection");
                    return true;
                }
                if (pLevelDecorations[object_idx].uCog ||
                    pLevelDecorations[object_idx].uEventID)
                    return true;
                return pLevelDecorations[object_idx].IsInteractive();
            }
            if (object_type == filter->object_type) {
                if (object_type != OBJECT_Actor) {
                    log->Warning("Default case reached in VIS");
                    return true;
                }

                // v10 = &pActors[object_idx];
                int aiState = 1 << HEXRAYS_LOBYTE(pActors[object_idx].uAIState);

                if (aiState & filter->no_at_ai_state)
                    return false;
                if (!(aiState & filter->at_ai_state))
                    return false;

                auto only_target_undead = filter->select_flags & TargetUndead;
                auto target_not_undead = MonsterStats::BelongsToSupertype(pActors[object_idx].pMonsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD) == 0;

                if (only_target_undead && target_not_undead)
                    return false;

                if (!(filter->select_flags & VisSelectFlags_1))
                    return true;

                auto relation = pActors[object_idx].GetActorsRelation(nullptr);
                if (relation == 0) return false;
                return true;
            }
            return false;
        }

        case VisObjectType_Face: {
            uint face_attrib = 0;
            bool no_event = true;
            if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                ODMFace *face = (ODMFace *)uD3DBillboardIdx_or_pBLVFace_or_pODMFace;
                no_event = face->sCogTriggeredID == 0;
                face_attrib = face->uAttributes;
            } else if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                BLVFace *face = (BLVFace *)uD3DBillboardIdx_or_pBLVFace_or_pODMFace;
                no_event = pIndoor->pFaceExtras[face->uFaceExtraID].uEventID == 0;
                face_attrib = face->uAttributes;
            } else {
                assert(false);
            }

            if (filter->object_type != OBJECT_BLVDoor) return true;

            auto invalid_face_attrib = face_attrib & filter->no_at_ai_state;
            if (no_event || invalid_face_attrib)  // face_attrib = 0x2009408 incorrect
                return false;
            return (face_attrib & filter->at_ai_state) != 0;
        }

        default:
            assert(false);
            return false;
    }
}

//----- (004C091D) --------------------------------------------------------
bool Vis::DoesRayIntersectBillboard(float fDepth,
                                    unsigned int uD3DBillboardIdx) {
    int v3;  // eax@3
    // signed int v5; // ecx@4
    // float v6; // ST04_4@6
    // float v7; // ST00_4@7
    // int v8; // eax@10
    // unsigned int v9; // eax@12
    //  int v10; // eax@17
    //  double v11; // st6@18
    //  double v12; // st7@18
    //  double v13; // st4@18
    //  float v14; // ST0C_4@22
    //  float v15; // ST08_4@22
    // float v16; // ST04_4@23
    // float v17; // ST00_4@24
    // signed int v18; // eax@27
    // unsigned int v19; // eax@29
    //  double v20; // st6@32
    //  double v21; // st7@32
    //  int v22; // eax@32
    //  double v23; // st7@36
    // void *v24; // esi@40
    //  float v25; // ST08_4@40
    // float v26; // ST04_4@41
    // float v27; // ST00_4@42
    //  int v28; // eax@45
    //  unsigned int v29; // eax@47
    //  char result; // al@48
    struct RenderVertexSoft pPickingRay[2];
    // int v31; // [sp+20h] [bp-DCh]@5
    struct RenderVertexSoft local_80[2];

    float test_x;
    float test_y;

    float t1_x;
    float t1_y;
    float t2_x;
    float t2_y;
    float swap_temp;
    //  int v37; // [sp+F0h] [bp-Ch]@5

    signed int v40;  // [sp+108h] [bp+Ch]@17

    /*static*/ Vis_SelectionList Vis_static_stru_F91E10;
    Vis_static_stru_F91E10.uNumPointers = 0;
    v3 = render->pBillboardRenderListD3D[uD3DBillboardIdx].sParentBillboardID;
    if (v3 == -1) return false;

    if (pBillboardRenderList[v3].screen_space_z > fDepth) return false;

    GetPolygonCenter(render->pBillboardRenderListD3D[/*v3*/uD3DBillboardIdx].pQuads, 4, &test_x, &test_y);
    // why check parent id v3? parent ID are wrong becasue of switching between pBillboardRenderListD3D and pBillboardRenderList

    CastPickRay(pPickingRay, test_x, test_y, fDepth);
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        PickIndoorFaces_Mouse(fDepth, pPickingRay, &Vis_static_stru_F91E10,
                              &vis_face_filter);
    else
        PickOutdoorFaces_Mouse(fDepth, pPickingRay, &Vis_static_stru_F91E10,
                               &vis_face_filter, false);
    Vis_static_stru_F91E10.create_object_pointers();
    sort_object_pointers(Vis_static_stru_F91E10.object_pointers, 0,
                         Vis_static_stru_F91E10.uNumPointers - 1);
    if (Vis_static_stru_F91E10.uNumPointers) {
        if (Vis_static_stru_F91E10.object_pointers[0]->depth >
            pBillboardRenderList[v3].screen_space_z)
            return true;
    } else if ((double)(pViewport->uScreen_TL_X) <= test_x &&
        (double)pViewport->uScreen_BR_X >= test_x &&
        (double)pViewport->uScreen_TL_Y <= test_y &&
        (double)pViewport->uScreen_BR_Y >= test_y) {
        return true;
    }

    for (v40 = 0; v40 < 4; ++v40) {
        test_x =
            render->pBillboardRenderListD3D[uD3DBillboardIdx].pQuads[v40].pos.x;
        test_y =
            render->pBillboardRenderListD3D[uD3DBillboardIdx].pQuads[v40].pos.y;
        if ((double)(pViewport->uScreen_TL_X) <= test_x &&
            (double)pViewport->uScreen_BR_X >= test_x &&
            (double)pViewport->uScreen_TL_Y <= test_y &&
            (double)pViewport->uScreen_BR_Y >= test_y) {
            CastPickRay(local_80, test_x, test_y, fDepth);
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
                PickIndoorFaces_Mouse(fDepth, local_80, &Vis_static_stru_F91E10,
                                      &vis_face_filter);
            else
                PickOutdoorFaces_Mouse(fDepth, local_80,
                                       &Vis_static_stru_F91E10,
                                       &vis_face_filter, false);
            Vis_static_stru_F91E10.create_object_pointers();
            sort_object_pointers(Vis_static_stru_F91E10.object_pointers, 0,
                                 Vis_static_stru_F91E10.uNumPointers - 1);
            if (!Vis_static_stru_F91E10.uNumPointers) return true;
            if (Vis_static_stru_F91E10.object_pointers[0]->depth >
                pBillboardRenderList[v3].screen_space_z)
                return true;
        }
    }

    if (v40 >= 4) {
        // if (uCurrentlyLoadedLevelType != LEVEL_Outdoor) return false;
        t1_x =
            render->pBillboardRenderListD3D[uD3DBillboardIdx].pQuads[0].pos.x;
        t2_x =
            render->pBillboardRenderListD3D[uD3DBillboardIdx].pQuads[3].pos.x;

        t1_y =
            render->pBillboardRenderListD3D[uD3DBillboardIdx].pQuads[0].pos.y;
        t2_y =
            render->pBillboardRenderListD3D[uD3DBillboardIdx].pQuads[1].pos.y;
        if (t1_x > t2_x) {
            swap_temp = t1_x;
            t1_x = t2_x;
            t2_x = swap_temp;
        }
        if (t1_y > t2_y)
            test_y = t1_y;
        else
            test_y = t2_y;

        Vis_static_stru_F91E10.uNumPointers = 0;

        test_x = (t2_x - t1_x) * 0.5;
        if ((double)(pViewport->uScreen_TL_X) <= test_x &&
            (double)pViewport->uScreen_BR_X >= test_x &&
            (double)pViewport->uScreen_TL_Y <= test_y &&
            (double)pViewport->uScreen_BR_Y >= test_y) {
            CastPickRay(local_80, test_x, test_y, fDepth);
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
                PickIndoorFaces_Mouse(fDepth, local_80, &Vis_static_stru_F91E10,
                                      &vis_face_filter);
            else
                PickOutdoorFaces_Mouse(fDepth, local_80,
                                       &Vis_static_stru_F91E10,
                                       &vis_face_filter, false);
            Vis_static_stru_F91E10.create_object_pointers();
            sort_object_pointers(Vis_static_stru_F91E10.object_pointers, 0,
                                 Vis_static_stru_F91E10.uNumPointers - 1);
            if (!Vis_static_stru_F91E10.uNumPointers) return true;
            if (Vis_static_stru_F91E10.object_pointers[0]->depth >
                pBillboardRenderList[v3].screen_space_z)
                return true;
        }
    }
    return false;
}
// F93E18: using guessed type char static_byte_F93E18_init;

//----- (004C0D32) --------------------------------------------------------
void Vis::PickIndoorFaces_Keyboard(float pick_depth, Vis_SelectionList *list,
                                   Vis_SelectionFilter *filter) {
    int result;          // eax@1
    signed int pFaceID;  // esi@2
    BLVFace *pFace;      // edi@4
    // unsigned int v7; // eax@6
    Vis_ObjectInfo *v8;  // eax@6
    signed int i;        // [sp+18h] [bp-8h]@1

    result = 0;
    for (i = 0; i < (signed int)pBspRenderer->num_faces; ++i) {
        pFaceID = pBspRenderer->faces[result].uFaceID;
        if (pFaceID >= 0) {
            if (pFaceID < (signed int)pIndoor->uNumFaces) {
                pFace = &pIndoor->pFaces[pFaceID];
                if (!pIndoorCameraD3D->IsCulled(&pIndoor->pFaces[pFaceID])) {
                    if (is_part_of_selection(pFace, filter)) {
                        v8 = DetermineFacetIntersection(
                            pFace, PID(OBJECT_BModel, pFaceID), pick_depth);
                        if (v8)
                            list->AddObject(v8->object, v8->object_type,
                                            v8->depth, v8->object_pid);
                    }
                }
            }
        }
        result = i + 1;
    }
}

void Vis::PickOutdoorFaces_Keyboard(float pick_depth, Vis_SelectionList *list,
                                    Vis_SelectionFilter *filter) {
    for (BSPModel &model : pOutdoor->pBModels) {
        int reachable;
        if (IsBModelVisible(&model, &reachable)) {
            if (reachable) {
                for (ODMFace &face : model.pFaces) {
                    if (is_part_of_selection(&face, filter)) {
                        BLVFace blv_face;
                        blv_face.FromODM(&face);

                        int pid =
                            PID(OBJECT_BModel, face.index | (model.index << 6));
                        if (Vis_ObjectInfo *object_info =
                                DetermineFacetIntersection(&blv_face, pid,
                                                           pick_depth)) {
                            list->AddObject(
                                object_info->object, object_info->object_type,
                                object_info->depth, object_info->object_pid);
                        }
                    }
                }
            }
        }
    }
}
