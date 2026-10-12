#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

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

    /**
     * @param name                      Name of a sprite in `sprites.lod`, case-insensitive.
     * @return                          The sprite, or `nullptr` if there is no such sprite.
     */
    GraphicsImage *getSprite(std::string_view name);

    /**
     * Marks every sprite loaded so far as kept by `releaseUnreservedSprites()`.
     */
    void reserveLoadedSprites();

    /**
     * Frees the sprites loaded since the last `reserveLoadedSprites()` call.
     */
    void releaseUnreservedSprites();

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

 private:
    std::vector<std::string> _spriteLoadOrder;
    size_t _reservedSpriteCount = 0;
};

extern AssetsManager *assets;
