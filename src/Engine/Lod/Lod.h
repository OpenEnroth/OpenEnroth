#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Version.h"


namespace Lod {
enum class OpenMode {
    Read,
    ReadWrite,
};


class Lod {
public:
    /**
     * Open an existing LOD file.
     *
     * This function loads an existing LOD file for either reading or reading+writing.
     *
     * @param filename          LOD filename to open.
     * @param mode              Opening mode.
     */
    static std::shared_ptr<Lod> open(const std::string &filename, OpenMode mode);

    /**
     * Creating a new LOD file.
     *
     * This function creates (potentially overwriting) a new LOD file.
     *
     * @param filename          LOD filename to create/overwrite.
     * @param description       Brief LOD description (mandatory in the file header).
     * @param version           LOD file version to create.
     */
    static std::shared_ptr<Lod> create(const std::string &filename, const std::string &description, Version version);

    /**
     * Closing the LOD file.
     *
     * This function closes the file and frees all associated resources, commiting any pending changes when in Write mode.
     */
    void close();

    /**
     * Changes the current directory.
     *
     * This function will change the current active directory in LOD.
     *
     * @param path              Path (can be relative to the current directory).
     */
    bool changeDirectory(const std::string& path);

    /**
     * List files in a directory.
     *
     * This function list all files in a path relative to the current directory.
     *
     * @param path              Path relative to the current directory.
     */
    std::vector<std::string> listFiles(const std::string &path);

    /**
     * Check if a file exists in LOD.
     *
     * This function checks if a file exists in LOD.
     *
     * @param filename          File name relative to the current directory (can be a path).
     */
    bool fileExists(const std::string& filename);

    /**
     * Read a file stored in LOD.
     *
     * This function reads a file at the path provided.
     *
     * @param path              File name relative to the current directory (can be a path).
     * @param outFileSize       Optional pointer to file size in bytes.
     * @param failIfMissing     Generate an error if the file is missing, otherwise issue a warning and return null.
     */
    std::shared_ptr<void> read(const std::string &path, size_t *outFileSize = nullptr, bool failIfMissing = false);

    /**
     * Write a file to LOD.
     *
     * This function writes a file to LOD at the path provided.
     *
     * @param path              File name relative to the current directory (can be a path).
     * @param fileData          File bytes.
     * @param fileDataSize      File size in bytes.
     */
    bool write(const std::string &path, const void *fileData, size_t fileDataSize);

    /**
     * Commit all the changes to LOD.
     *
     * This function commits all the changes buffered so far (e.g writes) to the LOD file.
     */
    bool commit();

private:
}
};  // namespace Lod
