#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "Library/Color/ColorTable.h"
#include "GUI/GUIFont.h"

class GraphicsImage;

class AssetsManager {
 public:
    AssetsManager() {}

    void releaseAllTextures();

    // TODO(captainurist): These are called back from GraphicsImage::Release, which is a questionable design.
    bool releaseImage(std::string_view name);
    bool releaseSprite(std::string_view name);
    bool releaseBitmap(std::string_view name);

    GraphicsImage *getImage_ColorKey(std::string_view name, Color colorkey = colorTable.TealMask);
    GraphicsImage *getImage_Paletted(std::string_view name);
    GraphicsImage *getImage_Solid(std::string_view name);
    GraphicsImage *getImage_Alpha(std::string_view name);

    GraphicsImage *getImage_PCXFromIconsLOD(std::string_view name);

    GraphicsImage *getBitmap(std::string_view name);
    GraphicsImage *getSprite(std::string_view name);

    // TODO(pskelton): Contain better
    // TODO(pskelton): Manager should have a ref to all loose textures created throuh CreateTexture_Blank also
    GraphicsImage *winnerCert{ nullptr };

    std::unique_ptr<GUIFont> pFontBookOnlyShadow;
    std::unique_ptr<GUIFont> pFontBookLloyds;
    std::unique_ptr<GUIFont> pFontArrus;
    std::unique_ptr<GUIFont> pFontLucida;
    std::unique_ptr<GUIFont> pFontBookTitle;
    std::unique_ptr<GUIFont> pFontBookCalendar;
    std::unique_ptr<GUIFont> pFontCreate;
    std::unique_ptr<GUIFont> pFontCChar;
    std::unique_ptr<GUIFont> pFontComic;
    std::unique_ptr<GUIFont> pFontSmallnum;

 protected:
    std::unordered_map<std::string, GraphicsImage *> bitmaps;
    std::unordered_map<std::string, GraphicsImage *> sprites;
    std::unordered_map<std::string, GraphicsImage *> images;
};

extern AssetsManager *assets;
