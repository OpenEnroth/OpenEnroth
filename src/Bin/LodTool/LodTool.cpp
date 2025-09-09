#include "LodToolOptions.h"

#include <cstdio>
#include <string>
#include <utility>
#include <algorithm>
#include <vector>

#include "Library/Image/ImageFunctions.h"
#include "Library/Image/Pcx.h"
#include "Library/Image/Png.h"
#include "Library/Lod/LodReader.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/FileSystem/Directory/DirectoryFileSystem.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/String/Format.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"
#include "Utility/UnicodeCrt.h"

RgbaImage renderFont(const LodFont &font) {
    int charHeight = font.height();
    int charWidth = 0;
    for (int c = 0; c < 255; c++)
        charWidth = std::max(charWidth, font.metrics(c).width);

    RgbaImage image = RgbaImage::solid(charWidth * 16, charHeight * 16, Color(0, 0, 0, 255));

    for (int c = 0; c < 255; c++) {
        int x0 = (c % 16) * charWidth;
        int y0 = (c / 16) * charHeight;

        GrayscaleImageView glyph = font.image(c);
        for (int y = 0; y < glyph.height(); y++) {
            for (int x = 0; x < glyph.width(); x++) {
                int gray = glyph[y][x];
                if (gray == 1)
                    gray = 128; // Make the shadow gray.
                image[y0 + y][x0 + x] = Color(gray, gray, gray, 255);
            }
        }
    }

    return image;
}

class DecodedEntries : public std::vector<std::pair<Blob, std::string>> {
 public:
    DecodedEntries &&operator()(Blob entry, std::string name) {
        // Need this helper b/c std::vector cannot be constructed from an initializer list with move-only elements.
        emplace_back(std::move(entry), std::move(name));
        return std::move(*this);
    }
};

DecodedEntries decodeLodEntry(Blob entry, std::string name, bool raw) {
    DecodedEntries result;

    if (raw)
        return result(std::move(entry), std::move(name));

    if (pcx::detect(entry))
        return result(png::encode(pcx::decode(entry)), name + ".png");

    LodFileFormat format = lod::magic(entry, name);

    if (format == LOD_FILE_IMAGE) {
        LodImage lodImage = lod::decodeImage(entry);
        return result(png::encode(makeRgbaImage(lodImage.image, lodImage.palette)), name + ".png");
    }

    if (format == LOD_FILE_PALETTE) {
        LodImage lodImage = lod::decodeImage(entry);
        RgbaImage palImage = RgbaImage::uninitialized(256, 1);
        for (size_t i = 0; i < 256; i++)
            palImage[0][i] = lodImage.palette.colors[i];
        return result(png::encode(palImage), name + ".png");
    }

    if (format == LOD_FILE_SPRITE) {
        LodSprite lodSprite = lod::decodeSprite(entry);
        return result(png::encode(lodSprite.image), name + ".png");
    }

    if (format == LOD_FILE_FONT) {
        LodFont lodFont = lod::decodeFont(entry);
        return result(std::move(entry), name)
                     (png::encode(renderFont(lodFont)), name + ".png");
    }

    // We have pcx images inside compressed entries, so to support this we just re-run the function.
    if (format == LOD_FILE_COMPRESSED || format == LOD_FILE_PSEUDO_IMAGE)
        return decodeLodEntry(lod::decodeCompressed(entry), std::move(name), raw);

    return result(std::move(entry), std::move(name));
}

int runLs(const LodToolOptions &options) {
    LodReader reader(options.lodPath, LOD_ALLOW_DUPLICATES);
    fmt::println("{}", fmt::join(reader.ls(), "\n"));
    return 0;
}

int runDump(const LodToolOptions &options) {
    LodReader reader(options.lodPath, LOD_ALLOW_DUPLICATES);

    fmt::println("Lod file: {}", options.lodPath);
    fmt::println("Version: {}", toString(reader.info().version));
    fmt::println("Description: {}", reader.info().description);
    fmt::println("Root folder: {}", reader.info().rootName);

    for (const std::string &name : reader.ls()) {
        Blob data = reader.read(name);
        LodFileFormat format = lod::magic(data, name);
        bool isCompressed = format == LOD_FILE_COMPRESSED || format == LOD_FILE_PSEUDO_IMAGE;
        if (isCompressed)
            data = lod::decodeCompressed(data);

        fmt::println("");
        fmt::println("Entry: {}", name);
        fmt::println("Format: {}", toString(lod::magic(data, name)));
        fmt::println("Size{}: {}", isCompressed ? " (uncompressed)" : "", data.size());
        fmt::println("Data{}:", isCompressed ? " (uncompressed)" : "");

        std::string line;
        for (size_t offset = 0; offset < data.size(); offset += 16) {
            std::string_view chunk = data.string_view().substr(offset, 16);

            fmt::println("    {:08X}: {: <40}  {}", offset, toHexDump(chunk), ascii::toPrintable(chunk, '.'));
        }
    }

    return 0;
}

int runCat(const LodToolOptions &options) {
    LodReader reader(options.lodPath, LOD_ALLOW_DUPLICATES);
    auto [data, _] = std::move(decodeLodEntry(reader.read(options.cat.entry), options.cat.entry, options.raw)[0]);
    return fwrite(data.data(), data.size(), 1, stdout) != 1 ? 1 : 0;
}

int runExtract(const LodToolOptions &options) {
    LodReader reader(options.lodPath, LOD_ALLOW_DUPLICATES);
    DirectoryFileSystem output(options.extract.output);

    for (const std::string &entryName : reader.ls())
        for (const auto &[data, name] : decodeLodEntry(reader.read(entryName), entryName, options.raw))
            output.write(name, data);

    return 0;
}

int main(int argc, char **argv) {
    try {
        UnicodeCrt _(argc, argv);
        LodToolOptions options = LodToolOptions::parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        switch (options.subcommand) {
        default: assert(false); [[fallthrough]];
        case LodToolOptions::SUBCOMMAND_LS: return runLs(options);
        case LodToolOptions::SUBCOMMAND_DUMP: return runDump(options);
        case LodToolOptions::SUBCOMMAND_CAT: return runCat(options);
        case LodToolOptions::SUBCOMMAND_EXTRACT: return runExtract(options);
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
