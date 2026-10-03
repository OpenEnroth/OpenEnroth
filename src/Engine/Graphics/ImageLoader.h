#pragma once

#include <string>

#include "Library/Image/Image.h"

class LodReader;
class ResourceManager;

class ImageLoader {
 public:
    virtual ~ImageLoader() = default;
    const std::string &GetResourceName() const { return this->resource_name; }

    virtual bool Load(RgbaImage *rgbaImage) = 0;

 protected:
    std::string resource_name;
};

class Icon_LOD_Loader : public ImageLoader {
 public:
    inline Icon_LOD_Loader(ResourceManager *resources, std::string_view filename) {
        this->resource_name = filename;
        this->resources = resources;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    ResourceManager *resources = nullptr;
};

class Buff_LOD_Loader : public ImageLoader {
 public:
    inline Buff_LOD_Loader(ResourceManager *resources, std::string_view filename) {
        this->resource_name = filename;
        this->resources = resources;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    ResourceManager *resources = nullptr;
};

class PCX_Loader : public ImageLoader {
 protected:
    bool InternalLoad(const Blob &data, RgbaImage *rgbaImage);
};

class PCX_LOD_Raw_Loader : public PCX_Loader {
 public:
    inline PCX_LOD_Raw_Loader(LodReader *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodReader *lod = nullptr;
};

class Bitmaps_LOD_Loader : public ImageLoader {
 public:
    inline Bitmaps_LOD_Loader(ResourceManager *resources, std::string_view filename) {
        this->resource_name = filename;
        this->resources = resources;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    ResourceManager *resources = nullptr;
};

class Bitmaps_GEN_Loader : public ImageLoader {
 public:
    explicit inline Bitmaps_GEN_Loader(std::string_view filename) {
        this->resource_name = filename;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;
};

class Sprites_LOD_Loader : public ImageLoader {
 public:
    inline Sprites_LOD_Loader(ResourceManager *resources, std::string_view filename) {
        this->resource_name = filename;
        this->resources = resources;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    ResourceManager *resources = nullptr;
};
