#include "TableSerialization.h"

#include <vector>

#include "Engine/Tables/PortraitFrameTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/TileTable.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/TextureFrameTable.h"

#include "Media/Audio/SoundList.h"

#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Exception.h"

#include "EntitySnapshots.h"
#include "CompositeSnapshots.h"

void deserialize(const TriBlob &src, PortraitFrameTable *dst) {
    dst->pFrames.clear();

    if (src.mm6)
        deserialize(src.mm6, &dst->pFrames, tags::append, tags::via<PortraitFrameData_MM7>);
    if (src.mm7)
        deserialize(src.mm7, &dst->pFrames, tags::append, tags::via<PortraitFrameData_MM7>);
    if (src.mm8)
        deserialize(src.mm8, &dst->pFrames, tags::append, tags::via<PortraitFrameData_MM7>);

    assert(!dst->pFrames.empty());
}

void deserialize(const TriBlob &src, DecorationList *dst) {
    dst->pDecorations.clear();

    if (src.mm6)
        deserialize(src.mm6, &dst->pDecorations, tags::append, tags::via<DecorationDesc_MM6>);
    if (src.mm7)
        deserialize(src.mm7, &dst->pDecorations, tags::append, tags::via<DecorationDesc_MM7>);
    if (src.mm8)
        deserialize(src.mm8, &dst->pDecorations, tags::append, tags::via<DecorationDesc_MM7>);

    assert(!dst->pDecorations.empty());
}

void deserialize(const TriBlob &src, IconFrameTable *dst) {
    dst->_frames.clear();
    deserialize(src.mm7, &dst->_frames, tags::append, tags::via<IconFrameData_MM7>);
    dst->_textures.resize(dst->_frames.size());

    assert(!dst->_frames.empty());
}

void deserialize(const TriBlob &src, MonsterList *dst) {
    std::vector<MonsterDesc> monsters;

    if (src.mm7)
        deserialize(src.mm7, &monsters, tags::append, tags::via<MonsterDesc_MM7>);

    if (monsters.size() != 277)
        throw Exception("Invalid monster list size, expected {}, got {}", 277, monsters.size());
    monsters.pop_back(); // Last one is unused.

    assert(monsters.size() == dst->monsters.size());
    dst->monsters.fill(MonsterDesc());
    for (size_t i = 0; MonsterId index : dst->monsters.indices())
        dst->monsters[index] = monsters[i++];
}

void deserialize(const TriBlob &src, ObjectList *dst) {
    dst->pObjects.clear();

    if (src.mm6)
        deserialize(src.mm6, &dst->pObjects, tags::append, tags::via<ObjectDesc_MM6>);
    if (src.mm7)
        deserialize(src.mm7, &dst->pObjects, tags::append, tags::via<ObjectDesc_MM7>);
    if (src.mm8)
        deserialize(src.mm8, &dst->pObjects, tags::append, tags::via<ObjectDesc_MM7>);

    assert(!dst->pObjects.empty());
}

void deserialize(const TriBlob &src, OverlayList *dst) {
    dst->pOverlays.clear();

    if (src.mm6)
        deserialize(src.mm6, &dst->pOverlays, tags::append, tags::via<OverlayDesc_MM7>);
    if (src.mm7)
        deserialize(src.mm7, &dst->pOverlays, tags::append, tags::via<OverlayDesc_MM7>);
    if (src.mm8)
        deserialize(src.mm8, &dst->pOverlays, tags::append, tags::via<OverlayDesc_MM7>);

    assert(!dst->pOverlays.empty());
}

void deserialize(const TriBlob &src, SpriteFrameTable *dst) {
    deserialize(src.mm7, dst, tags::via<SpriteFrameTable_MM7>);
}

void deserialize(const TriBlob &src, TextureFrameTable *dst) {
    deserialize(src.mm7, &dst->textures, tags::append, tags::via<TextureFrame_MM7>);

    assert(!dst->textures.empty());
}

void deserialize(const TriBlob &src, TileTable *dst) {
    deserialize(src.mm7, &dst->tiles, tags::append, tags::via<TileData_MM7>);

    assert(!dst->tiles.empty());
}

void deserialize(const TriBlob &src, SoundList *dst) {
    std::vector<SoundInfo> sounds;

    if (src.mm6)
        deserialize(src.mm6, &sounds, tags::append, tags::via<SoundInfo_MM6>);
    if (src.mm7)
        deserialize(src.mm7, &sounds, tags::append, tags::via<SoundInfo_MM7>);
    if (src.mm8)
        deserialize(src.mm8, &sounds, tags::append, tags::via<SoundInfo_MM7>);

    assert(!sounds.empty());

    // TODO(captainurist): there are duplicate ids in the sounds array, look into it.
    for (const SoundInfo &sound : sounds)
        dst->_mapSounds[sound.uSoundID] = sound;
}
