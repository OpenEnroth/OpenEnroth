#pragma once

#include "Utility/Memory/Blob.h"

class DecorationList;
class ObjectList;
class SoundList;
class IconFrameTable;
struct MonsterList;
struct OverlayList;
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
void deserialize(const Blob &src, DecorationList *dst);

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
void deserialize(const Blob &src, OverlayList *dst);

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
