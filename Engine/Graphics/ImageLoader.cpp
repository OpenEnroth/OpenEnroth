#include "Engine/ZlibWrapper.h"

#include "Engine/Graphics/Render.h"
#include "Engine/Graphics/RenderStruct.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/ImageFormatConverter.h"







bool ColorKey_LOD_Loader::Load(unsigned int *out_width, unsigned int *out_height, void **out_pixels, IMAGE_FORMAT *out_format)
{
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    auto tex = lod->GetTexture(lod->LoadTexture(this->resource_name.c_str(), TEXTURE_16BIT_PALETTE));
    if (tex->pBits & 512)
    {
        Log::Warning(L"Alpha texture is loaded as ColorKey (%S)", this->resource_name.c_str());
    }

    if (tex->pPalette16 && tex->paletted_pixels)
    {
        auto palette = tex->pPalette16;
        auto paletted_pixels = tex->paletted_pixels;

        auto width = tex->uTextureWidth;
        auto height = tex->uTextureHeight;
        auto pixels = new unsigned __int32[width * height];
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
        Log::Warning(L"Alpha texture is loaded as Image16bit (%S)", this->resource_name.c_str());

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
        Log::Warning(L"ColorKey texture is loaded as Alpha (%S)", this->resource_name.c_str());
    }

    if (tex->pPalette16 && tex->paletted_pixels)
    {
        auto palette = tex->pPalette16;
        auto paletted_pixels = tex->paletted_pixels;

        auto width = tex->uTextureWidth;
        auto height = tex->uTextureHeight;
        auto pixels = new unsigned __int32[width * height];
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











#pragma pack(push, 1)
struct PCXHeader1
{
    char magic;
    char version;
    char encoding;
    char bpp;
    __int16 left;
    __int16 up;
    __int16 right;
    __int16 bottom;
    __int16 hres;
    __int16 vres;
};

struct PCXHeader2
{
    char reserved;
    char planes;
    __int16 pitch;
    __int16 palette_info;
};
#pragma pack(pop)





bool PCX_Loader::DecodePCX(const unsigned char *pcx_data, unsigned __int16 *pOutPixels, unsigned int *width, unsigned int *height)
{
    unsigned char test_byte; // edx@3
    unsigned int read_offset; // ebx@37
    unsigned int row_position; // edi@40
    unsigned char value; // cl@63
    char count; // [sp+50h] [bp-Ch]@43
    unsigned short current_line; // [sp+54h] [bp-8h]@38
    unsigned short *dec_position;
    unsigned short *temp_dec_position;
    PCXHeader1 psx_head1;
    PCXHeader2 psx_head2;
    //	short int width, height;
    BYTE  color_map[48];	// Colormap for 16-color images


    memcpy(&psx_head1, pcx_data, 16);
    memcpy(&color_map, pcx_data + 16, 48);
    memcpy(&psx_head2, pcx_data + 64, 6);


    if (psx_head1.bpp != 8)
        return 3;
    *width = (short int)(psx_head1.right - psx_head1.left + 1);  // word @ 000014
    *height = (short int)(psx_head1.bottom - psx_head1.up + 1);  // word @ 000016


    unsigned int uNumPixels = *width * *height;		  // dword @ 000010

    memset(pOutPixels, 0, uNumPixels * sizeof(__int16));

    unsigned int r_mask = 0xF800;
    unsigned int num_r_bits = 5;
    unsigned int g_mask = 0x07E0;
    unsigned int num_g_bits = 6;
    unsigned int b_mask = 0x001F;
    unsigned int num_b_bits = 5;

    //При сохранении изображения подряд идущие пиксели одинакового цвета объединяются и вместо указания цвета для каждого пикселя
    //указывается цвет группы пикселей и их количество.
    read_offset = 128;
    if (psx_head2.planes != 3)
        return 0;
    current_line = 0;
    if (height > 0)
    {
        dec_position = pOutPixels;
        do
        {
            temp_dec_position = dec_position;
            row_position = 0;
            //decode red line
            if (psx_head2.pitch)
            {
                do
                {
                    test_byte = pcx_data[read_offset];
                    ++read_offset;
                    if ((test_byte & 0xC0) == 0xC0)//имеется ли объединение
                    {
                        value = pcx_data[read_offset];
                        ++read_offset;

                        if ((test_byte & 0x3F) > 0)
                        {
                            count = test_byte & 0x3F;//количество одинаковых пикселей
                            do
                            {
                                ++row_position;
                                //*temp_dec_position =0xFF000000;
                                //*temp_dec_position|=(unsigned long)value<<16;
                                *temp_dec_position |= r_mask & ((unsigned __int8)value << (num_g_bits + num_r_bits + num_b_bits - 8));
                                temp_dec_position++;
                                if (row_position == psx_head2.pitch)
                                    break;
                            } while (count-- != 1);
                        }
                    }
                    else
                    {
                        ++row_position;
                        //*temp_dec_position =0xFF000000; 
                        //*temp_dec_position|= (unsigned long)test_byte<<16;

                        *temp_dec_position |= r_mask & ((unsigned __int8)test_byte << (num_g_bits + num_r_bits + num_b_bits - 8));

                        temp_dec_position++;
                    }

                } while (row_position < psx_head2.pitch);
            }

            temp_dec_position = dec_position;
            row_position = 0;
            //decode green line
            while (row_position <  psx_head2.pitch)
            {
                test_byte = *(pcx_data + read_offset);
                ++read_offset;
                if ((test_byte & 0xC0) == 0xC0)
                {
                    value = *(pcx_data + read_offset);
                    ++read_offset;
                    if ((test_byte & 0x3F) > 0)
                    {
                        count = test_byte & 0x3F;
                        do
                        {
                            //*temp_dec_position|= (unsigned int)value<<8;
                            //temp_dec_position++;

                            *temp_dec_position |= g_mask & (unsigned __int16)((unsigned __int8)value << (num_g_bits + num_b_bits - 8));

                            temp_dec_position++;
                            ++row_position;
                            if (row_position == psx_head2.pitch)
                                break;

                        } while (count-- != 1);
                    }
                }
                else
                {
                    //*temp_dec_position |=(unsigned int) test_byte<<8;
                    //temp_dec_position++;

                    *temp_dec_position |= g_mask & (unsigned __int16)((unsigned __int8)test_byte << (num_g_bits + num_b_bits - 8));
                    temp_dec_position++;
                    ++row_position;
                }
            }

            temp_dec_position = dec_position;
            row_position = 0;
            //decode blue line
            while (row_position < psx_head2.pitch)
            {
                test_byte = *(pcx_data + read_offset);
                read_offset++;
                if ((test_byte & 0xC0) == 0xC0)
                {
                    value = *(pcx_data + read_offset);
                    ++read_offset;
                    if ((test_byte & 0x3F) > 0)
                    {
                        count = test_byte & 0x3F;
                        do
                        {
                            //*temp_dec_position|= value;
                            //temp_dec_position++;

                            *temp_dec_position |= value >> (8 - num_b_bits);
                            temp_dec_position++;

                            ++row_position;
                            if (row_position == psx_head2.pitch)
                                break;
                        } while (count-- != 1);
                    }
                }
                else
                {
                    //*temp_dec_position|= test_byte;
                    //temp_dec_position++;
                    *temp_dec_position |= test_byte >> (8 - num_b_bits);
                    temp_dec_position++;

                    ++row_position;
                }

            }
            ++current_line;
            dec_position += *width;
        } while (current_line < *height);
    }

    return true;
}



bool PCX_File_Loader::Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format)
{
    char color_map[48]; // [sp+Ch] [bp-98h]@7
    PCXHeader1 header1; // [sp+84h] [bp-20h]@7
    PCXHeader2 header2; // [sp+94h] [bp-10h]@7

    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    FILE *file = fopen(this->resource_name.c_str(), "rb");
    if (!file)
    {
        Log::Warning(L"Unable to load %s", this->resource_name.c_str());
        return false;
    }

    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    auto file_image = new unsigned char[filesize];
    fread(file_image, 1, filesize, file);
    fclose(file);


    memcpy(&header1, file_image, 0x10u);
    memcpy(color_map, file_image + 16, 0x30u);
    memcpy(&header2, file_image + 64, 6);
    if (header1.bpp != 8)
    {
        delete[] file_image;
        return false;
    }

    *width = header1.right - header1.left + 1;
    *height = header1.bottom - header1.up + 1;
    unsigned int num_pixels = *width * *height;
    *pixels = new unsigned short[num_pixels + 2];

    if (pixels)
    {
        if (!this->DecodePCX(file_image, (unsigned __int16 *)*pixels, width, height))
        {
            delete[] * pixels;
            *pixels = nullptr;
        }
        else
            *format = IMAGE_FORMAT_R5G6B5;
    }

    delete[] file_image;

    return *pixels != nullptr;
}




bool PCX_LOD_Loader::Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format)
{
    FILE *file; // eax@1
    void *v6; // ebx@5
    char color_map[48]; // [sp+Ch] [bp-98h]@7
    Texture_MM7 DstBuf; // [sp+3Ch] [bp-68h]@1
    PCXHeader1 header1; // [sp+84h] [bp-20h]@7
    PCXHeader2 header2; // [sp+94h] [bp-10h]@7
    size_t Count; // [sp+A0h] [bp-4h]@4
    unsigned char *Str1a; // [sp+ACh] [bp+8h]@5

    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    file = lod->FindContainer(this->resource_name.c_str(), 0);
    if (!file)
    {
        Log::Warning(L"Unable to load %s", this->resource_name.c_str());
        return false;
    }

    fread(&DstBuf, 1, 0x30u, file);
    Count = DstBuf.uTextureSize;
    if (DstBuf.uDecompressedSize)
    {
        Str1a = (unsigned char *)malloc(DstBuf.uDecompressedSize);
        v6 = malloc(DstBuf.uTextureSize);
        fread(v6, 1, Count, file);
        zlib::MemUnzip(Str1a, &DstBuf.uDecompressedSize, v6, DstBuf.uTextureSize);
        DstBuf.uTextureSize = DstBuf.uDecompressedSize;
        free(v6);
    }
    else
    {
        Str1a = (unsigned char *)malloc(DstBuf.uTextureSize);
        fread(Str1a, 1, Count, file);
    }

    memcpy(&header1, Str1a, 0x10u);
    memcpy(color_map, Str1a + 16, 0x30u);
    memcpy(&header2, Str1a + 64, 6);
    if (header1.bpp != 8)
    {
        free(Str1a);
        return false;
    }

    *width = header1.right - header1.left + 1;
    *height = header1.bottom - header1.up + 1;
    unsigned int num_pixels = *width * *height;
    *pixels = new unsigned short[num_pixels + 2];

    if (pixels)
    {
        if (!this->DecodePCX(Str1a, (unsigned __int16 *)*pixels, width, height))
        {
            delete[] * pixels;
            *pixels = nullptr;
        }
        else
            *format = IMAGE_FORMAT_R5G6B5;
    }

    free(Str1a);

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
    int num_pixels_bytes = num_pixels * IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT_R5G6B5);
    auto pixels = new unsigned __int16[num_pixels];
    if (pixels)
    {
        *width = tex->uTextureWidth;
        *height = tex->uTextureHeight;
        *format = IMAGE_FORMAT_R5G6B5;

        if (tex->pBits & 2) // hardware bitmap
        {
            HWLTexture* hwl = render->LoadHwlBitmap(this->resource_name.c_str());
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