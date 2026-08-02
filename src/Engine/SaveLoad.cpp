#include "SaveLoad.h"

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Resources/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/MapInfo.h"
#include "Engine/Time/Timer.h"
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

#include "Utility/String/Ascii.h"
#include "Utility/Exception.h"

SavegameList *pSavegameList = new SavegameList;
std::unordered_map<std::string, Blob> pMapDeltas;

void loadGame(int uSlot) {
    if (!pSavegameList->isSlotUsed(uSlot)) {
        pAudioPlayer->playUISound(SOUND_error);
        logger->warning("LoadGame: slot {} is empty", uSlot);
        return;
    }
    engine->_lastLoadedSaveFileName = pSavegameList->slots()[uSlot].fileName;

    // TODO(captainurist): remained from Party::Reset, doesn't really belong here (or in Party::Reset).
    current_character_screen_window = WINDOW_CharacterWindow_Stats;
    if (pParty->bTurnBasedModeOn) {
        pTurnEngine->End(false);
        pParty->bTurnBasedModeOn = false;
    }

    std::string filename = fmt::format("saves/{}", pSavegameList->slots()[uSlot].fileName);

    // Blob::copy below detaches from the underlying file so subsequent saves to the same path are not blocked.
    SaveGame state;
    deserialize(Blob::copy(ufs->read(filename)), &state, tags::via<SaveGame_MM7>);

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
        logger->error("Unable to find: {}!", state.header.locationName);
    }

    engine->_transitionMapId = pMapStats->GetMapInfo(state.header.locationName);

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
    std::string currentMapName = pMapStats->pInfos[engine->_currentLoadedMapId].fileName;

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

        currentLocationTime().lastVisitTime = pParty->GetPlayingTime();
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
            logger->warning("saveGame: failed to write autosave: {}", e.what());
            return {};
        }
        throw;
    }

    return header;
}

void autoSave() {
    saveGame(true, false, fmt::format("saves/{}", autosaveFileName));
}

void doSavegame(int uSlot) {
    assert(engine->_currentLoadedMapId != MAP_ARENA); // Not Arena.

    std::string fileName = pSavegameList->slots()[uSlot].fileName; // Overwrite an existing save.
    if (fileName.empty()) {
        for (int i = 0;; i++) { // New save, pick the first free numbered name.
            fileName = fmt::format("save{:03}.mm7", i);
            if (!ufs->exists(fmt::format("saves/{}", fileName)))
                break;
        }
    }

    saveGame(false, false, fmt::format("saves/{}", fileName), pSavegameList->slots()[uSlot].header.name);

    GUI_UpdateWindows();
    pGUIWindow_CurrentMenu = nullptr;
    current_screen_type = SCREEN_GAME;

    gameTimer->setPaused(false);
    engine->_statusBar->setEvent(LSTR_GAME_SAVED);
}

SavegameList::SavegameList() {
    _slots.assign(1, {}); // Just the new save slot.
}

void SavegameList::refresh() {
    releaseThumbnails();

    std::vector<std::string> saves;
    if (ufs->exists("saves"))
        for (const auto &entry : ufs->ls("saves"))
            if (entry.type == FILE_REGULAR && entry.name.ends_with(".mm7"))
                saves.push_back(entry.name);

    _slots.assign(saves.size() + 1, {}); // +1 for the new save slot.
    for (size_t i = 0; i < saves.size(); i++) {
        _slots[i].fileName = saves[i];
        _slots[i].isUsed = true; // The file exists, so the slot is loadable even before loadHeaders() is called.
    }
    _loadMenuSlots.clear();
    _saveMenuSlots.clear();
}

