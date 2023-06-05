#include "Engine/Graphics/OpenGL/TextureOpenGL.h"

#include <cassert>
#include <utility>

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/ErrorHandling.h"

Texture *TextureOpenGL::Create(RgbaImage image) {
    TextureOpenGL *tex = new TextureOpenGL(false);

    tex->_rgbaImage = std::move(image);
    tex->_initialized = true;
    tex->_initialized = render->MoveTextureToDevice(tex);
    assert(tex->_initialized);

    return tex;
}

Texture *TextureOpenGL::Create(unsigned int width, unsigned int height) {
    return Create(RgbaImage::solid(width, height, Color()));
}

Texture *TextureOpenGL::Create(std::unique_ptr<ImageLoader> loader) {
    auto tex = new TextureOpenGL();
    tex->_loader = std::move(loader);
    return tex;
}

int TextureOpenGL::GetOpenGlTexture(bool bLoad) {
    if (bLoad) {
        if (!this->_initialized) {
            this->LoadImageData();
        }

        // texture needs reloading to gpu
        if (this->ogl_texture < 0) {
            render->MoveTextureToDevice(this);
        }
    }

    return this->ogl_texture;
}

bool TextureOpenGL::LoadImageData() {
    if (!this->_initialized) {
        this->_initialized = this->_loader->Load(&_rgbaImage, &_indexedImage, &_palette);
        if (this->_initialized) {
            this->_initialized = render->MoveTextureToDevice(this);
            if (!this->_initialized) {
                __debugbreak();
            }
        }
    }

    return this->_initialized;
}

TextureOpenGL::~TextureOpenGL() {
    if (this->ogl_texture != -1) render->DeleteTexture(this);
}
