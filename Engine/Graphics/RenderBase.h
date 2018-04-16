#pragma once

#include "Engine/Graphics/Configuration.h"
#include "Engine/Graphics/IRender.h"

class RenderBase : public IRender {
    public:
        RenderBase(Graphics::Configuration *config)
            : IRender(config)
        {}

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
    unsigned int Billboard_ProbablyAddToListAndSortByZOrder(float z);
    void TransformBillboard(SoftwareBillboard *a2, RenderBillboard *pBillboard);
};
