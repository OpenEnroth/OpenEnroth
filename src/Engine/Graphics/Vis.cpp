#include "Engine/Graphics/Vis.h"

#include <cstdlib>
#include <algorithm>
#include <vector>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/OurMath.h"

#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/BspRenderer.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Objects/Actor.h"

#include "Library/Logger/Logger.h"

#include "Utility/Math/TrigLut.h"

static Vis_SelectionList Vis_static_sub_4C1944_stru_F8BDE8;

Vis_SelectionFilter vis_sprite_targets_filter = {
    VisObjectType_Sprite, OBJECT_Decoration, 0, 0, ExcludeType};  // 00F93E1C
Vis_SelectionFilter vis_allsprites_filter = {
    VisObjectType_Sprite, OBJECT_None, 0, 0, ExcludeType};  // 00F93E30
Vis_SelectionFilter vis_face_filter = {
    VisObjectType_Face, OBJECT_None, -1, 0, None};  // 00F93E44
Vis_SelectionFilter vis_door_filter = {
    VisObjectType_Face, OBJECT_Door, -1, static_cast<int>(FACE_HAS_EVENT), None };  // 00F93E58
Vis_SelectionFilter vis_decoration_noevent_filter = {
    VisObjectType_Sprite, OBJECT_Decoration, -1, 0, ExclusionIfNoEvent};  // 00F93E6C
Vis_SelectionFilter vis_items_filter = {
    VisObjectType_Any, OBJECT_Item, -1, 0, None };  // static to sub_44EEA7

//----- (004C1026) --------------------------------------------------------
Vis_ObjectInfo *Vis::DetermineFacetIntersection(BLVFace *face, Pid pid, float pick_depth) {
    //  char *v4; // eax@4
    //  signed int v5; // ecx@4
    Vec3f rayOrigin, rayStep;  // [sp+20h] [bp-70h]@17
                               //  int v20; // [sp+84h] [bp-Ch]@10

    static Vis_SelectionList SelectedPointersList;  // stru_F8FE00
    SelectedPointersList.uSize = 0;

    static bool _init_flag = false;
    static RenderVertexSoft static_DetermineFacetIntersection_array_F8F200[64];
    if (!_init_flag) {
        _init_flag = true;
        for (uint i = 0; i < 64; ++i)
            static_DetermineFacetIntersection_array_F8F200[i].flt_2C = 0.0f;
    }

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
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
    } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        const std::vector<Vec3i> &v = pOutdoor->model(pid).pVertices;
        for (uint i = 0; i < face->uNumVertices; ++i)
            static_DetermineFacetIntersection_array_F8F200[i].vWorldPosition = v[face->pVertexIDs[i]].toFloat();
    } else {
        assert(false);
    }

    pCamera3D->ViewTransform(
        static_DetermineFacetIntersection_array_F8F200, face->uNumVertices);
    pCamera3D->Project(static_DetermineFacetIntersection_array_F8F200,
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

    CastPickRay(screenspace_center_x, screenspace_center_y, pick_depth, &rayOrigin, &rayStep);

    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
        PickOutdoorFaces_Mouse(pick_depth, rayOrigin, rayStep, &SelectedPointersList,
                               &vis_face_filter, true);
    else if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        PickIndoorFaces_Mouse(pick_depth, rayOrigin, rayStep, &SelectedPointersList,
                              &vis_face_filter);
    else
        assert(false);

    SelectedPointersList.create_object_pointers();
    SelectedPointersList.sort_object_pointers();
    if (!SelectedPointersList.uSize) return nullptr;

    if (!SelectedPointersList.SelectionPointers(VisObjectType_Face, pid))
        return nullptr;

    if (SelectedPointersList.uSize)
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
            else if (pBillboardRenderList[billboard->sParentBillboardID].screen_space_z <
                     pBillboardRenderList[render->pBillboardRenderListD3D[v13].sParentBillboardID].screen_space_z)
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
    std::array<RenderVertexD3D3, 64> unk_F8EA00;

    assert(uNumVertices <= unk_F8EA00.size());
    std::copy(pVertices, pVertices + uNumVertices, unk_F8EA00.begin());

    SortVerticesByX(unk_F8EA00.data(), 0, uNumVertices - 1);
    *pCenterX =
        (unk_F8EA00[uNumVertices - 1].pos.x - unk_F8EA00[0].pos.x) * 0.5 +
        unk_F8EA00[0].pos.x;

    SortVerticesByY(unk_F8EA00.data(), 0, uNumVertices - 1);
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

    std::array<RenderVertexSoft, 64> array_F8DDF8;
    assert(num_vertices <= array_F8DDF8.size());
    std::copy(vertices, vertices + num_vertices, array_F8DDF8.begin());

    SortByScreenSpaceX(array_F8DDF8.data(), 0, num_vertices - 1);
    *out_center_x =
        (array_F8DDF8[num_vertices - 1].vWorldViewProjX -
         array_F8DDF8[0].vWorldViewProjX) *
        0.5 +
        array_F8DDF8[0].vWorldViewProjX;

    SortByScreenSpaceY(array_F8DDF8.data(), 0, num_vertices - 1);
    *out_center_y =
        (array_F8DDF8[num_vertices - 1].vWorldViewProjY -
         array_F8DDF8[0].vWorldViewProjY) *
        0.5 +
        array_F8DDF8[0].vWorldViewProjY;
}

