#include "SaveLoad.h"

#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Resources/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/MapInfo.h"
#include "Engine/Time/Timer.h"

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

#include "TurnEngine/TurnEngine.h"

SavegameList *pSavegameList = new SavegameList;
std::unordered_map<std::string, Blob> pMapDeltas;

void loadGame(int uSlot) {
    if (!pSavegameList->pSavegameUsedSlots[uSlot]) {
        pAudioPlayer->playUISound(SOUND_error);
        logger->warning("LoadGame: slot {} is empty", uSlot);
        return;
    }
    pSavegameList->selectedSlot = uSlot;
    pSavegameList->lastLoadedSave = pSavegameList->pFileList[uSlot];

    // TODO(captainurist): remained from Party::Reset, doesn't really belong here (or in Party::Reset).
    current_character_screen_window = WINDOW_CharacterWindow_Stats;
    if (pParty->bTurnBasedModeOn) {
        pTurnEngine->End(false);
        pParty->bTurnBasedModeOn = false;
    }

    std::string filename = fmt::format("saves/{}", pSavegameList->pFileList[uSlot]);

    SaveGame state;
    deserialize(ufs->read(filename), &state, tags::via<SaveGame_MM7>);

    // Move loaded state to global variables.
    *pParty = std::move(state.party);
    *pEventTimer = std::move(state.eventTimer);
    *pActiveOverlayList = std::move(state.overlays);
    pNPCStats->pNPCData = std::move(state.npcData);
    pNPCStats->pGroups = std::move(state.npcGroups);
    pMapDeltas = std::move(state.mapDeltas);

    // Patch up event timer.
    pEventTimer->setPaused(true); // We're loading the game now => event timer is paused.
    pEventTimer->setTurnBased(false);

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

    for (int i = 0; i < pSavegameList->numSavegameFiles; ++i) {
        if (pSavegameList->pSavegameThumbnails[i] != nullptr) {
            pSavegameList->pSavegameThumbnails[i]->release();
            pSavegameList->pSavegameThumbnails[i] = nullptr;
        }
    }

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
    state.eventTimer = *pEventTimer;
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

    // saving - please wait

    // if (current_screen_type == SCREEN_SAVEGAME) {
    //    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, saveload_ui_loadsave);
    //    render->DrawTextureNew(18 / 640.0f, 141 / 480.0f, saveload_ui_loadsave);
    //    int text_pos = pFontSmallnum->AlignText_Center(186, localization->str(190));
    //    pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, text_pos + 25, 219, 0, localization->str(190), 0, 0, 0);  // Сохранение
    //    text_pos = pFontSmallnum->AlignText_Center(186, pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].pName);
    //    pGUIWindow_CurrentMenu->DrawTextInRect(pFontSmallnum, text_pos + 25, 259, 0, pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].pName, 185, 0);
    //    text_pos = pFontSmallnum->AlignText_Center(186, localization->str(LSTR_PLEASE_WAIT));
    //    pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, text_pos + 25, 299, 0, localization->str(LSTR_PLEASE_WAIT), 0, 0, 0);  // Пожалуйста, подождите
    //    render->Present();
    //}

    auto [header, blob] = createSaveData(resetWorld, title);

    ufs->write(path, blob);

    return header;
}

void autoSave() {
    saveGame(true, false, "saves/autosave.mm7");
}

void doSavegame(int uSlot) {
    assert(engine->_currentLoadedMapId != MAP_ARENA); // Not Arena.

    pSavegameList->pSavegameHeader[uSlot] = saveGame(false, false, fmt::format("saves/save{:03}.mm7", uSlot),
                                                     pSavegameList->pSavegameHeader[uSlot].name);

    pSavegameList->selectedSlot = uSlot;

    GUI_UpdateWindows();
    pGUIWindow_CurrentMenu->Release();
    current_screen_type = SCREEN_GAME;

    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        if (pSavegameList->pSavegameThumbnails[i] != nullptr) {
            pSavegameList->pSavegameThumbnails[i]->release();
            pSavegameList->pSavegameThumbnails[i] = nullptr;
        }
    }

    pEventTimer->setPaused(false);
    engine->_statusBar->setEvent(LSTR_GAME_SAVED);
}

