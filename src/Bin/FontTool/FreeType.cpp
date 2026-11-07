#include "FreeType.h"

#include <cmath>
#include <string>
#include <vector>

#include FT_OUTLINE_H

#include "Utility/Exception.h"

static constexpr FT_Fixed OBLIQUE_SHEAR = 0x0366A; // tan(12°) in 16.16, same as `FT_GlyphSlot_Oblique`.

static std::string errorString(FT_Error error) {
    const char *message = FT_Error_String(error);
    return message ? std::string(message) : fmt::format("FreeType error {}", error);
}

FreeTypeLibrary::FreeTypeLibrary() {
    if (FT_Error error = FT_Init_FreeType(&_library))
        throw Exception("Couldn't initialize FreeType: {}", errorString(error));
}

FreeTypeLibrary::~FreeTypeLibrary() {
    FT_Done_FreeType(_library);
}

FreeTypeFace::FreeTypeFace(const FreeTypeLibrary &library, const std::string &path, int faceIndex) {
    if (FT_Error error = FT_New_Face(library.get(), path.c_str(), faceIndex, &_face))
        throw Exception("Couldn't open font '{}': {}", path, errorString(error));
}

FreeTypeFace::~FreeTypeFace() {
    FT_Done_Face(_face);
}

int FreeTypeFace::faceCount() const {
    return _face->num_faces;
}

std::string FreeTypeFace::family() const {
    return _face->family_name ? _face->family_name : "";
}

std::string FreeTypeFace::style() const {
    return _face->style_name ? _face->style_name : "";
}

bool FreeTypeFace::isScalable() const {
    return FT_IS_SCALABLE(_face);
}

int FreeTypeFace::unitsPerEm() const {
    return _face->units_per_EM;
}

std::vector<int> FreeTypeFace::strikeHeights() const {
    std::vector<int> result;
    for (int i = 0; i < _face->num_fixed_sizes; i++)
        result.push_back(_face->available_sizes[i].height);
    return result;
}

bool FreeTypeFace::hasGlyph(char32_t c) const {
    return FT_Get_Char_Index(_face, c) != 0;
}

std::optional<int> FreeTypeFace::outlineHeight(char32_t c) const {
    FT_UInt index = FT_Get_Char_Index(_face, c);
    if (index == 0 || FT_Load_Glyph(_face, index, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING) != 0)
        return std::nullopt;
    if (_face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
        return std::nullopt;

    FT_BBox box;
    FT_Outline_Get_CBox(&_face->glyph->outline, &box);
    return box.yMax - box.yMin;
}

bool FreeTypeFace::setPixelSize(int ppem) {
    return FT_Set_Pixel_Sizes(_face, 0, ppem) == 0;
}

bool FreeTypeFace::selectStrike(int index) {
    return FT_Select_Size(_face, index) == 0;
}

std::optional<RenderedGlyph> FreeTypeFace::render(char32_t c, RasterMode mode, bool oblique) const {
    FT_UInt index = FT_Get_Char_Index(_face, c);
    if (index == 0)
        return std::nullopt;

    FT_Matrix shear = {0x10000, oblique ? OBLIQUE_SHEAR : 0, 0, 0x10000};
    FT_Set_Transform(_face, &shear, nullptr);
    FT_Int32 target = mode == RASTER_MONO ? FT_LOAD_TARGET_MONO : FT_LOAD_TARGET_NORMAL;
    FT_Error error = FT_Load_Glyph(_face, index, FT_LOAD_RENDER | target);
    FT_Set_Transform(_face, nullptr, nullptr);
    if (error != 0)
        return std::nullopt;

    const FT_GlyphSlot slot = _face->glyph;
    const FT_Bitmap &bitmap = slot->bitmap;

    RenderedGlyph result;
    result.left = slot->bitmap_left;
    result.top = slot->bitmap_top;
    result.width = bitmap.width;
    result.rows = bitmap.rows;
    result.advance = static_cast<int>(std::lround(slot->advance.x / 64.0));
    result.coverage.resize(static_cast<size_t>(bitmap.width) * bitmap.rows);

    for (unsigned y = 0; y < bitmap.rows; y++) {
        const uint8_t *row = bitmap.buffer + static_cast<ptrdiff_t>(y) * bitmap.pitch;
        uint8_t *dst = result.coverage.data() + static_cast<size_t>(y) * bitmap.width;
        for (unsigned x = 0; x < bitmap.width; x++) {
            switch (bitmap.pixel_mode) {
            case FT_PIXEL_MODE_MONO:
                dst[x] = ((row[x >> 3] >> (7 - (x & 7))) & 1) ? 255 : 0;
                break;
            case FT_PIXEL_MODE_GRAY:
                dst[x] = bitmap.num_grays == 256 ? row[x] : row[x] * 255 / (bitmap.num_grays - 1);
                break;
            default:
                return std::nullopt; // Color and LCD bitmaps, nothing a 1999 bitmap font came from.
            }
        }
    }

    return result;
}
