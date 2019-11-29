#include "Engine/Graphics/Direct3D/TextureD3D.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"


Texture *TextureD3D::Create(unsigned int width, unsigned int height,
    IMAGE_FORMAT format, const void *pixels = nullptr) {

    if (width == 0 || height == 0) __debugbreak();

    auto tex = new TextureD3D();
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
            // tex->initialized = render->MoveTextureToDevice(tex); nope
            if (!tex->initialized) {
                __debugbreak;
            }
        }
    }

    return tex;
}


Texture *TextureD3D::Create(ImageLoader *loader) {
    auto tex = new TextureD3D();
    if (tex) {
        tex->loader = loader;
    }

    return tex;
}

IDirectDrawSurface *TextureD3D::GetDirectDrawSurface() {
    if (!this->initialized) {
        this->LoadImageData();
    }

    return this->dds;
}

IDirect3DTexture2 *TextureD3D::GetDirect3DTexture() {
    if (!this->initialized) {
        this->LoadImageData();
    }

    return this->d3dt;
}

bool TextureD3D::LoadImageData() {
    if (!this->initialized) {
        void *pixels;

        this->initialized =
            this->loader->Load(&width, &height, &pixels, &native_format);

        if (width == 0 || height == 0) __debugbreak();

        if (this->initialized && this->native_format != IMAGE_INVALID_FORMAT) {
            this->pixels[native_format] = pixels;

            // check power of two - temporary
            if ( (this->width & (this->width - 1)) == 0 && (this->height & (this->height - 1)) == 0 ) {
                this->initialized = render->MoveTextureToDevice(this);
            }
        }
    }

    return this->initialized;
}
