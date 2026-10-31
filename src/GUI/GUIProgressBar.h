#pragma once

#include <cstdint>

class GraphicsImage;

class GUIProgressBar {
 public:
    enum class Type {
        TYPE_None = 0,
        TYPE_Fullscreen = 1,
        TYPE_Box = 2
    };
    using enum Type;

    bool Initialize(Type type);
    void Reset();
    void Progress();
    void Release();
    void Draw();

    bool IsActive();

    int percent() const {
        return 100 * uProgressCurrent / PROGRESS_STEPS;
    }

 protected:
    static constexpr int PROGRESS_STEPS = 8; // Calls to Progress() in a map load, the bar is full after the last one.

    int uX = 0;
    int uY = 0;
    int uWidth = 0;
    int uHeight = 0;

    int uProgressCurrent = 0;
    Type uType = TYPE_None;

    GraphicsImage *progressbar_dungeon = nullptr;  // struct Texture_MM7 pBardata;
    GraphicsImage *progressbar_loading = nullptr;  // struct Texture_MM7 pLoadingProgress;
    GraphicsImage *loading_bg = nullptr;
    int turnHourIconId = 0;
};

extern GUIProgressBar *pGameLoadingUI_ProgressBar;
