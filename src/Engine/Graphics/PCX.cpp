#include "Engine/Graphics/PCX.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>

// TODO: this is not big-endian compatible!

enum {
    PCX_VERSION_2_5 = 0,
    PCX_VERSION_NOT_VALID = 1,
    PCX_VERSION_2_8_WITH_PALLETE = 2,
    PCX_VERSION_2_8_WITHOUT_PALLETE = 3,
    PCX_VERSION_WINDOWS = 4,
    PCX_VERSION_3_0 = 5
};

#pragma pack(push, 1)
struct PCXHeader {
    int8_t manufacturer;
    int8_t version;
    int8_t compression;
    int8_t bpp;
    int16_t xmin;
    int16_t ymin;
    int16_t xmax;
    int16_t ymax;
    int16_t hdpi;
    int16_t vdpi;
    int8_t pallete[48];
    int8_t reserved;
    int8_t nplanes;
    int16_t bytes_per_row;
    int16_t palette_info;
    int16_t hres;
    int16_t vres;
    int8_t filler[54];
};
#pragma pack(pop)

typedef struct bstreamer {
    const uint8_t *buffer, *buffer_end, *buffer_start;
} bstreamer;

static inline void bs_init(bstreamer *bs, const uint8_t *buf, int buf_size) {
    bs->buffer = bs->buffer_start = buf;
    bs->buffer_end = buf + buf_size;
}

static inline int bs_get_bytes_left(bstreamer *bs) {
    return bs->buffer_end - bs->buffer;
}

static inline unsigned int bs_get_byte(bstreamer *bs) {
    unsigned int byte;
    if (bs_get_bytes_left(bs) > 0) {
        byte = bs->buffer[0];
        bs->buffer++;
        return byte;
    }

    return 0;
}

static inline unsigned int bs_get_buffer(bstreamer *bs, uint8_t *dst, unsigned int size) {
    int size_min = std::min((unsigned int)(bs->buffer_end - bs->buffer), size);
    memcpy(dst, bs->buffer, size_min);
    bs->buffer += size_min;
    return size_min;
}

static int pcx_rle_decode(bstreamer *bs, uint8_t *dst, unsigned int bytes_per_scanline, int compressed) {
    unsigned int i = 0;
    unsigned char run, value;

    if (bs_get_bytes_left(bs) < 1)
        return -1;

    if (compressed) {
        while (i < bytes_per_scanline && bs_get_bytes_left(bs) > 0) {
            run = 1;
            value = bs_get_byte(bs);
            if (value >= 0xc0 && bs_get_bytes_left(bs) > 0) {
                run = value & 0x3f;
                value = bs_get_byte(bs);
            }
            while (i < bytes_per_scanline && run--)
                dst[i++] = value;
        }
    } else {
        bs_get_buffer(bs, dst, bytes_per_scanline);
    }
    return 0;
}

uint8_t *PCX::Decode(const void *pcx_data, size_t filesize, unsigned int *width, unsigned int *height, IMAGE_FORMAT *format, IMAGE_FORMAT requested_format) {
    PCXHeader *header = (PCXHeader *)pcx_data;

    // check that's PCX and its version
    if (header->manufacturer != 0x0a || header->version < PCX_VERSION_2_5 || header->version == PCX_VERSION_NOT_VALID || header->version > PCX_VERSION_3_0) {
        return nullptr;
    }

    *width = header->xmax - header->xmin + 1;
    *height = header->ymax - header->ymin + 1;

    unsigned int bytes_per_scanline = header->nplanes * header->bytes_per_row;

    //corruption check
    if (bytes_per_scanline < (*width * header->bpp * header->nplanes + 7) / 8 ||
        (!header->compression && bytes_per_scanline > (filesize - sizeof(PCXHeader)) / *height)) {
        return nullptr;
    }

    switch ((header->nplanes << 8) + header->bpp) {
        case 0x0308:
            if (requested_format == IMAGE_FORMAT_R5G6B5)
                *format = IMAGE_FORMAT_R5G6B5;
            else
                *format = IMAGE_FORMAT_A8B8G8R8;

            break;
        case 0x0108:
        case 0x0104:
        case 0x0102:
        case 0x0101:
        case 0x0401:
        case 0x0301:
        case 0x0201:
            // TODO: PAL8 color mode
            // fall through
        default:
            *format = IMAGE_INVALID_FORMAT;
            return nullptr;
    }

    size_t pixel_count = 0;
    if (*format == IMAGE_FORMAT_R5G6B5)
        pixel_count = *width * *height * 2;
    else
        pixel_count = *width * *height * 4;

    uint8_t *pixels = new uint8_t[pixel_count];
    if (!pixels)
        return nullptr;

    memset(pixels, 0, pixel_count * sizeof(uint8_t));

    bstreamer bs;
    unsigned int stride = 0;
    uint8_t *scanline = (uint8_t *)malloc(bytes_per_scanline + 32);
    bs_init(&bs, (uint8_t*)pcx_data + sizeof(PCXHeader), filesize - sizeof(PCXHeader));

    if (header->nplanes == 3 && header->bpp == 8) {
        for (unsigned int y = 0; y < *height; y++) {
            int ret = pcx_rle_decode(&bs, scanline, bytes_per_scanline, header->compression);
            if (ret < 0) {
                free(scanline);
                delete[] pixels;
                return nullptr;
            }

            for (unsigned int x = 0; x < *width; x++) {
                if (*format == IMAGE_FORMAT_R5G6B5) {
                    unsigned int r_mask = 0xF800;
                    unsigned int num_r_bits = 5;
                    unsigned int g_mask = 0x07E0;
                    unsigned int num_g_bits = 6;
                    unsigned int b_mask = 0x001F;
                    unsigned int num_b_bits = 5;
                    uint16_t tmp = 0;

                    tmp |= r_mask & (scanline[x] << (num_g_bits + num_r_bits + num_b_bits - 8));
                    tmp |= g_mask & (uint16_t)(scanline[x + header->bytes_per_row] << (num_g_bits + num_b_bits - 8));
                    tmp |= scanline[x + (header->bytes_per_row << 1)] >> (8 - num_b_bits);;

                    pixels[stride + 2 * x] = tmp & 0xff;
                    pixels[stride + 2 * x + 1] = tmp >> 8;
                } else {
                    pixels[stride + 4 * x + 0] = scanline[x];
                    pixels[stride + 4 * x + 1] = scanline[x + header->bytes_per_row];
                    pixels[stride + 4 * x + 2] = scanline[x + (header->bytes_per_row << 1)];
                    pixels[stride + 4 * x + 3] = 255;
                }
            }

            if (*format == IMAGE_FORMAT_R5G6B5)
                stride += *width * 2;
            else
                stride += *width * 4;
        }
    } else {
        // TODO: other planes/bpp variants
        free(scanline);
        delete[] pixels;
        return nullptr;
    }

    free(scanline);

    return pixels;
}

