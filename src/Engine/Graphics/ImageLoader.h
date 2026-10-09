#pragma once

#include <string>
#include <string_view>

#include "Library/Geometry/Size.h"
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
    virtual RgbaImage load() = 0;

    /**
     * @return                          Size of the image `load()` returns, or an empty size if it can't be known without
     *                                  loading.
     */
    virtual Sizei size() {
        return {};
    }

 protected:
    ResourceManager *resources() const { return _resources; }

 private:
    ResourceManager *_resources;
    std::string _name;
};

class Icon_LOD_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage load() override;
    virtual Sizei size() override;
};

class Buff_LOD_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage load() override;
    virtual Sizei size() override;
};

class Bitmaps_LOD_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage load() override;
    virtual Sizei size() override;
};

class Bitmaps_GEN_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage load() override;
    virtual Sizei size() override;
};

class Sprites_LOD_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage load() override;
    virtual Sizei size() override;
};
