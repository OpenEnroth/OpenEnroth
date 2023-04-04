#pragma once
#include <memory>
#include <string>

#include "Engine/Graphics/HWLContainer.h"
#include "Engine/Graphics/IRender.h"

class RenderBase : public IRender {
 public:
    inline RenderBase(
        std::shared_ptr<GameConfig> config,
        DecalBuilder *decal_builder,
        LightmapBuilder *lightmap_builder,
        SpellFxRenderer *spellfx,
        std::shared_ptr<ParticleEngine> particle_engine,
        Vis *vis,
        Logger *logger
    ) : IRender(config, decal_builder, lightmap_builder, spellfx, particle_engine, vis, logger) {
    }

    virtual bool Initialize() override;

    virtual void TransformBillboardsAndSetPalettesODM() override;
    virtual void DrawSpriteObjects() override;
    virtual void PrepareDecorationsRenderList_ODM() override;
    virtual void MakeParticleBillboardAndPush(SoftwareBillboard *a2,
                                                  Texture *texture,
                                                  unsigned int uDiffuse,
                                                  int angle) override;
    virtual float GetGamma() override;

    virtual HWLTexture *LoadHwlBitmap(const std::string &name) override;
    virtual HWLTexture *LoadHwlSprite(const std::string &name) override;

    virtual void SavePCXScreenshot() override;
    virtual void SavePCXImage32(const std::string &filename, const uint32_t *picture_data, const int width, const int height);
    virtual void SaveScreenshot(const std::string &filename, unsigned int width, unsigned int height) override;
    /**
    * @param width                         Final width of image to create.
    * @param height                        Final height of image to create.
    * @return                              Returns Blob containing packed pcx data and its size.
    */
    virtual Blob PackScreenshot(const unsigned int width, const unsigned int height) override;
    virtual Image *TakeScreenshot(unsigned int width, unsigned int height) override;

    virtual void DrawMasked(float u, float v, class Image *img,
        unsigned int color_dimming_level, uint32_t mask = 0xFFFFFFFF) override;
    virtual void DrawTextureGrayShade(float u, float v, class Image *a4) override;
    virtual void DrawTransparentRedShade(float u, float v, class Image *a4) override;
    virtual void DrawTransparentGreenShade(float u, float v, class Image *pTexture) override;
    virtual void ClearBlack() override;
    virtual void BillboardSphereSpellFX(struct SpellFX_Billboard *a1, int diffuse) override;
    virtual void DrawMonsterPortrait(Recti rc, SpriteFrame *Portrait_Sprite, int Y_Offset) override;
    virtual void DrawSpecialEffectsQuad(Texture *texture, int palette) override;
    virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() override;
    virtual void PresentBlackScreen() override;

 protected:
    unsigned int Billboard_ProbablyAddToListAndSortByZOrder(float z);
    void TransformBillboard(SoftwareBillboard *a2, RenderBillboard *pBillboard);

    HWLContainer pD3DBitmaps;
    HWLContainer pD3DSprites;
};
