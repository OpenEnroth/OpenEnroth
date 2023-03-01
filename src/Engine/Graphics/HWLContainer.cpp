#include "Engine/Graphics/HWLContainer.h"

#include <cstring>
#include <vector>

#include "Engine/IocContainer.h"
#include "Library/Compression/Compression.h"
#include "Library/Logger/Logger.h"
#include "Utility/String.h"

// TODO(captainurist): (void) fread is a bad practice.

#pragma pack(push, 1)
struct HWLHeader {
    uint32_t uSignature;
    uint32_t uDataOffset;
};
#pragma pack(pop)

HWLContainer::HWLContainer() {
    pFile = nullptr;
    log = Engine_::IocContainer::ResolveLogger();
}

HWLContainer::~HWLContainer() {
    if (pFile != nullptr) {
        fclose(this->pFile);
    }
}

bool HWLContainer::Open(const std::string &pFilename) {
    pFile = fopen(pFilename.c_str(), "rb");
    if (!pFile) {
        log->Warning("Failed to open file: {}", pFilename);
        return false;
    }

    HWLHeader header;
    (void) fread(&header, sizeof(HWLHeader), 1, pFile);
    if (memcmp(&header.uSignature, "D3DT", 4) != 0) {
        log->Warning("Invalid format: {}", pFilename);
        return false;
    }
    fseek(pFile, header.uDataOffset, SEEK_SET);

    typedef struct HWLNode {
        std::string sName;
        size_t uOffset = 0;
    } HWLNode;
    std::vector<HWLNode> vNodes;

    uint32_t uNumItems = 0;
    (void) fread(&uNumItems, 4, 1, pFile);
    char tmpName[21];
    for (unsigned int i = 0; i < uNumItems; ++i) {
        (void) fread(tmpName, 20, 1, pFile);
        tmpName[20] = 0;
        HWLNode node;
        node.sName = toLower(std::string(tmpName));
        node.uOffset = 0;
        vNodes.push_back(node);
    }

    for (unsigned int i = 0; i < uNumItems; ++i) {
        uint32_t uOffset = 0;
        (void) fread(&uOffset, 4, 1, pFile);
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
    (void) fread(&textureHeader, sizeof(HWLTextureHeader), 1, pFile);

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
        Blob buffer = Blob::Allocate(textureHeader.uCompressedSize);
        (void) fread(buffer.data(), buffer.size(), 1, pFile);
        buffer = zlib::Uncompress(buffer);
        memcpy(pTex->pPixels, buffer.data(), buffer.size()); // TODO: gotta check size here.
    } else {
        (void) fread(pTex->pPixels, 2, pTex->uWidth * pTex->uHeight, pFile);
    }

    return pTex;
}
