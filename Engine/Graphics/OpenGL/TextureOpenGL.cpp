#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/OpenGL/TextureOpenGL.h"

Texture *TextureOpenGL::Create(ImageLoader *loader)
{
    auto tex = new TextureOpenGL();
    if (tex)
    {
        tex->loader = loader;
    }

    return tex;
}

int TextureOpenGL::GetOpenGlTexture()
{
    if (!this->initialized)
    {
        this->LoadImageData();
    }

    return this->ogl_texture;
}


bool TextureOpenGL::LoadImageData()
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