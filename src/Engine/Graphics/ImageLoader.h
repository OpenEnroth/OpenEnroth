#pragma once

#include <string>

#include "Engine/LOD.h"

#include "Library/Color/Color.h"
#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

class Logger;

class ImageLoader {
 public:
    ImageLoader();
    virtual ~ImageLoader() = default;
    virtual std::string GetResourceName() const { return this->resource_name; }
    virtual std::string *GetResourceNamePtr() { return &this->resource_name; }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) = 0;

 protected:
    std::string resource_name;
    Logger *log;
};

class Paletted_Img_Loader : public ImageLoader {
 public:
    inline Paletted_Img_Loader(LODFile_IconsBitmaps *lod, const std::string &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LODFile_IconsBitmaps *lod;
};

class ColorKey_LOD_Loader : public ImageLoader {
 public:
    inline ColorKey_LOD_Loader(LODFile_IconsBitmaps *lod,
                               const std::string &filename, Color colorkey) {
        this->resource_name = filename;
        this->colorkey = colorkey;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    Color colorkey;
    LODFile_IconsBitmaps *lod;
};

class Image16bit_LOD_Loader : public ImageLoader {
 public:
    inline Image16bit_LOD_Loader(LODFile_IconsBitmaps *lod,
                                 const std::string &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LODFile_IconsBitmaps *lod;
};

class Alpha_LOD_Loader : public ImageLoader {
 public:
    inline Alpha_LOD_Loader(LODFile_IconsBitmaps *lod, const std::string &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LODFile_IconsBitmaps *lod;
};

class PCX_Loader : public ImageLoader {
 protected:
    bool InternalLoad(const Blob &data, RgbaImage *rgbaImage);
};

class PCX_File_Loader : public PCX_Loader {
 public:
    explicit inline PCX_File_Loader(const std::string &filename) {
        this->resource_name = filename;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

    LODFile_IconsBitmaps *lod;
};

class PCX_LOD_Raw_Loader : public PCX_Loader {
 public:
    inline PCX_LOD_Raw_Loader(LOD::File *lod, const std::string &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LOD::File *lod;
};

class PCX_LOD_Compressed_Loader : public PCX_Loader {
 public:
    inline PCX_LOD_Compressed_Loader(LOD::File *lod, const std::string &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LOD::File *lod;
};

class Bitmaps_LOD_Loader : public ImageLoader {
 public:
    inline Bitmaps_LOD_Loader(LODFile_IconsBitmaps *lod, const std::string &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LODFile_IconsBitmaps *lod;
};

class Sprites_LOD_Loader : public ImageLoader {
 public:
    inline Sprites_LOD_Loader(LODFile_Sprites *lod, const std::string &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(RgbaImage *rgbaImage, GrayscaleImage *indexedImage, Palette *palette) override;

 protected:
    LODFile_Sprites *lod;
};
