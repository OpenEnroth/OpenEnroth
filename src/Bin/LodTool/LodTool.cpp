#include "LodToolOptions.h"

#include <cstdio>
#include <string>

#include "Library/Lod/LodReader.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/String/Format.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"
#include "Utility/UnicodeCrt.h"

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
    if (!options.cat.raw) {
        LodFileFormat format = lod::magic(data, options.cat.entry);
        if (format == LOD_FILE_COMPRESSED || format == LOD_FILE_PSEUDO_IMAGE)
            data = lod::decodeCompressed(data);
    }
    return fwrite(data.data(), data.size(), 1, stdout) != 1;
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
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
