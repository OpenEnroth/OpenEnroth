#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "Utility/Memory/Blob.h"

class InputStream;
struct LodEntry;

/**
 * A single stop shop to read LOD files.
 * Even though LODs support a multi-directory structure, in reality vanilla games only ever had a single directory each.
 * 
 * Given that we don't plan to expand the LOD format support, when resolving the files this class always looks
 * into the first available directory, which is consistent with the vanilla behaviour.
 */
class LodReader final {
 public:
    LodReader();
    LodReader(std::string_view path);
    ~LodReader();

    /**
     * @param path                      Path to the LOD file to open for reading.
     * @throw Exception                 If the LOD couldn't be opened - e.g., if the file doesn't exist,
     *                                  or if it's not a LOD.
     */
    void open(std::string_view path);

    [[nodiscard]] bool isOpen() const {
        return !!_lod;
    }

    /**
     * @param filename                  Name of the LOD file entry.
     * @return                          Whether the file exists inside the LOD. The check is case-insensitive.
     */
    [[nodiscard]] bool exists(const std::string &filename) const;

    // TODO(captainurist): compression should be handled at the next layer, we should only have read() here, and it
    //                     should do what readRaw does now.
    /**
     * @param filename                  Name of the LOD file entry.
     * @return                          Contents of the file inside the LOD as a `Blob`.
     * @throws Exception                If file doesn't exist inside the LOD.
     */
    [[nodiscard]] Blob read(const std::string &filename) const;

    [[nodiscard]] Blob readRaw(const std::string &filename) const;

    /**
     * @return                          List of all files in a LOD.
     */
    [[nodiscard]] std::vector<std::string> ls() const;

    /**
     * @return                          Description of this LOD file, as specified in the LOD header.
     */
    [[nodiscard]] const std::string &description() const;

    /**
     * @return                          Name of the single folder inside this LOD file.
     */
    [[nodiscard]] const std::string &rootName() const;

 private:
    struct LodRegion {
        size_t offset = 0;
        size_t size = 0;
    };

 private:
    Blob _lod;
    std::string _path;
    std::string _description;
    std::string _rootName;
    std::unordered_map<std::string, LodRegion> _files;
};
