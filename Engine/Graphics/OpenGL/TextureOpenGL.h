#pragma once
#include "Engine/Graphics/Texture.h"

class TextureOpenGL : public Texture
{
	public:
		int GetOpenGlTexture();

	protected:
		friend class RenderOpenGL;

		static Texture *Create(ImageLoader *loader);

		inline TextureOpenGL(bool lazy_initialization = true) :
			Texture(lazy_initialization),
			ogl_texture(-1)
		{}

		int ogl_texture;

		virtual bool LoadImageData() override;
};