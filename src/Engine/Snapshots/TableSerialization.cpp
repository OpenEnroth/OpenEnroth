#include "TableSerialization.h"

#include <utility>
#include <vector>

#include "Engine/Tables/PortraitFrameTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/OverlayTable.h"
#include "Engine/Tables/TextureFrameTable.h"
#include "Engine/Tables/TileTable.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Tables/DecorationTable.h"

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
    deserialize(src, &dst->pFrames, tags::append, tags::each, tags::via<PortraitFrameData_MM7>);

    assert(!dst->pFrames.empty());
}

void deserialize(const Blob &src, DecorationTable *dst) {
    dst->decorations.clear();
    deserialize(src, &dst->decorations, tags::append, tags::each, tags::via<DecorationData_MM7>);

    assert(!dst->decorations.empty());
}

void deserialize(const Blob &src, IconFrameTable *dst) {
    dst->_frames.clear();
    deserialize(src, &dst->_frames, tags::append, tags::each, tags::via<IconFrameData_MM7>);
    dst->_textures.resize(dst->_frames.size());

    assert(!dst->_frames.empty());
}

void deserialize(const Blob &src, MonsterList *dst) {
    std::vector<MonsterDesc> monsters;

    deserialize(src, &monsters, tags::append, tags::each, tags::via<MonsterDesc_MM7>);

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
    deserialize(src, &dst->pObjects, tags::append, tags::each, tags::via<ObjectDesc_MM7>);

    assert(!dst->pObjects.empty());
}

void deserialize(const Blob &src, OverlayTable *dst) {
    dst->overlays.clear();
    deserialize(src, &dst->overlays, tags::append, tags::each, tags::via<OverlayData_MM7>);

    assert(!dst->overlays.empty());
}

void deserialize(const Blob &src, SpriteFrameTable *dst) {
    deserialize(src, dst, tags::via<SpriteFrameTable_MM7>);
}

void deserialize(const Blob &src, TextureFrameTable *dst) {
    deserialize(src, &dst->_frames, tags::append, tags::each, tags::via<TextureFrameData_MM7>);
    dst->_textures.resize(dst->_frames.size());

    assert(!dst->_frames.empty());
}

void deserialize(const Blob &src, SoundList *dst) {
    std::vector<SoundInfo> sounds;
    deserialize(src, &sounds, tags::append, tags::each, tags::via<SoundInfo_MM7>);

    assert(!sounds.empty());

    // TODO(captainurist): do this as a data patch.
    for (SoundInfo &sound : sounds) {
        if (sound.name == "Human Town Hall") {
            sound.soundId = SOUND_HumanTownHall; // MM7 files it as 34302, but playHouseSound derives 31401 from its room sound id.
        } else if (sound.soundId == SoundId(12071)) {
            sound.soundId = SOUND_AcidBurstImpact; // Acid Burst's impact, playSpellSound looks it up as the cast sound 12060 + 1.
        } else if (sound.name == "Dwarf Weapon Shop 01") {
            sound.soundId = SOUND_DwarfWeaponShop01; // MM7 files the dwarf smith in room 82 over the Elf Magic Shop lines, room 85 is free.
        } else if (sound.name == "Dwarf Weapon Shop 02") {
            sound.soundId = SOUND_DwarfWeaponShop02;
        } else if (sound.name == "Dwarf Weapon Shop 03") {
            sound.soundId = SOUND_DwarfWeaponShop03;
        } else if (sound.name == "Dwarf Weapon Shop 04") {
            sound.soundId = SOUND_DwarfWeaponShop04;
        }
    }

    for (const SoundInfo &sound : sounds)
        dst->_mapSounds[sound.soundId] = sound;
}

void deserialize(const Blob &src, TileTable *dst) {
    deserialize(src, &dst->_tiles, tags::append, tags::each, tags::via<TileData_MM7>);

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
