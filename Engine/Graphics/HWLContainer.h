#pragma once

#include <cstdint>
#include <cstdio>
#include <map>

#include "Engine/Strings.h"

class HWLTexture {
 public:
    inline HWLTexture() {}

    int uBufferWidth;
    int uBufferHeight;
    int uAreaWidth;
    int uAreaHeigth;
    unsigned int uWidth;
    unsigned int uHeight;
    int uAreaX;
    int uAreaY;
    uint16_t *pPixels;
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
