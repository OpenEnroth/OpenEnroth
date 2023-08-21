#include "Engine/Graphics/HWLContainer.h"

#include <cassert>
#include <cstring>
#include <vector>
#include <utility>

#include "Engine/EngineIocContainer.h"
#include "Library/Compression/Compression.h"
#include "Library/Logger/Logger.h"
#include "Utility/String.h"
#include "Utility/Memory/Blob.h"

#pragma pack(push, 1)
struct HWLHeader {
    uint32_t uSignature;
    uint32_t uDataOffset;
};
#pragma pack(pop)

HWLContainer::HWLContainer() {
    log = EngineIocContainer::ResolveLogger();
}

HWLContainer::~HWLContainer() {
    if (pFile != nullptr) {
        fclose(this->pFile);
    }
}

bool HWLContainer::Open(const std::string &pFilename) {
    assert(!pFile);

    pFile = fopen(pFilename.c_str(), "rb");
    if (!pFile) {
        log->warning("Failed to open file: {}", pFilename);
        return false;
    }

    HWLHeader header;
    if (fread(&header, sizeof(HWLHeader), 1, pFile) != 1)
        return false;

    if (memcmp(&header.uSignature, "D3DT", 4) != 0) {
        log->warning("Invalid format: {}", pFilename);
        return false;
    }
    fseek(pFile, header.uDataOffset, SEEK_SET);

    typedef struct HWLNode {
        std::string sName;
        size_t uOffset = 0;
    } HWLNode;
    std::vector<HWLNode> vNodes;

    uint32_t uNumItems = 0;
    if (fread(&uNumItems, 4, 1, pFile) != 1)
        return false;

    char tmpName[21];
    for (unsigned int i = 0; i < uNumItems; ++i) {
        if (fread(tmpName, 20, 1, pFile) != 1)
            return false;
        tmpName[20] = 0;
        HWLNode node;
        node.sName = toLower(std::string(tmpName));
        node.uOffset = 0;
        vNodes.push_back(node);
    }

    for (unsigned int i = 0; i < uNumItems; ++i) {
        uint32_t uOffset = 0;
        if (fread(&uOffset, 4, 1, pFile) != 1)
            return false;
        vNodes[i].uOffset = uOffset;
    }

    for (HWLNode &node : vNodes) {
        mNodes[node.sName] = node.uOffset;
    }

    return true;
}

#pragma pack(push, 1)
struct HWLTextureHeader {
    uint32_t uCompressedSize;
    uint32_t uBufferWidth;
    uint32_t uBufferHeight;
    uint32_t uAreaWidth;
    uint32_t uAreaHeigth;
    uint32_t uWidth;
    uint32_t uHeight;
    uint32_t uAreaX;
    uint32_t uAreaY;
};
#pragma pack(pop)

HWLTexture *HWLContainer::LoadTexture(const std::string &pName) {
    if (mNodes.size() == 0) {
        return nullptr;
    }

    std::map<std::string, size_t>::iterator it = mNodes.find(toLower(pName));
    if (it == mNodes.end()) {
        return nullptr;
    }
    size_t uOffset = it->second;

    fseek(pFile, uOffset, SEEK_SET);

    HWLTextureHeader textureHeader;
    if (fread(&textureHeader, sizeof(HWLTextureHeader), 1, pFile) != 1)
        return nullptr;

    HWLTexture *pTex = new HWLTexture;
    pTex->uBufferWidth = textureHeader.uBufferWidth;
    pTex->uBufferHeight = textureHeader.uBufferHeight;
    pTex->uAreaWidth = textureHeader.uAreaWidth;
    pTex->uAreaHeigth = textureHeader.uAreaHeigth;
    pTex->uWidth = textureHeader.uWidth;
    pTex->uHeight = textureHeader.uHeight;
    pTex->uAreaX = textureHeader.uAreaX;
    pTex->uAreaY = textureHeader.uAreaY;

    pTex->pPixels = new uint16_t[pTex->uWidth * pTex->uHeight];
    if (textureHeader.uCompressedSize) {
        Blob buffer = zlib::uncompress(Blob::read(pFile, textureHeader.uCompressedSize));
        memcpy(pTex->pPixels, buffer.data(), buffer.size()); // TODO: gotta check size here.
    } else {
        if (fread(pTex->pPixels, 2 * pTex->uWidth * pTex->uHeight, 1, pFile) != 1)
            return nullptr;
    }

    return pTex;
}
