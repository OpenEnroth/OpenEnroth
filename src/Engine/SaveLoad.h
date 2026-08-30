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
    std::array<NPCData, 501> npcData;
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
