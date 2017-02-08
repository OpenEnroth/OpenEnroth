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


inline unsigned int R5G6B5_extract_R(unsigned __int16 c) { return 8 * ((c >> 11) & 0x1F); }
inline unsigned int R5G6B5_extract_G(unsigned __int16 c) { return 4 * ((c >> 5) & 0x3F); }
inline unsigned int R5G6B5_extract_B(unsigned __int16 c) { return 8 * ((c >> 0) & 0x1F); }

inline bool Image_R5G6B5_to_R8G8B8(unsigned int num_pixels, const void *src_pixels, void *dst_pixels)
{
    auto src = (unsigned __int16 *)src_pixels;
    auto dst = (unsigned __int8 *)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i)
    {
        dst[i * 3 + 0] = R5G6B5_extract_R(src[i]);
        dst[i * 3 + 1] = R5G6B5_extract_G(src[i]);
        dst[i * 3 + 2] = R5G6B5_extract_B(src[i]);
    }

    return true;
}


inline unsigned int A1R5G5B5_extract_A(unsigned __int16 c) { return c & 0x8000 ? 255 : 0; }
inline unsigned int A1R5G5B5_extract_R(unsigned __int16 c) { return 8 * ((c >> 10) & 0x1F); }
inline unsigned int A1R5G5B5_extract_G(unsigned __int16 c) { return 8 * ((c >> 5) & 0x1F); }
inline unsigned int A1R5G5B5_extract_B(unsigned __int16 c) { return 8 * ((c >> 0) & 0x1F); }

inline bool Image_A1R5G5B5_to_R8G8B8A8(unsigned int num_pixels, const void *src_pixels, void *dst_pixels)
{
    auto src = (unsigned __int16 *)src_pixels;
    auto dst = (unsigned __int8 *)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i)
    {
        dst[i * 4 + 0] = A1R5G5B5_extract_R(src[i]);
        dst[i * 4 + 1] = A1R5G5B5_extract_G(src[i]);
        dst[i * 4 + 2] = A1R5G5B5_extract_B(src[i]);
        dst[i * 4 + 3] = A1R5G5B5_extract_A(src[i]);
    }

    return true;
}