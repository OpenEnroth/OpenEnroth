#include "SaveLoad.h"

#include <cassert>
#include <unordered_map>
#include <string>
#include <string_view>
#include <memory>
#include <utility>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/PartyPlacement.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Resources/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Tables/MapTable.h"
#include "Engine/Timer.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Renderer/Renderer.h"

#include "Engine/Objects/SpriteObject.h"

#include "Engine/Snapshots/CompositeSnapshots.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Snapshots/SnapshotSerialization.h"
#include "Library/Image/Pcx.h"
#include "Library/Logger/Logger.h"
#include "Library/LodFormats/LodFormats.h"

std::unordered_map<std::string, Blob> pMapDeltas;

void loadGame(std::string_view fileName) {
    std::string path = fmt::format("saves/{}", fileName);
    if (!ufs->exists(path)) {
        pAudioPlayer->playUISound(SOUND_error);
        MM_WARNING("loadGame: '{}' doesn't exist", fileName);
        return;
    }
    engine->_lastLoadedSaveFileName = fileName;

    // TODO(captainurist): remained from Party::Reset, doesn't really belong here (or in Party::Reset).
    current_character_screen_window = WINDOW_CharacterWindow_Stats;
    if (pParty->bTurnBasedModeOn) {
        pTurnEngine->End(false);
        pParty->bTurnBasedModeOn = false;
    }

    // Blob::copy below detaches from the underlying file so subsequent saves to the same path are not blocked.
    SaveGame state;
    deserialize(Blob::copy(ufs->read(path)), &state, tags::via<SaveGame_MM7>);

    // Move loaded state to global variables.
    *pParty = std::move(state.party);
    *gameTimer = std::move(state.eventTimer);
    *pActiveOverlayList = std::move(state.overlays);
    pNPCStats->pNPCData = std::move(state.npcData);
    pNPCStats->pGroups = std::move(state.npcGroups);
    pMapDeltas = std::move(state.mapDeltas);

    // Patch up the game timer.
    gameTimer->setPaused(true); // We're loading the game now => game timer is paused.
    gameTimer->setTurnBased(false);

    // We always start in realtime after loading a game.
    pParty->bTurnBasedModeOn = false;

    pParty->setActiveCharacterIndex(0);
    pParty->setActiveToFirstCanAct();

/*
    for (int i = 0; i < 4; ++i) {
        if (pParty->pCharacters[i].uQuickSpell) {
            AA1058_PartyQuickSpellSound[i].AddPartySpellSound(
                pParty->pCharacters[i].uQuickSpell, i + 1);
        }

        for (int j = 0; j < 2; ++j) {
            int uEquipIdx = pParty->pCharacters[i].pEquipment[j];
            if (uEquipIdx) {
                int pItemID = pParty->pCharacters[i]
                                  .pInventoryItemList[uEquipIdx - 1]
                                  .uItemID;
                if (pItemTable->pItems[pItemID].uEquipType == ITEM_TYPE_WAND &&
                    pItemID) {       // жезл
                    assert(false);  // looks like offset in player's inventory
                                     // and wand_lut much like case in 0042ECB5
                    stru_A750F8[i].AddPartySpellSound(
                        wand_spell_ids[pItemID], i + 9);
                }
            }
        }
    }
*/
    current_screen_type = SCREEN_GAME;

    SetUserInterface(pParty->alignment);

    if (!pGames_LOD->exists(state.header.locationName)) {
        MM_ERROR("Unable to find: {}!", state.header.locationName);
    }

    // TODO(captainurist): the start point is a placeholder, the save carries the party's position and the loaders
    //                     skip placement when loading. MapDestination has no way to say that.
    engine->_pendingTransition = MapDestination(pMapTable->GetMapInfo(state.header.locationName), MAP_START_POINT_PARTY);

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN | GAME_SETTINGS_SKIP_WORLD_UPDATE;

    // pAudioPlayer->SetMusicVolume(engine->config->music_level);
    // pAudioPlayer->SetMasterVolume(engine->config->sound_level);

    MM7Initialization();

    // TODO: disable flashing for all books until we save state to savegame file
    bFlashQuestBook = false;
    bFlashAutonotesBook = false;
    bFlashHistoryBook = false;
}

