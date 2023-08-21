#pragma once

#include <string>
#include <unordered_map>

#include "Library/Color/ColorTable.h"

class GraphicsImage;
struct Color;

class AssetsManager {
 public:
    AssetsManager() {}

    void releaseAllTextures();

    // TODO(captainurist): These are called back from GraphicsImage::Release, which is a questionable design.
    bool releaseImage(const std::string &name);
    bool releaseSprite(const std::string &name);
    bool releaseBitmap(const std::string &name);

    GraphicsImage *getImage_ColorKey(const std::string &name, Color colorkey = colorTable.TealMask);
    GraphicsImage *getImage_Paletted(const std::string &name);
    GraphicsImage *getImage_Solid(const std::string &name);
    GraphicsImage *getImage_Alpha(const std::string &name);

    GraphicsImage *getImage_PCXFromFile(const std::string &name);
    GraphicsImage *getImage_PCXFromIconsLOD(const std::string &name);

    GraphicsImage *getBitmap(const std::string &name);
    GraphicsImage *getSprite(const std::string &name);

    // TODO(pskelton): Contain better
    // TODO(pskelton): Manager should have a ref to all loose textures created throuh CreateTexture_Blank also
    GraphicsImage *winnerCert{ nullptr };

 protected:
    std::unordered_map<std::string, GraphicsImage *> bitmaps;
    std::unordered_map<std::string, GraphicsImage *> sprites;
    std::unordered_map<std::string, GraphicsImage *> images;
};

extern AssetsManager *assets;
