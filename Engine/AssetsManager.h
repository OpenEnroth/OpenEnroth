#pragma once

#include <map>
#include <string>

#include "Engine/AssetsClass.h"
#include "Engine/ErrorHandling.h"


class Image;
class Texture;


class AssetsManager {
public:
    AssetsManager(AssetsClass asset_source):
        _asset_source(asset_source)
    {
        if (_asset_source != Mm6Assets
            && _asset_source != Mm7Assets
            && _asset_source != Mm8Assets
        ) {
            Error("Unknown asset class: %d", (int)_asset_source);
        }
    }
    
    bool ReleaseAllImages();
    bool ReleaseAllSprites();

    bool ReleaseImage(const std::string& name);
    bool ReleaseSprite(const std::string& name);

    Texture *GetImage_ColorKey(const std::string & name, uint16_t colorkey);
    Texture *GetImage_Solid(const std::string& name);
    Texture *GetImage_Alpha(const std::string& name);

    Texture *GetImage_PCXFromFile(const std::string& name);
    Texture *GetImage_PCXFromIconsLOD(const std::string& name);
    Texture *GetImage_PCXFromNewLOD(const std::string& name);

    Texture *GetBitmap(const std::string& name);
    Texture *GetSprite(const std::string& name, unsigned int palette_id,
                       unsigned int lod_sprite_id);

    void* GetLocalization();

protected:
    AssetsClass _asset_source;

    std::map<std::string, Texture *> bitmaps;
    std::map<std::string, Texture *> sprites;
    std::map<std::string, Texture *> images;
};

extern AssetsManager *assets;
