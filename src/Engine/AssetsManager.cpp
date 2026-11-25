#include "Engine/AssetsManager.h"

#include <memory>
#include <string>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Graphics/Image.h"

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

void AssetsManager::evict(std::shared_ptr<GraphicsImage> &&image) {
    std::shared_ptr<GraphicsImage> local = std::move(image);
    if (!local)
        return;

    for (auto *cache : {&images, &bitmaps, &sprites}) {
        auto pos = cache->find(local->name());
        if (pos != cache->end() && pos->second == local) {
            if (local.use_count() == 2) // The cache and `local`.
                cache->erase(pos);
            return;
        }
    }
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getIcon(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end())
        i = images.emplace(filename, GraphicsImage::Create(std::make_unique<Icon_LOD_Loader>(engine->resources(), filename))).first;

    return i->second;
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getImage_Buff(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end())
        i = images.emplace(filename, GraphicsImage::Create(std::make_unique<Buff_LOD_Loader>(engine->resources(), filename))).first;

    return i->second;
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getLegacyImage(std::string_view name, LegacyLod_Loader::Source source,
                                             LegacyLod_Loader::Mode mode, Color colorkey) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end())
        i = images.emplace(filename, GraphicsImage::Create(std::make_unique<LegacyLod_Loader>(engine->resources(), filename, source, mode, colorkey))).first;

    return i->second;
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getImage_ColorKey(std::string_view name, Color colorkey) {
    return getLegacyImage(name, LegacyLod_Loader::Source::ICONS, LegacyLod_Loader::Mode::COLOR_KEY, colorkey);
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getImage_Paletted(std::string_view name) {
    return getLegacyImage(name, LegacyLod_Loader::Source::ICONS, LegacyLod_Loader::Mode::PALETTED, Color());
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getImage_Solid(std::string_view name) {
    return getLegacyImage(name, LegacyLod_Loader::Source::ICONS, LegacyLod_Loader::Mode::SOLID, Color());
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getImage_Alpha(std::string_view name) {
    return getLegacyImage(name, LegacyLod_Loader::Source::ICONS, LegacyLod_Loader::Mode::ALPHA, Color());
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getImage_PCXFromIconsLOD(std::string_view name, Color colorkey) {
    return getLegacyImage(name, LegacyLod_Loader::Source::ICONS, LegacyLod_Loader::Mode::PCX, colorkey);
}

bool AssetsManager::hasIconsLodEntry(std::string_view name) const {
    return engine->resources()->hasIcon(name);
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getImage_ColorKeyFromEnglishDLOD(std::string_view name, Color colorkey) {
    if (!engine->resources()->hasEnglishD(name))
        return noImage;
    return getLegacyImage(name, LegacyLod_Loader::Source::ENGLISH_D, LegacyLod_Loader::Mode::COLOR_KEY, colorkey);
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getImage_PCXFromEnglishDLOD(std::string_view name, Color colorkey) {
    if (!engine->resources()->hasEnglishD(name))
        return noImage;
    return getLegacyImage(name, LegacyLod_Loader::Source::ENGLISH_D, LegacyLod_Loader::Mode::PCX, colorkey);
}

bool AssetsManager::hasEnglishDLodEntry(std::string_view name) const {
    return engine->resources()->hasEnglishD(name);
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getImage_ColorKeyFromIconsUILOD(std::string_view name, Color colorkey) {
    if (!engine->resources()->hasIconsUi(name))
        return noImage;
    return getLegacyImage(name, LegacyLod_Loader::Source::ICONS_UI, LegacyLod_Loader::Mode::COLOR_KEY, colorkey);
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getBitmap(std::string_view name, bool generated) {
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

    return i->second;
}

const std::shared_ptr<GraphicsImage> &AssetsManager::getSprite(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        std::unique_ptr<GraphicsImage> image = GraphicsImage::Create(std::make_unique<Sprites_LOD_Loader>(engine->resources(), filename));
        if (!image->size())
            return noSprite;
        i = sprites.emplace(filename, std::move(image)).first;
    }

    return i->second;
}

void AssetsManager::releaseUnusedImages() {
    for (auto *cache : {&images, &bitmaps, &sprites})
        std::erase_if(*cache, [](const auto &entry) { return entry.second.use_count() == 1; });
}
