#pragma once

#include <memory>

#include "Engine/Graphics/Image.h"

class TextureOpenGL : public GraphicsImage {
 public:
    int GetOpenGlTexture(bool bLoad = true);

    virtual ~TextureOpenGL();

 protected:
    friend class RenderOpenGL;

    static TextureOpenGL *Create(RgbaImage image);
    static TextureOpenGL *Create(unsigned int width, unsigned int height);
    static TextureOpenGL *Create(std::unique_ptr<ImageLoader> loader);

    void SetOpenGlTexture(int ogl_texture) { this->ogl_texture = ogl_texture; }

    explicit TextureOpenGL(bool lazy_initialization = true)
        : GraphicsImage(lazy_initialization), ogl_texture(-1) {}

    int ogl_texture;

    virtual bool LoadImageData();
};
