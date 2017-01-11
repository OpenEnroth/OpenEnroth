#pragma once


typedef bool (*ImageFormatConverter)(unsigned int num_pixels, const void *src, void *dst);


inline unsigned __int32 R5G6B5_to_A8R8G8B8(unsigned __int16 color16, unsigned char alpha)
{
    unsigned __int32 c = color16;
    unsigned int b = (c & 31) * 8;
    unsigned int g = ((c >> 5) & 63) * 4;
    unsigned int r = ((c >> 11) & 31) * 8;

    return ((unsigned int)alpha << 24) | (r << 16) | (g << 8) | b;
}

inline unsigned __int16 A8R8G8B8_to_R5G6B5(unsigned __int32 c)
{
    unsigned __int32 b = ((c & 0xFF) / 8) & 31;
    unsigned __int32 g = (((c >> 8) & 0xFF) / 4) & 63;
    unsigned __int32 r = (((c >> 16) & 0xFF) / 8) & 31;

    return (unsigned __int16)(
        (r << 11) | (g << 5) | b
    );
}

inline bool Image_A8R8G8B8_to_R5G6B5(unsigned int num_pixels, const void *src_pixels, void *dst_pixels)
{
    auto src = (unsigned __int32 *)src_pixels;
    auto dst = (unsigned __int16 *)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i)
    {
        dst[i] = A8R8G8B8_to_R5G6B5(src[i]);
    }

    return true;
}

inline bool Image_R5G6B5_to_A8R8G8B8(unsigned int num_pixels, const void *src_pixels, void *dst_pixels)
{
    auto src = (unsigned __int16 *)src_pixels;
    auto dst = (unsigned __int32 *)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i)
    {
        dst[i] = R5G6B5_to_A8R8G8B8(src[i], 0xFF);
    }

    return true;
}