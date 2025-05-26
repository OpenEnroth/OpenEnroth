#include "Pcx.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <memory>

#include "Utility/Exception.h"

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

RgbaImage pcx::decode(const Blob &data) {
    if (data.size() < sizeof(PCXHeader))
        throw Exception("PCX image '{}' too small, expected at least {} bytes, got {}", data.displayPath(), sizeof(PCXHeader), data.size());

    const PCXHeader *header = static_cast<const PCXHeader *>(data.data());

    // check that's PCX and its version
    if (header->manufacturer != 0x0a)
        throw Exception("Invalid PCX starting byte in image '{}', expected {:02x}, got {:02x}", data.displayPath(), 0x0a, header->manufacturer);

    if (header->version < PCX_VERSION_2_5 || header->version == PCX_VERSION_NOT_VALID || header->version > PCX_VERSION_3_0)
        throw Exception("Invalid PCX version '{}' in '{}'", header->version, data.displayPath());

    size_t width = header->xmax - header->xmin + 1;
    size_t height = header->ymax - header->ymin + 1;

    unsigned int bytes_per_scanline = header->nplanes * header->bytes_per_row;

    //corruption check
    if (bytes_per_scanline < (width * header->bpp * header->nplanes + 7) / 8 ||
        (!header->compression && bytes_per_scanline > (data.size() - sizeof(PCXHeader)) / height)) {
        throw Exception("PCX header corrupted in '{}'", data.displayPath());
    }

    if ((header->nplanes != 3 && header->nplanes != 1) || header->bpp != 8)
        throw Exception("Unsupported PCX format in '{}', only 8-bit and 24-bit PCX images are supported", data.displayPath());

    RgbaImage result = RgbaImage::uninitialized(width, height);

    bstreamer bs;
    std::unique_ptr<uint8_t[], FreeDeleter> scanline(static_cast<uint8_t *>(malloc(bytes_per_scanline + 32)));
    bs_init(&bs, static_cast<const uint8_t *>(data.data()) + sizeof(PCXHeader), data.size() - sizeof(PCXHeader));

    for (unsigned int y = 0; y < height; y++) {
        int ret = pcx_rle_decode(&bs, scanline.get(), bytes_per_scanline, header->compression);
        if (ret < 0)
            throw Exception("PCX image data is corrupted in '{}'", data.displayPath());

        auto line = result[y];
        if (header->nplanes == 1) {
            for (unsigned int x = 0; x < width; x++)
                line[x] = Color(scanline[x], scanline[x], scanline[x]);
        } else {
            assert(header->nplanes == 3);
            for (unsigned int x = 0; x < width; x++)
                line[x] = Color(scanline[x], scanline[x + header->bytes_per_row], scanline[x + (header->bytes_per_row << 1)]);
        }
    }

    return result;
}

void *writePcxHeader(void *pcx_data, int width, int height) {
    int pitch = width;
    if (width & 1) {
        pitch = width + 1;
    }

    PCXHeader *header = static_cast<PCXHeader *>(pcx_data);
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

    return static_cast<uint8_t *>(pcx_data) + sizeof(PCXHeader);
}

void *encodeOneLine(void *pcx_data, void *line, size_t line_size) {
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

Blob pcx::encode(RgbaImageView image) {
    assert(image);

    size_t width = image.width();
    size_t height = image.height();

    // pcx lines are padded to next even byte boundary
    size_t pitch = width;
    if (pitch & 1)
        pitch = pitch + 1;

    // pcx file can be larger than uncompressed
    // pcx header and no compression @24bit worst case doubles in size
    size_t worstCase = sizeof(PCXHeader) + 3 * pitch * height * 2;
    std::unique_ptr<uint8_t[], FreeDeleter> pcx_data(static_cast<uint8_t *>(malloc(worstCase)));

    uint8_t *output = (uint8_t *) writePcxHeader(pcx_data.get(), width, height);

    std::unique_ptr<uint8_t[]> lineRGB(new uint8_t[3 * pitch]);
    uint8_t *lineR = lineRGB.get();
    uint8_t *lineG = lineRGB.get() + pitch;
    uint8_t *lineB = lineRGB.get() + 2 * pitch;
    const Color *input = image.pixels().data();

    for (int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            Color pixel = *input++;
            lineR[x] = pixel.r;
            lineG[x] = pixel.g;
            lineB[x] = pixel.b;
        }
        uint8_t *line = lineRGB.get();
        for (int p = 0; p < 3; p++) {
            output = (uint8_t *) encodeOneLine(output, line, pitch);
            line += pitch;
        }
    }

    size_t packed_size = output - pcx_data.get();
    assert(packed_size <= worstCase);
    return Blob::fromMalloc(pcx_data.release(), packed_size);
}
