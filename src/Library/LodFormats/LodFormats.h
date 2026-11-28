#pragma once

#include "Library/Image/Palette.h"
#include "Library/Geometry/Size.h"

#include "LodImage.h"
#include "LodSprite.h"
#include "LodFont.h"

class Blob;

/**
 * Functions for the formats stored inside LOD archives.
 *
 * Each format gets its own `detectXyzMm6` and `decodeXyzMm6` pair, suffixed with the game that introduced it, and the
 * unsuffixed `decodeXyz` takes every format of its kind. Encoding writes one specific format, so encode functions are
 * always suffixed. MM6 already has both font layouts, so the one `calig.fnt` uses is named MMZ.
 */
namespace lod {

bool detectCompressedDataMm6(const Blob &blob);
bool detectCompressedDataMm7(const Blob &blob);
bool detectCompressedPseudoImageMm6(const Blob &blob);
bool detectCompressedPseudoImageMm8(const Blob &blob);
bool detectImageMm6(const Blob &blob);
bool detectImageMm8(const Blob &blob);
bool detectPaletteMm6(const Blob &blob);
bool detectSpriteMm6(const Blob &blob);
bool detectFontMm6(const Blob &blob);
bool detectFontMmz(const Blob &blob);

/**
 * @param blob                          MM6 compressed data, the packed and unpacked sizes followed by zlib data,
 *                                      or by the data as is when the unpacked size is 0.
 * @return                              Uncompressed data.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
Blob decodeCompressedDataMm6(const Blob &blob);

/**
 * @param blob                          MM7 compressed data, 91969 and "mvii" followed by the packed and unpacked
 *                                      sizes and zlib data.
 * @return                              Uncompressed data.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
Blob decodeCompressedDataMm7(const Blob &blob);

/**
 * @param blob                          LOD compressed data in any format.
 * @return                              Uncompressed data.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
Blob decodeCompressedData(const Blob &blob);

/**
 * @param blob                          MM6 compressed pseudo image, an image header without an image, followed by
 *                                      zlib data.
 * @return                              Uncompressed data.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
Blob decodeCompressedPseudoImageMm6(const Blob &blob);

/**
 * @param blob                          MM8 compressed pseudo image, the MM6 one with a 64-byte name instead of a
 *                                      16-byte one.
 * @return                              Uncompressed data.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
Blob decodeCompressedPseudoImageMm8(const Blob &blob);

/**
 * @param blob                          LOD compressed pseudo image in any format.
 * @return                              Uncompressed data.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
Blob decodeCompressedPseudoImage(const Blob &blob);

/**
 * Decodes every compressed format above, and returns everything else as is.
 *
 * @param blob                          `Blob` from a LOD file.
 * @return                              Uncompressed `Blob`.
 * @throw Exception                     If the data is compressed, but can't be decompressed.
 */
Blob decodeMaybeCompressed(const Blob &blob);

/**
 * @param blob                          `Blob` to compress.
 * @return                              MM7 compressed data, as `decodeCompressedDataMm7` reads it.
 */
Blob encodeCompressedDataMm7(const Blob &blob);

/**
 * @param blob                          MM6 image, as read from a LOD file.
 * @return                              Decoded `LodImage`.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
LodImage decodeImageMm6(const Blob &blob);

/**
 * @param blob                          MM8 image, the MM6 one with a 64-byte name instead of a 16-byte one, as read
 *                                      from a LOD file.
 * @return                              Decoded `LodImage`.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
LodImage decodeImageMm8(const Blob &blob);

/**
 * @param blob                          LOD image in any format, as read from a LOD file.
 * @return                              Decoded `LodImage`.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
LodImage decodeImage(const Blob &blob);

/**
 * Reads the image header and returns the image size without decompressing the pixel data.
 *
 * @param blob                          MM6 image, as read from a LOD file.
 * @return                              Image size.
 * @throw Exception                     If the format is not recognized.
 */
Sizei decodeImageSizeMm6(const Blob &blob);

/**
 * Reads the image header and returns the image size without decompressing the pixel data.
 *
 * @param blob                          MM8 image, as read from a LOD file.
 * @return                              Image size.
 * @throw Exception                     If the format is not recognized.
 */
Sizei decodeImageSizeMm8(const Blob &blob);

/**
 * Reads the image header and returns the image size without decompressing the pixel data.
 *
 * @param blob                          LOD image in any format, as read from a LOD file.
 * @return                              Image size.
 * @throw Exception                     If the format is not recognized.
 */
Sizei decodeImageSize(const Blob &blob);

/**
 * @param blob                          MM6 palette, an image header without an image followed by the palette, as
 *                                      read from a LOD file.
 * @return                              Decoded `Palette`.
 * @throw Exception                     If the format is not recognized.
 */
Palette decodePaletteMm6(const Blob &blob);

/**
 * @param blob                          LOD palette in any format, as read from a LOD file.
 * @return                              Decoded `Palette`.
 * @throw Exception                     If the format is not recognized.
 */
Palette decodePalette(const Blob &blob);

/**
 * @param blob                          MM6 sprite, as read from a LOD file.
 * @return                              Decoded `LodSprite`.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
LodSprite decodeSpriteMm6(const Blob &blob);

/**
 * @param blob                          LOD sprite in any format, as read from a LOD file.
 * @return                              Decoded `LodSprite`.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
LodSprite decodeSprite(const Blob &blob);

/**
 * Reads the sprite header and returns the sprite size without decompressing the pixel data.
 *
 * @param blob                          MM6 sprite, as read from a LOD file.
 * @return                              Sprite size.
 * @throw Exception                     If the format is not recognized.
 */
Sizei decodeSpriteSizeMm6(const Blob &blob);

/**
 * Reads the sprite header and returns the sprite size without decompressing the pixel data.
 *
 * @param blob                          LOD sprite in any format, as read from a LOD file.
 * @return                              Sprite size.
 * @throw Exception                     If the format is not recognized.
 */
Sizei decodeSpriteSize(const Blob &blob);

/**
 * @param blob                          MM6 font, as read from a LOD file and decompressed.
 * @return                              Decoded `LodFont`.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
LodFont decodeFontMm6(const Blob &blob);

/**
 * @param blob                          MMZ font, the layout of `calig.fnt`, as read from a LOD file and decompressed.
 * @return                              Decoded `LodFont`.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
LodFont decodeFontMmz(const Blob &blob);

/**
 * @param blob                          LOD font in any format, as read from a LOD file and decompressed.
 * @return                              Decoded `LodFont`.
 * @throw Exception                     If the format is not recognized or the data is corrupt.
 */
LodFont decodeFont(const Blob &blob);

} // namespace lod
