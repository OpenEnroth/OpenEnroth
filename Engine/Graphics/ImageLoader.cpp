#include "Engine/Log.h"
#include "Engine/ZlibWrapper.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/ImageFormatConverter.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/PCX.h"







bool ColorKey_LOD_Loader::Load(unsigned int *out_width, unsigned int *out_height, void **out_pixels, IMAGE_FORMAT *out_format)
{
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    auto tex = lod->GetTexture(lod->LoadTexture(this->resource_name.c_str(), TEXTURE_16BIT_PALETTE));
    if (tex->pBits & 512)
    {
        logger->Warning(L"Alpha texture is loaded as ColorKey (%S)", this->resource_name.c_str());
    }

    if (tex->pPalette16 && tex->paletted_pixels)
    {
        auto palette = tex->pPalette16;
        auto paletted_pixels = tex->paletted_pixels;

        auto width = tex->uTextureWidth;
        auto height = tex->uTextureHeight;					//memory leak - background keeps relaoding?
        auto pixels = new unsigned __int32[width * height]; //Unhandled exception at 0x759DA832 in World of Might and Magic.exe: Microsoft C++ exception: std::bad_alloc at memory location 0x01CFEB10.
        if (pixels)
        {
            for (unsigned int y = 0; y < height; ++y)
            {
                for (unsigned int x = 0; x < width; ++x)
                {
                    auto pixel = tex->pPalette16[paletted_pixels[y * width + x]];
                    if (pixel == colorkey)
                        pixels[y * width + x] = 0x00000000;
                    else
                    {
                        extern unsigned __int32 Color32(unsigned __int16 color16);
                        pixels[y * width + x] = R5G6B5_to_A8R8G8B8(pixel, 255);
                    }
                }
            }
            *out_width = width;
            *out_height = height;
            *out_pixels = pixels;
            *out_format = IMAGE_FORMAT_A8R8G8B8;
        }
    }

    return *out_pixels != nullptr;
}


bool Image16bit_LOD_Loader::Load(unsigned int *out_width, unsigned int *out_height, void **out_pixels, IMAGE_FORMAT *out_format)
{
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    auto tex = lod->GetTexture(lod->LoadTexture(this->resource_name.c_str(), TEXTURE_16BIT_PALETTE));
    if (tex->pBits & 512)
        logger->Warning(L"Alpha texture is loaded as Image16bit (%S)", this->resource_name.c_str());

    if (tex->pPalette16 && tex->paletted_pixels)
    {
        auto palette = tex->pPalette16;
        auto paletted_pixels = tex->paletted_pixels;

        auto width = tex->uTextureWidth;
        auto height = tex->uTextureHeight;
        auto pixels = new unsigned __int16[width * height];
        if (pixels)
        {
            for (unsigned int y = 0; y < height; ++y)
                for (unsigned int x = 0; x < width; ++x)
                {
                    auto index = paletted_pixels[y * width + x];
                    auto pixel = tex->pPalette16[index];

                    pixels[y * width + x] = pixel;
                }
            *out_width = width;
            *out_height = height;
            *out_pixels = pixels;
            *out_format = IMAGE_FORMAT_R5G6B5;
        }
    }

    return *out_pixels != nullptr;
}






bool Alpha_LOD_Loader::Load(unsigned int *out_width, unsigned int *out_height, void **out_pixels, IMAGE_FORMAT *out_format)
{
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    auto tex = lod->GetTexture(
        lod->LoadTexture(this->resource_name.c_str(), TEXTURE_16BIT_PALETTE)
    );
    if (~tex->pBits & 512)
    {
        logger->Warning(L"ColorKey texture is loaded as Alpha (%S)", this->resource_name.c_str());
    }

    if (tex->pPalette16 && tex->paletted_pixels)
    {
        auto palette = tex->pPalette16;
        auto paletted_pixels = tex->paletted_pixels;

        auto width = tex->uTextureWidth;
        auto height = tex->uTextureHeight;
        auto pixels = new unsigned __int32[width * height]; //Unhandled exception at 0x759DA832 in World of Might and Magic.exe: Microsoft C++ exception: std::bad_alloc at memory location 0x009CEA4C.
        if (pixels)
        {
            for (unsigned int y = 0; y < height; ++y)
                for (unsigned int x = 0; x < width; ++x)
                {
                    auto index = paletted_pixels[y * width + x];
                    auto pixel = tex->pPalette16[index];
                    if (index == 0)
                        pixels[y * width + x] = 0x00000000;
                    else
                    {
                        extern unsigned __int32 Color32(unsigned __int16 color16);
                        pixels[y * width + x] = R5G6B5_to_A8R8G8B8(pixel, 255);
                    }
                }
            *out_width = width;
            *out_height = height;
            *out_pixels = pixels;
            *out_format = IMAGE_FORMAT_A8R8G8B8;
        }
    }

    return *out_pixels != nullptr;
}











bool PCX_Loader::DecodePCX(const unsigned char *pcx_data, unsigned __int16 *pOutPixels, unsigned int *width, unsigned int *height)
{
  return PCX::Decode(pcx_data, pOutPixels, width, height);
}



