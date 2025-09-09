#include "ArchiveReader.h"

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include "Library/Lod/LodReader.h"
#include "Library/Vid/VidReader.h"
#include "Library/Snd/SndReader.h"
#include "Library/Magic/Magic.h"

#include "Utility/Exception.h"

template<class Base, MagicFileFormat magicFormat>
class UniversalReader : public ArchiveReader {
 public:
    explicit UniversalReader(Blob data) {
        if constexpr (std::is_same_v<Base, LodReader>) {
            _base.open(std::move(data), LOD_ALLOW_DUPLICATES);
        } else {
            _base.open(std::move(data));
        }
    }

    virtual MagicFileFormat format() const override {
        return magicFormat;
    }

    virtual std::optional<LodInfo> info() const override {
        if constexpr (std::is_same_v<Base, LodReader>) {
            return _base.info();
        } else {
            return {};
        }
    }

    virtual Blob read(std::string_view filename) const override {
        return _base.read(filename);
    }

    virtual std::vector<std::string> ls() const override {
        return _base.ls();
    }

 private:
    Base _base;
};

std::unique_ptr<ArchiveReader> ArchiveReader::createArchiveReader(std::string_view path) {
    Blob data = Blob::fromFile(path);
    switch (magic(data)) {
    case MAGIC_LOD:
        return std::make_unique<UniversalReader<LodReader, MAGIC_LOD>>(std::move(data));
    case MAGIC_VID:
        return std::make_unique<UniversalReader<VidReader, MAGIC_VID>>(std::move(data));
    case MAGIC_SND:
        return std::make_unique<UniversalReader<SndReader, MAGIC_SND>>(std::move(data));
    default:
        throw Exception("File '{}' is not a LOD, VID or SND archive", path);
    }
}
