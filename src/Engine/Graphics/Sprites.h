#pragma once

#include <array>
#include <string>
#include <vector>

#include "Engine/Time/Duration.h"

#include "Utility/Memory/Blob.h"

struct LODSprite;
class DecorationDesc;
class GraphicsImage;

class Sprite {
 public:
    void Release();

    std::string pName;
    GraphicsImage *texture = nullptr;
    int uAreaX = 0; // TODO(captainurist): always zero,
    int uAreaY = 0; // TODO(captainurist): was intended to support sprite maps?
    int uWidth = 0; // Same as texture->width().
    int uHeight = 0;
    LODSprite *sprite_header = nullptr;
};

class SpriteFrame {
 public:
    std::string icon_name;
    std::string texture_name;

    /**
     * @return                              Returns int to Index of palette stored in palette manager.
     */
    int GetPaletteIndex();
    /**
     * @param index                         Index to set or default 0.
     */
    void ResetPaletteIndex(int index = 0);

    std::array<Sprite *, 8> hw_sprites = {{}};
    float scale = 1.0;
    int uFlags = 0;  // 128 for loaded - 1 for anim
    int uGlowRadius = 0;
    int uPaletteID = 0;
    Duration uAnimTime;
    Duration uAnimLength;
 private:
    int uPaletteIndex = 0;
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

    /**
     * Resets the uPaletteIndex of all loaded pSpriteSFrames. Called by PaletteManager on reset.
     */
    void ResetPaletteIndexes();

    std::vector<SpriteFrame> pSpriteSFrames;

    /** Indices into `pSpriteSFrames`, sorted by sprite name. Note that `pSpriteSFrames` itself is not sorted.
     * Contains only indices for 'a' (frontal?) sprites, so smaller in size than `pSpriteSFrames`. */
    std::vector<uint16_t> pSpriteEFrames;
};

extern struct SpriteFrameTable *pSpriteFrameTable;

SpriteFrame *LevelDecorationChangeSeason(const DecorationDesc *desc, Duration t, int month);
