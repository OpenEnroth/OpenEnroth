#pragma once

typedef bool (*ImageFormatConverter)(unsigned int num_pixels, const void *src,
                                     void *dst);

inline uint32_t R5G6B5_to_A8R8G8B8(uint16_t color16, unsigned char alpha) {
    uint32_t c = color16;
    unsigned int b = (c & 31) * 8;
    unsigned int g = ((c >> 5) & 63) * 4;
    unsigned int r = ((c >> 11) & 31) * 8;

    return ((unsigned int)alpha << 24) | (r << 16) | (g << 8) | b;
}

inline uint32_t R5G6B5_to_R8G8B8A8(uint16_t color16, unsigned char alpha) {  // eh what?? ABGR
    uint32_t c = color16;
    unsigned int b = (c & 31) * 8;
    unsigned int g = ((c >> 5) & 63) * 4;
    unsigned int r = ((c >> 11) & 31) * 8;

    return (((unsigned int)alpha << 24) & 0xFF000000) | ((b << 16) & 0x00FF0000) | ((g << 8) & 0x0000FF00) | (r & 0x000000FF);
}

inline uint16_t A8R8G8B8_to_R5G6B5(uint32_t c) {
    uint32_t b = ((c & 0xFF) / 8) & 31;
    uint32_t g = (((c >> 8) & 0xFF) / 4) & 63;
    uint32_t r = (((c >> 16) & 0xFF) / 8) & 31;

    return (uint16_t)((r << 11) | (g << 5) | b);
}

inline uint16_t A8R8G8B8_to_A1R5G5B5(uint32_t c) {
    uint32_t b = ((c & 0xFF) / 8) & 31;
    uint32_t g = (((c >> 8) & 0xFF) / 8) & 31;
    uint32_t r = (((c >> 16) & 0xFF) / 8) & 31;
    uint32_t a = (((c >> 24) & 0xFF)) == 255;

    return (uint16_t)( (a << 15) |(r << 10) | (g << 5) | b);
}


inline uint32_t A8R8G8B8_to_R8G8B8A8(uint32_t c) {  // eh waht ?? ABGR
    return (c & 0xFF000000) | (c & 0x000000FF) << 16 | (c & 0x0000FF00) | (c & 0x00FF0000) >> 16;
}

inline bool Image_A8R8G8B8_to_R8G8B8A8(unsigned int num_pixels, const void *src_pixels, void *dst_pixels) {
    auto src = (uint32_t*)src_pixels;
    auto dst = (uint32_t*)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i) {
        dst[i] = A8R8G8B8_to_R8G8B8A8(src[i]);
    }

    return true;
}

inline bool Image_A8R8G8B8_to_R5G6B5(unsigned int num_pixels,
                                     const void *src_pixels, void *dst_pixels) {
    auto src = (uint32_t*)src_pixels;
    auto dst = (uint16_t*)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i) {
        dst[i] = A8R8G8B8_to_R5G6B5(src[i]);
    }

    return true;
}

inline bool Image_A8R8G8B8_to_A1R5G5B5(unsigned int num_pixels,
    const void *src_pixels, void *dst_pixels) {

    auto src = (uint32_t*)src_pixels;
    auto dst = (uint16_t*)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i) {
        dst[i] = A8R8G8B8_to_A1R5G5B5(src[i]);
    }

    return true;
}

inline uint16_t R8G8B8A8_to_R5G6B5(uint32_t c) {
    uint32_t b = (((c >> 16) & 0xFF) / 8) & 31;
    uint32_t g = (((c >> 8) & 0xFF) / 4) & 63;
    uint32_t r = (((c) & 0xFF) / 8) & 31;

    return (uint16_t)((r << 11) | (g << 5) | b);
}

inline bool Image_R8G8B8A8_to_R5G6B5(unsigned int num_pixels,
    const void *src_pixels, void *dst_pixels) {
    auto src = (uint32_t *)src_pixels;
    auto dst = (uint16_t *)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i) {
        dst[i] = R8G8B8A8_to_R5G6B5(src[i]);
    }

    return true;
}

inline bool Image_R5G6B5_to_A8R8G8B8(unsigned int num_pixels,
                                     const void *src_pixels, void *dst_pixels) {
    auto src = (uint16_t*)src_pixels;
    auto dst = (uint32_t*)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i) {
        dst[i] = R5G6B5_to_A8R8G8B8(src[i], 0xFF);
    }

    return true;
}

inline bool Image_R5G6B5_to_R8G8B8A8(unsigned int num_pixels,
    const void *src_pixels, void *dst_pixels) {
    auto src = (uint16_t*)src_pixels;
    auto dst = (uint32_t*)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i) {
        dst[i] = R5G6B5_to_R8G8B8A8(src[i], 255);
    }

    return true;
}

inline unsigned int R5G6B5_extract_R(uint16_t c) {
    return 8 * ((c >> 11) & 0x1F);
}
inline unsigned int R5G6B5_extract_G(uint16_t c) {
    return 4 * ((c >> 5) & 0x3F);
}
inline unsigned int R5G6B5_extract_B(uint16_t c) {
    return 8 * ((c >> 0) & 0x1F);
}

inline bool Image_R5G6B5_to_R8G8B8(unsigned int num_pixels,
                                   const void *src_pixels, void *dst_pixels) {
    auto src = (uint16_t*)src_pixels;
    auto dst = (uint8_t*)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i) {
        dst[i * 3 + 0] = R5G6B5_extract_R(src[i]);
        dst[i * 3 + 1] = R5G6B5_extract_G(src[i]);
        dst[i * 3 + 2] = R5G6B5_extract_B(src[i]);
    }

    return true;
}

inline unsigned int A1R5G5B5_extract_A(uint16_t c) {
    return c & 0x8000 ? 255 : 0;
}
inline unsigned int A1R5G5B5_extract_R(uint16_t c) {
    return 8 * ((c >> 10) & 0x1F);
}
inline unsigned int A1R5G5B5_extract_G(uint16_t c) {
    return 8 * ((c >> 5) & 0x1F);
}
inline unsigned int A1R5G5B5_extract_B(uint16_t c) {
    return 8 * ((c >> 0) & 0x1F);
}

inline bool Image_A1R5G5B5_to_R8G8B8A8(unsigned int num_pixels,
                                       const void *src_pixels,
                                       void *dst_pixels) {
    auto src = (uint16_t*)src_pixels;
    auto dst = (uint8_t*)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i) {
        dst[i * 4 + 0] = A1R5G5B5_extract_R(src[i]);
        dst[i * 4 + 1] = A1R5G5B5_extract_G(src[i]);
        dst[i * 4 + 2] = A1R5G5B5_extract_B(src[i]);
        dst[i * 4 + 3] = A1R5G5B5_extract_A(src[i]);
    }

    return true;
}
