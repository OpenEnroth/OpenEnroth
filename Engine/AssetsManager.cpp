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




Texture *AssetsManager::GetImage_ColorKey(const String &name, uint16_t colorkey) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
        ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_ColorKey(name, colorkey);
        images[filename] = image;
        return image;
    }

    return i->second;
}



Texture *AssetsManager::GetImage_Solid(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_Solid(name);
        images[filename] = image;
        return image;
    }

    return i->second;
}

Texture *AssetsManager::GetImage_Alpha(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_Alpha(name);
        images[filename] = image;
        return image;
    }

    return i->second;
}

Texture *AssetsManager::GetImage_PCXFromIconsLOD(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_PCXFromIconsLOD(name);
        images[filename] = image;
        return image;
    }

    return i->second;
}

Texture *AssetsManager::GetImage_PCXFromNewLOD(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_PCXFromNewLOD(name);
        images[filename] = image;
        return image;
    }

    return i->second;
}

Texture *AssetsManager::GetImage_PCXFromFile(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_PCXFromFile(name);
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
