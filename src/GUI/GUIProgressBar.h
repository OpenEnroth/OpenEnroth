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

    bool Initialize(Type type);
    void Reset(uint8_t uMaxProgress);
    void Progress();
    void Release();
    void Draw();

    bool IsActive();

 protected:
    int uX = 0;
    int uY = 0;
    int uWidth = 0;
    int uHeight = 0;

    uint8_t uProgressMax = 0;
    uint8_t uProgressCurrent = 0;
    Type uType = TYPE_None;

    Image *progressbar_dungeon = nullptr;  // struct Texture_MM7 pBardata;
    Image *progressbar_loading = nullptr;  // struct Texture_MM7 pLoadingProgress;
    Image *loading_bg = nullptr;
};

extern GUIProgressBar *pGameLoadingUI_ProgressBar;
