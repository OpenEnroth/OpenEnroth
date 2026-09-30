#pragma once

#include <string>

#include "Engine/Data/ResourceMask.h"

#include "Library/Color/Color.h"
#include "Library/Image/Image.h"

class LodSpriteCache;
class LodTextureCache;
class LodReader;

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
    inline Icon_LOD_Loader(LodTextureCache *lod, std::string_view filename, ResourceMask mask) {
        this->resource_name = filename;
        this->lod = lod;
        this->mask = mask;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodTextureCache *lod = nullptr;
    ResourceMask mask;
};

class Buff_LOD_Loader : public ImageLoader {
 public:
    inline Buff_LOD_Loader(LodTextureCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodTextureCache *lod = nullptr;
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
    inline Bitmaps_LOD_Loader(LodTextureCache *lod, std::string_view filename, ResourceMask mask) {
        this->resource_name = filename;
        this->lod = lod;
        this->mask = mask;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodTextureCache *lod = nullptr;
    ResourceMask mask;
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
    inline Sprites_LOD_Loader(LodSpriteCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage) override;

 protected:
    LodSpriteCache *lod = nullptr;
};
