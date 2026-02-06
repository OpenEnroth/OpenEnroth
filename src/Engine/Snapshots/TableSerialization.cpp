#include "TableSerialization.h"

#include <vector>

#include "Engine/Tables/PortraitFrameTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/TextureFrameTable.h"
#include "Engine/Tables/TileTable.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Graphics/Overlays.h"

#include "Media/Audio/SoundList.h"

#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Exception.h"

#include "EntitySnapshots.h"
#include "CompositeSnapshots.h"
#include "Engine/Data/TileEnumFunctions.h"
#include "Library/Logger/Logger.h"
#include "Library/Serialization/Serialization.h"

void deserialize(const Blob &src, PortraitFrameTable *dst) {
    dst->pFrames.clear();
    deserialize(src, &dst->pFrames, tags::append, tags::via_each<PortraitFrameData_MM7>);

    assert(!dst->pFrames.empty());
}

void deserialize(const Blob &src, DecorationList *dst) {
    dst->pDecorations.clear();
    deserialize(src, &dst->pDecorations, tags::append, tags::via_each<DecorationDesc_MM7>);

    assert(!dst->pDecorations.empty());
}

void deserialize(const Blob &src, IconFrameTable *dst) {
    dst->_frames.clear();
    deserialize(src, &dst->_frames, tags::append, tags::via_each<IconFrameData_MM7>);
    dst->_textures.resize(dst->_frames.size());

    assert(!dst->_frames.empty());
}

void deserialize(const Blob &src, MonsterList *dst) {
    std::vector<MonsterDesc> monsters;

    deserialize(src, &monsters, tags::append, tags::via_each<MonsterDesc_MM7>);

    if (monsters.size() != 277)
        throw Exception("Invalid monster list size, expected {}, got {}", 277, monsters.size());
    monsters.pop_back(); // Last one is unused.

    assert(monsters.size() == dst->monsters.size());
    dst->monsters.fill(MonsterDesc());
    for (size_t i = 0; MonsterId index : dst->monsters.indices())
        dst->monsters[index] = monsters[i++];
}

void deserialize(const Blob &src, ObjectList *dst) {
    dst->pObjects.clear();
    deserialize(src, &dst->pObjects, tags::append, tags::via_each<ObjectDesc_MM7>);

    assert(!dst->pObjects.empty());
}

void deserialize(const Blob &src, OverlayList *dst) {
    dst->pOverlays.clear();
    deserialize(src, &dst->pOverlays, tags::append, tags::via_each<OverlayDesc_MM7>);

    assert(!dst->pOverlays.empty());
}

void deserialize(const Blob &src, SpriteFrameTable *dst) {
    deserialize(src, dst, tags::via<SpriteFrameTable_MM7>);
}

void deserialize(const Blob &src, TextureFrameTable *dst) {
    deserialize(src, &dst->_frames, tags::append, tags::via_each<TextureFrameData_MM7>);
    dst->_textures.resize(dst->_frames.size());

    assert(!dst->_frames.empty());
}

void deserialize(const Blob &src, SoundList *dst) {
    std::vector<SoundInfo> sounds;
    deserialize(src, &sounds, tags::append, tags::via_each<SoundInfo_MM7>);

    assert(!sounds.empty());

    // TODO(captainurist): there are duplicate ids in the sounds array, look into it.
    for (const SoundInfo &sound : sounds)
        dst->_mapSounds[sound.uSoundID] = sound;
}

void deserialize(const Blob &src, TileTable *dst) {
    deserialize(src, &dst->_tiles, tags::append, tags::via_each<TileData_MM7>);

    // Fill in the tileId map.
    for (size_t i = 0; i < dst->_tiles.size(); i++) {
        const TileData &tileData = dst->_tiles[i];
        if (tileData.tileset == TILESET_INVALID || tileData.variant == TILE_VARIANT_INVALID || tileData.textureName.empty())
            continue;

        std::pair key(tileData.tileset, tileData.variant);

        if (dst->_idByTilesetVariant.contains(key))
            continue; // Just skip duplicates. Yes, MM7 data contains duplicates.

        dst->_idByTilesetVariant[key] = i;
    }

    assert(!dst->_tiles.empty());
}
