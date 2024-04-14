#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Renderer.h"

class BaseRenderer : public Renderer {
 public:
    inline BaseRenderer(
        std::shared_ptr<GameConfig> config,
        DecalBuilder *decal_builder,
        SpellFxRenderer *spellfx,
        std::shared_ptr<ParticleEngine> particle_engine,
        Vis *vis
    ) : Renderer(config, decal_builder, spellfx, particle_engine, vis) {
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
    virtual void SavePCXImage32(std::string_view filename, RgbaImageView image);
    virtual void SaveScreenshot(std::string_view filename, unsigned int width, unsigned int height) override;
    virtual Blob PackScreenshot(const unsigned int width, const unsigned int height) override;
    virtual GraphicsImage *TakeScreenshot(unsigned int width, unsigned int height) override;

    virtual void DrawMasked(float u, float v, GraphicsImage *img,
        unsigned int color_dimming_level, Color mask = colorTable.White) override;
    virtual void DrawTextureGrayShade(float u, float v, GraphicsImage *a4) override;
    virtual void DrawTransparentRedShade(float u, float v, GraphicsImage *a4) override;
    virtual void DrawTransparentGreenShade(float u, float v, GraphicsImage *pTexture) override;
    virtual void ClearBlack() override;
    virtual void BillboardSphereSpellFX(SpellFX_Billboard *a1, Color diffuse) override;
    virtual void DrawMonsterPortrait(Recti rc, SpriteFrame *Portrait_Sprite, int Y_Offset) override;
    virtual void DrawSpecialEffectsQuad(GraphicsImage *texture, int palette) override;
    virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() override;
    virtual void PresentBlackScreen() override;

    virtual std::vector<Actor*> getActorsInViewport(int pDepth) override;

    virtual void CreateZBuffer() override;
    virtual void ClearZBuffer() override;
    virtual void ZDrawTextureAlpha(float u, float v, GraphicsImage *pTexture, int zVal) override;

    bool Reinitialize(bool firstInit) override;

    virtual Sizei GetRenderDimensions() override;
    virtual Sizei GetPresentDimensions() override;
    virtual Pointi mapScreenPointToRender(const Pointi &screenPoint) const override;

    virtual void SaveWinnersCertificate(std::string_view filePath) override;

 protected:
    unsigned int Billboard_ProbablyAddToListAndSortByZOrder(float z);
    void TransformBillboard(const SoftwareBillboard *a2, const RenderBillboard *pBillboard);

 protected:
    Sizei outputRender = {0, 0};
    Sizei outputPresent = {0, 0};

 private:
    void updateRenderDimensions();
};
