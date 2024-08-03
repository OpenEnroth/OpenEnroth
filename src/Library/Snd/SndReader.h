#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Utility/Memory/Blob.h"

#include "SndSnapshots.h"

/**
 * Reader for Might&Magic SND files.
 *
 * Note that compression is part of the container format in SND files, unlike in LOD, where it's part of the internal
 * lod-specific file formats. Thus, we're not exposing it as part of the interface, and there is no 'SndFormats'
 * library.
 */
class SndReader {
 public:
    SndReader();
    explicit SndReader(std::string_view path);
    explicit SndReader(Blob blob);
    ~SndReader();

    /**
     * @param path                      Path to the SND file to open for reading.
     * @throw Exception                 If the SND couldn't be opened - e.g., if the file doesn't exist,
     *                                  or if it's not in SND format.
     */
    void open(std::string_view path);

    /**
     * @param blob                      SND data.
     * @throw Exception                 If there are errors in the provided SND file.
     */
    void open(Blob blob);

    /**
     * Closes this SND reader & frees all associated resources.
     */
    void close();

    [[nodiscard]] bool isOpen() const {
        return !!_snd;
    }

    /**
     * @param filename                  Name of the SND file entry.
     * @return                          Whether the file exists inside the SND. The check is case-insensitive.
     */
    [[nodiscard]] bool exists(std::string_view filename) const;

    /**
     * @param filename                  Name of the SND file entry.
     * @return                          Contents of the file inside the SND as a `Blob`.
     * @throws Exception                If file doesn't exist inside the SND.
     */
    [[nodiscard]] Blob read(std::string_view filename) const;

    /**
     * @return                          List of all files in the SND.
     */
    [[nodiscard]] std::vector<std::string> ls() const;

 private:
    Blob _snd;
    std::unordered_map<std::string, SndEntry> _files;
};
