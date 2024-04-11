#include "Engine/AssetsManager.h"

#include <memory>

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Image.h"
#include "Engine/LodTextureCache.h"
#include "Engine/LodSpriteCache.h"

#include "GUI/GUIFont.h"

#include "Library/Logger/Logger.h"

#include "Utility/String.h"

AssetsManager *assets = new AssetsManager();

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
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        return false;
    }

    i->second->releaseRenderId();
    images.erase(filename);
    return true;
}

GraphicsImage *AssetsManager::getImage_Paletted(std::string_view name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Paletted_Img_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}


GraphicsImage *AssetsManager::getImage_ColorKey(std::string_view name, Color colorkey) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<ColorKey_LOD_Loader>(pIcons_LOD, filename, colorkey));
        images[filename] = image;
        return image;
    }

    return i->second;
}



GraphicsImage *AssetsManager::getImage_Solid(std::string_view name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Image16bit_LOD_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getImage_Alpha(std::string_view name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Alpha_LOD_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getImage_PCXFromIconsLOD(std::string_view name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<PCX_LOD_Compressed_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getImage_PCXFromFile(std::string_view name) {
    std::string filename = toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<PCX_File_Loader>(filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getBitmap(std::string_view name) {
    std::string filename = toLower(name);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Bitmaps_LOD_Loader>(pBitmaps_LOD, filename));
        bitmaps[filename] = image;
        return image;
    }

    return i->second;
}

bool AssetsManager::releaseBitmap(std::string_view name) {
    std::string filename = toLower(name);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        return false;
    }

    i->second->releaseRenderId();
    bitmaps.erase(filename);
    return true;
}

GraphicsImage *AssetsManager::getSprite(std::string_view name) {
    std::string filename = toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Sprites_LOD_Loader>(pSprites_LOD, filename));
        sprites[filename] = image;
        return image;
    }

    return i->second;
}

GUIFont *AssetsManager::getFont(std::string_view name) const {
    if (pFontBookOnlyShadow && pFontBookOnlyShadow->fontFile == name) {
        return pFontBookOnlyShadow.get();
    } else if(pFontBookLloyds && pFontBookLloyds->fontFile == name) {
        return pFontBookLloyds.get();
    } else if(pFontArrus && pFontArrus->fontFile == name) {
        return pFontArrus.get();
    } else if(pFontLucida && pFontLucida->fontFile == name) {
        return pFontLucida.get();
    } else if(pFontBookTitle && pFontBookTitle->fontFile == name) {
        return pFontBookTitle.get();
    } else if(pFontBookCalendar && pFontBookCalendar->fontFile == name) {
        return pFontBookCalendar.get();
    } else if(pFontCreate && pFontCreate->fontFile == name) {
        return pFontCreate.get();
    } else if(pFontCChar && pFontCChar->fontFile == name) {
        return pFontCChar.get();
    } else if (pFontComic && pFontComic->fontFile == name) {
        return pFontComic.get();
    } else if (pFontSmallnum && pFontSmallnum->fontFile == name) {
        return pFontSmallnum.get();
    }

    return nullptr;
}

bool AssetsManager::releaseSprite(std::string_view name) {
    std::string filename = toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        return false;
    }

    i->second->releaseRenderId();
    sprites.erase(filename);
    return true;
}