//----- (004C1542) --------------------------------------------------------
void Vis::PickBillboards_Mouse(float fPickDepth, float fX, float fY,
                               Vis_SelectionList *list,
                               Vis_SelectionFilter *filter) {
    for (int i = 0; i < render->uNumBillboardsToDraw; ++i) {
        RenderBillboardD3D *d3d_billboard = &render->pBillboardRenderListD3D[i];
        if (is_part_of_selection(i, filter) && IsPointInsideD3DBillboard(d3d_billboard, fX, fY)) {
            if (DoesRayIntersectBillboard(fPickDepth, i)) {
                RenderBillboard *billboard = &pBillboardRenderList[d3d_billboard->sParentBillboardID];

                list->AddObject(d3d_billboard->sParentBillboardID,
                                VisObjectType_Sprite, billboard->screen_space_z,
                                billboard->object_pid);
            }
        }
    }
}

//----- (004C1607) --------------------------------------------------------
bool Vis::IsPointInsideD3DBillboard(RenderBillboardD3D *billboard, float x, float y) {
    /*Not the original implementation.
    This function is redone to use GrayFace's mouse pick implementation to take
    only the visible
    parts of billboards into account - I don't really have too much of an idea
    how it actually works*/

    if (billboard->sParentBillboardID == -1) return false;

    float drX = billboard->pQuads[0].pos.x;
    float drW = billboard->pQuads[3].pos.x - drX;
    float drY = billboard->pQuads[0].pos.y;
    float drH = billboard->pQuads[1].pos.y - drY;

    // simple bounds check
    if (x > drX && x > billboard->pQuads[3].pos.x) return false;
    if (x < drX && x < billboard->pQuads[3].pos.x) return false;
    if (y > drY && y > billboard->pQuads[1].pos.y) return false;
    if (y < drY && y < billboard->pQuads[1].pos.y) return false;

    // for small items dont bother with the per pixel checks
    if (std::abs(drH) < 5 || std::abs(drW) < 5) {
        return true;
    }

    RgbaImageView rgba = billboard->texture->rgba();

    int sx = rgba.width() * (x - drX) / drW;
    int sy = rgba.height() * (y - drY) / drH;

    if (sx < 0 || sx >= rgba.width()) return false;
    if (sy < 0 || sy >= rgba.height()) return false;

    return rgba[sy][sx] != Color();
}

//----- (004C16B4) --------------------------------------------------------
void Vis::PickIndoorFaces_Mouse(float fDepth, const Vec3f &rayOrigin, const Vec3f &rayStep,
                                Vis_SelectionList *list,
                                Vis_SelectionFilter *filter) {
    RenderVertexSoft a1;

    for (int faceindex = 0; faceindex < (int)pIndoor->pFaces.size(); ++faceindex) {
        BLVFace *face = &pIndoor->pFaces[faceindex];
        if (is_part_of_selection(face, filter)) {
            if (pCamera3D->is_face_faced_to_cameraBLV(face)) {
                if (Intersect_Ray_Face(rayOrigin, rayStep, &a1, face, 0xFFFFFFFFu)) {
                    pCamera3D->ViewTransform(&a1, 1);
                    list->AddObject(&pIndoor->pFaces[faceindex], VisObjectType_Face, a1.vWorldViewPosition.x, Pid(OBJECT_Face, faceindex));
                }
            }
        }

        if (face->uAttributes & FACE_IsPicked)
            face->uAttributes |= FACE_OUTLINED;
        else
            face->uAttributes &= ~FACE_OUTLINED;
        face->uAttributes &= ~FACE_IsPicked;
    }
}

