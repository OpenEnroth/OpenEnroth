#pragma once
#include "Core/UIControl.h"
#include "..\..\Engine/Graphics/Render.h"


class MainMenuWindow: public UIControl
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
    // UIControl
    virtual bool OnMouseLeftClick(int x, int y) override;
    // UIControl
    virtual bool OnMouseRightClick(int x, int y) override;
    // UIControl
    //virtual bool OnMouseEnter() override;
    // UIControl
    //virtual bool OnMouseLeave() override;

  private:
	RGBTexture *background_texture;
};