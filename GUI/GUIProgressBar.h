#pragma once

#include "Engine/Graphics/Image.h"

class GUIProgressBar {
 public:
    enum Type {
        TYPE_None = 0,
        TYPE_Fullscreen = 1,
        TYPE_Box = 2
    };

    bool Initialize(Type type);
    void Reset(uint8_t uMaxProgress);
    void Progress();
    void Release();
    void Draw();

    int16_t uX;
    int16_t uY;
    int16_t uWidth;
    int16_t uHeight;
    char uProgressMax;
    char uProgressCurrent;
    Type uType;
    char field_10[8];
    struct Texture_MM7 field_E0;
    class Image *progressbar_dungeon;  // struct Texture_MM7 pBardata;
    class Image *progressbar_loading;  // struct Texture_MM7 pLoadingProgress;

    inline GUIProgressBar() : loading_bg(nullptr) {}

    class Image *loading_bg;
};

extern GUIProgressBar *pGameLoadingUI_ProgressBar;
