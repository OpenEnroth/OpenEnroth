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
    virtual void MakeParticleBillboardAndPush(const Particle& p) override;
    virtual float GetGamma() override;

    virtual void BillboardSphereSpellFX(SpellFX_Billboard *a1, Color diffuse) override;
    virtual void DrawMonsterPortrait(const Recti &rc, SpriteFrame *Portrait_Sprite, int Y_Offset) override;
    virtual void DrawSpecialEffectsQuad(GraphicsImage *texture, int palette) override;
    virtual void DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() override;

    virtual std::vector<Actor*> getActorsInViewport(int pDepth) override;

    bool Reinitialize(bool firstInit) override;

    virtual Sizei GetRenderDimensions() override;
    virtual Sizei GetPresentDimensions() override;
    virtual Pointi MapToRender(Pointi position) override;
    virtual Pointi MapToPresent(Pointi position) override;

 protected:
    unsigned int NextBillboardIndex();
    void SortBillboards();
    void TransformBillboard(const SoftwareBillboard *a2, const RenderBillboard *pBillboard);

 protected:
    Sizei outputRender = {0, 0};
    Sizei outputPresent = {0, 0};

 private:
    void updateRenderDimensions();
};