void SavegameList::Initialize() {
    pSavegameList->Reset();

    if (ufs->exists("saves")) {
        for (const auto &entry : ufs->ls("saves")) {
            if (entry.type == FILE_REGULAR && entry.name.ends_with(".mm7")) {
                pSavegameList->pFileList[pSavegameList->numSavegameFiles++] = entry.name;
                if (pSavegameList->numSavegameFiles == MAX_SAVE_SLOTS) {
                    break;
                }
            }
        }
    }

    if (pSavegameList->numSavegameFiles) {
        std::sort(pSavegameList->pFileList.begin(), pSavegameList->pFileList.begin() + pSavegameList->numSavegameFiles);
    }
}

SavegameList::SavegameList() { Reset(); }

void SavegameList::Reset() {
    pSavegameUsedSlots.fill(false);
    pSavegameThumbnails.fill(nullptr);

    for (int j = 0; j < MAX_SAVE_SLOTS; j++) {
        this->pFileList[j].clear();
    }

    numSavegameFiles = 0;
    // Reset position in case that last loaded save will not be found
    selectedSlot = 0;
    saveListPosition = 0;
}

void saveNewGame() {
    engine->_currentLoadedMapId = MAP_EMERALD_ISLAND;
    pParty->lastPos.x = 12552;
    pParty->lastPos.y = 800;
    pParty->lastPos.z = 193;

    pParty->pos.x = 12552;
    pParty->pos.y = 800;
    pParty->pos.z = 193;

    pParty->uFallStartZ = 193;

    pParty->_viewPrevPitch = 0;
    pParty->_viewPrevYaw = 512;

    pParty->_viewPitch = 0;
    pParty->_viewYaw = 512;

    saveGame(true, true, "saves/autosave.mm7");
}

void quickSaveGame() {
    assert(engine->_currentLoadedMapId != MAP_ARENA); // Not Arena.
    pSavegameList->Initialize();

    engine->config->gameplay.QuickSavesCount.cycleIncrement();
    std::string quickSaveName = getCurrentQuickSave();

    int uSlot = -1;
    // find QuickSave slot
    for (int i = 0; i < MAX_SAVE_SLOTS; ++i) {
        if (pSavegameList->pFileList[i] == quickSaveName) {
            uSlot = i;
            break;
        }
    }

    // not found - find free slot
    if (uSlot == -1) {
        for (int i = 0; i < MAX_SAVE_SLOTS; ++i) {
            if (pSavegameList->pFileList[i] == "") {
                uSlot = i;
                break;
            }
        }
    }

    // if no free slot error
    if (uSlot == -1) {
        logger->error("QuickSaveGame:: No free save game slots!");
        engine->config->gameplay.QuickSavesCount.cycleDecrement();
        pAudioPlayer->playUISound(SOUND_error);
        return;
    }

    pSavegameList->pSavegameHeader[uSlot].name = "Quicksave";
    pSavegameList->pSavegameHeader[uSlot] = saveGame(false, false, fmt::format("saves/{}", quickSaveName),
                                                     pSavegameList->pSavegameHeader[uSlot].name);
    engine->_statusBar->setEvent(LSTR_GAME_SAVED);
    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
}

int getQuickSaveSlot() {
    pSavegameList->Initialize();
    std::string quickSaveName = getCurrentQuickSave();

    int uSlot = -1;
    // find QuickSave slot
    for (int i = 0; i < MAX_SAVE_SLOTS; ++i) {
        if (pSavegameList->pFileList[i] == quickSaveName) {
            uSlot = i;
            // make sure this slot is activated for load
            pSavegameList->pSavegameUsedSlots[i] = true;
            break;
        }
    }

    return uSlot;
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