std::pair<SaveGameHeader, Blob> createSaveData(bool resetWorld, std::string_view title) {
    std::string currentMapName = pMapTable->pInfos[engine->_currentLoadedMapId].fileName;

    // Populate SaveGameState from global variables.
    SaveGame state;
    state.header.name = title;
    state.header.locationName = currentMapName;
    state.header.playingTime = pParty->GetPlayingTime();
    state.party = *pParty;
    state.eventTimer = *gameTimer;
    state.overlays = *pActiveOverlayList;
    state.npcData = pNPCStats->pNPCData;
    state.npcGroups = pNPCStats->pGroups;

    // Populate map deltas.
    if (resetWorld) {
        // New game - copy ddm & dlv files from games.lod.
        for (const std::string &name : pGames_LOD->ls())
            if (name.ends_with(".ddm") || name.ends_with(".dlv"))
                state.mapDeltas[name] = pGames_LOD->read(name);
    } else {
        // Location change - copy from current save & serialize current location delta.
        // Blob is move-only, so we need explicit copies.
        for (const auto &[key, value] : pMapDeltas)
            state.mapDeltas[key] = Blob::share(value);

        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            pIndoor->lastVisitTime = pParty->GetPlayingTime();
        } else {
            pOutdoor->lastVisitTime = pParty->GetPlayingTime();
        }
        CompactLayingItemsList();

        Blob uncompressed;
        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            serialize(*pIndoor, &uncompressed, tags::via<IndoorDelta_MM7>);
        } else {
            assert(uCurrentlyLoadedLevelType == LEVEL_OUTDOOR);
            serialize(*pOutdoor, &uncompressed, tags::via<OutdoorDelta_MM7>);
        }

        std::string deltaName = currentMapName;
        size_t pos = deltaName.find_last_of('.');
        deltaName[pos + 1] = 'd';
        state.mapDeltas[deltaName] = lod::encodeCompressed(uncompressed);
    }

    // Capture thumbnail.
    state.thumbnail = pcx::encode(render->MakeViewportScreenshot(150, 112));

    Blob blob;
    serialize(state, &blob, tags::via<SaveGame_MM7>);

    // Update pMapDeltas global with new state.
    pMapDeltas = std::move(state.mapDeltas);

    return {state.header, std::move(blob)};
}

SaveGameHeader saveGame(bool isAutoSave, bool resetWorld, std::string_view path, std::string_view title) {
    assert(isAutoSave || !title.empty());
    assert(engine->_currentLoadedMapId != MAP_ARENA || isAutoSave); // No manual saves in Arena.

    if (engine->_currentLoadedMapId == MAP_ARENA) {
        return {};
    }

    auto [header, blob] = createSaveData(resetWorld, title);

    try {
        ufs->write(path, blob);
    } catch (const std::exception &e) {
        if (isAutoSave) {
            MM_WARNING("saveGame: failed to write autosave: {}", e.what());
            return {};
        }
        throw;
    }

    return header;
}

void autoSave() {
    saveGame(true, false, fmt::format("saves/{}", autosaveFileName), localization->str(LSTR_AUTOSAVE));
}

void doSavegame(std::string fileName, std::string_view title) {
    assert(engine->_currentLoadedMapId != MAP_ARENA); // Not Arena.

    if (fileName.empty()) {
        for (int i = 0;; i++) { // New save, pick the first free numbered name.
            fileName = fmt::format("save{:03}.mm7", i);
            if (!ufs->exists(fmt::format("saves/{}", fileName)))
                break;
        }
    }

    saveGame(false, false, fmt::format("saves/{}", fileName), title);

    GUI_UpdateWindows();
    pGUIWindow_CurrentMenu = nullptr;
    current_screen_type = SCREEN_GAME;

    gameTimer->setPaused(false);
    engine->_statusBar->setEvent(LSTR_GAME_SAVED);
}

void saveNewGame() {
    engine->_currentLoadedMapId = MAP_EMERALD_ISLAND;
    engine->_lastLoadedSaveFileName.clear(); // Otherwise the save menu would pre-select a save from a previous playthrough.
    pParty->pos.x = 12552;
    pParty->pos.y = 800;
    pParty->pos.z = 193;

    pParty->uFallStartZ = 193;

    pParty->_viewPitch = 0;
    pParty->_viewYaw = 512;

    saveGame(true, true, fmt::format("saves/{}", autosaveFileName), localization->str(LSTR_AUTOSAVE));
}

void quickSaveGame() {
    assert(engine->_currentLoadedMapId != MAP_ARENA); // Not Arena.

    engine->config->gameplay.QuickSavesCount.cycleIncrement();
    saveGame(false, false, fmt::format("saves/{}", getCurrentQuickSave()), localization->str(LSTR_QUICKSAVE));
    engine->_statusBar->setEvent(LSTR_GAME_SAVED);
    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
}

void quickLoadGame() {
    std::string fileName = getCurrentQuickSave();

    if (ufs->exists(fmt::format("saves/{}", fileName))) {
        loadGame(fileName);
        uGameState = GAME_STATE_LOADING_GAME;
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    } else {
        MM_ERROR("QuickLoadGame:: No quick save could be found!");
        pAudioPlayer->playUISound(SOUND_error);
    }
}

std::string getCurrentQuickSave() {
    return fmt::format("{}{}.mm7", quickSaveFileNamePrefix, engine->config->gameplay.QuickSavesCount.value());
}