bool IsBModelVisible(BSPModel *model, int reachable_depth, bool *reachable) {
    // approx distance - for reachable checks
    float rayx = model->vBoundingCenter.x - pCamera3D->vCameraPos.x;
    float rayy = model->vBoundingCenter.y - pCamera3D->vCameraPos.y;
    int dist = int_get_vector_length(std::abs(static_cast<int>(rayx)), std::abs(static_cast<int>(rayy)), 0);
    *reachable = false;
    if (dist < model->sBoundingRadius + reachable_depth) *reachable = true;

    // to avoid small objects not showing up give a more generous radius
    float radius{ static_cast<float>(model->sBoundingRadius) };
    if (radius < 512.0f) radius = 512.0f;

    return IsSphereInFrustum(model->vBoundingCenter.toFloat(), radius);
}

bool IsSphereInFrustum(Vec3f center, float radius, Planef *frustum) {
    // center must be within all four of the camera frustum planes to be visible
    Vec3f planenormal;
    float planedist = 0;
    for (int i = 0; i < 4; i++) {
        if (frustum) {
            planenormal = frustum[i].normal;
            planedist = -frustum[i].dist;
        } else {  // use camera frustum
            planenormal.x = pCamera3D->FrustumPlanes[i].x;
            planenormal.y = pCamera3D->FrustumPlanes[i].y;
            planenormal.z = pCamera3D->FrustumPlanes[i].z;
            planedist = pCamera3D->FrustumPlanes[i].w;
        }

        if ((dot(center, planenormal) - planedist) < -radius) {
            return false;
        }
    }

    return true;
}

bool IsCylinderInFrustum(Vec3f center, float radius) {
    // center must be within left / right frustum planes to be visible
    for (int i = 0; i < 2; i++) {
        Vec3f planenormal{ pCamera3D->FrustumPlanes[i].x, pCamera3D->FrustumPlanes[i].y, pCamera3D->FrustumPlanes[i].z };
        float planedist{ pCamera3D->FrustumPlanes[i].w };
        if ((dot(center, planenormal) - planedist) < -radius) {
            return false;
        }
    }
    return true;
}

void Vis::PickOutdoorFaces_Mouse(float fDepth, const Vec3f &rayOrigin, const Vec3f &rayStep,
                                 Vis_SelectionList *list,
                                 Vis_SelectionFilter *filter,
                                 bool only_reachable) {
    if (!pOutdoor) return;

    for (BSPModel &model : pOutdoor->pBModels) {
        bool reachable;
        if (!IsBModelVisible(&model, fDepth, &reachable)) {
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
                if (Intersect_Ray_Face(rayOrigin, rayStep, &intersection,
                                       &blv_face, model.index)) {
                    pCamera3D->ViewTransform(&intersection, 1);
                    // int v13 = fixpoint_from_float(/*v12,
                    // */intersection.vWorldViewPosition.x); v13 &= 0xFFFF0000;
                    // v13 += Pid(OBJECT_Face, j | (i << 6));
                    Pid pid =
                        Pid(OBJECT_Face, face.index | (model.index << 6));
                    list->AddObject(&face, VisObjectType_Face,
                                    intersection.vWorldViewPosition.x, pid);
                }

                if (blv_face.uAttributes & FACE_IsPicked)
                    face.uAttributes |= FACE_OUTLINED;
                else
                    face.uAttributes &= ~FACE_OUTLINED;
                blv_face.uAttributes &= ~FACE_IsPicked;
            }
        }
    }
}

