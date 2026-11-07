#include "LodToolOptions.h"

#include <cstdio>
#include <string>
#include <string_view>
#include <utility>
#include <algorithm>
#include <vector>
#include <memory>

#include "Library/Image/ImageFunctions.h"
#include "Library/Image/Pcx.h"
#include "Library/Image/Png.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/Magic/Magic.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/String/Format.h"
#include "Utility/String/Ascii.h"
#include "Utility/System/Fs.h"
#include "Utility/Exception.h"
#include "Utility/UnicodeCrt.h"

#include "ArchiveReader.h"

static RgbaImage renderFont(const LodFont &font) {
    int charHeight = font.height();
    int charWidth = 0;
    for (int c = 0; c < 255; c++)
        charWidth = std::max(charWidth, font.metrics(c).width);

    RgbaImage image = RgbaImage::solid(Color(0, 0, 0, 255), charWidth * 16, charHeight * 16);

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

/**
 * @param format                        Format to describe.
 * @return                              Description of `format` that reads in the middle of a sentence.
 */
static std::string_view describe(MagicFileFormat format) {
    return format == MAGIC_UNRECOGNIZED ? "raw data" : formatDescription(format);
}

static bool isArchive(MagicFileFormat format) {
    return format == MAGIC_LOD || format == MAGIC_VID || format == MAGIC_SND;
}

static bool isCompressed(MagicFileFormat format) {
    return format == MAGIC_LOD_COMPRESSED_DATA || format == MAGIC_LOD_COMPRESSED_PSEUDO_IMAGE;
}

/**
 * @param path                          Path to a file, or to an entry inside an archive, as in `"games.lod/d.odm"`.
 * @return                              Contents of the file or the archive entry, as stored.
 * @throws Exception                    If there is no such file or entry.
 */
static Blob readFile(PathView path) {
    PathView tail;
    PathView prefix = fs::walk(path, &tail);

    FileStat stat = fs::stat(prefix);
    if (tail.isEmpty()) {
        if (stat.type != FILE_REGULAR)
            throw Exception("'{}' is not a file", path.displayString());
        return Blob::fromFile(prefix);
    }

    if (stat.type != FILE_REGULAR)
        throw Exception("'{}' doesn't exist", path.displayString());
    return ArchiveReader::createArchiveReader(Blob::fromFile(prefix))->read(tail.str());
}

static void writeFile(PathView path, const Blob &data) {
    FileOutputStream output(path);
    output.write(data);
    output.close();
}

/**
 * @param data                          Decompressed image, sprite, palette or font.
 * @param palettesLodPath               Path to bitmaps.lod to take sprite palettes from. Without it a sprite comes
 *                                      out grayscale, with palette indices as gray levels.
 * @return                              PNG image.
 * @throws Exception                    If `data` is in a format that has no PNG rendering.
 */
static Blob convertToPng(const Blob &data, PathView palettesLodPath) {
    switch (magic(data)) {
    case MAGIC_PNG:
        return Blob::share(data);
    case MAGIC_PCX:
        return png::encode(pcx::decode(data));
    case MAGIC_LOD_IMAGE: {
        LodImage lodImage = lod::decodeImage(data);
        return png::encode(makeRgbaImage(lodImage.image, lodImage.palette));
    }
    case MAGIC_LOD_PALETTE: {
        LodImage lodImage = lod::decodeImage(data);
        RgbaImage palImage = RgbaImage::uninitialized(256, 1);
        for (size_t i = 0; i < 256; i++)
            palImage[0][i] = lodImage.palette.colors[i];
        return png::encode(palImage);
    }
    case MAGIC_LOD_SPRITE: {
        LodSprite lodSprite = lod::decodeSprite(data);
        if (palettesLodPath.isEmpty())
            return png::encode(lodSprite.image);

        std::unique_ptr<ArchiveReader> paletteReader = ArchiveReader::createArchiveReader(Blob::fromFile(palettesLodPath));
        Palette palette = lod::decodePalette(paletteReader->read(fmt::format("pal{:03}", lodSprite.paletteId)));
        return png::encode(makeRgbaImage(lodSprite.image, palette));
    }
    case MAGIC_LOD_FONT:
        return png::encode(renderFont(lod::decodeFont(data)));
    default:
        throw Exception("Can't convert {} '{}' to png", describe(magic(data)), data.displayPath());
    }
}

static int runLs(const LodToolOptions &options) {
    PathView tail;
    PathView prefix = fs::walk(options.path, &tail);

    if (!tail.isEmpty()) {
        if (fs::stat(prefix).type != FILE_REGULAR || !ArchiveReader::createArchiveReader(Blob::fromFile(prefix))->exists(tail.str()))
            throw Exception("'{}' doesn't exist", options.path);
        fmt::println("{}", tail);
        return 0;
    }

    if (fs::stat(prefix).type == FILE_DIRECTORY) {
        std::vector<DirectoryEntry> entries = fs::ls(prefix);
        std::ranges::sort(entries, std::ranges::less(), &DirectoryEntry::name);
        for (const DirectoryEntry &entry : entries)
            fmt::println("{}", entry.name);
        return 0;
    }

    Blob data = Blob::fromFile(prefix);
    if (!isArchive(magic(data))) {
        fmt::println("{}", options.path); // Same as ls on a file.
        return 0;
    }

    for (const std::string &name : ArchiveReader::createArchiveReader(std::move(data))->ls())
        fmt::println("{}", name);
    return 0;
}

static int runCat(const LodToolOptions &options) {
    Blob data = readFile(options.path);
    if (!options.raw)
        data = lod::decodeMaybeCompressed(data);
    if (!data.empty() && fwrite(data.data(), data.size(), 1, stdout) != 1)
        return 1;
    return 0;
}

static int runExtract(const LodToolOptions &options) {
    Path output = options.output.isEmpty() ? Path(".") : options.output;

    Blob data = readFile(options.path);
    if (isArchive(magic(data))) {
        fs::mkdirs(output);
        std::unique_ptr<ArchiveReader> reader = ArchiveReader::createArchiveReader(std::move(data));
        for (const std::string &name : reader->ls()) {
            if (PathView(name).name() != name || name == "." || name == "..")
                throw Exception("Entry '{}' in '{}' is not a valid file name", name, options.path);
            Blob entry = reader->read(name);
            writeFile(output / name, options.raw ? std::move(entry) : lod::decodeMaybeCompressed(entry));
        }
        return 0;
    }

    bool isDirectory = output.name().empty() || fs::stat(output).type == FILE_DIRECTORY; // "out/" names a directory, like in cp.
    if (isDirectory)
        fs::mkdirs(output);
    writeFile(isDirectory ? output / PathView(options.path).name() : output, options.raw ? std::move(data) : lod::decodeMaybeCompressed(data));
    return 0;
}

static int runConvert(const LodToolOptions &options) {
    if (ascii::toLower(options.output.extension()) != ".png")
        throw Exception("Can't convert to '{}', only png output is supported", options.output);

    writeFile(options.output, convertToPng(lod::decodeMaybeCompressed(readFile(options.path)), options.palettesLodPath));
    return 0;
}

static int runMagic(const LodToolOptions &options) {
    Blob data = readFile(options.path);
    MagicFileFormat format = magic(data);
    if (!isCompressed(format)) {
        fmt::println("{}", formatDescription(format));
        return 0;
    }

    fmt::println("{} containing {}", formatDescription(format), describe(magic(lod::decodeMaybeCompressed(data))));
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
        case LodToolOptions::SUBCOMMAND_CAT: return runCat(options);
        case LodToolOptions::SUBCOMMAND_EXTRACT: return runExtract(options);
        case LodToolOptions::SUBCOMMAND_CONVERT: return runConvert(options);
        case LodToolOptions::SUBCOMMAND_MAGIC: return runMagic(options);
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
