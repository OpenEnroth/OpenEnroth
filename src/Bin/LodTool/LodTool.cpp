#include "LodToolOptions.h"

#include "Library/Lod/LodReader.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/Format.h"
#include "Utility/String.h"

int runDump(const LodToolOptions &options) {
    LodReader reader(options.lodPath, LOD_ALLOW_DUPLICATES);

    fmt::println("Lod file: {}", options.lodPath);
    fmt::println("Version: {}", toString(reader.info().version));
    fmt::println("Description: {}", reader.info().description);
    fmt::println("Root folder: {}", reader.info().rootName);

    for (const std::string &name : reader.ls()) {
        Blob data = reader.readRaw(name);
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

            fmt::println("    {:08X}: {: <40}  {}", offset, toHexDump(chunk), toPrintable(chunk, '.'));
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    try {
        LodToolOptions options = LodToolOptions::parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        switch (options.subcommand) {
        case LodToolOptions::SUBCOMMAND_DUMP: return runDump(std::move(options));
        default:
            assert(false);
            return 1;
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
