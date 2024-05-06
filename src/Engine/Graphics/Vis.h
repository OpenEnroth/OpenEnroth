#pragma once

#include "Engine/Graphics/RenderEntities.h"
#include "Engine/Objects/ActorEnums.h"
#include "Engine/Pid.h"

#include "Library/Geometry/Plane.h"

#include "Utility/Flags.h"

class BSPModel;
struct ODMFace;
struct BLVFace;
struct RenderVertexD3D3;

enum class VisObjectType : uint32_t {
    VisObjectType_Any = 0,
    VisObjectType_Sprite = 1,
    VisObjectType_Face = 2
};
using enum VisObjectType;

enum class VisSelectFlag : uint32_t {
    None = 0,
    VisSelectFlags_1 = 1,  // Used to check something that seems to be ally/enemy-based?
    ExcludeType = 2,
    ExclusionIfNoEvent = 4,
    TargetUndead = 8
};
using enum VisSelectFlag;
MM_DECLARE_FLAGS(VisSelectFlags, VisSelectFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(VisSelectFlags)

// NOTE: The variable names here are correct when the filter is used for VisObjectType_Sprite, but wrong for VisObjectType_Face
struct Vis_SelectionFilter {  // stru157
    VisObjectType vis_object_type;
    ObjectType object_type;
    int at_ai_state;
    int no_at_ai_state;
    VisSelectFlags select_flags;
};

extern Vis_SelectionFilter vis_sprite_targets_filter;  // 00F93E1C
extern Vis_SelectionFilter vis_allsprites_filter;  // 00F93E30
extern Vis_SelectionFilter vis_face_filter;      // 00F93E44
extern Vis_SelectionFilter vis_door_filter;      // 00F93E58
extern Vis_SelectionFilter vis_decoration_noevent_filter;  // 00F93E6C
extern Vis_SelectionFilter vis_items_filter;  // static to sub_44EEA7

struct Vis_PIDAndDepth {
    Pid pid;
    int depth = 0;
};

struct Vis_ObjectInfo {
    Pid object_pid = Pid();
    int16_t depth = -1;
    VisObjectType object_type = VisObjectType_Any;
};

struct Vis_SelectionList {
    enum class PointerCreationType { All = 0, Unique = 1 };
    using enum PointerCreationType;

    Vis_ObjectInfo *SelectionPointers(VisObjectType pVisObjectType, Pid pid);
    void create_object_pointers(PointerCreationType type = All);
    void sort_object_pointers();

    inline void AddObject(VisObjectType type, int depth, Pid pid) {
        object_pool[uSize].object_type = type;
        object_pool[uSize].depth = depth;
        object_pool[uSize].object_pid = pid;
        uSize++;
    }

    std::array<Vis_ObjectInfo, 512> object_pool;
    std::array<Vis_ObjectInfo*, 512> object_pointers = {{}};
    unsigned int uSize = 0;
};

class Vis {
 public:
    Vis_PIDAndDepth PickKeyboard(float pick_depth, Vis_SelectionFilter *sprite_filter, Vis_SelectionFilter *face_filter);
    Vis_PIDAndDepth PickMouse(float fDepth, float fMouseX, float fMouseY,
                              Vis_SelectionFilter *sprite_filter, Vis_SelectionFilter *face_filter);

    bool DoesRayIntersectBillboard(float fDepth, unsigned int uD3DBillboardIdx);

    Pid PickClosestActor(ObjectType object_type, unsigned int pick_depth,
                         VisSelectFlags selectFlags, int not_at_ai_state, int at_ai_state);

 private:
    void PickBillboards_Keyboard(float pick_depth, Vis_SelectionList *list,
                                 Vis_SelectionFilter *filter);
    void PickIndoorFaces_Keyboard(float pick_depth, Vis_SelectionList *list,
                                  Vis_SelectionFilter *filter);
    void PickOutdoorFaces_Keyboard(float pick_depth, Vis_SelectionList *list,
                                   Vis_SelectionFilter *filter);
    void PickBillboards_Mouse(float fPickDepth, float fX, float fY,
                              Vis_SelectionList *list,
                              Vis_SelectionFilter *filter);
    void PickIndoorFaces_Mouse(float fDepth, const Vec3f &rayOrigin, const Vec3f &rayStep,
                               Vis_SelectionList *list,
                               Vis_SelectionFilter *filter);
    void PickOutdoorFaces_Mouse(float fDepth, const Vec3f &rayOrigin, const Vec3f &rayStep,
                                Vis_SelectionList *list,
                                Vis_SelectionFilter *filter,
                                bool only_reachable);

    bool isBillboardPartOfSelection(int billboardId, Vis_SelectionFilter *filter);
    bool isFacePartOfSelection(ODMFace *odmFace, BLVFace *bvlFace, Vis_SelectionFilter *filter);

    Vis_ObjectInfo *DetermineFacetIntersection(BLVFace *face, Pid pid, float pick_depth);
    bool IsPolygonOccludedByBillboard(RenderVertexSoft *vertices,
                                      int num_vertices, float x, float y);
    /**
    * @param test_x                        Screen x position of ray.
    * @param test_y                        Screen y position of ray.
    * @param fDepth                        Maximum screen depth of ray.
    * @param fTestDepth                    Screen depth to test geometery against. 
    *
    * @return                              Whether no level geometery is hit at this location before fTestDepth.
    */
    bool DoesRayMissLevelGeom(float test_x, float test_y, float fDepth, float fTestDepth);
    void GetPolygonCenter(RenderVertexD3D3 *pVertices,
                          unsigned int uNumVertices, float *pCenterX,
                          float *pCenterY);
    void GetPolygonScreenSpaceCenter(RenderVertexSoft *vertices,
                                     int num_vertices, float *out_center_x,
                                     float *out_center_y);
    bool IsPointInsideD3DBillboard(RenderBillboardD3D *billboard, float x,
                                   float y);
    void SortVectors_x(RenderVertexSoft *pArray, int start, int end);
    Vis_PIDAndDepth get_object_zbuf_val(Vis_ObjectInfo *info);
    bool Intersect_Ray_Face(const Vec3f &origin, const Vec3f &step,
                            RenderVertexSoft *Intersection, BLVFace *pFace,
                            signed int pBModelID);
    bool CheckIntersectFace(BLVFace *pFace, Vec3f IntersectPoint, signed int sModelID);
    void CastPickRay(float fMouseX, float fMouseY, float fPickDepth, Vec3f *origin, Vec3f *step);
    void SortVerticesByX(RenderVertexD3D3 *pArray, unsigned int uStart,
                         unsigned int uEnd);
    void SortVerticesByY(RenderVertexD3D3 *pArray, unsigned int uStart,
                         unsigned int uEnd);
    void SortByScreenSpaceX(RenderVertexSoft *pArray, int start,
                            int end);
    void SortByScreenSpaceY(RenderVertexSoft *pArray, int start,
                            int end);

 private:
    Vis_SelectionList _selectionList;
};


/**
 * @param model                         Pointer to model to check against.
 * @param reachable_depth               A depth distance for checking interaction against.
 * @param[out] reachable                Whether the model is within the reachable depth specified.
 *
 * @return                              Whether the bounding radius of the model is visible within the camera frustum planes.
 */
bool IsBModelVisible(BSPModel *model, int reachable_depth, bool *reachable);

/**
 * @param center                        Vec3f of centre point of sphere.
 * @param radius                        Float of sphere radius.
 * @param frustum                       Ptr to set of planes to check against - camera used in not supplied.
 *
 * @return                              Whether the bounding radius of the sphere is visible within the frustum planes.
 */
bool IsSphereInFrustum(Vec3f center, float radius, Planef *frustum = nullptr);

/**
 * @param center                        Vec3f of centre point of cylinder.
 * @param radius                        Float of cylinder radius.
 *
 * @return                              Whether the cylinder is visible within the L/R camera frustum planes.
 */
bool IsCylinderInFrustum(Vec3f center, float radius);
