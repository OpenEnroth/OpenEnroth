#pragma once

#include <cstdint>
#include <memory>

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
    void Reset(int maxProgress);
    void Progress();
    void Release();
    void Draw();

    bool IsActive();

    int percent() const {
        return 100 * uProgressCurrent / uProgressMax;
    }

 protected:
    int uX = 0;
    int uY = 0;
    int uWidth = 0;
    int uHeight = 0;

    int uProgressMax = 0;
    int uProgressCurrent = 0;
    Type uType = TYPE_None;

    std::shared_ptr<GraphicsImage> progressbar_dungeon;            // struct Texture_MM7 pBardata;
    std::shared_ptr<GraphicsImage> progressbar_loading;            // struct Texture_MM7 pLoadingProgress;
    std::shared_ptr<GraphicsImage> loading_bg;
    int turnHourIconId = 0;
};

extern GUIProgressBar *pGameLoadingUI_ProgressBar;
