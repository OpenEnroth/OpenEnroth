#include "Engine/AssetsManager.h"

#include <algorithm>

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Texture.h"
#include "GUI/GUIFont.h"

#include "Utility/String.h"

AssetsManager *assets = new AssetsManager();

void AssetsManager::releaseAllTextures() {
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

bool AssetsManager::releaseImage(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        return false;
    }

    render->DeleteTexture(i->second);
    images.erase(filename);
    return true;
}

Texture *AssetsManager::getImage_Paletted(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_Paletted(name);
        images[filename] = image;
        return image;
    }

    return i->second;
}


Texture *AssetsManager::getImage_ColorKey(const std::string &name, Color colorkey) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_ColorKey(name, colorkey);
        images[filename] = image;
        return image;
    }

    return i->second;
}



Texture *AssetsManager::getImage_Solid(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_Solid(name);
        images[filename] = image;
        return image;
    }

    return i->second;
}

Texture *AssetsManager::getImage_Alpha(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_Alpha(name);
        images[filename] = image;
        return image;
    }

    return i->second;
}

Texture *AssetsManager::getImage_PCXFromIconsLOD(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_PCXFromIconsLOD(name);
        images[filename] = image;
        return image;
    }

    return i->second;
}

Texture *AssetsManager::getImage_PCXFromNewLOD(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_PCXFromNewLOD(name);
        images[filename] = image;
        return image;
    }

    return i->second;
}

Texture *AssetsManager::getImage_PCXFromFile(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = render->CreateTexture_PCXFromFile(name);
        images[filename] = image;
        return image;
    }

    return i->second;
}

Texture *AssetsManager::getBitmap(const std::string &name) {
    std::string filename = toLower(name);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        auto texture = render->CreateTexture(filename);
        bitmaps[filename] = texture;
        return texture;
    }

    return i->second;
}

bool AssetsManager::releaseBitmap(const std::string &name) {
    std::string filename = toLower(name);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        return false;
    }

    render->DeleteTexture(i->second);
    bitmaps.erase(filename);
    return true;
}

Texture *AssetsManager::getSprite(const std::string &name, unsigned int palette_id,
                                  unsigned int lod_sprite_id) {
    std::string filename = toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        auto texture = render->CreateSprite(filename, palette_id, lod_sprite_id);
        sprites[filename] = texture;
        return texture;
    }

    return i->second;
}

bool AssetsManager::releaseSprite(const std::string &name) {
    std::string filename = toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        return false;
    }

    render->DeleteTexture(i->second);
    sprites.erase(filename);
    return true;
}

