#pragma once

#include <array>
#include <string>
#include <vector>

#include "Engine/Time/Duration.h"

struct DecorationDesc;
class GraphicsImage;
struct LodSprite;

class Sprite {
 public:
    void Release();

    std::string pName;
    GraphicsImage *texture = nullptr;
    int uAreaX = 0; // TODO(captainurist): always zero,
    int uAreaY = 0; // TODO(captainurist): was intended to support sprite maps?
    int uWidth = 0; // Same as texture->width().
    int uHeight = 0;
    LodSprite *sprite_header = nullptr;
};

// TODO(captainurist) : move to Engine/Data and Engine/Tables

class SpriteFrame {
 public:
    std::string animationName;
    std::string textureName;
    std::array<Sprite *, 8> sprites = {{}};
    float scale = 1.0;
    int flags = 0;  // 128 for loaded - 1 for anim
    int glowRadius = 0;
    int paletteId = 0;
    Duration frameLength;
    Duration animationLength;
};

struct SpriteFrameTable {
    void ResetLoadedFlags();
    void InitializeSprite(signed int uSpriteID);

    /**
     * @param pSpriteName               Name of the sprite to find. Names are case-insensitive.
     * @return                          Index in `pSpriteSFrames` for the sprite, or 0 if sprite wasn't found.
     *                                  Conveniently, sprite 0 is a dummy sprite that actually exists.
     */
    int FastFindSprite(std::string_view pSpriteName);
    SpriteFrame *GetFrame(int uSpriteID, Duration uTime);
    SpriteFrame *GetFrameReversed(int uSpriteID, Duration time);

    std::vector<SpriteFrame> pSpriteSFrames;

    /** Indices into `pSpriteSFrames`, sorted by sprite name. Note that `pSpriteSFrames` itself is not sorted.
     * Contains only indices for 'a' (frontal?) sprites, so smaller in size than `pSpriteSFrames`. */
    std::vector<uint16_t> pSpriteEFrames;
};

extern SpriteFrameTable *pSpriteFrameTable;

SpriteFrame *LevelDecorationChangeSeason(const DecorationDesc *desc, Duration t, int month);
