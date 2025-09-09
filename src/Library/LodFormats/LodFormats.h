#pragma once

#include "LodFont.h"
#include "Library/Image/Palette.h"
#include "Library/Geometry/Size.h"

#include "LodImage.h"
#include "LodSprite.h"
#include "LodFont.h"

class Blob;

namespace lod {

bool detectCompressedData(const Blob &blob);
bool detectCompressedPseudoImage(const Blob &blob);
bool detectImage(const Blob &blob, bool *isPalette = nullptr);
bool detectSprite(const Blob &blob);
bool detectFont(const Blob &blob);

/**
 * This function processes compressed lod data.
 *
 * @param blob                          `Blob` from a LOD file.
 * @return                              Uncompressed `Palette`.
 * @throw Exception                     If the format is not recognized.
 */
Blob decodeCompressedData(const Blob &blob);

/**
 * This function processes compressed lod pseudo-images.
 *
 * @param blob                          `Blob` from a LOD file.
 * @return                              Uncompressed `Palette`.
 * @throw Exception                     If the format is not recognized.
 */
Blob decodeCompressedPseudoImage(const Blob &blob);

/**
 * This functions processes compressed lod data, and compressed lod pseudo-images.
 *
 * For everything else it just does nothing and returns the blob as is.
 *
 * @param blob                          `Blob` from a LOD file.
 * @return                              Uncompressed `Blob`.
 * @throw Exception                     If the format is not recognized.
 */
Blob decodeMaybeCompressed(const Blob &blob);

/**
 * This function compresses the provided `Blob` into the compressed lod data format.
 *
 * @param blob                          `Blob` to compress.
 * @return                              Compressed `Blob`.
 */
Blob encodeCompressed(const Blob &blob);

/**
 * This function processes lod images and lod palettes. In case of the former, the pixel data is ignored.
 *
 * @param blob                          `Blob` from a LOD file.
 * @return                              Decoded `Palette`.
 * @throw Exception                     If the format is not recognized.
 */
Palette decodePalette(const Blob &blob);

/**
 * This function processes lod images and lod palettes. In case of the latter, the pixel data will be empty.
 *
 * @param blob                          Image `Blob`, as read from a LOD file.
 * @return                              Decoded `LodImage`.
 * @throw Exception                     If the format is not recognized.
 */
LodImage decodeImage(const Blob &blob);

/**
 * This function processes lod images and lod palettes. It reads the image header and returns image size w/o
 * decompressing the pixel data. For lod palettes returned image size will be zero.
 *
 * @param blob                          Image `Blob`, as read from a LOD file.
 * @return                              Image size.
 * @throw Exception                     If the format is not recognized.
 */
Sizei decodeImageSize(const Blob &blob);

/**
 * This function processes lod sprites.
 *
 * @param blob                          Sprite `blob`, as read from a LOD file.
 * @return                              Decoded `LodSprite`.
 * @throw Exception                     If the format is not recognized.
 */
LodSprite decodeSprite(const Blob &blob);

/**
 * This function processes lod fonts.
 *
 * @param blob                          Font `blob`, as read from a LOD file.
 * @return                              Decoded `LodFont`.
 * @throw Exception                     If the format is not recognized.
 */
LodFont decodeFont(const Blob &blob);

} // namespace lod