bool PCX_File_Loader::Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format)
{
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    FILE *file = fopen(this->resource_name.c_str(), "rb");
    if (!file)
    {
        logger->Warning(L"Unable to load %s", this->resource_name.c_str());
        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    bool res = InternalLoad(file, filesize, width, height, pixels, format);

    fclose(file);

    return res;
}

bool PCX_File_Loader::InternalLoad(FILE* file, size_t filesize, unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format)
{
    unsigned char *file_image = new unsigned char[filesize];
    fread(file_image, 1, filesize, file);

    if (!PCX::IsValid(file_image)) {
        return false;
    }

    PCX::GetSize(file_image, width, height);
    unsigned int num_pixels = *width * *height;
    *pixels = new unsigned short[num_pixels + 2];

    if (pixels) {
        if (!this->DecodePCX(file_image, (unsigned __int16 *)*pixels, width, height)) {
            delete[] * pixels;
            *pixels = nullptr;
        } else {
            *format = IMAGE_FORMAT_R5G6B5;
        }
    }

    return *pixels != nullptr;
}

bool PCX_LOD_File_Loader::Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format)
{
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    size_t size;
    FILE *file = lod->FindContainer(this->resource_name.c_str(), 0, &size);
    if (!file) {
        logger->Warning(L"Unable to load %s", this->resource_name.c_str());
        return false;
    }

    return InternalLoad(file, size, width, height, pixels, format);
}



bool PCX_LOD_Loader::Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format)
{
    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    FILE *file = lod->FindContainer(this->resource_name.c_str(), 0);
    if (!file) {
        logger->Warning(L"Unable to load %s", this->resource_name.c_str());
        return false;
    }

    Texture_MM7 DstBuf;
    fread(&DstBuf, 1, 0x30u, file);
    size_t Count = DstBuf.uTextureSize;
    unsigned char *pcx_data;
    if (DstBuf.uDecompressedSize) {
        pcx_data = (unsigned char *)malloc(DstBuf.uDecompressedSize);
        void *v6 = malloc(DstBuf.uTextureSize);
        fread(v6, 1, Count, file);
        zlib::Uncompress(pcx_data, &DstBuf.uDecompressedSize, v6, DstBuf.uTextureSize);
        DstBuf.uTextureSize = DstBuf.uDecompressedSize;
        free(v6);
    } else {
        pcx_data = (unsigned char *)malloc(DstBuf.uTextureSize);
        fread(pcx_data, 1, Count, file);
    }

    if (!PCX::IsValid(pcx_data)) {
        free(pcx_data);
        return false;
    }

    PCX::GetSize(pcx_data, width, height);
    unsigned int num_pixels = *width * *height;
    *pixels = new unsigned short[num_pixels + 2];

    if (pixels) {
        if (!this->DecodePCX(pcx_data, (unsigned __int16 *)*pixels, width, height)) {
            delete[] * pixels;
            *pixels = nullptr;
        }
        else
            *format = IMAGE_FORMAT_R5G6B5;
    }

    free(pcx_data);

    return *pixels != nullptr;
}



bool Bitmaps_LOD_Loader::Load(unsigned int *width, unsigned int *height, void **out_pixels, IMAGE_FORMAT *format)
{
    *width = 0;
    *height = 0;
    *out_pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    auto tex = lod->GetTexture(
        lod->LoadTexture(this->resource_name.c_str())
    );

    int num_pixels = tex->uTextureWidth * tex->uTextureHeight;
    int num_pixels_bytes = num_pixels * IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT_A1R5G5B5);
    auto pixels = new unsigned __int16[num_pixels];
    if (pixels)
    {
        *width = tex->uTextureWidth;
        *height = tex->uTextureHeight;
        *format = IMAGE_FORMAT_A1R5G5B5;

        if (tex->pBits & 2) // hardware bitmap
        {
            HWLTexture *hwl = render->LoadHwlBitmap(this->resource_name.c_str());
            if (hwl)
            {
                // linear scaling
                for (int s = 0; s < tex->uTextureHeight; ++s)
                {
                    for (int t = 0; t < tex->uTextureWidth; ++t)
                    {
                        unsigned int resampled_x = t * hwl->uWidth / tex->uTextureWidth,
                                     resampled_y = s * hwl->uHeight / tex->uTextureHeight;
                        unsigned short sample = hwl->pPixels[resampled_y * hwl->uWidth + resampled_x];

                        pixels[s * tex->uTextureWidth + t] = sample;
                    }
                }

                delete[] hwl->pPixels;
                delete hwl;
            }

            *out_pixels = pixels;
            return true;
        }
    }

    return false;
}



bool Sprites_LOD_Loader::Load(unsigned int *width, unsigned int *height, void **out_pixels, IMAGE_FORMAT *format)
{
    *width = 0;
    *height = 0;
    *out_pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    HWLTexture *hwl = render->LoadHwlSprite(this->resource_name.c_str());
    if (hwl)
    {
        auto lod_sprite = &lod->pSpriteHeaders[this->lod_sprite_id];
        auto lod_hd_sprite = &lod->pHardwareSprites[this->lod_sprite_id];

        int dst_width = hwl->uWidth;
        int dst_height = hwl->uHeight;

        int num_pixels = dst_width * dst_height;
        int num_pixels_bytes = num_pixels * IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT_A1R5G5B5);
        auto pixels = new unsigned __int16[num_pixels];
        if (pixels)
        {
            // linear scaling
            for (int s = 0; s < dst_height; ++s)
            {
                for (int t = 0; t < dst_width; ++t)
                {
                    unsigned int
                        resampled_x = t * hwl->uWidth / dst_width,
                        resampled_y = s * hwl->uHeight / dst_height;

                    unsigned short sample = hwl->pPixels[resampled_y * hwl->uWidth + resampled_x];

                    pixels[s * dst_width + t] = sample;
                }
            }

            delete[] hwl->pPixels;
            delete hwl;


            *width = dst_width;
            *height = dst_height;
            *format = IMAGE_FORMAT_A1R5G5B5;
        }

        *out_pixels = pixels;
        return true;
    }

    return false;
}
