#pragma once

#include "Engine/Graphics/IRender.h"

class RenderBase : public IRender {
 public:
    explicit RenderBase()
        : IRender() {
    }

    virtual void TransformBillboardsAndSetPalettesODM();
    virtual void DrawSpriteObjects_ODM();
    virtual void MakeParticleBillboardAndPush_BLV(SoftwareBillboard *a2,
                                                  Texture *texture,
                                                  unsigned int uDiffuse,
                                                  int angle);
    virtual void MakeParticleBillboardAndPush_ODM(SoftwareBillboard *a2,
                                                  Texture *texture,
                                                  unsigned int uDiffuse,
                                                  int angle);

 protected:
    OSWindow *window;

    unsigned int Billboard_ProbablyAddToListAndSortByZOrder(float z);
    void TransformBillboard(SoftwareBillboard *a2, RenderBillboard *pBillboard);

    void PostInitialization();
};
