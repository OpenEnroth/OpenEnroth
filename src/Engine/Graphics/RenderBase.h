#pragma once

#include <memory>
#include <string>

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
                                              GraphicsImage *texture,
                                              Color uDiffuse,
                                              int angle) override;
    virtual float GetGamma() override;

    virtual void SavePCXScreenshot() override;
    virtual void SavePCXImage32(const std::string &filename, RgbaImageView image);
    virtual void SaveScreenshot(const std::string &filename, unsigned int width, unsigned int height) override;
    /**
    * @param width                         Final width of image to create.
    * @param height                        Final height of image to create.
    * @return                              Returns Blob containing packed pcx data and its size.
    */
    virtual Blob PackScreenshot(const unsigned int width, const unsigned int height) override;
    virtual GraphicsImage *TakeScreenshot(unsigned int width, unsigned int height) override;

    virtual void DrawMasked(float u, float v, class GraphicsImage *img,
        unsigned int color_dimming_level, Color mask = colorTable.White) override;
    virtual void DrawTextureGrayShade(float u, float v, class GraphicsImage *a4) override;
    virtual void DrawTransparentRedShade(float u, float v, class GraphicsImage *a4) override;
    virtual void DrawTransparentGreenShade(float u, float v, class GraphicsImage *pTexture) override;
    virtual void ClearBlack() override;
    virtual void BillboardSphereSpellFX(struct SpellFX_Billboard *a1, Color diffuse) override;
    virtual void DrawMonsterPortrait(Recti rc, SpriteFrame *Portrait_Sprite, int Y_Offset) override;
    virtual void DrawSpecialEffectsQuad(GraphicsImage *texture, int palette) override;
    virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() override;
    virtual void PresentBlackScreen() override;

    virtual GraphicsImage *CreateTexture_Paletted(const std::string &name) override;
    virtual GraphicsImage *CreateTexture_ColorKey(const std::string &name, Color colorkey) override;
    virtual GraphicsImage *CreateTexture_Solid(const std::string &name) override;
    virtual GraphicsImage *CreateTexture_Alpha(const std::string &name) override;

    virtual GraphicsImage *CreateTexture_PCXFromFile(const std::string &name) override;
    virtual GraphicsImage *CreateTexture_PCXFromIconsLOD(const std::string &name) override;
    virtual GraphicsImage *CreateTexture_PCXFromNewLOD(const std::string &name) override;
    virtual GraphicsImage *CreateTexture_PCXFromLOD(LOD::File *pLOD, const std::string &name) override;

    virtual GraphicsImage *CreateTexture_Blank(unsigned int width, unsigned int height) override;
    virtual GraphicsImage *CreateTexture_Blank(RgbaImage image) override;

    virtual GraphicsImage *CreateTexture(const std::string &name) override;
    virtual GraphicsImage *CreateSprite(const std::string &name) override;

 protected:
    unsigned int Billboard_ProbablyAddToListAndSortByZOrder(float z);
    void TransformBillboard(const SoftwareBillboard *a2, const RenderBillboard *pBillboard);
};
