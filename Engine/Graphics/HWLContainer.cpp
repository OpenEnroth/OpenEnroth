#include "Engine/Graphics/HWLContainer.h"

#include <cstring>
#include <vector>

#include "Engine/IocContainer.h"
#include "Engine/Log.h"
#include "Engine/ZlibWrapper.h"

#include "Platform/Api.h"


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

bool HWLContainer::Open(const String &pFilename) {
    pFile = fcaseopen(pFilename.c_str(), "rb");
    if (!pFile) {
        log->Warning("Failed to open file: %s", pFilename.c_str());
        return false;
    }

    HWLHeader header;
    fread(&header, sizeof(HWLHeader), 1, pFile);
    if (header.uSignature != 'TD3D') {
        log->Warning("Invalid format: %s", pFilename.c_str());
        return false;
    }
    fseek(pFile, header.uDataOffset, SEEK_SET);

    typedef struct HWLNode {
        String sName;
        size_t uOffset;
    } HWLNode;
    std::vector<HWLNode> vNodes;

    uint32_t uNumItems = 0;
    fread(&uNumItems, 4, 1, pFile);
    char tmpName[21];
    for (unsigned int i = 0; i < uNumItems; ++i) {
        fread(tmpName, 20, 1, pFile);
        tmpName[20] = 0;
        HWLNode node;
        node.sName = MakeLower(String(tmpName));
        node.uOffset = 0;
        vNodes.push_back(node);
    }

    for (unsigned int i = 0; i < uNumItems; ++i) {
        uint32_t uOffset = 0;
        fread(&uOffset, 4, 1, pFile);
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

HWLTexture *HWLContainer::LoadTexture(const String &pName) {
    if (mNodes.size() == 0) {
        return nullptr;
    }

    std::map<String, size_t>::iterator it = mNodes.find(MakeLower(pName));
    if (it == mNodes.end()) {
        return nullptr;
    }
    size_t uOffset = it->second;

    fseek(pFile, uOffset, SEEK_SET);

    HWLTextureHeader textureHeader;
    fread(&textureHeader, sizeof(HWLTextureHeader), 1, pFile);

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
        PMemBuffer buffer = AllocMemBuffer(textureHeader.uCompressedSize);
        fread((void*)buffer->GetData(), textureHeader.uCompressedSize, 1, pFile);
        buffer = zlib::Uncompress(buffer);
        memcpy(pTex->pPixels, buffer->GetData(), buffer->GetSize());
    } else {
        fread(pTex->pPixels, 2, pTex->uWidth * pTex->uHeight, pFile);
    }

    return pTex;
}
