#pragma once

#include <vector>

#include "../VectorTypes.h"
#include "Engine/Strings.h"

#define FACE_PORTAL             0x00000001  // portal/two-sided
#define FACE_CAN_SATURATE_COLOR 0x00000002  // guess at perception faces
#define FACE_FLOW_DIAGONAL      0x00000004  // Diagonal flow of the lava or water
#define FACE_UNKNOW5            0x00000006
#define FACE_UNKNOW6            0x00000008
#define FACE_FLUID              0x00000010          // wavy animated water or lava
#define FACE_FLOW_VERTICAL      0x00000020  // Vertical flow of the lava or water
#define FACE_DONT_CACHE_TEXTURE 0x00000040  // do not load face texture if it isn't loaded already
#define FACE_RENDERED           0x00000080  // face has been rendered - ie seen by party
#define FACE_XY_PLANE           0x00000100
#define FACE_XZ_PLANE           0x00000200
#define FACE_YZ_PLANE           0x00000400
#define FACE_FLOW_HORIZONTAL    0x00000800  // Horizontal flow of the lava or water
#define FACE_HAS_EVENT_HINT     0x00001000
#define FACE_INVISIBLE          0x00002000
#define FACE_TEXTURE_FRAME      0x00004000  // Texture ID is a frameset from TextureFrameTable, otherwise BitmapID
#define FACE_OUTLINED           0x00010000  // outline face edges
#define FACE_INDOOR_DOOR        0x00020000
#define FACE_TEXTURE_FLOW       0x00040000  // The texture moves slowly. For horizontal facets only.
#define FACE_HAS_EVENT          0x00100000
#define FACE_INDOOR_CARPET      0x00200000
#define FACE_INDOOR_SKY         0x00400000
#define FACE_UNKNOW3            0x00800000
#define FACE_UNKNOW4            0x01000000
#define FACE_CLICKABLE          0x02000000  // Event can be triggered by clicking on the facet.
#define FACE_PRESSURE_PLATE     0x04000000  // Event can be triggered by stepping on the facet.
#define FACE_INDICATE           0x06000000  // Event can be triggered by indicating on the facet.
#define FACE_UNKNOW1            0x08000000
#define FACE_UNKNOW2            0x10000000
#define FACE_ETHEREAL           0x20000000  // Untouchable. You can pass through it.
#define FACE_INDOOR_LAVA        0x40000000
#define FACE_PICKED             0x80000000

#pragma pack(push, 1)
struct BSPNode {
    int16_t uFront;
    int16_t uBack;
    int16_t uCoplanarOffset;
    int16_t uCoplanarSize;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BSPVertexBuffer {
    uint32_t uNumVertices;
    Vec3_int_ *pVertices;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BSPModelData {
    char pModelName[32];
    char pModelName2[32];
    int32_t field_40;
    uint32_t uNumVertices;
    uint32_t ppVertices;
    uint32_t uNumFaces;
    uint32_t uNumConvexFaces;
    uint32_t ppFaces;
    uint32_t ppFacesOrdering;
    uint32_t uNumNodes;
    uint32_t ppNodes;
    uint32_t uNumDecorations;
    int32_t sCenterX;
    int32_t sCenterY;
    Vec3_int_ vPosition;
    int32_t sMinX;
    int32_t sMinY;
    int32_t sMinZ;
    int32_t sMaxX;
    int32_t sMaxY;
    int32_t sMaxZ;
    int32_t sSomeOtherMinX;
    int32_t sSomeOtherMinY;
    int32_t sSomeOtherMinZ;
    int32_t sSomeOtherMaxX;
    int32_t sSomeOtherMaxY;
    int32_t sSomeOtherMaxZ;
    Vec3_int_ vBoundingCenter;
    int32_t sBoundingRadius;
};
#pragma pack(pop)

class Texture;

struct ODMFace {
    bool HasEventHint();

    static bool IsBackfaceNotCulled(struct RenderVertexSoft *a2,
                                    struct Polygon *polygon);

    inline bool Invisible() const {
        return (uAttributes & FACE_INVISIBLE) != 0;
    }
    inline bool Visible() const { return !Invisible(); }
    inline bool Portal() const { return (uAttributes & FACE_PORTAL) != 0; }
    inline bool Fluid() const { return (uAttributes & FACE_FLUID) != 0; }
    inline bool Indoor_sky() const {
        return (uAttributes & FACE_INDOOR_SKY) != 0;
    }
    inline bool Clickable() const {
        return (uAttributes & FACE_CLICKABLE) != 0;
    }
    inline bool Pressure_Plate() const {
        return (uAttributes & FACE_PRESSURE_PLATE) != 0;
    }
    inline bool Ethereal() const { return (uAttributes & FACE_ETHEREAL) != 0; }

    inline bool IsTextureFrameTable() {
        return this->uAttributes & FACE_TEXTURE_FRAME;
    }
    inline void ToggleIsTextureFrameTable() {
        this->uAttributes = this->uAttributes & FACE_TEXTURE_FRAME
                                ? this->uAttributes & ~FACE_TEXTURE_FRAME
                                : this->uAttributes | FACE_TEXTURE_FRAME;
    }

    void SetTexture(const String &filename);
    Texture *GetTexture();

    bool Deserialize(struct ODMFace_MM7 *);

    unsigned int index;
    struct Plane_int_ pFacePlane;
    int zCalc1;
    int zCalc2;
    int zCalc3;
    uint32_t uAttributes;
    uint16_t pVertexIDs[20];
    int16_t pTextureUIDs[20];
    int16_t pTextureVIDs[20];
    int16_t pXInterceptDisplacements[20];
    int16_t pYInterceptDisplacements[20];
    int16_t pZInterceptDisplacements[20];
    void *resource;  // __int16 uTextureID;
    int16_t sTextureDeltaU;
    int16_t sTextureDeltaV;
    struct BBox_short_ pBoundingBox;
    int16_t sCogNumber;
    int16_t sCogTriggeredID;
    int16_t sCogTriggerType;
    char field_128;
    char field_129;
    uint8_t uGradientVertex1;
    uint8_t uGradientVertex2;
    uint8_t uGradientVertex3;
    uint8_t uGradientVertex4;
    uint8_t uNumVertices;
    uint8_t uPolygonType;
    uint8_t uShadeType;
    uint8_t bVisible;
    char field_132;
    char field_133;
};

class BSPModel {
 public:
    void Release();

    unsigned int index;
    String pModelName;
    String pModelName2;
    int32_t field_40;
    int32_t sCenterX;
    int32_t sCenterY;
    Vec3_int_ vPosition;
    int32_t sMinX;
    int32_t sMinY;
    int32_t sMinZ;
    int32_t sMaxX;
    int32_t sMaxY;
    int32_t sMaxZ;
    int32_t sSomeOtherMinX;
    int32_t sSomeOtherMinY;
    int32_t sSomeOtherMinZ;
    int32_t sSomeOtherMaxX;
    int32_t sSomeOtherMaxY;
    int32_t sSomeOtherMaxZ;
    Vec3_int_ vBoundingCenter;
    int32_t sBoundingRadius;

    struct BSPVertexBuffer pVertices;
    std::vector<ODMFace> pFaces;
    uint16_t *pFacesOrdering;
    std::vector<BSPNode> pNodes;
};

class BSPModelList : public std::vector<BSPModel> {
 public:
    uint8_t *Load(uint8_t *data);
};
