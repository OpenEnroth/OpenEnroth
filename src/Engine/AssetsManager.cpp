#include "Engine/AssetsManager.h"

#include <memory>
#include <utility>

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Image.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/LodTextureCache.h"
#include "Engine/LodSpriteCache.h"
#include "GUI/GUIFont.h"
#include "Library/Logger/Logger.h"
#include "Utility/String.h"
#include "Library/Color/Color.h"

AssetsManager *assets = new AssetsManager();

void AssetsManager::releaseAllTextures() {
    logger->verbose("Render - Releasing Textures.");
    // clears any textures from gpu
    for (auto img : images) {
        img.second->releaseRenderId();
    }
    for (auto bit : bitmaps) {
        bit.second->releaseRenderId();
    }
    for (auto spr : sprites) {
        spr.second->releaseRenderId();
    }

    ReloadFonts();
}

bool AssetsManager::releaseImage(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        return false;
    }

    i->second->releaseRenderId();
    images.erase(filename);
    return true;
}

GraphicsImage *AssetsManager::getImage_Paletted(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Paletted_Img_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}


GraphicsImage *AssetsManager::getImage_ColorKey(const std::string &name, Color colorkey) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<ColorKey_LOD_Loader>(pIcons_LOD, filename, colorkey));
        images[filename] = image;
        return image;
    }

    return i->second;
}



GraphicsImage *AssetsManager::getImage_Solid(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Image16bit_LOD_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getImage_Alpha(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Alpha_LOD_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getImage_PCXFromIconsLOD(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<PCX_LOD_Compressed_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getImage_PCXFromFile(const std::string &name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<PCX_File_Loader>(filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getBitmap(const std::string &name) {
    std::string filename = toLower(name);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Bitmaps_LOD_Loader>(pBitmaps_LOD, filename));
        bitmaps[filename] = image;
        return image;
    }

    return i->second;
}

bool AssetsManager::releaseBitmap(const std::string &name) {
    std::string filename = toLower(name);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        return false;
    }

    i->second->releaseRenderId();
    bitmaps.erase(filename);
    return true;
}

GraphicsImage *AssetsManager::getSprite(const std::string &name) {
    std::string filename = toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Sprites_LOD_Loader>(pSprites_LOD, filename));
        sprites[filename] = image;
        return image;
    }

    return i->second;
}

bool AssetsManager::releaseSprite(const std::string &name) {
    std::string filename = toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        return false;
    }

    i->second->releaseRenderId();
    sprites.erase(filename);
    return true;
}

