#pragma once

#include "IDebugViewRenderer.h"

#include <Engine/Graphics/Renderer/OpenGLShader.h>

#include <memory>

struct nk_state;
struct nk_context;

struct nk_tex_font {
    uint32_t texid;
    struct nk_font *font;
};

class GraphicsImage;

class NuklearDebugViewRenderer : public IDebugViewRenderer {
 public:
    NuklearDebugViewRenderer(nk_context *context, bool useOGLES);
    ~NuklearDebugViewRenderer();
    void render(const Sizei &outputPresent, int &drawCalls) override;
    void reloadShaders() override;

    struct nk_image loadImage(GraphicsImage *img);
    void unloadImage(GraphicsImage *img);

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
