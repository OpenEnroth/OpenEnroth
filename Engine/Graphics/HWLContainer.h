#pragma once

#include <cstdint>
#include <cstdio>
#include <map>

#include "Engine/Strings.h"

class HWLTexture {
 public:
    inline HWLTexture() {}

    int uBufferWidth = 0;
    int uBufferHeight = 0;
    int uAreaWidth = 0;
    int uAreaHeigth = 0;
    unsigned int uWidth = 0;
    unsigned int uHeight = 0;
    int uAreaX = 0;
    int uAreaY = 0;
    uint16_t *pPixels = nullptr;
};

class Log;

class HWLContainer {
 public:
    HWLContainer();
    virtual ~HWLContainer();

    bool Open(const String &pFilename);

    HWLTexture *LoadTexture(const String &pName);

 protected:
    FILE *pFile;
    Log *log;
    std::map<String, size_t> mNodes;
};
