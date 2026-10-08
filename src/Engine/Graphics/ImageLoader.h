#pragma once

#include <string>
#include <string_view>

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
};

class Buff_LOD_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage load() override;
};

class Bitmaps_LOD_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage load() override;
};

class Bitmaps_GEN_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage load() override;
};

class Sprites_LOD_Loader : public ImageLoader {
 public:
    using ImageLoader::ImageLoader;

    virtual RgbaImage load() override;
};
