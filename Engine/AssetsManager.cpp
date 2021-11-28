#include "Engine/AssetsManager.h"

#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/LOD.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Texture.h"


AssetsManager* assets = nullptr;


static LODFile_IconsBitmaps* _load_icons_lod(const char* lod_name) {
    auto lod = new LODFile_IconsBitmaps();
    if (!lod->Load(assets_locator->LocateDataFile(lod_name), "icons")) {
        Error("%s missing\n\nPlease Reinstall.", lod_name);
    }

    return lod;
}


/*static */LODFile_IconsBitmaps* get_icons_lod() {
    static LODFile_IconsBitmaps* lod = nullptr;
    if (!lod) {
        lod = _load_icons_lod("icons.lod");
    }

    return lod;
}


/*static */LODFile_IconsBitmaps* get_events_lod() {
    static LODFile_IconsBitmaps* lod = nullptr;
    if (!lod) {
        lod = _load_icons_lod("Events.lod");
    }

    return lod;
}


static LOD::Container* get_english_t_lod() {
    static LOD::Container* lod = nullptr;
    if (!lod) {
        const char* lod_name = "EnglishT.lod";
        const char* lod_indices = "language";

        auto l = new LOD::Container();
        if (!l->Open(assets_locator->LocateDataFile(lod_name))) {
            Error("%s missing\n\nPlease Reinstall.", lod_name);
        }

        if (!l->OpenFolder(lod_indices)) {
            Error("%s is missing %s\n\nPlease Reinstall.", lod_name, lod_indices);
        }

        lod = l;
    }

    return lod;
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

bool AssetsManager::ReleaseImage(const String &name) {
    auto filename = name;
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

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

bool AssetsManager::ReleaseSprite(const String& name) {
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


void* AssetsManager::GetLocalization() {
    switch (_asset_source) {
    case Mm6Assets:
        return get_icons_lod()->LoadCompressed2("global.txt");
    case Mm7Assets:
        return get_events_lod()->LoadCompressed2("global.txt");
    case Mm8Assets:
        return get_english_t_lod()->LoadCompressed2("global.txt");
    }
}