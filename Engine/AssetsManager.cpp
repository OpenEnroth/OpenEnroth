#include "Engine/AssetsManager.h"

#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/LOD.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Texture.h"

AssetsManager *assets = new AssetsManager();

bool AssetsManager::ReleaseAllImages() { return true; }

bool AssetsManager::ReleaseImage(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        return false;
    }

    images.erase(filename);

    return true;
}

Image *AssetsManager::GetImage_ColorKey(const String &name, uint16_t colorkey) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image =
            Image::Create(new ColorKey_LOD_Loader(pIcons_LOD, name, colorkey));
        images[filename] = image;
        return image;
    }

    return i->second;
}

Image *AssetsManager::GetImage_Solid(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = Image::Create(new Image16bit_LOD_Loader(pIcons_LOD, name));
        images[filename] = image;
        return image;
    }

    return i->second;
}

Image *AssetsManager::GetImage_Alpha(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = Image::Create(new Alpha_LOD_Loader(pIcons_LOD, name));
        images[filename] = image;
        return image;
    }

    return i->second;
}

Image *AssetsManager::GetImage_PCXFromIconsLOD(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = Image::Create(new PCX_LOD_Loader(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

Image *AssetsManager::GetImage_PCXFromNewLOD(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = Image::Create(new PCX_LOD_Loader(pNew_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

Image *AssetsManager::GetImage_PCXFromFile(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = Image::Create(new PCX_File_Loader(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

Texture *AssetsManager::GetBitmap(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        auto texture = render->CreateTexture(filename);
        bitmaps[filename] = texture;
        return texture;
    }

    return i->second;
}

Texture *AssetsManager::GetSprite(const String &name, unsigned int palette_id,
                                  unsigned int lod_sprite_id) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        auto texture =
            render->CreateSprite(filename, palette_id, lod_sprite_id);
        sprites[filename] = texture;
        return texture;
    }

    return i->second;
}
