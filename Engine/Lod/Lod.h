#pragma once

#include <memory>
#include <numeric>
#include <string>

#include "Engine/Lod/Version.h"



namespace LOD {
class Lod {
public:
    Lod();
    virtual ~Lod();

    // Lod file factory
    static shared_ptr<Lod> Open(const std::string& filename);

    bool OpenFolder(const std::string& folder);
    virtual void Close();

    bool FileExists(const std::string& filename);
    void* LoadRaw(const std::string& pContainer, size_t* data_size = nullptr);
    void* LoadCompressed2(const std::string& filename, size_t* out_file_size = nullptr);
    void* LoadCompressed(const std::string& pContainer, size_t* data_size = nullptr);
    void Dispose(void* data_ptr);

    std::string GetSubNodeName(size_t index) const { return _current_folder->files[index].name; }
    size_t GetSubNodesCount() const { return _current_folder->files.size(); }
    int GetSubNodeIndex(const std::string& name) const;

    size_t GetTotalNumFiles() const {
        return std::accumulate(
            _index.begin(),
            _index.end(),
            size_t{ 0 },
            [](const size_t sum, const std::shared_ptr<const Directory>& dir) {
                return sum + dir->files.size();
            }
        );
    }


protected:
    FILE* FindFile(const std::string& filename, size_t* out_file_size = nullptr);
    bool OpenContainerFile(const std::string& filename);
    bool ParseHeader();
    //virtual void ResetSubIndices();

protected:
    std::string _filename;
    LOD_VERSION _version;
    std::string _description;
    std::vector<std::shared_ptr<Directory>> _index;
    std::shared_ptr<Directory> _current_folder = nullptr;
    FILE* _file = nullptr;
};
};  // namespace LOD


extern LOD::Lod* pGames_LOD;
