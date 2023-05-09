#pragma once

#include <string>
#include <unordered_map>

#include "Utility/Color.h"

class Image;
class Texture;

class AssetsManager {
 public:
    AssetsManager() {}

    void ReleaseAllTextures();

    bool ReleaseAllImages();
    bool ReleaseAllSprites();

    bool ReleaseImage(const std::string &name);
    bool ReleaseSprite(const std::string &name);

    Texture *GetImage_ColorKey(const std::string &name, uint16_t colorkey = colorTable.TealMask.c16());
    Texture *GetImage_Paletted(const std::string &name);
    Texture *GetImage_Solid(const std::string &name);
    Texture *GetImage_Alpha(const std::string &name);

    Texture *GetImage_PCXFromFile(const std::string &name);
    Texture *GetImage_PCXFromIconsLOD(const std::string &name);
    Texture *GetImage_PCXFromNewLOD(const std::string &name);

    Texture *GetBitmap(const std::string &name);
    Texture *GetSprite(const std::string &name, unsigned int palette_id,
                       unsigned int lod_sprite_id);

    // TODO(pskelton): Contain better
    // TODO(pskelton): Manager should have a ref to all loose textures created throuh CreateTexture_Blank also
    Texture *WinnerCert{ nullptr };

 protected:
    std::unordered_map<std::string, Texture *> bitmaps;
    std::unordered_map<std::string, Texture *> sprites;
    std::unordered_map<std::string, Texture *> images;
};

extern AssetsManager *assets;
