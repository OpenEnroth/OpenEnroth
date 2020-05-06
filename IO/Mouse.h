#pragma once
#include <memory>
#include <string>

#include "Engine/Point.h"

#include "Io/GameKey.h"
#include "Io/IMouseController.h"

class Image;
class OSWindow;

namespace Io {
    class Mouse {
    public:
        inline Mouse() : cursor_img(nullptr), window(nullptr) {
            pCursorBitmap_sysmem = nullptr;
            field_34 = 0;
            pCursorBitmap2_sysmem = nullptr;
            pPickedItem = nullptr;
            ptr_90 = nullptr;
            uMouseClickX = 0;
            uMouseClickY = 0;
        }

        void GetClickPos(unsigned int* pX, unsigned int* pY);
        void RemoveHoldingItem();
        void SetCursorBitmapFromItemID(unsigned int uItemID);
        void SetCurrentCursorBitmap();
        void SetCursorImage(const std::string &name);
        void _469AE4();
        void ClearCursor();
        void AllocCursorSystemMem();
        void* DoAllocCursorMem();
        Point GetCursorPos();
        void Initialize(std::shared_ptr<OSWindow> window);
        void SetActive(bool active);
        void Deactivate();
        void DrawCursor();
        void Activate();
        void ClearPickedItem();
        void DrawCursorToTarget();
        void ReadCursorWithItem();
        void ChangeActivation(int a1);
        void SetMouseClick(int x, int y);

        void UI_OnMouseLeftClick();

        unsigned int uPointingObjectID;
        unsigned int bActive = 0;
        int field_8;
        int field_C;
        unsigned int bInitialized = 0;
        unsigned int bRedraw = 0;
        int field_18 = 0;
        int field_1C = 0;
        int field_20 = 0;
        Image* cursor_img;
        std::shared_ptr<OSWindow> window;
        uint16_t* pCursorBitmap_sysmem;
        int field_34;
        uint8_t* pCursorBitmap2_sysmem;
        Image* pPickedItem;
        int uCursorWithItemX;
        int uCursorWithItemY;
        int field_50 = 0;
        int field_54 = 0;
        int uCursorBitmapPitch;
        int field_5C[13];
        void* ptr_90;
        int field_94;
        int field_98 = 0;
        int field_9C = 0;
        int field_A0 = 0;
        int field_A4 = 0;
        int field_A8 = 0;
        int field_AC = 0;
        int field_B0;
        int field_B4;
        int field_B8;
        int field_BC;
        int field_C0;
        Point pCursorBitmapPos;
        int uCursorBitmapWidth;
        int uCursorBitmapHeight;
        int field_D4;
        int field_D8;
        int field_DC;
        int field_E0;
        int pCursorBitmapRect_x;
        int pCursorBitmapRect_y;
        int pCursorBitmapRect_w;
        int pCursorBitmapRect_z;
        char field_F4;
        std::string cursor_name;
        int field_100 = 0;
        int field_104 = 0;
        unsigned int uMouseClickX;
        unsigned int uMouseClickY;

        std::shared_ptr<IMouseController> mouseController = nullptr;
    };
}

extern std::shared_ptr<Io::Mouse> mouse;
