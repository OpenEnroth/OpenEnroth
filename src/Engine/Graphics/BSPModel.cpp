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
    face.pVertexIDs = const_cast<int16_t *>(this->pVertexIDs.data());
    return face.Contains(pos, model_idx, slack, override_plane);
}
