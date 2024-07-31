#pragma once

#include <string>
#include <functional>

#include "Library/Color/Color.h"
#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

class LodSpriteCache;
class LodTextureCache;
class LodReader;

class ImageLoader {
 public:
    virtual ~ImageLoader() = default;
    const std::string &GetResourceName() const { return this->resource_name; }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) = 0;

 protected:
    std::string resource_name;
};

class Paletted_Img_Loader : public ImageLoader {
 public:
    inline Paletted_Img_Loader(LodTextureCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LodTextureCache *lod;
};

class ColorKey_LOD_Loader : public ImageLoader {
 public:
    inline ColorKey_LOD_Loader(LodTextureCache *lod, std::string_view filename, Color colorkey) {
        this->resource_name = filename;
        this->colorkey = colorkey;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    Color colorkey;
    LodTextureCache *lod;
};

class Image16bit_LOD_Loader : public ImageLoader {
 public:
    inline Image16bit_LOD_Loader(LodTextureCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LodTextureCache *lod;
};

class Alpha_LOD_Loader : public ImageLoader {
 public:
    inline Alpha_LOD_Loader(LodTextureCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LodTextureCache *lod;
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

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LodReader *lod;
};

class PCX_LOD_Compressed_Loader : public PCX_Loader {
 public:
    // TODO(captainurist): this is the next level of ugly, redo.
    template<class Lod>
    inline PCX_LOD_Compressed_Loader(Lod *lod, std::string_view filename) {
        resource_name = filename;
        blob_func = [this, lod] {
            return lod->LoadCompressedTexture(resource_name);
        };
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    std::function<Blob()> blob_func;
};

class Bitmaps_LOD_Loader : public ImageLoader {
 public:
    inline Bitmaps_LOD_Loader(LodTextureCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LodTextureCache *lod;
};

class Sprites_LOD_Loader : public ImageLoader {
 public:
    inline Sprites_LOD_Loader(LodSpriteCache *lod, std::string_view filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LodSpriteCache *lod;
};
