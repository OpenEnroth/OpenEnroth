#pragma once

#include <string>
#include <string_view>

#include "Library/Geometry/Size.h"
#include "Library/Color/Color.h"
#include "Library/Image/Image.h"

class ResourceManager;

class ImageLoader {
 public:
    ImageLoader(ResourceManager *resources, std::string_view name) : _resources(resources), _name(name) {}
    virtual ~ImageLoader() = default;

    const std::string &name() const { return _name; }

    /**
     * @return                          Loaded image, or an empty image if loading failed.
     */
    virtual RgbaImage loadRgba() = 0;

    /**
     * @return                          Size of the image `loadRgba()` returns, read without loading it. Empty exactly
     *                                  when `loadRgba()` fails.
     */
    virtual Sizei loadSize() = 0;

 protected:
    ResourceManager *resources() const { return _resources; }

 private:
    ResourceManager *_resources;
    std::string _name;
};

class Icon_LOD_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage loadRgba() override;
    virtual Sizei loadSize() override;
};

class Buff_LOD_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage loadRgba() override;
    virtual Sizei loadSize() override;
};

/**
 * Loads an icon the way MM6-MM8 UI code asks for it, with the transparency chosen by the caller rather than by the
 * resource mask table. Used for icons that aren't in the mask table, e.g. MM8-only ones.
 */
class LegacyLod_Loader : public ImageLoader {
 public:
    enum class Source {
        ICONS,
        ENGLISH_D,
        ICONS_UI
    };

    enum class Mode {
        PALETTED, // Palette as stored.
        COLOR_KEY, // Palette entries equal to the color key are transparent, or entry 0 if the image says so.
        SOLID, // Entry 0 is transparent only if the image says so.
        ALPHA, // Entry 0 is always transparent.
        PCX // A PCX file, pixels equal to the color key are transparent.
    };

    LegacyLod_Loader(ResourceManager *resources, std::string_view name, Source source, Mode mode, Color colorKey = Color()) :
        ImageLoader(resources, name), _source(source), _mode(mode), _colorKey(colorKey) {}

    virtual RgbaImage loadRgba() override;
    virtual Sizei loadSize() override;

 private:
    Source _source;
    Mode _mode;
    Color _colorKey;
};

class Bitmaps_LOD_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage loadRgba() override;
    virtual Sizei loadSize() override;
};

class Bitmaps_GEN_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage loadRgba() override;
    virtual Sizei loadSize() override;
};

class Sprites_LOD_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage loadRgba() override;
    virtual Sizei loadSize() override;
};
