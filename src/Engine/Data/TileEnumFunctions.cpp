#include "TileEnumFunctions.h"

#include <cassert>

SoundId walkSoundForTileset(Tileset tileset, bool isRunning) {
    switch (tileset) {
    default:
        assert(false);
        [[fallthrough]];
    case TILESET_INVALID:
        return isRunning ? SOUND_RunDirt : SOUND_WalkDirt;
    case TILESET_GRASS:
        return isRunning ? SOUND_RunGrass : SOUND_WalkGrass;
    case TILESET_SNOW:
        return isRunning ? SOUND_RunSnow : SOUND_WalkSnow;
    case TILESET_DESERT:
        return isRunning ? SOUND_RunDesert : SOUND_WalkDesert;
    case TILESET_COOLED_LAVA:
        return isRunning ? SOUND_RunCooledLava : SOUND_WalkCooledLava;
    case TILESET_DIRT:
        return isRunning ? SOUND_RunDirt : SOUND_WalkDirt; // Water sounds were used.
    case TILESET_WATER:
        return isRunning ? SOUND_RunWater : SOUND_WalkWater; // Dirt sounds were used.
    case TILESET_BADLANDS:
        return isRunning ? SOUND_RunBadlands : SOUND_WalkBadlands;
    case TILESET_SWAMP:
        return isRunning ? SOUND_RunSwamp : SOUND_WalkSwamp;
    case TILESET_TROPICAL:
        return isRunning ? SOUND_RunGrass : SOUND_WalkGrass; // TODO(Nik-RE-dev): is that correct?
    case TILESET_CITY:
        return isRunning ? SOUND_RunGround : SOUND_WalkGround; // TODO(Nik-RE-dev): is that correct?
    case TILESET_ROAD_GRASS_COBBLE:
    case TILESET_ROAD_GRASS_DIRT:
    case TILESET_ROAD_SNOW_COBBLE:
    case TILESET_ROAD_SNOW_DIRT:
    case TILESET_ROAD_SAND_COBBLE:
    case TILESET_ROAD_SAND_DIRT:
    case TILESET_ROAD_VOLCANO_COBBLE:
    case TILESET_ROAD_VOLCANO_DIRT:
    case TILESET_ROAD_CRACKED_COBBLE:
    case TILESET_ROAD_CRACKED_DIRT:
    case TILESET_ROAD_SWAMP_COBBLE:
    case TILESET_ROAD_SWAMP_DIRT:
    case TILESET_ROAD_TROPICAL_COBBLE:
    case TILESET_ROAD_TROPICAL_DIRT:
        return isRunning ? SOUND_RunRoad : SOUND_WalkRoad;
    case TILESET_ROAD_CITY_STONE:
        return isRunning ? SOUND_RunGround : SOUND_WalkGround; // TODO(Nik-RE-dev): is that correct?
    }
}

int foodRequiredForTileset(Tileset tileset) {
    switch (tileset) {
    case TILESET_GRASS:
        return 1;
    case TILESET_SNOW:
    case TILESET_SWAMP:
        return 3;
    case TILESET_COOLED_LAVA:
    case TILESET_BADLANDS:
        return 4;
    case TILESET_DESERT:
        return 5;
    default:
        return 2;
    }
}