//----- (004C1944) --------------------------------------------------------
Pid Vis::PickClosestActor(ObjectType object_type, unsigned int pick_depth,
                                     VisSelectFlags select_flags, int not_at_ai_state, int at_ai_state) {
    Vis_SelectionFilter selectionFilter;  // [sp+18h] [bp-20h]@3

    static Vis_SelectionList Vis_static_sub_4C1944_stru_F8BDE8;

    selectionFilter.vis_object_type = VisObjectType_Sprite;
    selectionFilter.object_type = object_type;
    selectionFilter.at_ai_state = at_ai_state;
    selectionFilter.no_at_ai_state = not_at_ai_state;
    selectionFilter.select_flags = select_flags;
    Vis_static_sub_4C1944_stru_F8BDE8.uSize = 0;
    PickBillboards_Keyboard(pick_depth, &Vis_static_sub_4C1944_stru_F8BDE8,
                            &selectionFilter);
    Vis_static_sub_4C1944_stru_F8BDE8.create_object_pointers(Vis_SelectionList::Unique);
    Vis_static_sub_4C1944_stru_F8BDE8.sort_object_pointers();

    if (!Vis_static_sub_4C1944_stru_F8BDE8.uSize) return Pid();
    return Vis_static_sub_4C1944_stru_F8BDE8.object_pointers[0]->object_pid;
}

// depth sort
//----- (004C1ABA) --------------------------------------------------------
void Vis::SortVectors_x(RenderVertexSoft *pArray, int start, int end) {
    auto cmp = [](const RenderVertexSoft &l, const RenderVertexSoft &r) {
        return l.vWorldViewPosition.x < r.vWorldViewPosition.x;
    };
    std::stable_sort(pArray + start, pArray + end + 1, cmp);
}

Vis_PIDAndDepth InvalidPIDAndDepth() {
    Vis_PIDAndDepth result;
    result.depth = 0;
    result.object_pid = Pid();
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
            log->warning("Undefined type requested for: CVis::get_object_zbuf_val()");
            return InvalidPIDAndDepth();
    }
}

//----- (004C1BF1) --------------------------------------------------------
Vis_PIDAndDepth Vis::get_picked_object_zbuf_val() {
    if (!default_list.uSize) return InvalidPIDAndDepth();

    return get_object_zbuf_val(default_list.object_pointers[0]);
}

//----- (004C1C0C) --------------------------------------------------------
bool Vis::Intersect_Ray_Face(const Vec3f &origin, const Vec3f &step,
                             RenderVertexSoft *Intersection, BLVFace *pFace,
                             signed int pBModelID) {
    if (pFace->isPortal() || pFace->Invisible())
        return false;

    // c1 = -d-(n*p0)
    float c1 = -pFace->facePlane.signedDistanceTo(origin);
    if (c1 > 0)
        return false;

    // c2 = n*u
    float c2 = dot(pFace->facePlane.normal, step);
    if (fuzzyIsNull(c2))
        return false; // Ray is parallel to face plane.

    // t = -d-(n*p0)/n*u
    float t = c1 / c2;  // How far is crossing the line in percent for 0 to 1
    if (t < 0 || t > 1)
        return false;

    // p(t) = p0 + tu;
    Intersection->vWorldPosition = origin + t * step;

    if (!CheckIntersectBModel(pFace, Intersection->vWorldPosition.toInt(), pBModelID))
        return false;

    return true;
}

