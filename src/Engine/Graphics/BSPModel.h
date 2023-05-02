#pragma once

#include <array>
#include <vector>
#include <string>

#include "Utility/Flags.h"
#include "Utility/Geometry/Plane.h"
#include "Utility/Geometry/BBox.h"

enum class FaceAttribute : uint32_t {
    FACE_IsPortal          = 0x00000001,
    FACE_IsSecret          = 0x00000002,
    FACE_FlowDown          = 0x00000004,
    FACE_TexAlignDown      = 0x00000008,
    FACE_IsFluid           = 0x00000010,
    FACE_FlowUp            = 0x00000020,
    FACE_FlowLeft          = 0x00000040,
    FACE_SeenByParty       = 0x00000080,
    FACE_XY_PLANE          = 0x00000100,
    FACE_XZ_PLANE          = 0x00000200,
    FACE_YZ_PLANE          = 0x00000400,
    FACE_FlowRight         = 0x00000800,
    FACE_TexAlignLeft      = 0x00001000,
    FACE_IsInvisible       = 0x00002000,
    FACE_TEXTURE_FRAME     = 0x00004000,  // Texture ID is a frameset from TextureFrameTable, otherwise BitmapID
    FACE_TexAlignRight     = 0x00008000,
    FACE_OUTLINED          = 0x00010000,  // outline face debug
    FACE_TexAlignBottom    = 0x00020000,
    FACE_TexMoveByDoor     = 0x00040000,
    FACE_UNKOWN_10         = 0x00080000,  // MMExt: TriggerByTouch, doesn't work anymore
    FACE_HAS_EVENT         = 0x00100000,  // MMExt: IsEventJustHint, [MM7+]
    FACE_INDOOR_CARPET     = 0x00200000,  // MMExt: AlternativeSound
    FACE_INDOOR_SKY        = 0x00400000,  // MMExt: outdoor in software mode: horizontal flow
    FACE_FlipNormalU       = 0x00800000,
    FACE_FlipNormalV       = 0x01000000,
    FACE_CLICKABLE         = 0x02000000,  // Event can be triggered by clicking on the facet.
    FACE_PRESSURE_PLATE    = 0x04000000,  // Event can be triggered by stepping on the facet.
    FACE_INDICATE          = 0x06000000,  // face has status bar string on hover
    FACE_TriggerByMonster  = 0x08000000,
    FACE_TriggerByObject   = 0x10000000,
    FACE_ETHEREAL          = 0x20000000,  // Untouchable. You can pass through it.
                                          // MMExt: great for vertical facets of stairs.
                                          // [MM7+] Shouldn't be used for sloped floor, like it's used in MM6.
    FACE_IsLava            = 0x40000000,
    FACE_IsPicked          = 0x80000000,  // mouse is hovering
                                          // TODO: MMExt: HasData, are we talking about BLVFaceExtra?
};
using enum FaceAttribute;
MM_DECLARE_FLAGS(FaceAttributes, FaceAttribute)
MM_DECLARE_OPERATORS_FOR_FLAGS(FaceAttributes)

// door attr
enum class DoorAttribute : uint32_t {
    DOOR_TRIGGERED         = 0x00000001,
    DOOR_SETTING_UP        = 0x00000002,
    DOOR_NOSOUND           = 0x00000004,
};
using enum DoorAttribute;
MM_DECLARE_FLAGS(DoorAttributes, DoorAttribute)
MM_DECLARE_OPERATORS_FOR_FLAGS(DoorAttributes)

/*  291 */
enum class PolygonType : uint8_t {
    POLYGON_Invalid = 0x0,
    POLYGON_VerticalWall = 0x1,
    POLYGON_unk = 0x2,
    POLYGON_Floor = 0x3,
    POLYGON_InBetweenFloorAndWall = 0x4,
    POLYGON_Ceiling = 0x5,
    POLYGON_InBetweenCeilingAndWall = 0x6,
};
using enum PolygonType;

struct BSPNode {
    int16_t uFront;
    int16_t uBack;
    int16_t uBSPFaceIDOffset;
    int16_t uNumBSPFaces;
};

