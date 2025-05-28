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
    }

    Pointi position() const;
    void setPosition(Pointi position);

    void RemoveHoldingItem();
    void SetCursorBitmapFromItemID(ItemId uItemID);
    void SetCurrentCursorBitmap();
    void SetCursorImage(std::string_view name);
    void ClearCursor();
    void AllocCursorSystemMem();
    void *DoAllocCursorMem();
    void Initialize();
    void DrawCursor();
    void DrawPickedItem();

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
    Pointi pickedItemOffset = {}; // Offset of the item's bitmap relative to cursor position, always non-positive.
    Pointi pCursorBitmapPos{};
    std::string cursor_name;
    Pointi _position;

    bool _mouseLook = false;
    Pointi _mouseLookChange;
};
}  // namespace Io

extern std::shared_ptr<Io::Mouse> mouse;
