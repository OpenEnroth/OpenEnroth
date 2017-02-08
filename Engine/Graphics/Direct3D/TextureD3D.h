#pragma once
#include "Engine/Graphics/Texture.h"

struct IDirectDrawSurface;
struct IDirect3DTexture2;
class TextureD3D : public Texture
{
    public:
        IDirectDrawSurface *GetDirectDrawSurface();
        IDirect3DTexture2  *GetDirect3DTexture();

    protected:
        friend class Render;

        static Texture *Create(ImageLoader *loader);

        void SetDirectDrawSurface(IDirectDrawSurface *dds) { this->dds = dds; }
        void SetDirect3DTexture2(IDirect3DTexture2 *d3dt) { this->d3dt = d3dt; }

        inline TextureD3D(bool lazy_initialization = true) :
            Texture(lazy_initialization),
            dds(nullptr),
            d3dt(nullptr)
        {}

        IDirectDrawSurface *dds;
        IDirect3DTexture2  *d3dt;

        virtual bool LoadImageData() override;
};