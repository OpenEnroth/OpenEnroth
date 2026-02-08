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
        return attributes & FACE_IsInvisible;
    }
    inline bool Visible() const { return !Invisible(); }
    inline bool Portal() const { return attributes & FACE_IsPortal; }
    inline bool Fluid() const { return attributes & FACE_IsFluid; }
    inline bool Indoor_sky() const {
        return attributes & FACE_INDOOR_SKY;
    }
    inline bool Clickable() const {
        return attributes & FACE_CLICKABLE;
    }
    inline bool Pressure_Plate() const {
        return attributes & FACE_PRESSURE_PLATE;
    }
    inline bool Ethereal() const { return attributes & FACE_ETHEREAL; }

    inline bool IsAnimated() {
        return this->attributes & FACE_ANIMATED;
    }
    inline void ToggleIsAnimated() {
        this->attributes = this->attributes & FACE_ANIMATED
                                ? this->attributes & ~FACE_ANIMATED
                                : this->attributes | FACE_ANIMATED;
    }

    void SetTexture(std::string_view filename);
    GraphicsImage *GetTexture();

    // TODO: does this really have to be two separate functions?
    /**
     * @see BLVFace::Contains
     */
    bool Contains(const Vec3f &pos, int model_idx, int slack = 0, FaceAttributes override_plane = 0) const;

    int index = 0;
    Planef facePlane;
    PlaneZCalcf zCalc;
    FaceAttributes attributes = 0;
    std::array<int16_t, 20> vertexIds = {{}};
    std::array<int16_t, 20> textureUs = {{}};
    std::array<int16_t, 20> textureVs = {{}};

    // details store for array texture
    GraphicsImage *texture = nullptr; // Face texture, or nullptr if this face is animated.
    int animationId = 0; // Index into pTextureFrameTable for animated faces.
    int texunit = -1;
    int texlayer = -1;

    int16_t textureDeltaU = 0;
    int16_t textureDeltaV = 0;
    BBoxf boundingBox;
    int16_t cogNumber = 0;
    int16_t eventId = 0;
    uint8_t numVertices = 0;
    PolygonType polygonType = POLYGON_Invalid;
};

class BSPModel {
 public:
    int index = 0;
    int32_t field_40 = 0; // visibility flag TODO(pskelton): use for map tooltip checking or remove
    Vec3f position {};
    BBoxi boundingBox = {0, 0, 0, 0, 0, 0};
    Vec3f boundingCenter {};
    float boundingRadius = 0;

    std::vector<Vec3f> vertices;
    std::vector<ODMFace> faces;
    std::vector<BSPNode> nodes;
};
