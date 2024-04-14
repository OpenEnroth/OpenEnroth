#include "RmlFileInterface.h"

#include <Utility/DataPath.h>

#include <cstdio>

RmlFileInterface::~RmlFileInterface() {
}

Rml::FileHandle RmlFileInterface::Open(const Rml::String &path) {
    return (Rml::FileHandle)fopen(path.c_str(), "rb");
}

void RmlFileInterface::Close(Rml::FileHandle file) {
    fclose((FILE *)file);
}

size_t RmlFileInterface::Read(void *buffer, size_t size, Rml::FileHandle file) {
    return fread(buffer, 1, size, (FILE *)file);
}

bool RmlFileInterface::Seek(Rml::FileHandle file, long offset, int origin) {
    return fseek((FILE *)file, offset, origin) == 0;
}

size_t RmlFileInterface::Tell(Rml::FileHandle file) {
    return ftell((FILE *)file);
}
