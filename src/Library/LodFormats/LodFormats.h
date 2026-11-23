#pragma once

#include "Library/Image/Palette.h"
#include "Library/Geometry/Size.h"

#include "LodImage.h"
#include "LodSprite.h"
#include "LodFont.h"

class Blob;

/**
 * Decoders for the formats stored inside LOD archives.
 *
 * Each format gets its own `detectXyz` and `decodeXyz` pair, named after the game that introduced it. Where one kind
 * of entry comes in several formats, `detectAnyXyz` and `decodeAnyXyz` try each of them in turn.
 */
namespace lod {

bool detectMm7CompressedData(const Blob &blob);
bool detectMm6CompressedPseudoImage(const Blob &blob);
bool detectMm6Image(const Blob &blob);
bool detectMm6Palette(const Blob &blob);
bool detectMm6Sprite(const Blob &blob);
bool detectMm6Font(const Blob &blob);
bool detectMmxFont(const Blob &blob);
bool detectAnyFont(const Blob &blob);

/**
 * @param blob                          MM7 compressed data, a "mvii" signature and two sizes followed by zlib data.
 * @return                              Uncompressed data.
 * @throw Exception                     If the format is not recognized.
 */
Blob decodeMm7CompressedData(const Blob &blob);

/**
 * @param blob                          MM6 compressed pseudo image, an image header without an image, followed by
 *                                      zlib data.
 * @return                              Uncompressed data.
 * @throw Exception                     If the format is not recognized.
 */
Blob decodeMm6CompressedPseudoImage(const Blob &blob);

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
 * @return                              MM7 compressed data, as `decodeMm7CompressedData` reads it.
 */
Blob encodeMm7CompressedData(const Blob &blob);

/**
 * @param blob                          MM6 image, as read from a LOD file.
 * @return                              Decoded `LodImage`.
 * @throw Exception                     If the format is not recognized.
 */
LodImage decodeMm6Image(const Blob &blob);

/**
 * Reads the image header and returns the image size without decompressing the pixel data.
 *
 * @param blob                          MM6 image, as read from a LOD file.
 * @return                              Image size.
 * @throw Exception                     If the format is not recognized.
 */
Sizei decodeMm6ImageSize(const Blob &blob);

/**
 * @param blob                          MM6 palette, an image header without an image followed by the palette, as
 *                                      read from a LOD file.
 * @return                              Decoded `Palette`.
 * @throw Exception                     If the format is not recognized.
 */
Palette decodeMm6Palette(const Blob &blob);

/**
 * @param blob                          MM6 sprite, as read from a LOD file.
 * @return                              Decoded `LodSprite`.
 * @throw Exception                     If the format is not recognized.
 */
LodSprite decodeMm6Sprite(const Blob &blob);

/**
 * Reads the sprite header and returns the sprite size without decompressing the pixel data.
 *
 * @param blob                          MM6 sprite, as read from a LOD file.
 * @return                              Sprite size.
 * @throw Exception                     If the format is not recognized.
 */
Sizei decodeMm6SpriteSize(const Blob &blob);

/**
 * @param blob                          MM6 font, as read from a LOD file and decompressed.
 * @return                              Decoded `LodFont`.
 * @throw Exception                     If the format is not recognized.
 */
LodFont decodeMm6Font(const Blob &blob);

/**
 * @param blob                          MMX font, the layout of `calig.fnt`, as read from a LOD file and decompressed.
 * @return                              Decoded `LodFont`.
 * @throw Exception                     If the format is not recognized.
 */
LodFont decodeMmxFont(const Blob &blob);

/**
 * @param blob                          MM6 or MMX font, as read from a LOD file and decompressed.
 * @return                              Decoded `LodFont`.
 * @throw Exception                     If the format is not recognized.
 */
LodFont decodeAnyFont(const Blob &blob);

} // namespace lod
