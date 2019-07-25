#pragma once

#include "Engine/Graphics/HWLContainer.h"
#include "Engine/Graphics/IRender.h"

class RenderBase : public IRender {
 public:
    inline RenderBase()
        : IRender(), window(nullptr) {
    }

    virtual bool Initialize(OSWindow *window);

    virtual void TransformBillboardsAndSetPalettesODM();
    virtual void DrawSpriteObjects_ODM();
    virtual void MakeParticleBillboardAndPush(SoftwareBillboard *a2,
                                                  Texture *texture,
                                                  unsigned int uDiffuse,
                                                  int angle);

    virtual HWLTexture *LoadHwlBitmap(const String &name);
    virtual HWLTexture *LoadHwlSprite(const String &name);

 protected:
    OSWindow *window;

    unsigned int Billboard_ProbablyAddToListAndSortByZOrder(float z);
    void TransformBillboard(SoftwareBillboard *a2, RenderBillboard *pBillboard);

    void PostInitialization();

    HWLContainer pD3DBitmaps;
    HWLContainer pD3DSprites;
};
