#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "Library/Color/ColorTable.h"

#include "Engine/Graphics/ImageLoader.h"
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

    // The getters below keep the caller-chosen transparency of MM6-MM8 UI code instead of the resource mask table.
    // They fall back to a "pending" placeholder on a missing icons.lod entry, use hasIconsLodEntry() to tell.
    const std::shared_ptr<GraphicsImage> &getImage_ColorKey(std::string_view name, Color colorkey = colorTable.TealMask);
    const std::shared_ptr<GraphicsImage> &getImage_Paletted(std::string_view name);
    const std::shared_ptr<GraphicsImage> &getImage_Solid(std::string_view name);
    const std::shared_ptr<GraphicsImage> &getImage_Alpha(std::string_view name);
    const std::shared_ptr<GraphicsImage> &getImage_PCXFromIconsLOD(std::string_view name, Color colorkey = Color());
    bool hasIconsLodEntry(std::string_view name) const;

    // MM8-only: the same from EnglishD.lod. Returns an empty pointer if EnglishD.lod isn't loaded or lacks the entry.
    const std::shared_ptr<GraphicsImage> &getImage_ColorKeyFromEnglishDLOD(std::string_view name, Color colorkey = colorTable.TealMask);
    const std::shared_ptr<GraphicsImage> &getImage_PCXFromEnglishDLOD(std::string_view name, Color colorkey = Color());
    bool hasEnglishDLodEntry(std::string_view name) const;

    // MM8-only: the same from icons.UI.lod. Returns an empty pointer if icons.UI.lod isn't loaded or lacks the entry.
    const std::shared_ptr<GraphicsImage> &getImage_ColorKeyFromIconsUILOD(std::string_view name, Color colorkey = colorTable.TealMask);

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

 private:
    const std::shared_ptr<GraphicsImage> &getLegacyImage(std::string_view name, LegacyLod_Loader::Source source, LegacyLod_Loader::Mode mode, Color colorkey);

 protected:
    std::unordered_map<std::string, std::shared_ptr<GraphicsImage>> bitmaps;
    std::unordered_map<std::string, std::shared_ptr<GraphicsImage>> sprites;
    std::unordered_map<std::string, std::shared_ptr<GraphicsImage>> images;
    std::shared_ptr<GraphicsImage> noSprite; // Empty, returned for a sprite that doesn't exist.
    std::shared_ptr<GraphicsImage> noImage; // Empty, returned for an image whose archive or entry is absent.
};

extern AssetsManager *assets;
