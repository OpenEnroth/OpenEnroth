#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Texture.h"

Texture *Texture::Create(ImageLoader *loader)
{
    auto tex = new Texture();
    if (tex)
    {
        tex->loader = loader;
    }

    return tex;
}

IDirectDrawSurface *Texture::GetDirectDrawSurface()
{
    if (!this->initialized)
    {
        this->LoadImageData();
    }

    if (this->initialized)
    {
        return this->dds;
    }

    return nullptr;
}



IDirect3DTexture2  *Texture::GetDirect3DTexture()
{
    if (!this->initialized)
    {
        this->LoadImageData();
    }

    if (this->initialized)
    {
        return this->d3dt;
    }

    return nullptr;
}


bool Texture::LoadImageData()
{
    if (!this->initialized)
    {
        void *pixels;

        this->initialized = this->loader->Load(&width, &height, &pixels, &native_format);
        if (this->initialized && this->native_format != IMAGE_INVALID_FORMAT)
        {
            this->pixels[native_format] = pixels;

            bool resample = false;
            this->initialized = render->LoadTexture(
                this->loader->GetResourceName().c_str(),
                resample,
                (void **)&this->dds,
                (void **)&this->d3dt
            );
        }
    }

    return this->initialized;
}