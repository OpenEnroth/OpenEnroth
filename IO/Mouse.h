#pragma once
#include "OSWindow.h"

#include "Engine/VectorTypes.h"



/*  107 */
#pragma pack(push, 1)
class ThreadWard
{
public:
    //----- (00438B0B) --------------------------------------------------------
    inline ThreadWard()
    {
        InitializeCriticalSection(&csAsyncMouse);
        InitializeCriticalSection(&cs2);
        InitializeCriticalSection(&cs3);
        InitializeCriticalSection(&csAsyncKeyboard);
    }

    //----- (00438B54) --------------------------------------------------------
    virtual ~ThreadWard()
    {
        ThreadWard *v1; // esi@1

        v1 = this;
        //this->vdestructor_ptr = &ThreadWard_destructors;
        DeleteCriticalSection(&this->csAsyncKeyboard);
        DeleteCriticalSection(&v1->cs3);
        DeleteCriticalSection(&v1->cs2);
        DeleteCriticalSection(&v1->csAsyncMouse);
    }

    void(***vdestructor_ptr)(ThreadWard *, bool);
    _RTL_CRITICAL_SECTION csAsyncMouse;
    _RTL_CRITICAL_SECTION cs2;
    _RTL_CRITICAL_SECTION cs3;
    _RTL_CRITICAL_SECTION csAsyncKeyboard;
};
#pragma pack(pop)

/*  106 */
#pragma pack(push, 1)
class Mouse
{
    public:
        //----- (00467E4E) --------------------------------------------------------
        inline Mouse() :
            window(nullptr),
            cursor_img(nullptr)
        {
            pCursorBitmap_sysmem = nullptr;
            field_34 = 0;
            pCursorBitmap2_sysmem = nullptr;
            pCursorBitmap3_sysmembits_16bit = nullptr;
            ptr_90 = nullptr;
            pWard = nullptr;
            uMouseClickX = 0;
            uMouseClickY = 0;
        }

        void GetClickPos(unsigned int *pX, unsigned int *pY);
        void RemoveHoldingItem();
        void SetCursorBitmapFromItemID(unsigned int uItemID);
        void SetCurrentCursorBitmap();
        void SetCursorImage(const String &name);
        LONG _469AE4();
        void ClearCursor();
        void AllocCursorSystemMem();
        void *DoAllocCursorMem();
        POINT *GetCursorPos(POINT *p);
        void Initialize(OSWindow *window);
        void SetActive(bool active);
        void Deactivate();
        void DrawCursor();
        void Activate();
        void _469E24();
        void DrawCursorToTarget();
        void ReadCursorWithItem();
        void ChangeActivation(int a1);
        void SetMouseClick(int x, int y);

        static void UI_OnMouseLeftClick(int *pXY); // idb
        static bool UI_OnKeyDown(unsigned int vkKey);


        unsigned int uPointingObjectID;
        unsigned int bActive;
        int field_8;
        int field_C;
        unsigned int bInitialized;
        unsigned int bRedraw;
        int field_18;
        int field_1C;
        int field_20;
        Image *cursor_img;
        OSWindow *window; //HWND hWnd;
        unsigned __int16 *pCursorBitmap_sysmem;
        int field_34;
        unsigned __int8 *pCursorBitmap2_sysmem;
        unsigned __int16 *pCursorBitmap3_sysmembits_16bit;
        int uCursorWithItemX;
        int uCursorWithItemY;
        int uCursorWithItemW;
        int uCursorWithItemZ;
        int field_50;
        int field_54;
        int uCursorBitmapPitch;
        int field_5C[13];
        void *ptr_90;
        int field_94;
        int field_98;
        int field_9C;
        int field_A0;
        int field_A4;
        int field_A8;
        int field_AC;
        int field_B0;
        int field_B4;
        int field_B8;
        int field_BC;
        int field_C0;
        Vec2_int_ pCursorBitmapPos;
        int uCursorBitmapWidth;
        int uCursorBitmapHeight;
        int field_D4;
        int field_D8;
        int field_DC;
        int field_E0;
        Vec4_int_ pCursorBitmapRect;
        char field_F4;
        String cursor_name;//char pCurrentCursorName[11];
        int field_100;
        int field_104;
        unsigned int uMouseClickX;
        unsigned int uMouseClickY;
        void/*ThreadWard*/ *pWard;
};
#pragma pack(pop)





extern Mouse *pMouse;