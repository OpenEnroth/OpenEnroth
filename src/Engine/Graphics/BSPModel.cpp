#include "Engine/Graphics/BSPModel.h"

#include "Engine/AssetsManager.h"
#include "Engine/Tables/TextureFrameTable.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Time/Timer.h"
#include "Engine/Graphics/Indoor.h"

// ODMFace

GraphicsImage *ODMFace::GetTexture() {
    if (this->IsAnimated()) {
        // TODO(captainurist): probably should be pMiscTimer, not pEventTimer.
        return pTextureFrameTable->animationFrame(this->animationId, pEventTimer->time());
    } else {
        return this->texture;
    }
}

void ODMFace::SetTexture(std::string_view filename) {
    if (this->IsAnimated()) {
        this->animationId = pTextureFrameTable->animationId(filename);
        if (this->animationId != -1) {
            return;
        }

        // Failed to find animated texture so disable
        this->ToggleIsAnimated();
    }

    this->texture = assets->getBitmap(filename);
    this->texlayer = -1;
    this->texunit = -1;
}

bool ODMFace::HasEventHint() {
    return hasEventHint(this->eventId);
}

bool ODMFace::Contains(const Vec3f &pos, int model_idx, int slack, FaceAttributes override_plane) const {
    BLVFace face;
    face.FromODM(this);
    return face.Contains(pos, model_idx, slack, override_plane);
}
