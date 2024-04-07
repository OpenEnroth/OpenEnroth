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
        field_34 = 0;
        pCursorBitmap2_sysmem = nullptr;
        pPickedItem = nullptr;
        ptr_90 = nullptr;
        uMouseX = 0;
        uMouseY = 0;
    }

    void GetClickPos(int *pX, int *pY);
    void RemoveHoldingItem();
    void SetCursorBitmapFromItemID(ItemId uItemID);
    void SetCurrentCursorBitmap();
    void SetCursorImage(std::string_view name);
    void _469AE4();
    void ClearCursor();
    void AllocCursorSystemMem();
    void *DoAllocCursorMem();
    Pointi GetCursorPos();
    void Initialize();
    void SetActive(bool active);
    void Deactivate();
    void DrawCursor();
    void Activate();
    void ClearPickedItem();
    void DrawCursorToTarget();
    void DrawPickedItem();
    void ChangeActivation(int a1);
    void SetMouseClick(int x, int y);

    void UI_OnMouseLeftClick();

    Pid uPointingObjectID;
    int bActive = 0;
    int field_8 = 0;
    int field_C = 0;
    int bInitialized = 0;
    int field_18 = 0;
    int field_1C = 0;
    int field_20 = 0;
    GraphicsImage *cursor_img = nullptr;
    uint16_t *pCursorBitmap_sysmem = nullptr;
    int field_34 = 0;
    uint8_t *pCursorBitmap2_sysmem = nullptr;
    GraphicsImage *pPickedItem = nullptr;
    int uCursorWithItemX = 0;
    int uCursorWithItemY = 0;
    int field_50 = 0;
    int field_54 = 0;
    int uCursorBitmapPitch = 0;
    int field_5C[13]{};
    void *ptr_90 = nullptr;
    int field_94 = 0;
    int field_98 = 0;
    int field_9C = 0;
    int field_A0 = 0;
    int field_A4 = 0;
    int field_A8 = 0;
    int field_AC = 0;
    int field_B0 = 0;
    int field_B4 = 0;
    int field_B8 = 0;
    int field_BC = 0;
    int field_C0 = 0;
    Pointi pCursorBitmapPos{};
    int uCursorBitmapWidth = 0;
    int uCursorBitmapHeight = 0;
    int field_D4 = 0;
    int field_D8 = 0;
    int field_DC = 0;
    int field_E0 = 0;
    int pCursorBitmapRect_x = 0;
    int pCursorBitmapRect_y = 0;
    int pCursorBitmapRect_w = 0;
    int pCursorBitmapRect_z = 0;
    char field_F4 = 0;
    std::string cursor_name;
    int field_100 = 0;
    int field_104 = 0;
    int uMouseX = 0;
    int uMouseY = 0;
};
}  // namespace Io

extern std::shared_ptr<Io::Mouse> mouse;
