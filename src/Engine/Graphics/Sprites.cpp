#include "Engine/Graphics/Sprites.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <string>

#include "Engine/OurMath.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/Image.h"
#include "Engine/LodSpriteCache.h"
#include "Engine/Seasons.h"

#include "Library/Logger/Logger.h"
#include "Library/LodFormats/LodFormats.h"

#include "Utility/String/Ascii.h"

SpriteFrameTable *pSpriteFrameTable;

void Sprite::Release() {
    delete this->sprite_header;
    this->sprite_header = nullptr;
    this->texture->Release();
    this->texture = nullptr;
    this->pName = "null";
}

//----- (0044D4F6) --------------------------------------------------------
void SpriteFrameTable::ResetLoadedFlags() {
    for (SpriteFrame &spriteFrame : pSpriteSFrames)
        spriteFrame.flags &= ~0x80;
}

//----- (0044D513) --------------------------------------------------------
void SpriteFrameTable::InitializeSprite(signed int uSpriteID) {
    std::string spriteName;

    if (uSpriteID <= pSpriteSFrames.size()) {
        if (uSpriteID >= 0) {
            unsigned iter_uSpriteID = uSpriteID;
            //if (iter_uSpriteID == 603) assert(false);

            int uFlags = pSpriteSFrames[iter_uSpriteID].flags;

            if (!(uFlags & 0x0080)) {  // not loaded
                pSpriteSFrames[iter_uSpriteID].flags |= 0x80;  // set loaded

                while (1) {
                    if (uFlags & 0x10) {  // single frame per frame sequence
                        Sprite *sprite = pSprites_LOD->loadSprite(pSpriteSFrames[iter_uSpriteID].textureName);
                        if (sprite == nullptr)
                            logger->warning("Sprite {} not loaded!", pSpriteSFrames[iter_uSpriteID].textureName);
                        for (unsigned i = 0; i < 8; ++i)
                            pSpriteSFrames[iter_uSpriteID].sprites[i] = sprite;
                    } else if (uFlags & 0x10000) {
                        for (unsigned i = 0; i < 8; ++i) {
                            switch (i) {
                                case 3:
                                case 4:
                                case 5:
                                    spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "4";
                                    break;
                                case 2:
                                case 6:
                                    spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "2";
                                    break;
                                case 0:
                                case 1:
                                case 7:
                                    spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "0";
                                    break;
                            }
                            Sprite *sprite = pSprites_LOD->loadSprite(spriteName);
                            // pSpriteSFrames[iter_uSpriteID].pHwSpriteIDs[i]=v12;
                            assert(sprite);
                            pSpriteSFrames[iter_uSpriteID].sprites[i] = sprite;
                        }

                    } else if (uFlags & 0x40) {  // part of monster fidgeting seq
                        for (unsigned i = 0; i < 8; ++i) {
                            switch (i) {
                                case 0:
                                    spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "0";
                                    break;
                                case 4:
                                    spriteName = pSpriteSFrames[iter_uSpriteID].textureName;
                                    spriteName.erase(spriteName.size() - 3, 3);
                                    spriteName = spriteName + "stA4";
                                    break;
                                case 3:
                                case 5:
                                    spriteName = pSpriteSFrames[iter_uSpriteID].textureName;
                                    spriteName.erase(spriteName.size() - 3, 3);
                                    spriteName = spriteName + "stA3";
                                    break;
                                case 2:
                                case 6:
                                    spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "2";
                                    break;
                                case 1:
                                case 7:
                                    spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "1";
                                    break;
                            }
                            Sprite *sprite = pSprites_LOD->loadSprite(spriteName);
                            // pSpriteSFrames[iter_uSpriteID].pHwSpriteIDs[i]=v12;
                            assert(sprite);
                            pSpriteSFrames[iter_uSpriteID].sprites[i] = sprite;
                        }
                    } else {
                        for (unsigned i = 0; i < 8; ++i) {
                            if (((0x0100 << i) & pSpriteSFrames[iter_uSpriteID].flags)) {  // mirrors
                                switch (i) {
                                    case 1:
                                        spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "7";
                                        break;
                                    case 2:
                                        spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "6";
                                        break;
                                    case 3:
                                        spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "5";
                                        break;
                                    case 4:
                                        spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "4";
                                        break;
                                    case 5:
                                        spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "3";
                                        break;
                                    case 6:
                                        spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "2";
                                        break;
                                    case 7:
                                        spriteName = pSpriteSFrames[iter_uSpriteID].textureName + "1";
                                        break;
                                }
                            } else {
                                // some names already passed through with codes attached
                                if (pSpriteSFrames[iter_uSpriteID].textureName.size() < 7) {
                                    spriteName = fmt::format("{}{}", pSpriteSFrames[iter_uSpriteID].textureName, i);
                                } else {
                                    spriteName = pSpriteSFrames[iter_uSpriteID].textureName;
                                    // assert(false);
                                }
                            }

                            Sprite *sprite = pSprites_LOD->loadSprite(spriteName);
                            // pSpriteSFrames[iter_uSpriteID].pHwSpriteIDs[i]=v12;
                            assert(sprite);
                            pSpriteSFrames[iter_uSpriteID].sprites[i] = sprite;
                        }
                    }

                    if (!(pSpriteSFrames[iter_uSpriteID].flags & 1)) {
                        return;
                    }
                    ++iter_uSpriteID;
                }
            }
        }
    }
}

//----- (0044D813) --------------------------------------------------------
int SpriteFrameTable::FastFindSprite(std::string_view pSpriteName) {
    auto cmp = [this] (uint16_t index, std::string_view name) {
        return ascii::noCaseLess(pSpriteSFrames[index].animationName, name);
    };

    auto pos = std::lower_bound(pSpriteEFrames.begin(), pSpriteEFrames.end(), pSpriteName, cmp);
    if (pos == pSpriteEFrames.end())
        return 0;

    return ascii::noCaseEquals(pSpriteSFrames[*pos].animationName, pSpriteName) ? *pos : 0;
}

//----- (0044D8D0) --------------------------------------------------------
SpriteFrame *SpriteFrameTable::GetFrame(int uSpriteID, Duration uTime) {
    SpriteFrame *v4 = &pSpriteSFrames[uSpriteID];
    if (~v4->flags & 1 || !v4->animationLength)
        return v4;

    // uAnimLength / uAnimTime = actual number of frames in sprite
    for (Duration t = uTime % v4->animationLength; t >= v4->frameLength; ++v4)
        t -= v4->frameLength;

    // TODO(pskelton): investigate and fix properly - dragon breath is missing last two frames??
    // quick fix so it doesnt return empty sprite
    while (v4->sprites[0] == NULL) {
        //assert(false);
        --v4;
    }

    return v4;
}

//----- (0044D91F) --------------------------------------------------------
SpriteFrame *SpriteFrameTable::GetFrameReversed(int uSpriteID, Duration time) {
    SpriteFrame *sprite = &pSpriteSFrames[uSpriteID];
    if (!(sprite->flags & 1) || !sprite->animationLength)
        return sprite;

    for (Duration t = sprite->animationLength - time % sprite->animationLength; t >= sprite->frameLength; ++sprite)
        t -= sprite->frameLength;

    return sprite;
}

SpriteFrame *LevelDecorationChangeSeason(const DecorationDesc *desc, Duration t, int month) {
    int spriteId = spriteIdForSeason(desc->uSpriteID, month);
    if (spriteId != desc->uSpriteID)
        pSpriteFrameTable->InitializeSprite(spriteId);
    return pSpriteFrameTable->GetFrame(spriteId, t);
}
