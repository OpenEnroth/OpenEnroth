#include "TileEnumFunctions.h"

#include <cassert>

SoundId walkSoundForTileSet(TileSet tileSet, bool isRunning) {
    switch (tileSet) {
    default:
        assert(false);
        [[fallthrough]];
    case TILE_SET_INVALID:
        return isRunning ? SOUND_RunDirt : SOUND_WalkDirt;
    case TILE_SET_GRASS:
        return isRunning ? SOUND_RunGrass : SOUND_WalkGrass;
    case TILE_SET_SNOW:
        return isRunning ? SOUND_RunSnow : SOUND_WalkSnow;
    case TILE_SET_DESERT:
        return isRunning ? SOUND_RunDesert : SOUND_WalkDesert;
    case TILE_SET_COOLED_LAVA:
        return isRunning ? SOUND_RunCooledLava : SOUND_WalkCooledLava;
    case TILE_SET_DIRT:
        return isRunning ? SOUND_RunDirt : SOUND_WalkDirt; // Water sounds were used.
    case TILE_SET_WATER:
        return isRunning ? SOUND_RunWater : SOUND_WalkWater; // Dirt sounds were used.
    case TILE_SET_BADLANDS:
        return isRunning ? SOUND_RunBadlands : SOUND_WalkBadlands;
    case TILE_SET_SWAMP:
        return isRunning ? SOUND_RunSwamp : SOUND_WalkSwamp;
    case TILE_SET_TROPICAL:
        return isRunning ? SOUND_RunGrass : SOUND_WalkGrass; // TODO(Nik-RE-dev): is that correct?
    case TILE_SET_CITY:
        return isRunning ? SOUND_RunGround : SOUND_WalkGround; // TODO(Nik-RE-dev): is that correct?
    case TILE_SET_ROAD_GRASS_COBBLE:
    case TILE_SET_ROAD_GRASS_DIRT:
    case TILE_SET_ROAD_SNOW_COBBLE:
    case TILE_SET_ROAD_SNOW_DIRT:
    case TILE_SET_ROAD_SAND_COBBLE:
    case TILE_SET_ROAD_SAND_DIRT:
    case TILE_SET_ROAD_VOLCANO_COBBLE:
    case TILE_SET_ROAD_VOLCANO_DIRT:
    case TILE_SET_ROAD_CRACKED_COBBLE:
    case TILE_SET_ROAD_CRACKED_DIRT:
    case TILE_SET_ROAD_SWAMP_COBBLE:
    case TILE_SET_ROAD_SWAMP_DIRT:
    case TILE_SET_ROAD_TROPICAL_COBBLE:
    case TILE_SET_ROAD_TROPICAL_DIRT:
        return isRunning ? SOUND_RunRoad : SOUND_WalkRoad;
    case TILE_SET_ROAD_CITY_STONE:
        return isRunning ? SOUND_RunGround : SOUND_WalkGround; // TODO(Nik-RE-dev): is that correct?
    }
}
