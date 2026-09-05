#pragma once

#include "Utility/Memory/Blob.h"

struct DecorationTable;
class ObjectList;
class SoundList;
class IconFrameTable;
struct MonsterList;
struct OverlayTable;
struct PortraitFrameTable;
struct SpriteFrameTable;
class TextureFrameTable;
class TileTable;

/**
 * @offset 0x00494C0F
 */
void deserialize(const Blob &src, PortraitFrameTable *dst);

/**
 * @offset 0x0045864C
 */
void deserialize(const Blob &src, DecorationTable *dst);

/**
 * @offset 0x00495056
 */
void deserialize(const Blob &src, IconFrameTable *dst);

/**
 * @offset 0x004598AF
 */
void deserialize(const Blob &src, MonsterList *dst);

void deserialize(const Blob &src, ObjectList *dst);

/**
 * @offset 0x00458E08
 */
void deserialize(const Blob &src, OverlayTable *dst);

/**
 * @offset 0x0044D9D7
 */
void deserialize(const Blob &src, SpriteFrameTable *dst);

void deserialize(const Blob &src, TextureFrameTable *dst);

void deserialize(const Blob &src, SoundList *dst);

/**
 * @offset 0x00488000
 */
void deserialize(const Blob &src, TileTable *dst);
