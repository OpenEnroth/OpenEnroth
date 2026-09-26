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
    void Reset(uint8_t uMaxProgress);
    void Progress();
    void Release();
    void Draw();

    bool IsActive();

    /**
     * @return                          Number of loading screens started so far, including the ones that weren't drawn
     *                                  because of `no_loading_screen`.
     */
    int startCount() const {
        return _startCount;
    }

 protected:
    int uX = 0;
    int uY = 0;
    int uWidth = 0;
    int uHeight = 0;

    uint8_t uProgressMax = 0;
    uint8_t uProgressCurrent = 0;
    Type uType = TYPE_None;

    GraphicsImage *progressbar_dungeon = nullptr;  // struct Texture_MM7 pBardata;
    GraphicsImage *progressbar_loading = nullptr;  // struct Texture_MM7 pLoadingProgress;
    GraphicsImage *loading_bg = nullptr;
    int turnHourIconId = 0;
    int _startCount = 0;
};

extern GUIProgressBar *pGameLoadingUI_ProgressBar;
