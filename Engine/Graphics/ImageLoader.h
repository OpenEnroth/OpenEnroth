#pragma once
#include <string>

#include "Engine/IocContainer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Lod/LodIconsBitmaps.h"
#include "Engine/Lod/LodSprites.h"

using EngineIoc = Engine_::IocContainer;


class ImageLoader {
public:
    inline ImageLoader() {
         this->log = EngineIoc::ResolveLogger();
    }
    virtual ~ImageLoader() {}

    virtual std::string GetResourceName() const { return this->resource_name; }

    virtual bool Load(
        unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format
    ) = 0;

 protected:
    std::string resource_name;
    Log *log;
};

class ColorKey_LOD_Loader : public ImageLoader {
public:
    inline ColorKey_LOD_Loader(
        LODFile_IconsBitmaps *lod,
        const std::string &filename,
        uint16_t colorkey
    ) {
        this->resource_name = filename;
        this->colorkey = colorkey;
        this->lod = lod;
    }

    virtual bool Load(
        unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format
    );

 protected:
    uint16_t colorkey;
    LODFile_IconsBitmaps *lod;
};

class Image16bit_LOD_Loader : public ImageLoader {
public:
    inline Image16bit_LOD_Loader(
        LODFile_IconsBitmaps *lod,
        const std::string &filename
    ) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(
        unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format
    );

protected:
    LODFile_IconsBitmaps *lod;
};

class Alpha_LOD_Loader : public ImageLoader {
public:
    inline Alpha_LOD_Loader(LODFile_IconsBitmaps *lod, const std::string &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(
        unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format
    );

 protected:
    LODFile_IconsBitmaps *lod;
};

class PCX_Loader : public ImageLoader {
protected:
    bool DecodePCX(
        const void *pcx_data, uint16_t *pOutPixels, unsigned int *width, unsigned int *height
    );
    bool InternalLoad(
        void *file,
        size_t size,
        unsigned int *width,
        unsigned int *height,
        void **pixels,
        IMAGE_FORMAT *format
    );
};

class PCX_File_Loader : public PCX_Loader {
public:
    explicit inline PCX_File_Loader(const std::string &filename) {
        this->resource_name = filename;
    }

    virtual bool Load(
        unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format
    );

    LODFile_IconsBitmaps *lod;
};

class PCX_LOD_Raw_Loader : public PCX_Loader {
 public:
    inline PCX_LOD_Raw_Loader(std::shared_ptr<Lod::Reader> lod, const std::string &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(
        unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format
    );

 protected:
    std::shared_ptr<Lod::Reader> lod;
};

class PCX_LOD_Compressed_Loader : public PCX_Loader {
 public:
    inline PCX_LOD_Compressed_Loader(Lod::Reader* lod, const std::string &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(
        unsigned int *width, unsigned int *height, void **out_pixels, IMAGE_FORMAT *format
    );

 protected:
    Lod::Reader* lod;
};

class Bitmaps_LOD_Loader : public ImageLoader {
 public:
    inline Bitmaps_LOD_Loader(LODFile_IconsBitmaps *lod, const std::string &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(
        unsigned int *width, unsigned int *height, void **out_pixels, IMAGE_FORMAT *format
    );

 protected:
    LODFile_IconsBitmaps *lod;
};

class Sprites_LOD_Loader : public ImageLoader {
public:
    inline Sprites_LOD_Loader(
        LODFile_Sprites *lod,
        unsigned int palette_id,
        const std::string &filename,
        /*refactor*/ unsigned int lod_sprite_id
    ) {
        this->resource_name = filename;
        this->lod = lod;
        this->palette_id = palette_id;
        this->lod_sprite_id = lod_sprite_id;
    }

    virtual bool Load(
        unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format
    );

protected:
    LODFile_Sprites *lod;
    unsigned int palette_id;
    /*refactor*/ unsigned int lod_sprite_id;
};