//----- (004C1D2B) --------------------------------------------------------
bool Vis::CheckIntersectBModel(BLVFace *pFace, Vec3i IntersectPoint, signed int sModelID) {
    if (!pFace->pBounding.contains(IntersectPoint))
        return false;

    // sModelID == -1 means we're indoor, and -1 == MODEL_INDOOR, so this call just works.
    if (!pFace->Contains(IntersectPoint, sModelID))
        return false;

    if (engine->config->debug.ShowPickedFace.value()) {
        pFace->uAttributes |= FACE_IsPicked;

        // save debug pick line for later
        debugpick.vWorldPosition.x = IntersectPoint.x;
        debugpick.vWorldPosition.y = IntersectPoint.y;
        debugpick.vWorldPosition.z = IntersectPoint.z;
    }


    return true;
    /*
      int v5; // esi@10
      bool v6; // edi@10
      signed int v10; // ebx@14
      int v11; // edi@16
      signed int v12; // ST28_4@18
      int64_t v13; // qtt@18
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
                + ((signed int)(((uint64_t)(v13 /
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

//----- (0046A0A1) --------------------------------------------------------
int UnprojectX(int x) {
    return TrigLUT.atan2(pCamera3D->ViewPlaneDistPixels, pViewport->uScreenCenterX - x);
}

//----- (0046A0F6) --------------------------------------------------------
int UnprojectY(int y) {
    return TrigLUT.atan2(pCamera3D->ViewPlaneDistPixels, pViewport->uScreenCenterY - y);
}

//----- (004C248E) --------------------------------------------------------
void Vis::CastPickRay(float fMouseX, float fMouseY, float fPickDepth, Vec3f *origin, Vec3f *step) {
    origin->z = pCamera3D->vCameraPos.z;
    origin->x = pCamera3D->vCameraPos.x;
    origin->y = pCamera3D->vCameraPos.y;

    int yawAngle = (pCamera3D->_viewYaw + UnprojectX(fMouseX)) & TrigLUT.uDoublePiMask;
    int pitchAngle = (-pCamera3D->_viewPitch + UnprojectY(fMouseY)) & TrigLUT.uDoublePiMask;
    *step = Vec3f::fromPolar(fPickDepth, yawAngle, pitchAngle);
}

//----- (004C2551) --------------------------------------------------------
Vis_ObjectInfo *Vis_SelectionList::SelectionPointers(VisObjectType pVisObjectType,
                                                     Pid pid) {
    // unsigned int v3; // esi@1
    // signed int v4; // edx@1
    // char *v5; // eax@2
    // Vis_ObjectInfo *result; // eax@6

    // v3 = this->uSize;
    if (this->uSize > 0) {
        for (uint i = 0; i < this->uSize; ++i) {
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
            for (uint i = 0; i < uSize; ++i)
                object_pointers[i] = &object_pool[i];
        } break;

        case Unique:  // seems quite retarted; the inner if condition will never
                      // trigger, since we compare pointers, not values.
                      // pointers will always be unique
        {             // but it may be decompilation error thou
            bool create = true;

            for (uint i = 0; i < uSize; ++i) {
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
            logger->warning("Unknown pointer creation flag passed to ::create_object_pointers()");
    }
}

//----- (004C264A) --------------------------------------------------------
void Vis_SelectionList::sort_object_pointers() {
    auto cmp = [](Vis_ObjectInfo *l, Vis_ObjectInfo *r) {
        return l->depth < r->depth;
    };
    std::stable_sort(object_pointers.data(), object_pointers.data() + uSize, cmp);
}

//----- (004C26D0) --------------------------------------------------------
void Vis::SortVerticesByX(RenderVertexD3D3 *pArray, unsigned int uStart, unsigned int uEnd) {
    auto cmp = [](const RenderVertexD3D3 &l, const RenderVertexD3D3 &r) {
        return l.pos.x < r.pos.x;
    };
    std::stable_sort(pArray + uStart, pArray + uEnd + 1, cmp);
}

//----- (004C27AD) --------------------------------------------------------
void Vis::SortVerticesByY(RenderVertexD3D3 *pArray, unsigned int uStart, unsigned int uEnd) {
    auto cmp = [](const RenderVertexD3D3 &l, const RenderVertexD3D3 &r) {
        return l.pos.y < r.pos.y;
    };
    std::stable_sort(pArray + uStart, pArray + uEnd + 1, cmp);
}

//----- (004C288E) --------------------------------------------------------
void Vis::SortByScreenSpaceX(RenderVertexSoft *pArray, int start, int end) {  // сортировка по возрастанию экранных координат х
    auto cmp = [](const RenderVertexSoft &l, const RenderVertexSoft &r) {
        return l.vWorldViewProjX < r.vWorldViewProjX;
    };
    std::stable_sort(pArray + start, pArray + end + 1, cmp);
}

//----- (004C297E) --------------------------------------------------------
void Vis::SortByScreenSpaceY(RenderVertexSoft *pArray, int start, int end) {
    auto cmp = [](const RenderVertexSoft &l, const RenderVertexSoft &r) {
        return l.vWorldViewProjY < r.vWorldViewProjY;
    };
    std::stable_sort(pArray + start, pArray + end + 1, cmp);
}

//----- (004C04AF) --------------------------------------------------------
Vis::Vis() {
    this->log = EngineIocContainer::ResolveLogger();
}

//----- (004C05CC) --------------------------------------------------------
bool Vis::PickKeyboard(float pick_depth, Vis_SelectionList *list,
                       Vis_SelectionFilter *sprite_filter,
                       Vis_SelectionFilter *face_filter) {
    if (!list) list = &default_list;
    list->uSize = 0;

    PickBillboards_Keyboard(pick_depth, list, sprite_filter);
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        PickIndoorFaces_Keyboard(pick_depth, list, face_filter);
    else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
        PickOutdoorFaces_Keyboard(pick_depth, list, face_filter);
    else
        assert(false);

    list->create_object_pointers(Vis_SelectionList::Unique);
    list->sort_object_pointers();

    return true;
}

//----- (004C0646) --------------------------------------------------------
bool Vis::PickMouse(float fDepth, float fMouseX, float fMouseY,
                    Vis_SelectionFilter *sprite_filter,
                    Vis_SelectionFilter *face_filter) {
    Vec3f rayOrigin, rayStep;  // [sp+1Ch] [bp-60h]@1

    default_list.uSize = 0;
    CastPickRay(fMouseX, fMouseY, fDepth, &rayOrigin, &rayStep);

    // log->Info("Sx: {}, Sy: {}, Sz: {} \n Fx: {}, Fy: {}, Fz: {}",
    //     pMouseRay->vWorldPosition.x, pMouseRay->vWorldPosition.y, pMouseRay->vWorldPosition.z,
    //     (pMouseRay+1)->vWorldPosition.x, (pMouseRay + 1)->vWorldPosition.y, (pMouseRay + 1)->vWorldPosition.z);

    PickBillboards_Mouse(fDepth, fMouseX, fMouseY, &default_list, sprite_filter);

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        PickIndoorFaces_Mouse(fDepth, rayOrigin, rayStep, &default_list, face_filter);
    } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        PickOutdoorFaces_Mouse(fDepth, rayOrigin, rayStep, &default_list, face_filter, false);
    } else {
        log->warning("Picking mouse in undefined level");  // picking in main menu is
                                                  // default (buggy) game
                                                  // behaviour. should've
                                                  // returned false in
                                                  // Game::PickMouse
        return false;
    }
    default_list.create_object_pointers(Vis_SelectionList::All);
    default_list.sort_object_pointers();

    return true;
}

//----- (004C06F8) --------------------------------------------------------
void Vis::PickBillboards_Keyboard(float pick_depth, Vis_SelectionList *list,
                                  Vis_SelectionFilter *filter) {
    for (int i = 0; i < render->uNumBillboardsToDraw; ++i) {
        RenderBillboardD3D *d3d_billboard = &render->pBillboardRenderListD3D[i];

        if (is_part_of_selection(i, filter)) {
            if (DoesRayIntersectBillboard(pick_depth, i)) {
                RenderBillboard *billboard = &pBillboardRenderList[d3d_billboard->sParentBillboardID];

                list->AddObject(d3d_billboard->sParentBillboardID,
                                VisObjectType_Sprite, billboard->screen_space_z,
                                billboard->object_pid);
            }
        }
    }
}

// tests the object against selection filter to determine whether it can be
// picked or not
//----- (004C0791) --------------------------------------------------------
bool Vis::is_part_of_selection(const Vis_Object &what, Vis_SelectionFilter *filter) {
    switch (filter->vis_object_type) {
        case VisObjectType_Any:
            return true;

        case VisObjectType_Sprite: {
            int parentBillboardId =
                render->pBillboardRenderListD3D[std::get<int>(what)].sParentBillboardID;

            if (parentBillboardId == -1)
                return false;

            // v5 = filter->select_flags;
            int object_idx = pBillboardRenderList[parentBillboardId].object_pid.id();
            ObjectType object_type = pBillboardRenderList[parentBillboardId].object_pid.type();
            if (filter->select_flags & ExcludeType) {
                return object_type != filter->object_type;
            }
            if (filter->select_flags & ExclusionIfNoEvent) {
                if (object_type != filter->object_type) return true;
                if (filter->object_type != OBJECT_Decoration) {
                    log->warning("Unsupported \"exclusion if no event\" type in CVis::is_part_of_selection");
                    return true;
                }
                if (pLevelDecorations[object_idx].uCog ||
                    pLevelDecorations[object_idx].uEventID)
                    return true;
                return pLevelDecorations[object_idx].IsInteractive();
            }
            if (object_type == filter->object_type) {
                if (object_type != OBJECT_Actor) {
                    log->warning("Default case reached in VIS");
                    return true;
                }

                // v10 = &pActors[object_idx];
                int aiState = 1 << static_cast<int>(pActors[object_idx].aiState); // TODO(captainurist): Flags.

                if (aiState & filter->no_at_ai_state)
                    return false;
                if (!(aiState & filter->at_ai_state))
                    return false;

                auto only_target_undead = filter->select_flags & TargetUndead;
                auto target_not_undead = MonsterStats::BelongsToSupertype(pActors[object_idx].monsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD) == 0;

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
            FaceAttributes face_attrib;
            bool no_event = true;
            if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                ODMFace *face = std::get<ODMFace *>(what);
                no_event = face->sCogTriggeredID == 0;
                face_attrib = face->uAttributes;
            } else if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                BLVFace *face = std::get<BLVFace *>(what);
                no_event = pIndoor->pFaceExtras[face->uFaceExtraID].uEventID == 0;
                face_attrib = face->uAttributes;
            } else {
                assert(false);
            }

            if (filter->object_type != OBJECT_Door) return true;

            FaceAttributes invalid_face_attrib = face_attrib & FaceAttributes(filter->no_at_ai_state);
            if (no_event || invalid_face_attrib)  // face_attrib = 0x2009408 incorrect
                return false;
            return face_attrib & FaceAttributes(filter->at_ai_state);
        }

        default:
            assert(false);
            return false;
    }
}

//----- (004C091D) --------------------------------------------------------
bool Vis::DoesRayIntersectBillboard(float fDepth, unsigned int uD3DBillboardIdx) {
    int billboardId;
    Vec3f rayOrigin, rayStep;
    Vec3f rayOrigin2, rayStep2;

    float test_x;
    float test_y;

    float t1_x;
    float t1_y;
    float t2_x;
    float t2_y;

    /*static*/ Vis_SelectionList Vis_static_stru_F91E10;
    Vis_static_stru_F91E10.uSize = 0;
    billboardId = render->pBillboardRenderListD3D[uD3DBillboardIdx].sParentBillboardID;
    if (billboardId == -1)
        return false;

    if (pBillboardRenderList[billboardId].screen_space_z > fDepth)
        return false;

    // test polygon center first
    GetPolygonCenter(render->pBillboardRenderListD3D[/*billboardId*/uD3DBillboardIdx].pQuads.data(), 4, &test_x, &test_y);
    // why check parent id billboardId? parent ID are wrong becasue of switching between pBillboardRenderListD3D and pBillboardRenderList

    CastPickRay(test_x, test_y, fDepth, &rayOrigin, &rayStep);
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        PickIndoorFaces_Mouse(fDepth, rayOrigin, rayStep, &Vis_static_stru_F91E10, &vis_face_filter);
    else
        PickOutdoorFaces_Mouse(fDepth, rayOrigin, rayStep, &Vis_static_stru_F91E10, &vis_face_filter, false);
    Vis_static_stru_F91E10.create_object_pointers();
    Vis_static_stru_F91E10.sort_object_pointers();
    if (Vis_static_stru_F91E10.uSize) {
        if (Vis_static_stru_F91E10.object_pointers[0]->depth > pBillboardRenderList[billboardId].screen_space_z) {
            return true;
        }
    } else if ((double)(pViewport->uScreen_TL_X) <= test_x && (double)pViewport->uScreen_BR_X >= test_x &&
               (double)pViewport->uScreen_TL_Y <= test_y && (double)pViewport->uScreen_BR_Y >= test_y) {
        return true;
    }


    // test four corners of quad
    for (RenderVertexD3D3 &vertex : render->pBillboardRenderListD3D[uD3DBillboardIdx].pQuads) {
        test_x = vertex.pos.x;
        test_y = vertex.pos.y;
        if ((double)(pViewport->uScreen_TL_X) <= test_x && (double)pViewport->uScreen_BR_X >= test_x &&
            (double)pViewport->uScreen_TL_Y <= test_y && (double)pViewport->uScreen_BR_Y >= test_y) {
            CastPickRay(test_x, test_y, fDepth, &rayOrigin2, &rayStep2);
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                PickIndoorFaces_Mouse(fDepth, rayOrigin2, rayStep2, &Vis_static_stru_F91E10, &vis_face_filter);
            } else {
                PickOutdoorFaces_Mouse(fDepth, rayOrigin2, rayStep2, &Vis_static_stru_F91E10, &vis_face_filter, false);
            }
            Vis_static_stru_F91E10.create_object_pointers();
            Vis_static_stru_F91E10.sort_object_pointers();
            if (!Vis_static_stru_F91E10.uSize) {
                return true;
            }
            if (Vis_static_stru_F91E10.object_pointers[0]->depth > pBillboardRenderList[billboardId].screen_space_z) {
                return true;
            }
        }
    }

    // reverse quad and test center
    // if (uCurrentlyLoadedLevelType != LEVEL_OUTDOOR) return false;
    t1_x = render->pBillboardRenderListD3D[uD3DBillboardIdx].pQuads[0].pos.x;
    t2_x = render->pBillboardRenderListD3D[uD3DBillboardIdx].pQuads[3].pos.x;

    t1_y = render->pBillboardRenderListD3D[uD3DBillboardIdx].pQuads[0].pos.y;
    t2_y = render->pBillboardRenderListD3D[uD3DBillboardIdx].pQuads[1].pos.y;
    if (t1_x > t2_x) {
        std::swap(t1_x, t2_x);
    }
    if (t1_y > t2_y)
        test_y = t1_y;
    else
        test_y = t2_y;

    Vis_static_stru_F91E10.uSize = 0;

    test_x = (t2_x + t1_x) * 0.5;
    if ((double)(pViewport->uScreen_TL_X) <= test_x && (double)pViewport->uScreen_BR_X >= test_x &&
        (double)pViewport->uScreen_TL_Y <= test_y && (double)pViewport->uScreen_BR_Y >= test_y) {
        CastPickRay(test_x, test_y, fDepth, &rayOrigin2, &rayStep2);
        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            PickIndoorFaces_Mouse(fDepth, rayOrigin2, rayStep2, &Vis_static_stru_F91E10, &vis_face_filter);
        } else {
            PickOutdoorFaces_Mouse(fDepth, rayOrigin2, rayStep2, &Vis_static_stru_F91E10, &vis_face_filter, false);
        }
        Vis_static_stru_F91E10.create_object_pointers();
        Vis_static_stru_F91E10.sort_object_pointers();
        if (!Vis_static_stru_F91E10.uSize) {
            return true;
        }
        if (Vis_static_stru_F91E10.object_pointers[0]->depth > pBillboardRenderList[billboardId].screen_space_z) {
            return true;
        }
    }
    return false;
}
// F93E18: using guessed type char static_byte_F93E18_init;

