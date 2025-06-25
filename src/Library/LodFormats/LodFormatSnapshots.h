#pragma once

#include <cstdint>
#include <array>

#include "Library/Binary/MemCopySerialization.h"

struct LodFontHeader;
struct LodFontMetrics;
struct LodFontAtlas;

#pragma pack(push, 1)

struct LodCompressionHeader_MM6 {
    std::uint32_t version; // Always 91969.
    std::array<char, 4> signature; // Always "mvii".
    std::uint32_t dataSize;
    std::uint32_t decompressedSize; // Size of the decompressed data, 0 if data is not compressed.
};
static_assert(sizeof(LodCompressionHeader_MM6) == 16);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LodCompressionHeader_MM6)

struct LodImageHeader_MM6 {
    std::array<char, 16> name;

    // Note: for a palette-only image, all the rest are zeros.

    uint32_t size; // Width * height, size of the first mipmap. Can be followed by smaller size bitmaps, down to 16x16.
    uint32_t dataSize; // Size of the pixel data that follows, in bytes.
                       // Can be larger than `size` if mipmaps are present. Can be compressed. Doesn't include palette.
    uint16_t width;
    uint16_t height;
    int16_t widthLn2;  // log2(width) for power of 2 sizes, otherwise 0.
    int16_t heightLn2;  // log2(height) for power of 2 sizes, otherwise 0.
    int16_t widthMinus1; // Seems to be set only for power of 2 sizes. Can be random garbage.
    int16_t heightMinus1; // Seems to be set only for power of 2 sizes. Can be random garbage.
    int16_t paletteId; // Palette id, actually palette is stored with the image, so this field isn't needed.
    int16_t anotherPaletteId; // Seems to always be zero?
    uint32_t decompressedSize; // Size of the decompressed pixel data, 0 if pixel data is not compressed.
    uint32_t flags; // 0x0002 - has mipmaps
                    // 0x0100 - not an image, but a text file (???)
                    // 0x0200 - 0th palette entry is transparent, else colorkey
                    // 0x0400 - don't free buffers (???)
                    // See https://github.com/GrayFace/MMExtension/blob/4d6600f164315f38157591d7f0307a86594c22ef/Src/RSPak/Extra/RSLod.pas#L542
};
static_assert(sizeof(LodImageHeader_MM6) == 48);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LodImageHeader_MM6)

/**
 * @see https://github.com/GrayFace/MMExtension/blob/4d6600f164315f38157591d7f0307a86594c22ef/Src/RSPak/Extra/RSLod.pas#L553
 */
struct LodSpriteHeader_MM6 {
    std::array<char, 12> name;
    uint32_t dataSize; // Size of the pixel data, in bytes.
    uint16_t width; // SW width.
    uint16_t height; // SW height, also the number of `LODSpriteLine`s that follow.
    uint16_t paletteId; // Default palette id, references "palXXX".
    uint16_t unk_0; // Always 0.
    uint16_t emptyBottomLines; // Number of clear lines at the bottom (tail of sprite lines array).
                               // They are still set in sprite lines array, so this info is redundant.
    uint16_t flags; // Used at runtime only?
    uint32_t decompressedSize; // Size of the decompressed pixel data, 0 if pixel data is not compressed.
};
static_assert(sizeof(LodSpriteHeader_MM6) == 32);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LodSpriteHeader_MM6)

struct LodSpriteLine_MM6 {
    int16_t begin;
    int16_t end;
    uint32_t offset;
};
static_assert(sizeof(LodSpriteLine_MM6) == 8);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LodSpriteLine_MM6)

/**
 * @see https://github.com/might-and-magic/fnt-generator/blob/master/fntgen.py#L217
 * @see https://github.com/GrayFace/MMExtension/blob/master/Scripts/Structs/01%20common%20structs.lua#L3034
 */
struct LodFontHeader_MM7 {
    uint8_t firstChar; // Always 30 or 31. Internet says can also be 64.
    uint8_t lastChar; // Always 255.
    uint8_t field_3; // Always 8.
    uint8_t field_4; // Always 0.
    uint8_t field_5; // Always 0.
    uint8_t height;
    uint8_t field_7; // Always 0.
    uint8_t field_8; // Always 0.
    uint32_t paletteCount; // Always 0.
    std::array<uint32_t, 5> palettes; // Always 0. Looks like these were pointers originally.
};
static_assert(sizeof(LodFontHeader_MM7) == 32);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LodFontHeader_MM7)

void reconstruct(const LodFontHeader_MM7 &src, LodFontHeader *dst);

struct LodFontMetrics_MM7 {
    int32_t leftSpacing;
    int32_t width;
    int32_t rightSpacing;
};
static_assert(sizeof(LodFontMetrics_MM7) == 12);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LodFontMetrics_MM7)

void reconstruct(const LodFontMetrics_MM7 &src, LodFontMetrics *dst);

struct LodFontAtlas_MM7 {
    std::array<LodFontMetrics_MM7, 256> metrics;
    std::array<uint32_t, 256> offsets;
};
static_assert(sizeof(LodFontAtlas_MM7) == 4096);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LodFontAtlas_MM7)

void reconstruct(const LodFontAtlas_MM7 &src, LodFontAtlas *dst);

/**
 * This is used for 'calig.fnt' in MM7 'icons.lod'. This font is not loaded by the engine. Not sure if it's from
 * pre-MM6 days or just a result of some experiments by the dev team.
 */
struct LodFontAtlas_MMX {
    std::array<uint8_t, 256> widths;
    std::array<uint32_t, 256> offsets;
};
static_assert(sizeof(LodFontAtlas_MMX) == 1280);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LodFontAtlas_MMX)

void reconstruct(const LodFontAtlas_MMX &src, LodFontAtlas *dst);

#pragma pack(pop)
