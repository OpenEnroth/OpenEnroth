#include "LodToolOptions.h"

#include <cstdio>
#include <string>

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

bool isPcx(const Blob &blob) {
    if (blob.size() < 4)
        return false;

    // Check for PCX signature:
    // - s[0] should be 0x0A (PCX identifier).
    // - s[1] should be one of the valid version numbers (0x00, 0x02, 0x03, 0x04, 0x05)
    // - s[2] should be 0x01 (indicating RLE encoding)
    // - s[3] should be one of the common bits per pixel values (0x01, 0x02, 0x04, 0x08)
    std::string_view s = blob.string_view();
    if (s[0] != '\x0A')
        return false;
    if (s[1] != '\x00' && s[1] != '\x02' && s[1] != '\x03' && s[1] != '\x04' && s[1] != '\x05')
        return false;
    if (s[2] != '\x01')
        return false;
    if (s[3] != '\x01' && s[3] != '\x02' && s[3] != '\x04' && s[3] != '\x08')
        return false;
    return true;
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
    Blob data = reader.read(options.cat.entry);
    // TODO(captainurist): cat should work the same way as extract & spit out pngs?
    if (!options.cat.raw) {
        LodFileFormat format = lod::magic(data, options.cat.entry);
        if (format == LOD_FILE_COMPRESSED || format == LOD_FILE_PSEUDO_IMAGE)
            data = lod::decodeCompressed(data);
    }
    return fwrite(data.data(), data.size(), 1, stdout) != 1 ? 1 : 0;
}

int runExtract(const LodToolOptions &options) {
    LodReader reader(options.lodPath, LOD_ALLOW_DUPLICATES);
    DirectoryFileSystem output(options.extract.output);

    for (const std::string &entryName : reader.ls()) {
        Blob data = reader.read(entryName);
        Blob result;
        std::string resultName;

        if (options.extract.raw) {
            result = Blob::share(data);
            resultName = entryName;
        } else if (isPcx(data)) {
            result = png::encode(pcx::decode(data));
            resultName = (entryName.ends_with(".pcx") ? entryName.substr(0, entryName.size() - 4) : entryName) + ".png";
        } else {
            LodFileFormat format = lod::magic(data, entryName);

            if (format == LOD_FILE_IMAGE) {
                LodImage lodImage = lod::decodeImage(data);
                result = png::encode(makeRgbaImage(lodImage.image, lodImage.palette));
                resultName = entryName + ".png";
            } else if (format == LOD_FILE_PALETTE) {
                LodImage lodImage = lod::decodeImage(data);
                RgbaImage palImage = RgbaImage::uninitialized(256, 1);
                for (size_t i = 0; i < 256; i++)
                    palImage[0][i] = lodImage.palette.colors[i];
                result = png::encode(palImage);
                resultName = entryName + ".png";
            } else if (format == LOD_FILE_SPRITE) {
                LodSprite sprite = lod::decodeSprite(data);
                result = png::encode(sprite.image);
                resultName = entryName + ".png";
            } else if (format == LOD_FILE_COMPRESSED || format == LOD_FILE_PSEUDO_IMAGE) {
                result = lod::decodeCompressed(data);
                resultName = entryName;
            } else {
                result = Blob::share(data);
                resultName = entryName;
            }
        }

        output.write(resultName, result);
    }

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
