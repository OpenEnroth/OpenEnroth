#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/LOD.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Texture.h"




AssetsManager *assets = new AssetsManager();


bool AssetsManager::ReleaseAllImages()
{
    return true;
}

Image *AssetsManager::GetImage_16BitColorKey(const String &name, unsigned __int16 colorkey)
{

	// should this check if "name" is already in assets list??

    return Image::Create(
        new ColorKey_LOD_Loader(pIcons_LOD, name, colorkey)
    );
}


Image *AssetsManager::GetImage_16Bit(const String &name)
{
    return Image::Create(
        new Image16bit_LOD_Loader(pIcons_LOD, name)
    );
}


Image *AssetsManager::GetImage_16BitAlpha(const String &name)
{
    return Image::Create(
        new Alpha_LOD_Loader(pIcons_LOD, name)
    );
}


Image *AssetsManager::GetImage_PCXFromIconsLOD(const String &filename)
{
    return Image::Create(
        new PCX_LOD_Loader(pIcons_LOD, filename)
    );
}


Image *AssetsManager::GetImage_PCXFromNewLOD(const String &filename)
{
    return Image::Create(
        new PCX_LOD_Loader(pNew_LOD, filename)
    );
}


Image *AssetsManager::GetImage_PCXFromFile(const String &filename)
{
    return Image::Create(
        new PCX_File_Loader(pIcons_LOD, filename)
    );
}




Texture *AssetsManager::GetBitmap(const String &name)
{
	/*return Texture::Create(
	new Bitmaps_LOD_Loader(pBitmaps_LOD, name)
	);*/
	auto filename = name;
	std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

	auto i = this->bitmaps.find(filename);
	if (i == this->bitmaps.end())
	{
		auto texture = render->CreateTexture(filename);
		this->bitmaps[filename] = texture;

		return texture;
	}

	return i->second;
}


Texture *AssetsManager::GetSprite(const String &name, unsigned int palette_id, /*refactor*/unsigned int lod_sprite_id)
{
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    auto i = this->sprites.find(filename);
    if (i == this->sprites.end())
    {
        auto texture = render->CreateSprite(filename, palette_id, lod_sprite_id);
        this->sprites[filename] = texture;

        return texture;
    }

    return i->second;
}