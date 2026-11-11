#include "Render.h"

#include <algorithm>
#include <string>
#include <utility>

#include "Library/Font/FontBuilder.h"

#include "Utility/String/Format.h"

std::string RenderSettings::toString() const {
    std::string gray = mode == RASTER_MONO ? "" : fmt::format(" --gray --threshold {}", threshold);
    std::string spacing;
    if (inkOffset != 0)
        spacing += fmt::format(" --ink-offset {}", inkOffset);
    if (extraAdvance != 0)
        spacing += fmt::format(" --extra-advance {}", extraAdvance);
    return fmt::format("--ppem {}{}{}{} --baseline {} --height {}{}", ppem, gray, bold ? " --bold" : "",
                       oblique ? " --oblique" : "", baseline, height, spacing);
}

InkMask makeInkMask(const RenderedGlyph &glyph, int threshold, bool bold) {
    InkMask result;
    result.left = glyph.left;
    result.top = glyph.top;
    result.rows = glyph.rows;
    result.width = glyph.width + (bold && glyph.width > 0 ? 1 : 0);
    result.advance = glyph.advance + (bold ? 1 : 0);
    result.pixels.assign(static_cast<size_t>(result.width) * result.rows, 0);

    for (int y = 0; y < glyph.rows; y++) {
        for (int x = 0; x < glyph.width; x++) {
            if (glyph.coverage[y * glyph.width + x] < threshold)
                continue;
            result.pixels[y * result.width + x] = 1;
            if (bold)
                result.pixels[y * result.width + x + 1] = 1;
        }
    }

    return result;
}

Font renderFont(FreeTypeFace &face, const std::u32string &characters, const RenderSettings &settings) {
    FontBuilder builder(settings.height);

    for (char32_t c : characters) {
        std::optional<RenderedGlyph> glyph = face.render(c, settings.mode, settings.oblique);
        if (!glyph)
            continue;
        InkMask mask = makeInkMask(*glyph, settings.mode == RASTER_MONO ? 1 : settings.threshold, settings.bold);

        if (mask.width == 0) {
            if (mask.advance + settings.extraAdvance <= 0)
                continue;
            int advance = mask.advance + settings.extraAdvance;
            builder.add(c, GlyphMetrics(0, advance, 0), GrayscaleImage::solid(0, advance, settings.height));
            continue;
        }

        GrayscaleImage image = GrayscaleImage::solid(0, mask.width, settings.height);
        for (int y = 0; y < mask.rows; y++) {
            int cellY = settings.baseline - mask.top + y;
            if (cellY < 0 || cellY >= settings.height)
                continue;
            for (int x = 0; x < mask.width; x++)
                image[cellY][x] = mask.pixels[y * mask.width + x];
        }

        int left = mask.left + settings.inkOffset;
        int right = mask.advance + settings.extraAdvance - left - mask.width;
        builder.add(c, GlyphMetrics(left, mask.width, right), image);
    }

    return std::move(builder).build();
}
