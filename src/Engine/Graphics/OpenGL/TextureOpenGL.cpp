#include "Engine/Graphics/OpenGL/TextureOpenGL.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/ErrorHandling.h"

Texture *TextureOpenGL::Create(unsigned int width, unsigned int height, const Color *pixels) {
    TextureOpenGL *tex = new TextureOpenGL(false);

    tex->initialized = true;
    tex->width = width;
    tex->height = height;
    unsigned int num_pixels = tex->GetWidth() * tex->GetHeight();
    unsigned int num_pixels_bytes = num_pixels * sizeof(Color);
    tex->pixels = new Color[num_pixels];
    if (pixels) {
        memcpy(tex->pixels, pixels, num_pixels_bytes);
    } else {
        memset(tex->pixels, 0, num_pixels_bytes);
    }

    if (tex->initialized) {
        tex->initialized = render->MoveTextureToDevice(tex);
        if (!tex->initialized) {
            __debugbreak();
        }
    }

    return tex;
}

Texture *TextureOpenGL::Create(ImageLoader *loader) {
    auto tex = new TextureOpenGL();
    if (tex) {
        tex->loader = loader;
    }

    return tex;
}

int TextureOpenGL::GetOpenGlTexture(bool bLoad) {
    if (bLoad) {
        if (!this->initialized) {
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
    if (!this->initialized) {
        Color *pixels = nullptr;
        uint8_t *palette = nullptr;
        uint8_t *palettepixels = nullptr;

        this->initialized = this->loader->Load(&width, &height, &pixels, &palette, &palettepixels);
        if (this->initialized) {
            this->pixels = pixels;
            this->palette24 = palette;
            this->palettepixels = palettepixels;
            this->initialized = render->MoveTextureToDevice(this);
            if (!this->initialized) {
                __debugbreak();
            }
        }
    }

    return this->initialized;
}

TextureOpenGL::~TextureOpenGL() {
    if (this->ogl_texture != -1) render->DeleteTexture(this);
}
