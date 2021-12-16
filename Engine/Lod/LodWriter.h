#pragma once
#include <memory>
#include <string>

#include "Engine/Lod/Lod.h"


namespace LOD {
class LodWriter : public Lod {
public:
    //WriteableFile();
    static std::shared_ptr<LodWriter> New(const std::string& filename, LOD_VERSION version, const std::string& description);
    bool AddDirectory(const std::string& name, bool open = true);
    bool AddFile(const std::string& filename, const void* file_ptr, size_t file_size);
    //bool LoadFile(const std::string &filename, bool readonly);

    //bool AppendFileToCurrentDirectory(const std::string& file_name, const void* file_bytes, size_t file_size);
    //bool AddFileToCurrentDirectory(const std::string& file_name, const void *file_bytes, size_t file_size, int flags = 0);

    virtual void Close() override;
    void CloseWriteFile();
    //int OpenTmpWriteFile();
    //int FixDirectoryOffsets();
    void Commit();
    //int CreateEmptyLod(LOD::FileHeader *pHeader, const std::string& lod_name, const std::string& folder_name);

    //void FreeSubIndexAndIO();

    //void ClearSubNodes() {
    //    if (_current_folder) {
    //        _current_folder->files.clear();
    //    }
    //}

protected:
    //virtual void ResetSubIndices();
    void SortDirectories();
};
};  // namespace LOD


extern LOD::WriteableFile* pNew_LOD;
