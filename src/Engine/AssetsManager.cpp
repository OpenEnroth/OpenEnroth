#include "Engine/AssetsManager.h"

#include <memory>
#include <string>

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Image.h"
#include "Engine/LodTextureCache.h"
#include "Engine/LodSpriteCache.h"

#include "GUI/GUIFont.h"

#include "Library/Logger/Logger.h"

#include "Utility/String/Ascii.h"

AssetsManager *assets = new AssetsManager();

static void ReloadFonts() {
    if (assets->pFontBookOnlyShadow)
        assets->pFontBookOnlyShadow->CreateFontTex();
    if (assets->pFontBookLloyds)
        assets->pFontBookLloyds->CreateFontTex();
    if (assets->pFontArrus)
        assets->pFontArrus->CreateFontTex();
    if (assets->pFontLucida)
        assets->pFontLucida->CreateFontTex();
    if (assets->pFontBookTitle)
        assets->pFontBookTitle->CreateFontTex();
    if (assets->pFontBookCalendar)
        assets->pFontBookCalendar->CreateFontTex();
    if (assets->pFontCreate)
        assets->pFontCreate->CreateFontTex();
    if (assets->pFontCChar)
        assets->pFontCChar->CreateFontTex();
    if (assets->pFontComic)
        assets->pFontComic->CreateFontTex();
    if (assets->pFontSmallnum)
        assets->pFontSmallnum->CreateFontTex();
}

void AssetsManager::releaseAllTextures() {
    logger->trace("Render - Releasing Textures.");
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

bool AssetsManager::releaseImage(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        return false;
    }

    i->second->releaseRenderId();
    images.erase(filename);
    return true;
}

GraphicsImage *AssetsManager::getImage_Paletted(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Paletted_Img_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}


GraphicsImage *AssetsManager::getImage_ColorKey(std::string_view name, Color colorkey) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<ColorKey_LOD_Loader>(pIcons_LOD, filename, colorkey));
        images[filename] = image;
        return image;
    }

    return i->second;
}



GraphicsImage *AssetsManager::getImage_Solid(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Image16bit_LOD_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getImage_Alpha(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Alpha_LOD_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getImage_PCXFromIconsLOD(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<PCX_LOD_Compressed_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getBitmap(std::string_view name, bool generated) {
    std::string filename = ascii::toLower(name);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        GraphicsImage *image = nullptr;
        if (generated) {
            image = GraphicsImage::Create(std::make_unique<Bitmaps_GEN_Loader>(filename));
        } else {
            image = GraphicsImage::Create(std::make_unique<Bitmaps_LOD_Loader>(pBitmaps_LOD, filename));
        }
        bitmaps[filename] = image;
        return image;
    }

    return i->second;
}

bool AssetsManager::releaseBitmap(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        return false;
    }

    i->second->releaseRenderId();
    bitmaps.erase(filename);
    return true;
}

GraphicsImage *AssetsManager::getSprite(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Sprites_LOD_Loader>(pSprites_LOD, filename));
        sprites[filename] = image;
        return image;
    }

    return i->second;
}

bool AssetsManager::releaseSprite(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        return false;
    }

    i->second->releaseRenderId();
    sprites.erase(filename);
    return true;
}

