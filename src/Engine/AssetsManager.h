#pragma once

#include <map>
#include <string>

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
    bool ReleaseSprite(const std::string& name);

    Texture *GetImage_ColorKey(const std::string &name, uint16_t colorkey = colorTable.TealMask.C16());
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
    Texture* WinnerCert{ nullptr };

 protected:
    std::map<std::string, Texture *> bitmaps;
    std::map<std::string, Texture *> sprites;
    std::map<std::string, Texture *> images;
};

extern AssetsManager *assets;
