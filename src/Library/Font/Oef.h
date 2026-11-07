#pragma once

#include "Library/Font/Font.h"

class Blob;

namespace oef {

/**
 * @param blob                          `Blob` to check.
 * @return                              Whether the blob looks like an OpenEnroth font file.
 */
bool detect(const Blob &blob);

/**
 * Decodes an OpenEnroth font file.
 *
 * @param blob                          Font `Blob`.
 * @return                              Decoded `Font`, keyed by Unicode code point.
 * @throw Exception                     If the format is not recognized, or the data is malformed.
 */
Font decode(const Blob &blob);

/**
 * Encodes a font into the OpenEnroth font format.
 *
 * @param font                          Font to encode. Must be valid.
 * @return                              Encoded `Blob`.
 */
Blob encode(const Font &font);

} // namespace oef
