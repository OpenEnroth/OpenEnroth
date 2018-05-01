#include "Engine/Graphics/PCX.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>

#pragma pack(push, 1)
struct PCXHeader {
    int8_t manufacturer;
    int8_t version;
    int8_t encoding;
    int8_t bpp;
    int16_t left;
    int16_t up;
    int16_t right;
    int16_t bottom;
    int16_t hdpi;
    int16_t vdpi;
    int8_t color_map[48];
    int8_t reserved;
    int8_t planes;
    int16_t pitch;
    int16_t palette_info;
    int16_t hres;
    int16_t vres;
    int8_t filler[54];
};
#pragma pack(pop)

bool PCX::IsValid(const void *pcx_data) {
    PCXHeader *header = (PCXHeader *)pcx_data;
    return (header->bpp == 8) && (header->planes == 3);
}

void PCX::GetSize(const void *pcx_data, unsigned int *width, unsigned int *height) {
    PCXHeader *header = (PCXHeader *)pcx_data;
    *width = header->right - header->left + 1;
    *height = header->bottom - header->up + 1;
}

bool PCX::Decode(const void *pcx_data, uint16_t *pOutPixels,
                 unsigned int *width, unsigned int *height) {
    PCXHeader *header = (PCXHeader *)pcx_data;

    if (!IsValid(pcx_data)) {
        return false;
    }

    GetSize(pcx_data, width, height);

    memset(pOutPixels, 0, *width * *height * sizeof(int16_t));

    unsigned int r_mask = 0xF800;
    unsigned int num_r_bits = 5;
    unsigned int g_mask = 0x07E0;
    unsigned int num_g_bits = 6;
    unsigned int b_mask = 0x001F;
    unsigned int num_b_bits = 5;

    unsigned char test_byte;      // edx@3
    unsigned int row_position;    // edi@40
    unsigned char value;          // cl@63
    char count;                   // [sp+50h] [bp-Ch]@43
    unsigned short *dec_position;
    unsigned short *temp_dec_position;

    uint8_t *input = (uint8_t*)pcx_data;

    // При сохранении изображения подряд идущие пиксели одинакового цвета
    // объединяются и вместо указания цвета для каждого пикселя указывается цвет
    // группы пикселей и их количество.
    unsigned int read_offset = sizeof(PCXHeader);
    unsigned short current_line = 0;
    if (*height > 0) {
        dec_position = pOutPixels;
        do {
            temp_dec_position = dec_position;
            row_position = 0;
            // decode red line
            if (header->pitch) {
                do {
                    test_byte = input[read_offset];
                    ++read_offset;
                    if ((test_byte & 0xC0) == 0xC0) {  // имеется ли объединение
                        value = input[read_offset];
                        ++read_offset;

                        if ((test_byte & 0x3F) > 0) {
                            count = test_byte &
                                    0x3F;  // количество одинаковых пикселей
                            do {
                                ++row_position;
                                *temp_dec_position |=
                                    r_mask & ((uint8_t)value
                                              << (num_g_bits + num_r_bits +
                                                  num_b_bits - 8));
                                temp_dec_position++;
                                if (row_position == header->pitch) break;
                            } while (count-- != 1);
                        }
                    } else {
                        ++row_position;
                        *temp_dec_position |=
                            r_mask &
                            ((uint8_t)test_byte
                             << (num_g_bits + num_r_bits + num_b_bits - 8));
                        temp_dec_position++;
                    }
                } while (row_position < header->pitch);
            }

            temp_dec_position = dec_position;
            row_position = 0;
            // decode green line
            while (row_position < header->pitch) {
                test_byte = *(input + read_offset);
                ++read_offset;
                if ((test_byte & 0xC0) == 0xC0) {
                    value = *(input + read_offset);
                    ++read_offset;
                    if ((test_byte & 0x3F) > 0) {
                        count = test_byte & 0x3F;
                        do {
                            *temp_dec_position |=
                                g_mask &
                                (uint16_t)((uint8_t)value
                                           << (num_g_bits + num_b_bits - 8));

                            temp_dec_position++;
                            ++row_position;
                            if (row_position == header->pitch) break;
                        } while (count-- != 1);
                    }
                } else {
                    *temp_dec_position |=
                        g_mask & (uint16_t)((uint8_t)test_byte
                                            << (num_g_bits + num_b_bits - 8));
                    temp_dec_position++;
                    ++row_position;
                }
            }

            temp_dec_position = dec_position;
            row_position = 0;
            // decode blue line
            while (row_position < header->pitch) {
                test_byte = *(input + read_offset);
                read_offset++;
                if ((test_byte & 0xC0) == 0xC0) {
                    value = *(input + read_offset);
                    ++read_offset;
                    if ((test_byte & 0x3F) > 0) {
                        count = test_byte & 0x3F;
                        do {
                            *temp_dec_position |= value >> (8 - num_b_bits);
                            temp_dec_position++;

                            ++row_position;
                            if (row_position == header->pitch) break;
                        } while (count-- != 1);
                    }
                } else {
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

void *WritePCXHeader(void *pcx_data, int width, int height) {
    int pitch = width;
    if (width & 1) {
        pitch = width + 1;
    }

    PCXHeader *header = (PCXHeader *)pcx_data;
    memset(header, 0, sizeof(PCXHeader));
    header->left = 0;
    header->up = 0;
    header->right = width - 1;
    header->bottom = height - 1;
    header->pitch = pitch;
    header->manufacturer = 10;
    header->version = 5;
    header->encoding = 1;
    header->bpp = 8;
    header->hdpi = 75;
    header->vdpi = 75;
    header->planes = 3;
    header->palette_info = 1;

    return (uint8_t *)pcx_data + sizeof(PCXHeader);
}

void *EncodeOneLine(void *pcx_data, void *line, size_t line_size) {
    uint8_t *input = (uint8_t *)line;
    uint8_t *output = (uint8_t *)pcx_data;

    for (int i = 0; i < line_size; i++) {
        uint8_t value = *input++;
        uint8_t count = 1;
        while ((count < 63) && (i < (line_size - 1)) && (input[1] == value)) {
            input++;
            count++;
            i++;
        }
        if ((count > 1) || ((value & 0xC0) != 0)) {
            *output++ = 0xC0 + count;
        }
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

void Encode(Format f, const void *picture_data, unsigned int width,
            unsigned int height, void *pcx_data, int max_buff_size,
            unsigned int *packed_size) {
    uint8_t *output = (uint8_t *)WritePCXHeader(pcx_data, width, height);

    int pitch = width;
    if (width & 1) {
        pitch = width + 1;
    }

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

    if (packed_size != nullptr) {
        *packed_size = output - (uint8_t*)pcx_data;
    }
}

void PCX::Encode16(const void *picture_data, unsigned int width, unsigned int height,
                   void *pcx_data, int max_buff_size,
                   unsigned int *packed_size) {
    Format f(16, 0xF800, 0x07E0, 0x001F);
    Encode(f, picture_data, width, height, pcx_data, max_buff_size,
           packed_size);
}

void PCX::Encode32(const void *picture_data, unsigned int width, unsigned int height,
                   void *pcx_data, int max_buff_size,
                   unsigned int *packed_size) {
    Format f(32, 0x00FF0000, 0x0000FF00, 0x000000FF);
    Encode(f, picture_data, width, height, pcx_data, max_buff_size,
           packed_size);
}
