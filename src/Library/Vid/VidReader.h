#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Utility/Memory/Blob.h"

/**
 * Reader for Might&Magic VID files.
 */
class VidReader {
 public:
    VidReader();
    explicit VidReader(std::string_view path);
    explicit VidReader(Blob blob);
    ~VidReader();

    /**
     * @param path                      Path to the VID file to open for reading.
     * @throw Exception                 If the VID couldn't be opened - e.g., if the file doesn't exist,
     *                                  or if it's not in VID format.
     */
    void open(std::string_view path);

    /**
     * @param blob                      VID data.
     * @throw Exception                 If there are errors in the provided VID file.
     */
    void open(Blob blob);

    /**
     * Closes this VID reader & frees all associated resources.
     */
    void close();

    [[nodiscard]] bool isOpen() const {
        return !!_vid;
    }

    /**
     * @param filename                  Name of the VID file entry.
     * @return                          Whether the file exists inside the VID. The check is case-insensitive.
     */
    [[nodiscard]] bool exists(std::string_view filename) const;

    /**
     * @param filename                  Name of the VID file entry.
     * @return                          Contents of the file inside the VID as a `Blob`.
     * @throws Exception                If file doesn't exist inside the VID.
     */
    [[nodiscard]] Blob read(std::string_view filename) const;

    /**
     * @return                          List of all files in the VID.
     */
    [[nodiscard]] std::vector<std::string> ls() const;

 private:
    struct VidRegion {
        size_t offset = 0;
        size_t size = 0;
    };

 private:
    Blob _vid;
    std::unordered_map<std::string, VidRegion> _files;
};
