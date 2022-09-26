#include "Engine/Graphics/BSPModel.h"

#include <cstring>
#include <cstdlib>
#include <algorithm>

#include "Engine/AssetsManager.h"
#include "Engine/Events.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Time.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Serialization/MemoryInput.h"
#include "Engine/VectorTypes.h"


#pragma pack(push, 1)
struct ODMFace_MM7 {
    struct Planei pFacePlane;
    int zCalc1;
    int zCalc2;
    int zCalc3;
    unsigned int uAttributes;
    std::array<uint16_t, 20> pVertexIDs;
    std::array<int16_t, 20> pTextureUIDs;
    std::array<int16_t, 20> pTextureVIDs;
    std::array<int16_t, 20> pXInterceptDisplacements;
    std::array<int16_t, 20> pYInterceptDisplacements;
    std::array<int16_t, 20> pZInterceptDisplacements;
    int16_t uTextureID;
    int16_t sTextureDeltaU;
    int16_t sTextureDeltaV;
    BBoxs pBoundingBox;
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
#pragma pack(pop)

void BSPModelList::Load(MemoryInput *stream) {
    static_assert(sizeof(BSPModelData) == 188, "Wrong type size");
    static_assert(sizeof(BSPNode) == 8, "Wrong type size");
    static_assert(sizeof(Vec3i) == 12, "Wrong type size");

    std::vector<BSPModelData> models;
    stream->ReadVector(&models);

    unsigned int model_index = 0;
    for (BSPModelData &model : models) {
        this->emplace_back();
        BSPModel &new_model = this->back();

        new_model.index = model_index;
        new_model.pModelName = model.pModelName;
        new_model.pModelName2 = model.pModelName2;
        new_model.field_40 = model.field_40;
        new_model.sCenterX = model.sCenterX;
        new_model.sCenterY = model.sCenterY;
        new_model.vPosition = model.vPosition;
        new_model.pBoundingBox.x1 = model.sMinX;
        new_model.pBoundingBox.y1 = model.sMinY;
        new_model.pBoundingBox.z1 = model.sMinZ;
        new_model.pBoundingBox.x2 = model.sMaxX;
        new_model.pBoundingBox.y2 = model.sMaxY;
        new_model.pBoundingBox.z2 = model.sMaxZ;
        new_model.sSomeOtherMinX = model.sSomeOtherMinX;
        new_model.sSomeOtherMinY = model.sSomeOtherMinY;
        new_model.sSomeOtherMinZ = model.sSomeOtherMinZ;
        new_model.sSomeOtherMaxX = model.sSomeOtherMaxX;
        new_model.sSomeOtherMaxY = model.sSomeOtherMaxY;
        new_model.sSomeOtherMaxZ = model.sSomeOtherMaxZ;
        new_model.vBoundingCenter = model.vBoundingCenter;
        new_model.sBoundingRadius = model.sBoundingRadius;

        stream->ReadSizedVector(&new_model.pVertices, model.uNumVertices);

        std::vector<ODMFace_MM7> mm7faces;
        stream->ReadSizedVector(&mm7faces, model.uNumFaces);
        new_model.pFaces.resize(mm7faces.size());
        for (size_t i = 0; i < mm7faces.size(); i++) {
            new_model.pFaces[i].index = i;
            new_model.pFaces[i].Deserialize(&mm7faces[i]);
        }

        stream->ReadSizedVector(&new_model.pFacesOrdering, model.uNumFaces);
        stream->ReadSizedVector(&new_model.pNodes, model.uNumNodes);

        std::string textureName;
        for (size_t i = 0; i < model.uNumFaces; ++i) {
            stream->ReadSizedString(&textureName, 10);
            new_model.pFaces[i].SetTexture(textureName);

            if (new_model.pFaces[i].sCogTriggeredID) {
                if (new_model.pFaces[i].HasEventHint())
                    new_model.pFaces[i].uAttributes |= FACE_HAS_EVENT;
                else
                    new_model.pFaces[i].uAttributes &= ~FACE_HAS_EVENT;
            }
        }

        model_index++;
    }
}

// ODMFace

Texture *ODMFace::GetTexture() {
    if (this->IsTextureFrameTable()) {
        return pTextureFrameTable->GetFrameTexture(
            (int64_t)this->resource, pEventTimer->uTotalGameTimeElapsed);
    } else {
        return (Texture *)this->resource;
    }
}

void ODMFace::SetTexture(const std::string &filename) {
    if (this->IsTextureFrameTable()) {
        this->resource = (void *)pTextureFrameTable->FindTextureByName(filename.c_str());
        if (this->resource != (void *)-1) {
            return;
        }

        this->ToggleIsTextureFrameTable();
    }

    this->resource = assets->GetBitmap(filename);
    this->texlayer = -1;
    this->texunit = -1;
}

bool ODMFace::Deserialize(const ODMFace_MM7 *mm7) {
    this->pFacePlaneOLD = mm7->pFacePlane;
    this->pFacePlane.vNormal.x = this->pFacePlaneOLD.vNormal.x / 65536.0;
    this->pFacePlane.vNormal.y = this->pFacePlaneOLD.vNormal.y / 65536.0;
    this->pFacePlane.vNormal.z = this->pFacePlaneOLD.vNormal.z / 65536.0;
    this->pFacePlane.dist = this->pFacePlaneOLD.dist / 65536.0;

    this->zCalc.Init(this->pFacePlaneOLD);
    this->uAttributes = FaceAttributes(mm7->uAttributes);
    this->pVertexIDs = mm7->pVertexIDs;
    this->pTextureUIDs = mm7->pTextureUIDs;
    this->pTextureVIDs = mm7->pTextureVIDs;
    this->pXInterceptDisplacements = mm7->pXInterceptDisplacements;
    this->pYInterceptDisplacements = mm7->pYInterceptDisplacements;
    this->pZInterceptDisplacements = mm7->pZInterceptDisplacements;
    this->resource = nullptr;
    this->sTextureDeltaU = mm7->sTextureDeltaU;
    this->sTextureDeltaV = mm7->sTextureDeltaV;
    this->pBoundingBox = mm7->pBoundingBox;
    this->sCogNumber = mm7->sCogNumber;
    this->sCogTriggeredID = mm7->sCogTriggeredID;
    this->sCogTriggerType = mm7->sCogTriggerType;
    this->field_128 = mm7->field_128;
    this->field_129 = mm7->field_129;
    this->uGradientVertex1 = mm7->uGradientVertex1;
    this->uGradientVertex2 = mm7->uGradientVertex2;
    this->uGradientVertex3 = mm7->uGradientVertex3;
    this->uGradientVertex4 = mm7->uGradientVertex4;
    this->uNumVertices = mm7->uNumVertices;
    this->uPolygonType = PolygonType(mm7->uPolygonType);
    this->uShadeType = mm7->uShadeType;
    this->bVisible = mm7->bVisible;
    this->field_132 = mm7->field_132;
    this->field_133 = mm7->field_133;

    return true;
}

bool ODMFace::HasEventHint() {
    signed int event_index;  // eax@1
    _evt_raw *start_evt;
    _evt_raw *end_evt;

    event_index = 0;
    if ((uLevelEVT_NumEvents - 1) <= 0) return false;
    while (pLevelEVT_Index[event_index].event_id!= this->sCogTriggeredID) {
        ++event_index;
        if (event_index >= (signed int)(uLevelEVT_NumEvents - 1)) return false;
    }
    end_evt =
        (_evt_raw
             *)&pLevelEVT[pLevelEVT_Index[event_index + 1].uEventOffsetInEVT];
    start_evt =
        (_evt_raw *)&pLevelEVT[pLevelEVT_Index[event_index].uEventOffsetInEVT];
    if ((end_evt->_e_type != EVENT_Exit) ||
        (start_evt->_e_type != EVENT_MouseOver))
        return false;
    else
        return true;
}

bool ODMFace::Contains(const Vec3i &pos, int model_idx, int slack, FaceAttributes override_plane) const {
    BLVFace face;
    face.uAttributes = this->uAttributes;
    face.uNumVertices = this->uNumVertices;
    face.pVertexIDs = const_cast<uint16_t *>(this->pVertexIDs.data());
    return face.Contains(pos, model_idx, slack, override_plane);
}
