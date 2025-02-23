#pragma once

#include <string>

#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"
#include "Library/Geometry/Size.h"

#include "LodFormatEnums.h"

class Blob;

struct LodSprite {
    GrayscaleImage image;
    int paletteId = 0;
};

struct LodImage {
    GrayscaleImage image;
    Palette palette;
    bool zeroIsTransparent = false; // Means that zero palette entry should be treated as transparent.
                                    // This, however, is sometimes overridden in user code.
};

namespace lod {

LodFileFormat magic(const Blob &blob, std::string_view fileName);

/**
 * This functions processes `LOD_FILE_COMPRESSED`, `LOD_FILE_PSEUDO_IMAGE` and `LOD_FILE_RAW` formats, returning
 * uncompressed data.
 *
 * In case of `LOD_FILE_RAW`, it just does nothing and returns the blob as is.
 *
 * @param blob                          `Blob` from a LOD file.
 * @return                              Uncompressed `Blob`.
 * @throw Exception                     If the provided `Blob` is of unsupported type.
 */
Blob decodeCompressed(const Blob &blob);

/**
 * This function compresses the provided `Blob` into the `LOD_FILE_COMPRESSED` format.
 *
 * @param blob                          `Blob` to compress.
 * @return                              Compressed `Blob` in `LOD_FILE_COMPRESSED` format.
 */
Blob encodeCompressed(const Blob &blob);

/**
 * This function processes `LOD_FILE_PALETTE` and `LOD_FILE_IMAGE` formats. In case of the latter, the pixel data
 * is ignored.
 *
 * @param blob                          `Blob` from a LOD file.
 * @return                              Decoded `Palette`.
 * @throw Exception                     If the provided `Blob` is of unsupported type.
 */
Palette decodePalette(const Blob &blob);

/**
 * This function processes `LOD_FILE_IMAGE` and `LOD_FILE_PALETTE` formats. In case of the latter, the pixel data
 * will be empty.
 *
 * @param blob                          Image `Blob`, as read from a LOD file.
 * @return                              Decoded `LodImage`.
 */
LodImage decodeImage(const Blob &blob);

/**
 * This function processes `LOD_FILE_IMAGE` and `LOD_FILE_PALETTE` formats. It reads the image header and returns image
 * size w/o decompressing the pixel data. For `LOD_FILE_PALETTE` returned image size will be zero.
 *
 * @param blob                          Image `Blob`, as read from a LOD file.
 * @return                              Image size.
 */
Sizei decodeImageSize(const Blob &blob);

/**
 * This function processes `LOD_FILE_SPRITE` format.
 *
 * @param blob                          Sprite `blob`, as read from a LOD file.
 * @return                              Decoded `LodSprite`.
 */
LodSprite decodeSprite(const Blob &blob);

} // namespace lod
