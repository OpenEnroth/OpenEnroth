#include "Engine/AssetsManager.h"

#include <cassert>
#include <memory>
#include <string>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Resources/LodSpriteCache.h"

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

AssetsManager::AssetsManager() = default;
AssetsManager::~AssetsManager() = default;

void AssetsManager::releaseAllTextures() {
    MM_TRACE("Render - Releasing Textures.");
    // clears any textures from gpu
    for (auto *cache : {&images, &bitmaps, &sprites})
        for (const auto &[name, image] : *cache)
            image->releaseRenderId();

    ReloadFonts();
}

void AssetsManager::releaseImage(GraphicsImage *image) {
    for (auto *cache : {&images, &bitmaps, &sprites}) {
        auto pos = cache->find(image->name());
        if (pos != cache->end() && pos->second.get() == image) {
            cache->erase(pos);
            return;
        }
    }

    assert(false && "Image is not in the cache");
}

GraphicsImage *AssetsManager::getIcon(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end())
        i = images.emplace(filename, GraphicsImage::Create(std::make_unique<Icon_LOD_Loader>(engine->resources(), filename))).first;

    return i->second.get();
}

GraphicsImage *AssetsManager::getImage_Buff(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end())
        i = images.emplace(filename, GraphicsImage::Create(std::make_unique<Buff_LOD_Loader>(engine->resources(), filename))).first;

    return i->second.get();
}

GraphicsImage *AssetsManager::getBitmap(std::string_view name, bool generated) {
    std::string filename = ascii::toLower(name);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        std::unique_ptr<ImageLoader> loader;
        if (generated) {
            loader = std::make_unique<Bitmaps_GEN_Loader>(engine->resources(), filename);
        } else {
            loader = std::make_unique<Bitmaps_LOD_Loader>(engine->resources(), filename);
        }
        i = bitmaps.emplace(filename, GraphicsImage::Create(std::move(loader))).first;
    }

    return i->second.get();
}

GraphicsImage *AssetsManager::getSprite(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end())
        i = sprites.emplace(filename, GraphicsImage::Create(std::make_unique<Sprites_LOD_Loader>(engine->resources(), filename))).first;

    return i->second.get();
}