void SavegameList::loadHeaders() {
    refresh();

    for (int i = 0; i < saveFileCount(); ++i) {
        SavegameSlot &slot = _slots[i];
        std::string path = fmt::format("saves/{}", slot.fileName);
        if (!ufs->exists(path)) {
            slot.isUsed = false;
            slot.header.name = localization->str(LSTR_EMPTY_SAVE);
            continue;
        }

        SaveGameLite save;
        deserialize(ufs->read(path), &save, tags::via<SaveGameLite_MM7>);
        slot.header = save.header;

        if (ascii::noCaseEquals(slot.fileName, autosaveFileName))
            slot.header.name = localization->str(LSTR_AUTOSAVE);

        if (slot.header.name.empty()) // blank so add something - suspect quicksaves
            slot.header.name = slot.fileName.substr(0, slot.fileName.size() - 4);

        try {
            slot.thumbnail = GraphicsImage::Create(pcx::decode(save.thumbnail)); // TODO(captainurist): lazy-load.

            if (slot.thumbnail->width() == 0) {
                slot.thumbnail->release();
                slot.thumbnail = nullptr;
            }
        } catch (const Exception &e) {
            logger->debug("pSavegameList thumbnail exception: {}", e.what()); // swallow it - bad pcx thumbnail is fine
            slot.thumbnail = nullptr;
        }
    }

    // Sort saves by display name, with file name as a tie-breaker.
    auto slotLess = [](const SavegameSlot &l, const SavegameSlot &r) {
        if (int result = ascii::noCaseCompare(l.header.name, r.header.name))
            return result < 0;
        return l.fileName < r.fileName;
    };
    std::ranges::sort(_slots.begin(), _slots.begin() + saveFileCount(), slotLess);

    // All saves are shown in the load menu. Autosave & quicksaves are loadable, but shouldn't be manually saved
    // over, so they are not shown in the save menu, which shows the new save slot first instead.
    _loadMenuSlots.clear();
    _saveMenuSlots.clear();
    _saveMenuSlots.push_back(saveFileCount()); // New save slot goes first.
    for (int i = 0; i < saveFileCount(); i++) {
        _loadMenuSlots.push_back(i);
        const std::string &fileName = _slots[i].fileName;
        if (!ascii::noCaseEquals(fileName, autosaveFileName) &&
            !ascii::noCaseStartsWith(fileName, engine->config->gameplay.QuickSaveName.value()))
            _saveMenuSlots.push_back(i);
    }
}

void SavegameList::releaseThumbnails() {
    for (SavegameSlot &slot : _slots)
        if (slot.thumbnail != nullptr) {
            slot.thumbnail->release();
            slot.thumbnail = nullptr;
        }
}

void saveNewGame() {
    engine->_currentLoadedMapId = MAP_EMERALD_ISLAND;
    pParty->pos.x = 12552;
    pParty->pos.y = 800;
    pParty->pos.z = 193;

    pParty->uFallStartZ = 193;

    pParty->_viewPitch = 0;
    pParty->_viewYaw = 512;

    saveGame(true, true, fmt::format("saves/{}", autosaveFileName));
}

void quickSaveGame() {
    assert(engine->_currentLoadedMapId != MAP_ARENA); // Not Arena.

    engine->config->gameplay.QuickSavesCount.cycleIncrement();
    saveGame(false, false, fmt::format("saves/{}", getCurrentQuickSave()), "Quicksave");
    engine->_statusBar->setEvent(LSTR_GAME_SAVED);
    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
}

int getQuickSaveSlot() {
    pSavegameList->refresh();

    const std::vector<SavegameSlot> &slots = pSavegameList->slots();
    auto pos = std::ranges::find(slots, getCurrentQuickSave(), &SavegameSlot::fileName);
    return pos == slots.end() ? -1 : pos - slots.begin();
}

void quickLoadGame() {
    int uSlot = getQuickSaveSlot();

    if (uSlot != -1) {
        loadGame(uSlot);
        uGameState = GAME_STATE_LOADING_GAME;
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    } else {
        logger->error("QuickLoadGame:: No quick save could be found!");
        pAudioPlayer->playUISound(SOUND_error);
    }
}

std::string getCurrentQuickSave() {
    return fmt::format("{}{}.mm7", engine->config->gameplay.QuickSaveName.value(), engine->config->gameplay.QuickSavesCount.value());
}
