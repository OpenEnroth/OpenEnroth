#pragma once

#include "IOverlayRenderer.h"

#include <Engine/Graphics/Renderer/OpenGLShader.h>

#include <memory>

struct nk_state;
struct nk_context;

struct nk_tex_font {
    uint32_t texid;
    struct nk_font *font;
};

class GraphicsImage;

class NuklearOverlayRenderer : public IOverlayRenderer {
 public:
    NuklearOverlayRenderer(nk_context *context, bool useOGLES);
    ~NuklearOverlayRenderer();
    virtual void render(const Sizei &outputPresent, int *drawCalls) override;
    virtual void reloadShaders() override;

 private:
    bool _createDevice();
    struct nk_tex_font* _loadFont(const char *font_path, size_t font_size);
    void _cleanup();

    std::unique_ptr<nk_state> _state;
    nk_context *_context;
    bool _useOGLES;
    OpenGLShader _shader;
    nk_tex_font *_defaultFont;
};
