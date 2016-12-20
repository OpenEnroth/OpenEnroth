#pragma once
#include "Engine/Strings.h"
#include "Engine/Lod.h"

#include "Engine/Graphics/Image.h"

class ImageLoader
{
    public:
        virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format) = 0;
};




class ColorKey_LOD_Loader : public ImageLoader
{
    public:
        inline ColorKey_LOD_Loader(LODFile_IconsBitmaps *lod, const String &filename, unsigned __int16 colorkey)
        {
            this->filename = filename;
            this->colorkey = colorkey;
            this->lod = lod;
        }

        virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format);



    protected:
        String                filename;
        unsigned __int16      colorkey;
        LODFile_IconsBitmaps *lod;
};



class Image16bit_LOD_Loader : public ImageLoader
{
    public:
        inline Image16bit_LOD_Loader(LODFile_IconsBitmaps *lod, const String &filename)
        {
            this->filename = filename;
            this->lod = lod;
        }

        virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format);



    protected:
        String                filename;
        LODFile_IconsBitmaps *lod;
};

class Alpha_LOD_Loader : public ImageLoader
{
    public:
        inline Alpha_LOD_Loader(LODFile_IconsBitmaps *lod, const String &filename)
        {
            this->filename = filename;
            this->lod = lod;
        }

        virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format);



    protected:
        String                filename;
        LODFile_IconsBitmaps *lod;
};




class PCX_Loader : public ImageLoader
{
    protected:
        bool DecodePCX(const unsigned char *pcx_data, unsigned __int16 *pOutPixels, unsigned int *width, unsigned int *height);
};

class PCX_File_Loader : public PCX_Loader
{
    public:
        inline PCX_File_Loader(LODFile_IconsBitmaps *lod, const String &filename)
        {
            this->filename = filename;
            this->lod = lod;
        }

        virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format);



    protected:
        String                filename;
        LODFile_IconsBitmaps *lod;
};


class PCX_LOD_Loader : public PCX_Loader
{
    public:
        inline PCX_LOD_Loader(LOD::File *lod, const String &filename)
        {
            this->filename = filename;
            this->lod = lod;
        }

        virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format);



    protected:
        String     filename;
        LOD::File *lod;
};