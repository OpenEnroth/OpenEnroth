#include "Engine/Graphics/BSPModel.h"

#include <cstring>
#include <cstdlib>
#include <algorithm>

#include "Engine/AssetsManager.h"
#include "Engine/Events.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Time.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Serialization/Deserializer.h"
#include "Engine/Serialization/LegacyImages.h"


void BSPModelList::Load(Deserializer *stream) {
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

        stream->ReadSizedLegacyVector<ODMFace_MM7>(&new_model.pFaces, model.uNumFaces);
        for (size_t i = 0; i < new_model.pFaces.size(); i++)
            new_model.pFaces[i].index = i;

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
