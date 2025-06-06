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
struct TextureFrameTable;
struct TileTable;

struct TriBlob {
    Blob mm6;
    Blob mm7;
    Blob mm8;
};

/**
 * @offset 0x00494C0F
 */
void deserialize(const TriBlob &src, PortraitFrameTable *dst);

/**
 * @offset 0x0045864C
 */
void deserialize(const TriBlob &src, DecorationList *dst);

/**
 * @offset 0x00495056
 */
void deserialize(const TriBlob &src, IconFrameTable *dst);

/**
 * @offset 0x004598AF
 */
void deserialize(const TriBlob &src, MonsterList *dst);

void deserialize(const TriBlob &src, ObjectList *dst);

/**
 * @offset 0x00458E08
 */
void deserialize(const TriBlob &src, OverlayList *dst);

/**
 * @offset 0x0044D9D7
 */
void deserialize(const TriBlob &src, SpriteFrameTable *dst);

void deserialize(const TriBlob &src, TextureFrameTable *dst);

/**
 * @offset 0x00488000
 */
void deserialize(const TriBlob &src, TileTable *dst);

void deserialize(const TriBlob &src, SoundList *dst);
