#pragma once
#include "Engine/Graphics/Image.h"

struct IDirectDrawSurface;
struct IDirect3DTexture2;
class Texture : public Image
{
    public:
        inline Texture(bool lazy_initialization = true) :
            Image(lazy_initialization),
            dds(nullptr),
            d3dt(nullptr)
        {}

        static Texture *Create(ImageLoader *);

        IDirectDrawSurface *GetDirectDrawSurface();
        IDirect3DTexture2  *GetDirect3DTexture();

    protected:
        IDirectDrawSurface *dds;
        IDirect3DTexture2  *d3dt;

        virtual bool LoadImageData() override;
};