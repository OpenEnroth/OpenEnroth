#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Direct3D/TextureD3D.h"

Texture *TextureD3D::Create(ImageLoader *loader)
{
    auto tex = new TextureD3D();
    if (tex)
    {
        tex->loader = loader;
    }

    return tex;
}

IDirectDrawSurface *TextureD3D::GetDirectDrawSurface()
{
    if (!this->initialized)
    {
        this->LoadImageData();
    }

    return this->dds;
}



IDirect3DTexture2  *TextureD3D::GetDirect3DTexture()
{
    if (!this->initialized)
    {
        this->LoadImageData();
    }

    return this->d3dt;
}


bool TextureD3D::LoadImageData()
{
    if (!this->initialized)
    {
        void *pixels;

        this->initialized = this->loader->Load(&width, &height, &pixels, &native_format);
        if (this->initialized && this->native_format != IMAGE_INVALID_FORMAT)
        {
            this->pixels[native_format] = pixels;

            bool resample = false;
            this->initialized = render->MoveTextureToDevice(this);
        }
    }

    return this->initialized;
}