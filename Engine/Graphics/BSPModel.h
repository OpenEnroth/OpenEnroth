#pragma once

#include <vector>
#include <string>

#include "../VectorTypes.h"

#define FACE_IsPortal           0x00000001
#define FACE_IsSecret           0x00000002
#define FACE_FlowDown           0x00000004
#define FACE_TexAlignDown       0x00000008
#define FACE_IsFluid            0x00000010
#define FACE_FlowUp             0x00000020
#define FACE_FlowLeft           0x00000040
#define FACE_SeenByParty        0x00000080
#define FACE_XY_PLANE           0x00000100
#define FACE_XZ_PLANE           0x00000200
#define FACE_YZ_PLANE           0x00000400
#define FACE_FlowRight          0x00000800
#define FACE_TexAlignLeft       0x00001000
#define FACE_IsInvisible        0x00002000
#define FACE_TEXTURE_FRAME      0x00004000  // Texture ID is a frameset from TextureFrameTable, otherwise BitmapID
#define FACE_TexAlignRight      0x00008000
#define FACE_OUTLINED           0x00010000  // outline face debug
#define FACE_TexAlignBottom     0x00020000
#define FACE_TexMoveByDoor      0x00040000
#define FACE_UNKOWN_10          0x00080000
#define FACE_HAS_EVENT          0x00100000
#define FACE_INDOOR_CARPET      0x00200000
#define FACE_INDOOR_SKY         0x00400000
#define FACE_FlipNormalU        0x00800000
#define FACE_FlipNormalV        0x01000000
#define FACE_CLICKABLE          0x02000000  // Event can be triggered by clicking on the facet.
#define FACE_PRESSURE_PLATE     0x04000000  // Event can be triggered by stepping on the facet.
#define FACE_INDICATE           0x06000000  // face has status bar string on hover
#define FACE_TriggerByMonster   0x08000000
#define FACE_TriggerByObject    0x10000000
#define FACE_ETHEREAL           0x20000000  // Untouchable. You can pass through it.
#define FACE_IsLava             0x40000000
#define FACE_IsPicked           0x80000000  // mouse is hovering

// door attr
#define DOOR_TRIGGERED          0x00000001
#define DOOR_SETTING_UP         0x00000002
#define DOOR_NOSOUND            0x00000004

#pragma pack(push, 1)
struct BSPNode {
    int16_t uFront;
    int16_t uBack;
    int16_t uBSPFaceIDOffset;
    int16_t uNumBSPFaces;
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
    char pModelName[32] {};
    char pModelName2[32] {};
    int32_t field_40 = 0;
    uint32_t uNumVertices = 0;
    uint32_t ppVertices = 0;
    uint32_t uNumFaces = 0;
    uint32_t uNumConvexFaces = 0;
    uint32_t ppFaces = 0;
    uint32_t ppFacesOrdering = 0;
    uint32_t uNumNodes = 0;
    uint32_t ppNodes = 0;
    uint32_t uNumDecorations = 0;
    int32_t sCenterX = 0;
    int32_t sCenterY = 0;
    Vec3_int_ vPosition {};
    int32_t sMinX = 0;
    int32_t sMinY = 0;
    int32_t sMinZ = 0;
    int32_t sMaxX = 0;
    int32_t sMaxY = 0;
    int32_t sMaxZ = 0;
    int32_t sSomeOtherMinX = 0;
    int32_t sSomeOtherMinY = 0;
    int32_t sSomeOtherMinZ = 0;
    int32_t sSomeOtherMaxX = 0;
    int32_t sSomeOtherMaxY = 0;
    int32_t sSomeOtherMaxZ = 0;
    Vec3_int_ vBoundingCenter {};
    int32_t sBoundingRadius = 0;
};
#pragma pack(pop)

class Texture;

struct ODMFace {
    bool HasEventHint();

    inline bool Invisible() const {
        return (uAttributes & FACE_IsInvisible) != 0;
    }
    inline bool Visible() const { return !Invisible(); }
    inline bool Portal() const { return (uAttributes & FACE_IsPortal) != 0; }
    inline bool Fluid() const { return (uAttributes & FACE_IsFluid) != 0; }
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

    void SetTexture(const std::string &filename);
    Texture *GetTexture();

    bool Deserialize(struct ODMFace_MM7 *);

    unsigned int index = 0;
    struct Plane_float_ pFacePlane;
    struct Plane_int_ pFacePlaneOLD;
    int zCalc1 = 0;
    int zCalc2 = 0;
    int zCalc3 = 0;
    uint32_t uAttributes = 0;
    uint16_t pVertexIDs[20] {};
    int16_t pTextureUIDs[20] {};
    int16_t pTextureVIDs[20] {};
    int16_t pXInterceptDisplacements[20] {};
    int16_t pYInterceptDisplacements[20] {};
    int16_t pZInterceptDisplacements[20] {};
    void *resource = nullptr;  // __int16 uTextureID;
    std::string resourcename;

    int16_t sTextureDeltaU = 0;
    int16_t sTextureDeltaV = 0;
    struct BBox_short_ pBoundingBox {};
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
    uint8_t uPolygonType = 0;
    uint8_t uShadeType = 0;  // could be minimum shade
    uint8_t bVisible = 0;
    char field_132 = 0;
    char field_133 = 0;
};

class BSPModel {
 public:
    void Release();

    unsigned int index = 0;
    std::string pModelName;
    std::string pModelName2;
    int32_t field_40 = 0;
    int32_t sCenterX = 0;
    int32_t sCenterY = 0;
    Vec3_int_ vPosition {};
    BBox_int_ pBoundingBox = {0, 0, 0, 0, 0, 0};
    int32_t sSomeOtherMinX = 0;
    int32_t sSomeOtherMinY = 0;
    int32_t sSomeOtherMinZ = 0;
    int32_t sSomeOtherMaxX = 0;
    int32_t sSomeOtherMaxY = 0;
    int32_t sSomeOtherMaxZ = 0;
    Vec3_int_ vBoundingCenter {};
    int32_t sBoundingRadius = 0;

    struct BSPVertexBuffer pVertices {};
    std::vector<ODMFace> pFaces;
    uint16_t *pFacesOrdering = nullptr;
    std::vector<BSPNode> pNodes;
};

class BSPModelList : public std::vector<BSPModel> {
 public:
    uint8_t *Load(uint8_t *data);
};