//----- (004C0D32) --------------------------------------------------------
void Vis::PickIndoorFaces_Keyboard(float pick_depth, Vis_SelectionList *list, Vis_SelectionFilter *filter) {
    for (int i = 0; i < pBspRenderer->num_faces; ++i) {
        int pFaceID = pBspRenderer->faces[i].uFaceID;
        BLVFace *pFace = &pIndoor->pFaces[pFaceID];
        if (pCamera3D->is_face_faced_to_cameraBLV(pFace)) {
            if (is_part_of_selection(pFace, filter)) {
                Vis_ObjectInfo *v8 = DetermineFacetIntersection(pFace, Pid(OBJECT_Face, pFaceID), pick_depth);
                if (v8)
                    list->AddObject(v8->object, v8->object_type, v8->depth, v8->object_pid);
            }
        }
    }
}

void Vis::PickOutdoorFaces_Keyboard(float pick_depth, Vis_SelectionList *list,
                                    Vis_SelectionFilter *filter) {
    for (BSPModel &model : pOutdoor->pBModels) {
        bool reachable;
        if (IsBModelVisible(&model, pick_depth, &reachable)) {
            if (reachable) {
                for (ODMFace &face : model.pFaces) {
                    if (is_part_of_selection(&face, filter)) {
                        BLVFace blv_face;
                        blv_face.FromODM(&face);

                        Pid pid = Pid(OBJECT_Face, face.index | (model.index << 6));
                        if (Vis_ObjectInfo *object_info =
                                DetermineFacetIntersection(&blv_face, pid, pick_depth)) {
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
