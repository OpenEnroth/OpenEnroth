#pragma once

#include <map>

#include "Engine/Strings.h"

class Image;
class Texture;

class AssetsManager {
 public:
    AssetsManager() {}

    bool ReleaseAllImages();

    bool ReleaseImage(const String &name);

    Texture *GetImage_ColorKey(const String &name, uint16_t colorkey);
    Texture *GetImage_Solid(const String &name);
    Texture *GetImage_Alpha(const String &name);

    Texture *GetImage_PCXFromFile(const String &name);
    Texture *GetImage_PCXFromIconsLOD(const String &name);
    Texture *GetImage_PCXFromNewLOD(const String &name);

    Texture *GetBitmap(const String &name);
    Texture *GetSprite(const String &name, unsigned int palette_id,
                       unsigned int lod_sprite_id);

 protected:
    std::map<String, Texture *> bitmaps;
    std::map<String, Texture *> sprites;
    std::map<String, Texture *> images;
};

extern AssetsManager *assets;
