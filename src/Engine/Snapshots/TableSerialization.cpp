#include "TableSerialization.h"

#include "Engine/Tables/CharacterFrameTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/TileTable.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/TextureFrameTable.h"

#include "Library/Snapshots/SnapshotSerialization.h"

#include "EntitySnapshots.h"
#include "CompositeSnapshots.h"

void deserialize(const TriBlob &src, PlayerFrameTable *dst) {
    dst->pFrames.clear();

    if (src.mm6)
        deserialize(src.mm6, &dst->pFrames, append(), via<PlayerFrame_MM7>());
    if (src.mm7)
        deserialize(src.mm7, &dst->pFrames, append(), via<PlayerFrame_MM7>());
    if (src.mm8)
        deserialize(src.mm8, &dst->pFrames, append(), via<PlayerFrame_MM7>());

    assert(!dst->pFrames.empty());
}

void deserialize(const TriBlob &src, ChestList *dst) {
    dst->vChests.clear();

    if (src.mm6)
        deserialize(src.mm6, &dst->vChests, append(), via<ChestDesc_MM7>());
    if (src.mm7)
        deserialize(src.mm7, &dst->vChests, append(), via<ChestDesc_MM7>());
    if (src.mm8)
        deserialize(src.mm8, &dst->vChests, append(), via<ChestDesc_MM7>());

    assert(!dst->vChests.empty());
}

void deserialize(const TriBlob &src, DecorationList *dst) {
    dst->pDecorations.clear();

    if (src.mm6)
        deserialize(src.mm6, &dst->pDecorations, append(), via<DecorationDesc_MM6>());
    if (src.mm7)
        deserialize(src.mm7, &dst->pDecorations, append(), via<DecorationDesc_MM7>());
    if (src.mm8)
        deserialize(src.mm8, &dst->pDecorations, append(), via<DecorationDesc_MM7>());

    assert(!dst->pDecorations.empty());
}

void deserialize(const TriBlob &src, IconFrameTable *dst) {
    dst->pIcons.clear();
    deserialize(src.mm7, &dst->pIcons, append(), via<IconFrame_MM7>());

    for (size_t i = 0; i < dst->pIcons.size(); ++i)
        dst->pIcons[i].id = i;

    assert(!dst->pIcons.empty());
}

void deserialize(const TriBlob &src, MonsterList *dst) {
    dst->pMonsters.clear();

    if (src.mm6)
        deserialize(src.mm6, &dst->pMonsters, append(), via<MonsterDesc_MM6>());
    if (src.mm7)
        deserialize(src.mm7, &dst->pMonsters, append(), via<MonsterDesc_MM7>());
    if (src.mm8)
        deserialize(src.mm8, &dst->pMonsters, append(), via<MonsterDesc_MM7>());

    assert(!dst->pMonsters.empty());
}

void deserialize(const TriBlob &src, ObjectList *dst) {
    dst->pObjects.clear();

    if (src.mm6)
        deserialize(src.mm6, &dst->pObjects, append(), via<ObjectDesc_MM6>());
    if (src.mm7)
        deserialize(src.mm7, &dst->pObjects, append(), via<ObjectDesc_MM7>());
    if (src.mm8)
        deserialize(src.mm8, &dst->pObjects, append(), via<ObjectDesc_MM7>());

    assert(!dst->pObjects.empty());
}

void deserialize(const TriBlob &src, OverlayList *dst) {
    dst->pOverlays.clear();

    if (src.mm6)
        deserialize(src.mm6, &dst->pOverlays, append(), via<OverlayDesc_MM7>());
    if (src.mm7)
        deserialize(src.mm7, &dst->pOverlays, append(), via<OverlayDesc_MM7>());
    if (src.mm8)
        deserialize(src.mm8, &dst->pOverlays, append(), via<OverlayDesc_MM7>());

    assert(!dst->pOverlays.empty());
}

void deserialize(const TriBlob &src, SpriteFrameTable *dst) {
    deserialize(src.mm7, dst, via<SpriteFrameTable_MM7>());
}

void deserialize(const TriBlob &src, TextureFrameTable *dst) {
    deserialize(src.mm7, &dst->textures, append(), via<TextureFrame_MM7>());

    assert(!dst->textures.empty());
}

void deserialize(const TriBlob &src, TileTable *dst) {
    deserialize(src.mm7, &dst->tiles, append(), via<TileDesc_MM7>());

    assert(!dst->tiles.empty());
}