class Texture;

struct ODMFace {
    bool HasEventHint();

    inline bool Invisible() const {
        return uAttributes & FACE_IsInvisible;
    }
    inline bool Visible() const { return !Invisible(); }
    inline bool Portal() const { return uAttributes & FACE_IsPortal; }
    inline bool Fluid() const { return uAttributes & FACE_IsFluid; }
    inline bool Indoor_sky() const {
        return uAttributes & FACE_INDOOR_SKY;
    }
    inline bool Clickable() const {
        return uAttributes & FACE_CLICKABLE;
    }
    inline bool Pressure_Plate() const {
        return uAttributes & FACE_PRESSURE_PLATE;
    }
    inline bool Ethereal() const { return uAttributes & FACE_ETHEREAL; }

    inline bool IsTextureFrameTable() {
        return this->uAttributes & FACE_TEXTURE_FRAME;
    }
    inline void ToggleIsTextureFrameTable() {
        this->uAttributes = this->uAttributes & FACE_TEXTURE_FRAME
                                ? this->uAttributes & ~FACE_TEXTURE_FRAME
                                : this->uAttributes | FACE_TEXTURE_FRAME;
    }

    void SetTexture(const std::string &filename);
    Texture *GetTexture();

    // TODO: does this really have to be two separate functions?
    /**
     * @see BLVFace::Contains
     */
    bool Contains(const Vec3i &pos, int model_idx, int slack = 0, FaceAttributes override_plane = 0) const;

    unsigned int index = 0;
    Planef facePlane;
    PlaneZCalcf zCalc;
    FaceAttributes uAttributes = 0;
    std::array<int16_t, 20> pVertexIDs = {{}};
    std::array<int16_t, 20> pTextureUIDs = {{}};
    std::array<int16_t, 20> pTextureVIDs = {{}};
    std::array<int16_t, 20> pXInterceptDisplacements = {{}};
    std::array<int16_t, 20> pYInterceptDisplacements = {{}};
    std::array<int16_t, 20> pZInterceptDisplacements = {{}};

    // details store for array texture
    void *resource = nullptr;  // int16_t uTextureID;;
    int texunit = -1;
    int texlayer = -1;

    int16_t sTextureDeltaU = 0;
    int16_t sTextureDeltaV = 0;
    // TODO(pskelton): Geometry should be float
    BBoxs pBoundingBox;
    int16_t sCogNumber = 0;
    int16_t sCogTriggeredID = 0;
    int16_t sCogTriggerType = 0;
    char field_128 = 0;
    char field_129 = 0;
    uint8_t uGradientVertex1 = 0;
    uint8_t uGradientVertex2 = 0;
    uint8_t uGradientVertex3 = 0;
    uint8_t uGradientVertex4 = 0;
    uint8_t uNumVertices = 0;
    PolygonType uPolygonType = POLYGON_Invalid;
    uint8_t uShadeType = 0;  // could be minimum shade
    uint8_t bVisible = 0;
    char field_132 = 0;
    char field_133 = 0;
};

class BSPModel {
 public:
    unsigned int index = 0;
    std::string pModelName;
    std::string pModelName2;
    int32_t field_40 = 0; // visibility flag TODO(pskelton): use for map tooltip checking or remove
    int32_t sCenterX = 0;
    int32_t sCenterY = 0;
    Vec3i vPosition {};
    BBoxi pBoundingBox = {0, 0, 0, 0, 0, 0};
    int32_t sSomeOtherMinX = 0;
    int32_t sSomeOtherMinY = 0;
    int32_t sSomeOtherMinZ = 0;
    int32_t sSomeOtherMaxX = 0;
    int32_t sSomeOtherMaxY = 0;
    int32_t sSomeOtherMaxZ = 0;
    Vec3i vBoundingCenter {};
    int32_t sBoundingRadius = 0;

    std::vector<Vec3i> pVertices;
    std::vector<ODMFace> pFaces;
    std::vector<uint16_t> pFacesOrdering;
    std::vector<BSPNode> pNodes;
};
