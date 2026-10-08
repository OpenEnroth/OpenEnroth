#pragma once

#include <string>
#include <string_view>

#include "Library/Image/Image.h"

class ResourceManager;

class ImageLoader {
 public:
    ImageLoader(ResourceManager *resources, std::string_view name) : resources(resources), resource_name(name) {}
    virtual ~ImageLoader() = default;

    const std::string &GetResourceName() const { return this->resource_name; }

    /**
     * @return                          Loaded image, or an empty image if loading failed.
     */
    virtual RgbaImage load() = 0;

 protected:
    ResourceManager *resources = nullptr;
    std::string resource_name;
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
