#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Version.h"


namespace Lod {
enum class OpenMode {
    New,
    Read,
    ReadWrite,
};


class Lod {
public:
    // Lod management
    static std::shared_ptr<Lod> open(OpenMode mode);
    static std::shared_ptr<Lod> create(const std::string &filename, const std::string &description, Version version);

    void close();

    // Access/listing files/directories inside Lod
    bool fileExists(const std::string &filename);
    std::vector<std::string> listFiles(const std::string &path);
    bool changeDirectory(const std::string &path);

    // Read / Write files
    std::shared_ptr<void> read(const std::string &path, size_t *outFileSize = nullptr, bool failIfMissing = false);
    bool write(const std::string &path, const void *fileData, size_t fileDataSize);

private:
}
};  // namespace Lod
