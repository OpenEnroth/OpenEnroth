#pragma once

#include <array>
#include <unordered_map>
#include <string>
#include <string_view>
#include <utility>

#include "Engine/Graphics/Overlays.h"
#include "Engine/Party.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Timer.h"

#include "Core/Time/Time.h"

#include "Utility/Memory/Blob.h"

// Autosave file name. Note that it's not localized, unlike the autosave title displayed in-game.
constexpr std::string_view autosaveFileName = "autosave.mm7";

// Quicksave file name prefix. Full quicksave file names also contain a number, e.g. "quicksave1.mm7".
constexpr std::string_view quickSaveFileNamePrefix = "quicksave";

struct SaveGameHeader {
    std::string name; // Save name, as displayed in the save list in-game.
    std::string locationName; // Name of the map, e.g. "out01.odm".
    Time playingTime; // Game time of the save.
};

struct SaveGame {
    SaveGameHeader header;
    Party party;
    Timer eventTimer;
    ActiveOverlayList overlays;
    // 551 entries, the size of an MM8 save's `npcdata.bin`. Indices 528-550 are filler in `npcdata.txt`,
    // so only the first `NPCTable::pNPCData.size()` (528) entries are copied into/out of live gameplay
    // state, see `SaveLoad.cpp`.
    std::array<NPCData, 551> npcData;
    std::array<uint16_t, 51> npcGroups;
    std::unordered_map<std::string, Blob> mapDeltas;
    Blob thumbnail;
};

struct SaveGameLite {
    SaveGameHeader header;
    Blob thumbnail;
};

/** Runtime storage for map deltas from the currently loaded save. */
extern std::unordered_map<std::string, Blob> pMapDeltas;

// The current map's actor list, stored by `createSaveData` under one fixed key and restored by
// `EngineController::loadGame` after the normal load. Absent in older saves.
// `mapDeltas` round-trips through a LOD archive whose entry names are a fixed `std::array<char, 16>`
// (`LodFileEntry_MM7`/`_MM8`, anything longer is silently truncated) and whose read side only keeps
// entries ending in `.ddm`/`.dlv`, so the key is short and `.ddm`-suffixed even though it isn't a map delta.
constexpr std::string_view kMm8LiveActorsKey = "mm8actors.ddm";

// The current map's sprite objects, kept the way `kMm8LiveActorsKey` keeps its actors: a spell in flight is a sprite object, and
// a save made while it flies holds it.
constexpr std::string_view kMm8LiveSpritesKey = "mm8sprite.ddm";

void loadGame(std::string_view fileName);
std::pair<SaveGameHeader, Blob> createSaveData(bool resetWorld, std::string_view title);
SaveGameHeader saveGame(bool isAutoSave, bool resetWorld, std::string_view path, std::string_view title = {});
void autoSave();

/**
 * Saves the game.
 *
 * @param fileName                  Name of the file to save into, e.g. "save000.mm7". Pass an empty string to save
 *                                  into the first free `saveNNN.mm7`.
 * @param title                     Save title to display in the save list in-game.
 */
void doSavegame(std::string fileName, std::string_view title);
void saveNewGame();

void quickSaveGame();
void quickLoadGame();
std::string getCurrentQuickSave();
