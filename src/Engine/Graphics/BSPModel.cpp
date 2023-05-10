#include "Engine/Graphics/BSPModel.h"

#include <cstring>
#include <cstdlib>
#include <algorithm>

#include "Engine/AssetsManager.h"
#include "Engine/Events/Processor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Time.h"
#include "Engine/Graphics/Indoor.h"

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
    return hasEventHint(this->sCogTriggeredID);
}

bool ODMFace::Contains(const Vec3i &pos, int model_idx, int slack, FaceAttributes override_plane) const {
    BLVFace face;
    face.uAttributes = this->uAttributes;
    face.uNumVertices = this->uNumVertices;
    face.pVertexIDs = const_cast<int16_t *>(this->pVertexIDs.data());
    return face.Contains(pos, model_idx, slack, override_plane);
}