void *WritePCXHeader(void *pcx_data, int width, int height) {
    int pitch = width;
    if (width & 1) {
        pitch = width + 1;
    }

    PCXHeader *header = (PCXHeader *)pcx_data;
    memset(header, 0, sizeof(PCXHeader));
    header->xmin = 0;
    header->ymin = 0;
    header->xmax = width - 1;
    header->ymax = height - 1;
    header->bytes_per_row = pitch;
    header->manufacturer = 10;
    header->version = 5;
    header->compression = 1;
    header->bpp = 8;
    header->hdpi = 75;
    header->vdpi = 75;
    header->nplanes = 3;
    header->palette_info = 1;

    return (uint8_t *)pcx_data + sizeof(PCXHeader);
}

void *EncodeOneLine(void *pcx_data, void *line, size_t line_size) {
    uint8_t *input = (uint8_t *)line;
    uint8_t *end = input + line_size;
    uint8_t *output = (uint8_t *)pcx_data;

    while (input < end) {
        uint8_t value = *input++;

        int count = 1;
        while (count < 63 && input < end && *input == value) {
            input++;
            count++;
        }

        if (count > 1 || (value & 0xC0) != 0)
            *output++ = 0xC0 + count;
        *output++ = value;
    }

    return output;
}

struct ColorFormat {
    explicit ColorFormat(uint32_t m);
    uint32_t mask;
    uint32_t shift;
    uint32_t bits;
};

ColorFormat::ColorFormat(uint32_t m) {
    shift = 0;
    for (int i = 0; i < 16; i++) {
        if (m & 1) {
            break;
        }
        shift++;
        m >>= 1;
    }
    mask = m;
    bits = 0;
    for (int i = 0; i < 16; i++) {
        if (!(m & 1)) {
            break;
        }
        bits++;
        m >>= 1;
    }
}

struct Format {
    Format(size_t bpp, uint32_t rm, uint32_t gm, uint32_t bm)
        : bytes(bpp / 8), r(rm), g(gm), b(bm) {}
    size_t bytes;
    ColorFormat r;
    ColorFormat g;
    ColorFormat b;
};

Blob PCX::Encode(const void *picture_data, const unsigned int width, const unsigned int height) {
    assert(picture_data != nullptr && width != 0 & height != 0);
    Format f(32, 0x000000FF, 0x0000FF00, 0x00FF0000);

    // pcx lines are padded to next even byte boundary
    int pitch = width;
    if (width & 1) {
        pitch = width + 1;
    }

    // pcx file can be larger than uncompressed
    // pcx header and no compression @24bit worst case doubles in size
    size_t worstCase = sizeof(PCXHeader) + 3 * pitch * height * 2;
    uint8_t *pcx_data = (uint8_t*)malloc(worstCase);

    uint8_t *output = (uint8_t *)WritePCXHeader(pcx_data, width, height);

    uint8_t *lineRGB = new uint8_t[3 * pitch];
    uint8_t *lineR = (uint8_t *)lineRGB;
    uint8_t *lineG = (uint8_t *)lineRGB + pitch;
    uint8_t *lineB = (uint8_t *)lineRGB + 2 * pitch;
    uint8_t *input = (uint8_t *)picture_data;

    for (int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            uint32_t pixel;
            memcpy(&pixel, input, f.bytes);
            input += f.bytes;
            lineR[x] = ((pixel >> f.r.shift) & f.r.mask) << (8 - f.r.bits);
            lineG[x] = ((pixel >> f.g.shift) & f.g.mask) << (8 - f.g.bits);
            lineB[x] = ((pixel >> f.b.shift) & f.b.mask) << (8 - f.b.bits);
        }
        uint8_t *line = lineRGB;
        for (int p = 0; p < 3; p++) {
            output = (uint8_t *)EncodeOneLine(output, line, pitch);
            line += pitch;
        }
    }

    delete[] lineRGB;

    size_t packed_size = output - (uint8_t*)pcx_data;
    assert(packed_size <= worstCase);
    return Blob::fromMalloc(pcx_data, packed_size);
}
