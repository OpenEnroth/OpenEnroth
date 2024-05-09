#pragma once

#include <array>
#include <vector>
#include <string>

#include "Library/Geometry/Plane.h"
#include "Library/Geometry/BBox.h"

#include "FaceEnums.h"

class GraphicsImage;

struct BSPNode {
    int uFront;
    int uBack;
    int16_t uBSPFaceIDOffset;
    int16_t uNumBSPFaces;
};


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

    void SetTexture(std::string_view filename);
    GraphicsImage *GetTexture();

    // TODO: does this really have to be two separate functions?
    /**
     * @see BLVFace::Contains
     */
    bool Contains(const Vec3f &pos, int model_idx, int slack = 0, FaceAttributes override_plane = 0) const;

    unsigned int index = 0;
    Planef facePlane;
    PlaneZCalcf zCalc;
    FaceAttributes uAttributes = 0;
    std::array<int16_t, 20> pVertexIDs = {{}};
    std::array<int16_t, 20> pTextureUIDs = {{}};
    std::array<int16_t, 20> pTextureVIDs = {{}};

    // details store for array texture
    void *resource = nullptr;  // TODO(captainurist): this can be an int64, deal away with weird casts.
    int texunit = -1;
    int texlayer = -1;

    int16_t sTextureDeltaU = 0;
    int16_t sTextureDeltaV = 0;
    BBoxf pBoundingBox;
    int16_t sCogNumber = 0;
    int16_t sCogTriggeredID = 0;
    int16_t sCogTriggerType = 0;
    uint8_t uNumVertices = 0;
    PolygonType uPolygonType = POLYGON_Invalid;
    uint8_t uShadeType = 0;  // could be minimum shade
    uint8_t bVisible = 0;
};

class BSPModel {
 public:
    unsigned int index = 0;
    std::string pModelName;
    std::string pModelName2;
    int32_t field_40 = 0; // visibility flag TODO(pskelton): use for map tooltip checking or remove
    int32_t sCenterX = 0;
    int32_t sCenterY = 0;
    Vec3f vPosition {};
    BBoxi pBoundingBox = {0, 0, 0, 0, 0, 0};
    int32_t sSomeOtherMinX = 0;
    int32_t sSomeOtherMinY = 0;
    int32_t sSomeOtherMinZ = 0;
    int32_t sSomeOtherMaxX = 0;
    int32_t sSomeOtherMaxY = 0;
    int32_t sSomeOtherMaxZ = 0;
    Vec3f vBoundingCenter {};
    float sBoundingRadius = 0;

    std::vector<Vec3f> pVertices;
    std::vector<ODMFace> pFaces;
    std::vector<uint16_t> pFacesOrdering;
    std::vector<BSPNode> pNodes;
};
