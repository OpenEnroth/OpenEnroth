#pragma once

#include <memory>
#include <string>

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Pid.h"

#include "Library/Geometry/Point.h"

class GraphicsImage;

namespace Io {
class Mouse {
 public:
    inline Mouse() : cursor_img(nullptr) {
        pCursorBitmap_sysmem = nullptr;
        pCursorBitmap2_sysmem = nullptr;
        uMouseX = 0;
        uMouseY = 0;
    }

    void GetClickPos(int *pX, int *pY);
    void RemoveHoldingItem();
    void SetCursorBitmapFromItemID(ItemId uItemID);
    void SetCurrentCursorBitmap();
    void SetCursorImage(std::string_view name);
    void ClearCursor();
    void AllocCursorSystemMem();
    void *DoAllocCursorMem();
    Pointi GetCursorPos();
    void Initialize();
    void DrawCursor();
    void DrawPickedItem();
    void SetMousePosition(int x, int y);

    void UI_OnMouseLeftClick();

    void SetMouseLook(bool look);
    void ToggleMouseLook();
    void DoMouseLook();

    Pid uPointingObjectID;
    int field_8 = 0;
    bool _arrowCursor = 0;
    int bInitialized = 0;
    GraphicsImage *cursor_img = nullptr;
    uint16_t *pCursorBitmap_sysmem = nullptr;
    uint8_t *pCursorBitmap2_sysmem = nullptr;
    int pickedItemOffsetX = 0;
    int pickedItemOffsetY = 0;
    Pointi pCursorBitmapPos{};
    std::string cursor_name;
    int uMouseX = 0;
    int uMouseY = 0;

    bool _mouseLook = false;
    Pointi _mouseLookChange;
};
}  // namespace Io

extern std::shared_ptr<Io::Mouse> mouse;
