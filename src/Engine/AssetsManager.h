#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "GUI/GUIFont.h"

class GraphicsImage;

class AssetsManager {
 public:
    AssetsManager();
    ~AssetsManager();

    void releaseAllTextures();

    /**
     * Removes an image from the cache if nobody else holds it. An image that's still held stays cached until
     * `releaseUnusedImages()` finds it unused.
     *
     * @param image                     Image returned by one of the getters below, moved in and left empty. Can be
     *                                  null, and can be an image that's not in the cache, then this function does
     *                                  nothing.
     */
    void evict(std::shared_ptr<GraphicsImage> &&image);

    const std::shared_ptr<GraphicsImage> &getIcon(std::string_view name);
    const std::shared_ptr<GraphicsImage> &getImage_Buff(std::string_view name);

    const std::shared_ptr<GraphicsImage> &getBitmap(std::string_view name, bool generated = false);

    /**
     * @param name                      Name of a sprite in `sprites.lod`, case-insensitive.
     * @return                          The sprite, or an empty pointer if there is no such sprite.
     */
    const std::shared_ptr<GraphicsImage> &getSprite(std::string_view name);

    /**
     * Removes all images that are held by nothing but the cache.
     */
    void releaseUnusedImages();

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
    std::unordered_map<std::string, std::shared_ptr<GraphicsImage>> bitmaps;
    std::unordered_map<std::string, std::shared_ptr<GraphicsImage>> sprites;
    std::unordered_map<std::string, std::shared_ptr<GraphicsImage>> images;
    std::shared_ptr<GraphicsImage> noSprite; // Empty, returned for a sprite that doesn't exist.
};

extern AssetsManager *assets;
