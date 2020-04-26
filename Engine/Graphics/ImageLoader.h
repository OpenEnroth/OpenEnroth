#pragma once
#include "Engine/LOD.h"
#include "Engine/IocContainer.h"
#include "Engine/Strings.h"

#include "Engine/Graphics/Image.h"

using EngineIoc = Engine_::IocContainer;

class ImageLoader {
 public:
    inline ImageLoader() {
         this->log = EngineIoc::ResolveLogger();
    }
    virtual ~ImageLoader() {}
    virtual String GetResourceName() const { return this->resource_name; }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels,
                      IMAGE_FORMAT *format) = 0;

 protected:
    String resource_name;
    Log *log;
};

class ColorKey_LOD_Loader : public ImageLoader {
 public:
    inline ColorKey_LOD_Loader(LODFile_IconsBitmaps *lod,
                               const String &filename, uint16_t colorkey) {
        this->resource_name = filename;
        this->colorkey = colorkey;
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels,
                      IMAGE_FORMAT *format);

 protected:
    uint16_t colorkey;
    LODFile_IconsBitmaps *lod;
};

class Image16bit_LOD_Loader : public ImageLoader {
 public:
    inline Image16bit_LOD_Loader(LODFile_IconsBitmaps *lod,
                                 const String &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels,
                      IMAGE_FORMAT *format);

 protected:
    LODFile_IconsBitmaps *lod;
};

class Alpha_LOD_Loader : public ImageLoader {
 public:
    inline Alpha_LOD_Loader(LODFile_IconsBitmaps *lod, const String &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels,
                      IMAGE_FORMAT *format);

 protected:
    LODFile_IconsBitmaps *lod;
};

class PCX_Loader : public ImageLoader {
 protected:
    bool DecodePCX(const void *pcx_data, uint16_t *pOutPixels,
                   unsigned int *width, unsigned int *height);
};

class PCX_File_Loader : public PCX_Loader {
 public:
    inline PCX_File_Loader(LODFile_IconsBitmaps *lod, const String &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels,
                      IMAGE_FORMAT *format);

 protected:
    bool InternalLoad(void *file, size_t size, unsigned int *width,
                      unsigned int *height, void **pixels,
                      IMAGE_FORMAT *format);
    LODFile_IconsBitmaps *lod;
};

class PCX_LOD_File_Loader : public PCX_File_Loader {
 public:
    inline PCX_LOD_File_Loader(LOD::File *lod, const String &filename)
        : PCX_File_Loader(nullptr, filename) {
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels,
                      IMAGE_FORMAT *format);

 protected:
    LOD::File *lod;
};

class PCX_LOD_Loader : public PCX_Loader {
 public:
    inline PCX_LOD_Loader(LOD::File *lod, const String &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height,
                      void **out_pixels, IMAGE_FORMAT *format);

 protected:
    LOD::File *lod;
};

class Bitmaps_LOD_Loader : public ImageLoader {
 public:
    inline Bitmaps_LOD_Loader(LODFile_IconsBitmaps *lod,
                              const String &filename) {
        this->resource_name = filename;
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height,
                      void **out_pixels, IMAGE_FORMAT *format);

 protected:
    LODFile_IconsBitmaps *lod;
};

class Sprites_LOD_Loader : public ImageLoader {
 public:
    inline Sprites_LOD_Loader(LODFile_Sprites *lod, unsigned int palette_id,
                              const String &filename,
                              /*refactor*/ unsigned int lod_sprite_id) {
        this->resource_name = filename;
        this->lod = lod;
        this->palette_id = palette_id;
        this->lod_sprite_id = lod_sprite_id;
    }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels,
                      IMAGE_FORMAT *format);

 protected:
    LODFile_Sprites *lod;
    unsigned int palette_id;
    /*refactor*/ unsigned int lod_sprite_id;
};
