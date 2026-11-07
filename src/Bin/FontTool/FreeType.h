#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

enum class RasterMode {
    RASTER_MONO, // `FT_LOAD_TARGET_MONO`, which also hints for whole pixels, the way Windows rendered fonts in 1999.
    RASTER_GRAY, // `FT_LOAD_TARGET_NORMAL`, anti-aliased.
};
using enum RasterMode;

/**
 * Glyph as rendered by FreeType.
 */
struct RenderedGlyph {
    int left = 0; // Offset of the bitmap from the pen position, `bitmap_left`.
    int top = 0; // Rows from the baseline up to the top row of the bitmap, `bitmap_top`.
    int width = 0;
    int rows = 0;
    int advance = 0; // Horizontal advance in whole pixels.
    std::vector<uint8_t> coverage; // `rows` rows of `width` pixels, 0-255. Mono renders are 0 or 255.
};

class FreeTypeLibrary {
 public:
    FreeTypeLibrary();
    ~FreeTypeLibrary();

    FreeTypeLibrary(const FreeTypeLibrary &) = delete;
    FreeTypeLibrary &operator=(const FreeTypeLibrary &) = delete;

    [[nodiscard]] FT_Library get() const {
        return _library;
    }

 private:
    FT_Library _library = nullptr;
};

class FreeTypeFace {
 public:
    /**
     * @param library                   Library to open the face with. Faces of one library must stay on one thread.
     * @param path                      Path to a font file.
     * @param faceIndex                 Index of the face in the file, for collections like `.ttc` and `.fon`.
     * @throws Exception                If the face can't be opened.
     */
    FreeTypeFace(const FreeTypeLibrary &library, const std::string &path, int faceIndex);
    ~FreeTypeFace();

    FreeTypeFace(const FreeTypeFace &) = delete;
    FreeTypeFace &operator=(const FreeTypeFace &) = delete;

    [[nodiscard]] int faceCount() const;
    [[nodiscard]] std::string family() const;
    [[nodiscard]] std::string style() const;
    [[nodiscard]] bool isScalable() const;
    [[nodiscard]] int unitsPerEm() const;

    /**
     * @return                          Pixel heights of the bitmap strikes in this face, in strike index order.
     */
    [[nodiscard]] std::vector<int> strikeHeights() const;

    [[nodiscard]] bool hasGlyph(char32_t c) const;

    /**
     * @param c                         Character to look up.
     * @return                          Height of the glyph's outline in font units, or `std::nullopt` if there's no
     *                                  such glyph or its outline is empty.
     */
    [[nodiscard]] std::optional<int> outlineHeight(char32_t c);

    /**
     * @param ppem                      Pixels per em to render at.
     * @return                          Whether the size was set.
     */
    bool setPixelSize(int ppem);

    /**
     * @param index                     Index of a bitmap strike, see `strikeHeights`.
     * @return                          Whether the strike was selected.
     */
    bool selectStrike(int index);

    /**
     * @param c                         Character to render.
     * @param mode                      How to rasterize.
     * @param oblique                   Whether to slant the outline the way FreeType and GDI simulate italics.
     * @return                          Rendered glyph, or `std::nullopt` if the face has no glyph for `c`, or it
     *                                  failed to render.
     */
    [[nodiscard]] std::optional<RenderedGlyph> render(char32_t c, RasterMode mode, bool oblique);

 private:
    FT_Face _face = nullptr;
};
