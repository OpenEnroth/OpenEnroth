#pragma once

#include <cstdint>

class Image;

class GUIProgressBar {
 public:
    enum Type {
        TYPE_None = 0,
        TYPE_Fullscreen = 1,
        TYPE_Box = 2
    };

 public:
    GUIProgressBar();

    bool Initialize(Type type);
    void Reset(uint8_t uMaxProgress);
    void Progress();
    void Release();
    void Draw();

    bool IsActive();

 protected:
    int uX;
    int uY;
    int uWidth;
    int uHeight;

    uint8_t uProgressMax;
    uint8_t uProgressCurrent;
    Type uType;

    Image *progressbar_dungeon;  // struct Texture_MM7 pBardata;
    Image *progressbar_loading;  // struct Texture_MM7 pLoadingProgress;

    Image *loading_bg;
};

extern GUIProgressBar *pGameLoadingUI_ProgressBar;
