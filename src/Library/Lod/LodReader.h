#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "Utility/Memory/Blob.h"

#include "LodEnums.h"
#include "LodInfo.h"

class InputStream;

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
    LodReader(std::string_view path, LodOpenFlags openFlags = 0);
    LodReader(Blob blob, LodOpenFlags openFlags = 0);
    ~LodReader();

    /**
     * @param path                      Path to the LOD file to open for reading.
     * @param openFlags                 Open flags.
     * @throw Exception                 If the LOD couldn't be opened - e.g., if the file doesn't exist,
     *                                  or if it's not a LOD.
     */
    void open(std::string_view path, LodOpenFlags openFlags = 0);

    /**
     * @param blob                      LOD data.
     * @param openFlags                 Open flags.
     * @throw Exception                 If there are errors in the provided LOD file.
     */
    void open(Blob blob, LodOpenFlags openFlags = 0);

    /**
     * Closes this LOD reader & frees all associated resources.
     */
    void close();

    [[nodiscard]] bool isOpen() const {
        return !!_lod;
    }

    /**
     * @param filename                  Name of the LOD file entry.
     * @return                          Whether the file exists inside the LOD. The check is case-insensitive.
     */
    [[nodiscard]] bool exists(std::string_view filename) const;

    /**
     * @param filename                  Name of the LOD file entry.
     * @return                          Contents of the file inside the LOD as a `Blob`.
     * @throws Exception                If file doesn't exist inside the LOD.
     */
    [[nodiscard]] Blob read(std::string_view filename) const;

    /**
     * @return                          List of all files in a LOD.
     */
    [[nodiscard]] std::vector<std::string> ls() const;

    /**
     * @return                          LOD info, containing LOD version, description of this LOD file as specified in
     *                                  the LOD header, and a name of the single folder inside this LOD file.
     */
    [[nodiscard]] const LodInfo &info() const;

 private:
    struct LodRegion {
        size_t offset = 0;
        size_t size = 0;
    };

 private:
    Blob _lod;
    LodInfo _info;
    std::unordered_map<std::string, LodRegion> _files;
};
