#include "LodToolOptions.h"

#include <cstdio>
#include <string>
#include <utility>
#include <algorithm>
#include <vector>
#include <memory>

#include "Library/Image/ImageFunctions.h"
#include "Library/Image/Pcx.h"
#include "Library/Image/Png.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/FileSystem/Directory/DirectoryFileSystem.h"
#include "Library/Serialization/Serialization.h"
#include "Library/Magic/Magic.h"

#include "Utility/String/Format.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"
#include "Utility/UnicodeCrt.h"

#include "ArchiveReader.h"

static const char *formatDescription(MagicFileFormat format) {
    switch (format) {
    default:
        assert(false);
        [[fallthrough]];
    case MAGIC_UNRECOGNIZED:
        return "Raw data";
    case MAGIC_LOD:
        return "LOD archive";
    case MAGIC_LOD_COMPRESSED_DATA:
        return "LOD compressed file";
    case MAGIC_LOD_COMPRESSED_PSEUDO_IMAGE:
        return "LOD compressed pseudo image";
    case MAGIC_LOD_IMAGE:
        return "LOD image";
    case MAGIC_LOD_PALETTE:
        return "LOD palette";
    case MAGIC_LOD_SPRITE:
        return "LOD sprite";
    case MAGIC_LOD_FONT:
        return "LOD font";
    case MAGIC_VID:
        return "VID archive";
    case MAGIC_SND:
        return "SND archive";
    case MAGIC_PNG:
        return "PNG image";
    case MAGIC_PCX:
        return "PCX image";
    case MAGIC_WAV:
        return "WAV file";
    }
}

static RgbaImage renderFont(const LodFont &font) {
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

    MagicFileFormat format = magic(entry);

    if (format == MAGIC_PCX)
        return result(png::encode(pcx::decode(entry)), name + ".png");

    if (format == MAGIC_WAV)
        return result(std::move(entry), name.ends_with(".wav") ? name : name + ".wav");

    if (format == MAGIC_PNG)
        return result(std::move(entry), name.ends_with(".png") ? name : name + ".png");

    if (format == MAGIC_LOD_IMAGE) {
        LodImage lodImage = lod::decodeImage(entry);
        return result(png::encode(makeRgbaImage(lodImage.image, lodImage.palette)), name + ".png");
    }

    if (format == MAGIC_LOD_PALETTE) {
        LodImage lodImage = lod::decodeImage(entry);
        RgbaImage palImage = RgbaImage::uninitialized(256, 1);
        for (size_t i = 0; i < 256; i++)
            palImage[0][i] = lodImage.palette.colors[i];
        return result(png::encode(palImage), name + ".png");
    }

    if (format == MAGIC_LOD_SPRITE) {
        LodSprite lodSprite = lod::decodeSprite(entry);
        return result(png::encode(lodSprite.image), name + ".png");
    }

    if (format == MAGIC_LOD_FONT) {
        LodFont lodFont = lod::decodeFont(entry);
        return result(std::move(entry), name)
                     (png::encode(renderFont(lodFont)), name + ".png");
    }

    // We have pcx images inside compressed entries, so to support this we just re-run the function.
    if (format == MAGIC_LOD_COMPRESSED_DATA || format == MAGIC_LOD_COMPRESSED_PSEUDO_IMAGE)
        return decodeLodEntry(lod::decodeMaybeCompressed(entry), std::move(name), raw);

    return result(std::move(entry), std::move(name));
}

int runLs(const LodToolOptions &options) {
    std::unique_ptr<ArchiveReader> reader = ArchiveReader::createArchiveReader(options.path);
    fmt::println("{}", fmt::join(reader->ls(), "\n"));
    return 0;
}

int runDump(const LodToolOptions &options) {
    std::unique_ptr<ArchiveReader> reader = ArchiveReader::createArchiveReader(options.path);

    fmt::println("Archive file: {}", options.path);
    fmt::println("Archive format: {}", formatDescription(reader->format()));
    if (auto info = reader->info()) {
        fmt::println("LOD version: {}", toString(info->version));
        fmt::println("LOD description: {}", info->description);
        fmt::println("LOD root folder: {}", info->rootName);
    }

    for (const std::string &name : reader->ls()) {
        Blob data = reader->read(name);
        MagicFileFormat format = magic(data);
        bool isCompressed = format == MAGIC_LOD_COMPRESSED_DATA || format == MAGIC_LOD_COMPRESSED_PSEUDO_IMAGE;
        if (isCompressed)
            data = lod::decodeMaybeCompressed(data);

        fmt::println("");
        fmt::println("Entry: {}", name);
        fmt::println("Format: {}", formatDescription(magic(data)));
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
    std::unique_ptr<ArchiveReader> reader = ArchiveReader::createArchiveReader(options.path);
    auto [data, _] = std::move(decodeLodEntry(reader->read(options.cat.entry), options.cat.entry, options.raw)[0]);
    return fwrite(data.data(), data.size(), 1, stdout) != 1 ? 1 : 0;
}

int runExtract(const LodToolOptions &options) {
    std::unique_ptr<ArchiveReader> reader = ArchiveReader::createArchiveReader(options.path);
    DirectoryFileSystem output(options.extract.output);

    for (const std::string &entryName : reader->ls())
        for (const auto &[data, name] : decodeLodEntry(reader->read(entryName), entryName, options.raw))
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
