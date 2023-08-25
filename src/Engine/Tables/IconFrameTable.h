#pragma once

#include <array>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#include "Utility/Memory/Blob.h"

class GraphicsImage;

class Icon {
 public:
    inline Icon() : img(nullptr) {}

    inline void SetAnimationName(const std::string &name) {
        anim_name = name;
    }
    inline const std::string &GetAnimationName() const { return anim_name; }

    inline void SetAnimLength(unsigned int anim_length) {
        this->anim_length = anim_length;
    }
    inline unsigned int GetAnimLength() const { return this->anim_length; }

    inline void SetAnimTime(unsigned int anim_time) {
        this->anim_time = anim_time;
    }
    inline unsigned int GetAnimTime() const { return this->anim_time; }

    GraphicsImage *GetTexture();

    std::string pTextureName;
    int16_t uFlags = 0;
    int id = 0;

 protected:
    std::string anim_name;
    unsigned int anim_length = 0;
    unsigned int anim_time = 0;
    GraphicsImage *img = nullptr;
};

struct IconFrameTable {
    Icon *GetIcon(unsigned int idx);
    Icon *GetIcon(const char *pIconName);
    unsigned int FindIcon(const std::string &pIconName);
    Icon *GetFrame(unsigned int uIconID, unsigned int frame_time);
    void InitializeAnimation(unsigned int uIconID);
    int FromFileTxt(const char *Args);
    // int GetIconAnimLength(unsigned int uIconID);

    std::vector<Icon> pIcons;
};

class UIAnimation {
 public:
    class Icon *icon;

    int16_t uAnimTime;
    int16_t uAnimLength;
    int16_t x;
    int16_t y;
};

extern struct IconFrameTable *pIconsFrameTable;

extern UIAnimation *pUIAnim_Food;
extern UIAnimation *pUIAnim_Gold;
extern UIAnimation *pUIAnum_Torchlight;
extern UIAnimation *pUIAnim_WizardEye;

extern std::array<UIAnimation *, 4> pUIAnims;
