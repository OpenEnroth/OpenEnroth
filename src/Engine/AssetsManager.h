#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "Library/Color/ColorTable.h"
#include "GUI/GUIFont.h"

class GraphicsImage;

class AssetsManager {
 public:
    AssetsManager();
    ~AssetsManager();

    void releaseAllTextures();

    /**
     * Removes an image from the cache and frees it.
     *
     * @param image                     Image returned by one of the getters below.
     */
    void releaseImage(GraphicsImage *image);

    GraphicsImage *getIcon(std::string_view name);
    GraphicsImage *getImage_Buff(std::string_view name);

    GraphicsImage *getBitmap(std::string_view name, bool generated = false);
    GraphicsImage *getSprite(std::string_view name);

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
    std::unordered_map<std::string, std::unique_ptr<GraphicsImage>> bitmaps;
    std::unordered_map<std::string, std::unique_ptr<GraphicsImage>> sprites;
    std::unordered_map<std::string, std::unique_ptr<GraphicsImage>> images;
};

extern AssetsManager *assets;
