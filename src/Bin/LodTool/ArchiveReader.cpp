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

template<class Base>
class UniversalReader : public ArchiveReader {
 public:
    explicit UniversalReader(Blob data) {
        if constexpr (std::is_same_v<Base, LodReader>) {
            _base.open(std::move(data), LOD_ALLOW_DUPLICATES);
        } else {
            _base.open(std::move(data));
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

std::unique_ptr<ArchiveReader> ArchiveReader::createArchiveReader(Blob data) {
    switch (magic(data)) {
    case MAGIC_LOD:
        return std::make_unique<UniversalReader<LodReader>>(std::move(data));
    case MAGIC_VID:
        return std::make_unique<UniversalReader<VidReader>>(std::move(data));
    case MAGIC_SND:
        return std::make_unique<UniversalReader<SndReader>>(std::move(data));
    default:
        throw Exception("File '{}' is not a LOD, VID or SND archive", data.displayPath());
    }
}
