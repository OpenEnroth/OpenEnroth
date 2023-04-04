#include "Engine/AssetsManager.h"

#include <algorithm>

#include "Engine/LOD.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Texture.h"
#include "GUI/GUIFont.h"

AssetsManager *assets = new AssetsManager();

void AssetsManager::ReleaseAllTextures() {
    logger->info("Render - Releasing Textures.");
    // clears any textures from gpu
    for (auto img : images) {
        render->DeleteTexture(img.second);
    }
    for (auto bit : bitmaps) {
        render->DeleteTexture(bit.second);
    }
    for (auto spr : sprites) {
        render->DeleteTexture(spr.second);
    }

    ReloadFonts();

    return;
}

bool AssetsManager::ReleaseAllImages() {
    return true;

    // this will dereference things
    for (auto it = images.cbegin(), next = it; it != images.cend(); it = next) {
        next++;
        it->second->Release();
    }
    return true;
}

bool AssetsManager::ReleaseImage(const std::string &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        return false;
    }

    images.erase(filename);
    return true;
}

Texture *AssetsManager::GetImage_Paletted(const std::string &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(),
        ::tolower);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_Paletted(name);
        images[filename] = image;
        return image;
    }

    return i->second;
}


Texture *AssetsManager::GetImage_ColorKey(const std::string &name, uint16_t colorkey) {
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



Texture *AssetsManager::GetImage_Solid(const std::string &name) {
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

Texture *AssetsManager::GetImage_Alpha(const std::string &name) {
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

Texture *AssetsManager::GetImage_PCXFromIconsLOD(const std::string &name) {
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

Texture *AssetsManager::GetImage_PCXFromNewLOD(const std::string &name) {
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

Texture *AssetsManager::GetImage_PCXFromFile(const std::string &name) {
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

Texture *AssetsManager::GetBitmap(const std::string &name) {
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

Texture *AssetsManager::GetSprite(const std::string &name, unsigned int palette_id,
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

bool AssetsManager::ReleaseSprite(const std::string &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        return false;
    }

    sprites.erase(filename);
    return true;
}

bool AssetsManager::ReleaseAllSprites() {
    return true;

    // this will dereference things
    for (auto it = sprites.cbegin(), next = it; it != sprites.cend(); it = next) {
        next++;
        it->second->Release();
    }
    return true;
}
