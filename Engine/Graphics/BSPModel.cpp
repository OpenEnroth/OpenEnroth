#include "Engine/Graphics/BSPModel.h"

#include <cstdlib>

#include "Engine/AssetsManager.h"
#include "Engine/Events.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Time.h"

#pragma pack(push, 1)
struct ODMFace_MM7 {
    struct Plane_int_ pFacePlane;
    int zCalc1;
    int zCalc2;
    int zCalc3;
    unsigned int uAttributes;
    uint16_t pVertexIDs[20];
    uint16_t pTextureUIDs[20];
    uint16_t pTextureVIDs[20];
    int16_t pXInterceptDisplacements[20];
    int16_t pYInterceptDisplacements[20];
    int16_t pZInterceptDisplacements[20];
    int16_t uTextureID;
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
#pragma pack(pop)

uint8_t *BSPModelList::Load(uint8_t *data) {
    static_assert(sizeof(BSPModelData) == 188, "Wrong type size");
    static_assert(sizeof(BSPNode) == 8, "Wrong type size");
    static_assert(sizeof(Vec3_int_) == 12, "Wrong type size");

    uint32_t uNumBModels;
    memcpy(&uNumBModels, data, 4);  // количество BModel'ей
    data += 4;

    std::vector<BSPModelData> models;
    for (unsigned int i = 0; i < uNumBModels; ++i) {
        BSPModelData model;
        memcpy(&model, data, sizeof(BSPModelData));
        data += sizeof(BSPModelData);
        models.push_back(model);
    }

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
        new_model.sMinX = model.sMinX;
        new_model.sMinY = model.sMinY;
        new_model.sMinZ = model.sMinZ;
        new_model.sMaxX = model.sMaxX;
        new_model.sMaxY = model.sMaxY;
        new_model.sMaxZ = model.sMaxZ;
        new_model.sSomeOtherMinX = model.sSomeOtherMinX;
        new_model.sSomeOtherMinY = model.sSomeOtherMinY;
        new_model.sSomeOtherMinZ = model.sSomeOtherMinZ;
        new_model.sSomeOtherMaxX = model.sSomeOtherMaxX;
        new_model.sSomeOtherMaxY = model.sSomeOtherMaxY;
        new_model.sSomeOtherMaxZ = model.sSomeOtherMaxZ;
        new_model.vBoundingCenter = model.vBoundingCenter;
        new_model.sBoundingRadius = model.sBoundingRadius;

        unsigned int verticesSize = model.uNumVertices * sizeof(Vec3_int_);
        new_model.pVertices.pVertices = (Vec3_int_ *)malloc(verticesSize);
        memcpy(new_model.pVertices.pVertices, data, verticesSize);
        data += verticesSize;

        for (size_t i = 0; i < model.uNumFaces; i++) {
            new_model.pFaces.emplace_back();
            ODMFace &face = new_model.pFaces.back();
            face.index = i;
            face.Deserialize((ODMFace_MM7 *)data);
            data += sizeof(ODMFace_MM7);
        }

        unsigned int facesOrderingSize = model.uNumFaces * sizeof(short);
        new_model.pFacesOrdering = (uint16_t *)malloc(facesOrderingSize);
        memcpy(new_model.pFacesOrdering, data, facesOrderingSize);
        data += facesOrderingSize;

        for (size_t i = 0; i < model.uNumNodes; i++) {
            BSPNode node;
            memcpy(&node, data, sizeof(BSPNode));
            new_model.pNodes.push_back(node);
            data += sizeof(BSPNode);
        }

        const char *textureFilenames =
            (const char *)malloc(10 * model.uNumFaces);
        memcpy((char *)textureFilenames, data, 10 * model.uNumFaces);
        data += 10 * model.uNumFaces;
        for (unsigned int j = 0; j < model.uNumFaces; ++j) {
            const char *texFilename = &textureFilenames[j * 10];
            new_model.pFaces[j].SetTexture(texFilename);
            if (new_model.pFaces[j].sCogTriggeredID) {
                if (new_model.pFaces[j].HasEventHint())
                    new_model.pFaces[j].uAttributes |= FACE_HAS_EVENT;
                else
                    new_model.pFaces[j].uAttributes &= ~FACE_HAS_EVENT;
            }
        }

        free((void *)textureFilenames);

        model_index++;
    }

    return data;
}

void BSPModel::Release() {
    free(this->pVertices.pVertices);
    this->pVertices.pVertices = 0;
    free(this->pFacesOrdering);
    this->pFacesOrdering = nullptr;
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

void ODMFace::SetTexture(const String &filename) {
    if (this->IsTextureFrameTable()) {
        this->resource = (void *)pTextureFrameTable->FindTextureByName(filename.c_str());
        if (this->resource != (void *)-1) {
            return;
        }

        this->ToggleIsTextureFrameTable();
    }

    this->resource = assets->GetBitmap(filename);
}

bool ODMFace::Deserialize(ODMFace_MM7 *mm7) {
    memcpy(&this->pFacePlane, &mm7->pFacePlane, sizeof(this->pFacePlane));
    this->zCalc1 = mm7->zCalc1;
    this->zCalc2 = mm7->zCalc2;
    this->zCalc3 = mm7->zCalc3;
    this->uAttributes = mm7->uAttributes;
    memcpy(this->pVertexIDs, mm7->pVertexIDs, sizeof(this->pVertexIDs));
    memcpy(this->pTextureUIDs, mm7->pTextureUIDs, sizeof(this->pTextureUIDs));
    memcpy(this->pTextureVIDs, mm7->pTextureVIDs, sizeof(this->pTextureVIDs));
    memcpy(this->pXInterceptDisplacements, mm7->pXInterceptDisplacements,
           sizeof(this->pXInterceptDisplacements));
    memcpy(this->pYInterceptDisplacements, mm7->pYInterceptDisplacements,
           sizeof(this->pYInterceptDisplacements));
    memcpy(this->pZInterceptDisplacements, mm7->pZInterceptDisplacements,
           sizeof(this->pZInterceptDisplacements));
    this->resource = nullptr;
    this->sTextureDeltaU = mm7->sTextureDeltaU;
    this->sTextureDeltaV = mm7->sTextureDeltaV;
    memcpy(&this->pBoundingBox, &mm7->pBoundingBox, sizeof(this->pBoundingBox));
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
    this->uPolygonType = mm7->uPolygonType;
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
    while (pLevelEVT_Index[event_index].uEventID != this->sCogTriggeredID) {
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
