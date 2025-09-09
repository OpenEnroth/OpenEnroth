#include "SaveLoad.h"

#include <cassert>
#include <algorithm>
#include <string>
#include <memory>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/EngineFileSystem.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/MapInfo.h"
#include "Engine/Time/Timer.h"

#include "Engine/Graphics/ImageLoader.h"
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
#include "Library/Lod/LodWriter.h"
#include "TurnEngine/TurnEngine.h"

SavegameList *pSavegameList = new SavegameList;

static LodInfo makeSaveLodInfo() {
    LodInfo result;
    result.version = LOD_VERSION_MM7;
    result.rootName = "chapter";
    result.description = "newmaps for MMVII";
    return result;
}

void LoadGame(int uSlot) {
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

    // Note that we're using Blob::copy so that the memory mapping for the savefile is not held by the LOD reader.
    pSave_LOD->close();
    pSave_LOD->open(Blob::copy(ufs->read(filename)), LOD_ALLOW_DUPLICATES);

    SaveGameHeader header;
    deserialize(*pSave_LOD, &header, tags::via<SaveGame_MM7>);

    // Patch up event timer, which was updated by the deserialize call above.
    pEventTimer->setPaused(true); // We're loading the game now => event timer is paused.
    pEventTimer->setTurnBased(false);

    // TODO(captainurist): incapsulate this too
    pParty->bTurnBasedModeOn = false; // We always start in realtime after loading a game.
    for (size_t i = 0; i < 4; i++) {
        Character *player = &pParty->pCharacters[i];
        for (size_t j = 0; j < 5; j++) {
            if (!player->vBeacons[j]) {
                continue;
            }
            LloydBeacon &beacon = *player->vBeacons[j];
            std::string str = fmt::format("lloyd{}{}.pcx", i + 1, j + 1);
            //beacon.image = Image::Create(new PCX_LOD_Raw_Loader(pNew_LOD, str));
            beacon.image = GraphicsImage::Create(std::make_unique<PCX_LOD_Raw_Loader>(pSave_LOD.get(), str));
            beacon.image->rgba(); // Force load!
        }
    }

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

    if (!pGames_LOD->exists(header.locationName)) {
        logger->error("Unable to find: {}!", header.locationName);
    }

    engine->_transitionMapId = pMapStats->GetMapInfo(header.locationName);

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN | GAME_SETTINGS_SKIP_WORLD_UPDATE;

    for (int i = 0; i < pSavegameList->numSavegameFiles; ++i) {
        if (pSavegameList->pSavegameThumbnails[i] != nullptr) {
            pSavegameList->pSavegameThumbnails[i]->Release();
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

std::pair<SaveGameHeader, Blob> CreateSaveData(bool resetWorld, std::string_view title) {
    std::pair<SaveGameHeader, Blob> result;
    auto &[resultHeader, resultBlob] = result;
    BlobOutputStream lodStream(&resultBlob);
    LodWriter lodWriter(&lodStream, makeSaveLodInfo());

    std::string currentMapName = pMapStats->pInfos[engine->_currentLoadedMapId].fileName;

    if (resetWorld) {
        // New game - copy ddm & dlv files.
        for (const std::string &name : pGames_LOD->ls())
            if (name.ends_with(".ddm") || name.ends_with(".dlv"))
                lodWriter.write(name, pGames_LOD->read(name));
    } else {
        // Location change - copy map data from the old save & serialize current location delta.
        for (const std::string &name : pSave_LOD->ls())
            lodWriter.write(name, pSave_LOD->read(name));

        currentLocationTime().last_visit = pParty->GetPlayingTime();
        CompactLayingItemsList();

        Blob uncompressed;
        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            serialize(*pIndoor, &uncompressed, tags::via<IndoorDelta_MM7>);
        } else {
            assert(uCurrentlyLoadedLevelType == LEVEL_OUTDOOR);
            serialize(*pOutdoor, &uncompressed, tags::via<OutdoorDelta_MM7>);
        }

        std::string file_name = currentMapName;
        size_t pos = file_name.find_last_of(".");
        file_name[pos + 1] = 'd';
        lodWriter.write(file_name, lod::encodeCompressed(uncompressed));
    }

    lodWriter.write("image.pcx", pcx::encode(render->MakeViewportScreenshot(150, 112)));

    resultHeader.name = title;
    resultHeader.locationName = currentMapName;
    resultHeader.playingTime = pParty->GetPlayingTime();
    serialize(resultHeader, &lodWriter, tags::via<SaveGame_MM7>);

    // TODO(captainurist): incapsulate this too
    for (size_t i = 0; i < 4; ++i) {  // 4 - players
        Character *player = &pParty->pCharacters[i];
        for (size_t j = 0; j < 5; ++j) {  // 5 - images
            if (!player->vBeacons[j]) {
                continue;
            }
            LloydBeacon &beacon = *player->vBeacons[j];
            GraphicsImage *image = beacon.image;
            if (beacon.uBeaconTime.isValid() && image != nullptr) {
                assert(image->rgba());
                std::string str = fmt::format("lloyd{}{}.pcx", i + 1, j + 1);
                lodWriter.write(str, pcx::encode(image->rgba()));
            }
        }
    }

    // Apparently vanilla had two bugs canceling each other out:
    // 1. Broken binary search implementation when looking up LOD entries.
    // 2. Writing additional duplicate entry at the end of a saves LOD file.
    // Our code doesn't support duplicate entries, so we just add a dummy entry
    lodWriter.write("z.bin", Blob::fromString("dummy"));
    lodWriter.close();
    lodStream.close();
    return result;
}

SaveGameHeader SaveGame(bool isAutoSave, bool resetWorld, std::string_view path, std::string_view title) {
    assert(isAutoSave || !title.empty());
    assert(engine->_currentLoadedMapId != MAP_ARENA || isAutoSave); // No manual saves in Arena.

    if (engine->_currentLoadedMapId == MAP_ARENA) {
        return {};
    }

    // saving - please wait

    // if (current_screen_type == SCREEN_SAVEGAME) {
    //    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, saveload_ui_loadsave);
    //    render->DrawTextureNew(18 / 640.0f, 141 / 480.0f, saveload_ui_loadsave);
    //    int text_pos = pFontSmallnum->AlignText_Center(186, localization->GetString(190));
    //    pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, text_pos + 25, 219, 0, localization->GetString(190), 0, 0, 0);  // Сохранение
    //    text_pos = pFontSmallnum->AlignText_Center(186, pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].pName);
    //    pGUIWindow_CurrentMenu->DrawTextInRect(pFontSmallnum, text_pos + 25, 259, 0, pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].pName, 185, 0);
    //    text_pos = pFontSmallnum->AlignText_Center(186, localization->GetString(LSTR_PLEASE_WAIT));
    //    pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, text_pos + 25, 299, 0, localization->GetString(LSTR_PLEASE_WAIT), 0, 0, 0);  // Пожалуйста, подождите
    //    render->Present();
    //}

    auto [header, blob] = CreateSaveData(resetWorld, title);

    ufs->write(path, blob);

    pSave_LOD->open(std::move(blob), LOD_ALLOW_DUPLICATES);

    return std::move(header);
}

void AutoSave() {
    SaveGame(true, false, "saves/autosave.mm7");
}

void DoSavegame(int uSlot) {
    assert(engine->_currentLoadedMapId != MAP_ARENA); // Not Arena.

    pSavegameList->pSavegameHeader[uSlot] = SaveGame(false, false, fmt::format("saves/save{:03}.mm7", uSlot),
                                                     pSavegameList->pSavegameHeader[uSlot].name);

    pSavegameList->selectedSlot = uSlot;

    GUI_UpdateWindows();
    pGUIWindow_CurrentMenu->Release();
    current_screen_type = SCREEN_GAME;

    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        if (pSavegameList->pSavegameThumbnails[i] != nullptr) {
            pSavegameList->pSavegameThumbnails[i]->Release();
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

void SaveNewGame() {
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

    SaveGame(true, true, "saves/autosave.mm7");
}

void QuickSaveGame() {
    assert(engine->_currentLoadedMapId != MAP_ARENA); // Not Arena.
    pSavegameList->Initialize();

    engine->config->gameplay.QuickSavesCount.cycleIncrement();
    std::string quickSaveName = GetCurrentQuickSave();

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
    pSavegameList->pSavegameHeader[uSlot] = SaveGame(false, false, fmt::format("saves/{}", quickSaveName),
                                                     pSavegameList->pSavegameHeader[uSlot].name);
    engine->_statusBar->setEvent(LSTR_GAME_SAVED);
    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
}

int GetQuickSaveSlot() {
    pSavegameList->Initialize();
    std::string quickSaveName = GetCurrentQuickSave();

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

void QuickLoadGame() {
    int uSlot = GetQuickSaveSlot();

    if (uSlot != -1) {
        LoadGame(uSlot);
        uGameState = GAME_STATE_LOADING_GAME;
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    } else {
        logger->error("QuickLoadGame:: No quick save could be found!");
        pAudioPlayer->playUISound(SOUND_error);
    }
}

std::string GetCurrentQuickSave() {
    return fmt::format("{}{}.mm7", engine->config->gameplay.QuickSaveName.value(), engine->config->gameplay.QuickSavesCount.value());
}
