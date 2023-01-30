#pragma once
#include <memory>
#include <string>

#include "Engine/Graphics/HWLContainer.h"
#include "Engine/Graphics/IRender.h"

class RenderBase : public IRender {
 public:
    inline RenderBase(
        std::shared_ptr<Application::GameConfig> config,
        DecalBuilder* decal_builder,
        LightmapBuilder* lightmap_builder,
        SpellFxRenderer* spellfx,
        std::shared_ptr<ParticleEngine> particle_engine,
        Vis* vis,
        Logger* logger
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
    virtual void SavePCXImage16(const std::string& filename, uint16_t* picture_data, int width, int height);
    virtual void SaveScreenshot(const std::string& filename, unsigned int width, unsigned int height) override;
    virtual void PackScreenshot(unsigned int width, unsigned int height,
        void* out_data, unsigned int data_size, unsigned int* screenshot_size) override;
    virtual Image* TakeScreenshot(unsigned int width, unsigned int height) override;

    virtual void DrawMasked(float u, float v, class Image* img,
        unsigned int color_dimming_level, uint32_t mask = 0xFFFFFFFF) override;
    virtual void DrawTextureGrayShade(float u, float v, class Image* a4) override;
    virtual void DrawTransparentRedShade(float u, float v, class Image* a4) override;
    virtual void DrawTransparentGreenShade(float u, float v, class Image* pTexture) override;

 protected:
    unsigned int Billboard_ProbablyAddToListAndSortByZOrder(float z);
    void TransformBillboard(SoftwareBillboard *a2, RenderBillboard *pBillboard);

    HWLContainer pD3DBitmaps;
    HWLContainer pD3DSprites;
};
