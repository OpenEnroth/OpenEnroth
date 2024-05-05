#include "Engine/Tables/IconFrameTable.h"

#include "Engine/AssetsManager.h"

#include "Utility/String/Ascii.h"

GraphicsImage *Icon::GetTexture() {
    if (!this->img) {
        this->img = assets->getImage_ColorKey(this->pTextureName);
    }

    return this->img;
}

Icon *IconFrameTable::GetIcon(unsigned int idx) {
    if (idx < pIcons.size()) return &this->pIcons[idx];
    return nullptr;
}

Icon *IconFrameTable::GetIcon(const char *pIconName) {
    for (unsigned int i = 0; i < pIcons.size(); i++) {
        if (ascii::noCaseEquals(pIconName, this->pIcons[i].GetAnimationName()))
            return &this->pIcons[i];
    }
    return nullptr;
}

//----- (00494F3A) --------------------------------------------------------
unsigned int IconFrameTable::FindIcon(std::string_view pIconName) {
    for (size_t i = 0; i < pIcons.size(); i++) {
        if (ascii::noCaseEquals(pIconName, this->pIcons[i].GetAnimationName()))
            return i;
    }
    return 0;
}

//----- (00494F70) --------------------------------------------------------
Icon *IconFrameTable::GetFrame(unsigned int uIconID, Duration frame_time) {
    if (this->pIcons[uIconID].uFlags & 1 && this->pIcons[uIconID].GetAnimLength()) {
        Duration t = frame_time;

        t = t % this->pIcons[uIconID].GetAnimLength();

        int i;
        for (i = uIconID; t >= this->pIcons[i].GetAnimTime(); i++)
            t -= this->pIcons[i].GetAnimTime();
        return &this->pIcons[i];
    } else {
        return &this->pIcons[uIconID];
    }
}
