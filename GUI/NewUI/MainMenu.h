#pragma once
#include "Core/UIControl.h"
#include "..\..\Engine/Graphics/Render.h"


class MainMenuWindow : public UIControl
{
    public:
        static MainMenuWindow *Create();

        bool Initialize();

    // UIControl
        virtual void Show() override;

    // UIControl
        virtual bool Focused() override;

    // UIControl
        virtual bool OnKey(int key) override;
        virtual bool OnMouseLeftClick(int x, int y) override;
        virtual bool OnMouseRightClick(int x, int y) override;
        //virtual bool OnMouseEnter() override;
        //virtual bool OnMouseLeave() override;

    private:
};