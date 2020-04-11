#include "Engine/Graphics/OpenGL/TextureOpenGL.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/ErrorHandling.h"

Texture *TextureOpenGL::Create(unsigned int width, unsigned int height,
    IMAGE_FORMAT format, const void *pixels = nullptr) {

    auto tex = new TextureOpenGL(false);
    if (tex) {
        tex->initialized = true;
        tex->width = width;
        tex->height = height;
        tex->native_format = format;
        unsigned int num_pixels = tex->GetWidth() * tex->GetHeight();
        unsigned int num_pixels_bytes =
            num_pixels * IMAGE_FORMAT_BytesPerPixel(format);
        tex->pixels[format] = new unsigned char[num_pixels_bytes];
        if (pixels) {
            memcpy(tex->pixels[format], pixels, num_pixels_bytes);
        } else {
            memset(tex->pixels[format], 0, num_pixels_bytes);
        }

        if (tex->initialized && tex->native_format != IMAGE_INVALID_FORMAT) {
            // tex->pixels[format] = pixels;
            tex->initialized = render->MoveTextureToDevice(tex);
            if (!tex->initialized) {
                __debugbreak();
            }
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

int TextureOpenGL::GetOpenGlTexture() {
    if (!this->initialized) {
        this->LoadImageData();
    }

    if (this->ogl_texture < 0) {
        __debugbreak();  // prob not loaded in as texture and recast so problems
    }

    return this->ogl_texture;
}

bool TextureOpenGL::LoadImageData() {
    if (!this->initialized) {
        void *pixels;

        this->initialized =
            this->loader->Load(&width, &height, &pixels, &native_format);
        if (this->initialized && this->native_format != IMAGE_INVALID_FORMAT) {
            this->pixels[native_format] = pixels;
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
