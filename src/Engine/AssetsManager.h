#pragma once

#include <string>
#include <unordered_map>

#include "Utility/Color.h"

class Image;
class Texture;

class AssetsManager {
 public:
    AssetsManager() {}

    void releaseAllTextures();

    bool releaseImage(const std::string &name);
    bool releaseSprite(const std::string &name);
    bool releaseBitmap(const std::string &name);

    Texture *getImage_ColorKey(const std::string &name, uint16_t colorkey = colorTable.TealMask.c16());
    Texture *getImage_Paletted(const std::string &name);
    Texture *getImage_Solid(const std::string &name);
    Texture *getImage_Alpha(const std::string &name);

    Texture *getImage_PCXFromFile(const std::string &name);
    Texture *getImage_PCXFromIconsLOD(const std::string &name);
    Texture *getImage_PCXFromNewLOD(const std::string &name);

    Texture *getBitmap(const std::string &name);
    Texture *getSprite(const std::string &name, unsigned int palette_id,
                       unsigned int lod_sprite_id);

    // TODO(pskelton): Contain better
    // TODO(pskelton): Manager should have a ref to all loose textures created throuh CreateTexture_Blank also
    Texture *winnerCert{ nullptr };

 protected:
    std::unordered_map<std::string, Texture *> bitmaps;
    std::unordered_map<std::string, Texture *> sprites;
    std::unordered_map<std::string, Texture *> images;
};

extern AssetsManager *assets;
