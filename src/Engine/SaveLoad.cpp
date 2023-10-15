#include "SaveLoad.h"

#include <cassert>
#include <filesystem>
#include <algorithm>
#include <string>
#include <memory>

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/IRender.h"

#include "Engine/Objects/SpriteObject.h"

#include "Engine/Snapshots/CompositeSnapshots.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Snapshots/SnapshotSerialization.h"
#include "Library/Image/PCX.h"
#include "Library/Compression/Compression.h"
#include "Library/Logger/Logger.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/Lod/LodWriter.h"

#include "Utility/DataPath.h"

struct SavegameList *pSavegameList = new SavegameList;

static LodInfo makeSaveLodInfo() {
    LodInfo result;
    result.version = LOD_VERSION_MM7;
    result.rootName = "chapter";
    result.description = "newmaps for MMVII";
    return result;
}

void LoadGame(unsigned int uSlot) {
    if (!pSavegameList->pSavegameUsedSlots[uSlot]) {
        pAudioPlayer->playUISound(SOUND_error);
        logger->warning("LoadGame: slot {} is empty", uSlot);
        return;
    }
    pSavegameList->selectedSlot = uSlot;
    pSavegameList->lastLoadedSave = pSavegameList->pFileList[uSlot];

    pParty->Reset();

    // uCurrentlyLoadedLevelType = LEVEL_NULL;

    std::string filename = makeDataPath("saves", pSavegameList->pFileList[uSlot]);
    std::string to_file_path = makeDataPath("data", "new.lod");

    pSave_LOD->close();

    std::error_code ec;
    if (!std::filesystem::copy_file(filename, to_file_path, std::filesystem::copy_options::overwrite_existing, ec))
        Error("Failed to copy: %s", filename.c_str());

    pSave_LOD->open(to_file_path, LOD_ALLOW_DUPLICATES);

    SaveGameHeader header;
    deserialize(*pSave_LOD, &header, tags::via<SaveGame_MM7>);

    // TODO(captainurist): incapsulate this too
    pParty->bTurnBasedModeOn = false; // We always start in realtime after loading a game.
    for (size_t i = 0; i < 4; i++) {
        Character *player = &pParty->pCharacters[i];
        for (size_t j = 0; j < 5; j++) {
            if (j >= player->vBeacons.size()) {
                continue;
            }
            LloydBeacon &beacon = player->vBeacons[j];
            std::string str = fmt::format("lloyd{}{}.pcx", i + 1, j + 1);
            //beacon.image = Image::Create(new PCX_LOD_Raw_Loader(pNew_LOD, str));
            beacon.image = GraphicsImage::Create(std::make_unique<PCX_LOD_Raw_Loader>(pSave_LOD.get(), str));
            beacon.image->rgba(); // Force load!
        }
    }

    pParty->setActiveCharacterIndex(0);
    pParty->setActiveToFirstCanAct();

/*
    for (uint i = 0; i < 4; ++i) {
        if (pParty->pCharacters[i].uQuickSpell) {
            AA1058_PartyQuickSpellSound[i].AddPartySpellSound(
                pParty->pCharacters[i].uQuickSpell, i + 1);
        }

        for (uint j = 0; j < 2; ++j) {
            uint uEquipIdx = pParty->pCharacters[i].pEquipment.pIndices[j];
            if (uEquipIdx) {
                int pItemID = pParty->pCharacters[i]
                                  .pInventoryItemList[uEquipIdx - 1]
                                  .uItemID;
                if (pItemTable->pItems[pItemID].uEquipType == ITEM_TYPE_WAND &&
                    pItemID) {       // жезл
                    __debugbreak();  // looks like offset in player's inventory
                                     // and wand_lut much like case in 0042ECB5
                    stru_A750F8[i].AddPartySpellSound(
                        wand_spell_ids[pItemID], i + 9);
                }
            }
        }
    }
*/
    current_screen_type = SCREEN_GAME;

    SetUserInterface(pParty->alignment, true);

    pEventTimer->Resume();
    pEventTimer->StopGameTime();

    if (!pGames_LOD->exists(header.locationName)) {
        Error("Unable to find: %s!", header.locationName.c_str());
    }

    pCurrentMapName = header.locationName;

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN | GAME_SETTINGS_SKIP_WORLD_UPDATE;

    for (int i = 0; i < pSavegameList->numSavegameFiles; ++i) {
        if (pSavegameList->pSavegameThumbnails[i] != nullptr) {
            pSavegameList->pSavegameThumbnails[i]->Release();
            pSavegameList->pSavegameThumbnails[i] = nullptr;
        }
    }

    // pAudioPlayer->SetMusicVolume(engine->config->music_level);
    // pAudioPlayer->SetMasterVolume(engine->config->sound_level);

    // TODO: what is this magic? old party position correction with current angle settings?
    // TODO(captainurist): might be a source of non-determinism, just drop this.
    if (engine->config->settings.TurnSpeed.value() > 0) {
        pParty->_viewYaw = engine->config->settings.TurnSpeed.value() * pParty->_viewYaw / engine->config->settings.TurnSpeed.value();
    }
    MM7Initialization();

    // TODO: disable flashing for all books until we save state to savegame file
    bFlashQuestBook = false;
    bFlashAutonotesBook = false;
    bFlashHistoryBook = false;
}

SaveGameHeader SaveGame(bool IsAutoSAve, bool NotSaveWorld, const std::string &title) {
    assert(IsAutoSAve || !title.empty());

    s_SavedMapName = pCurrentMapName;
    if (pCurrentMapName == "d05.blv") { // arena
        return {};
    }

    int pPositionX = pParty->pos.x;
    int pPositionY = pParty->pos.y;
    int pPositionZ = pParty->pos.z;
    int partyViewYaw = pParty->_viewYaw;
    int partyViewPitch = pParty->_viewPitch;
    pParty->pos.x = pParty->lastPos.x;
    pParty->pos.z = pParty->lastPos.z;
    pParty->pos.y = pParty->lastPos.y;

    pParty->uFallStartZ = pParty->lastPos.z;

    pParty->_viewYaw = pParty->_viewPrevYaw;
    pParty->_viewPitch = pParty->_viewPrevPitch;

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

    pSave_LOD->close();
    LodWriter lodWriter(makeDataPath("data", "new.lod"), makeSaveLodInfo());

    LodReader lodReader(makeDataPath("data", "new.lod"), LOD_ALLOW_DUPLICATES);
    for (const std::string &name : lodReader.ls())
        lodWriter.write(name, lodReader.read(name));
    lodReader.close();

    lodWriter.write("image.pcx", render->PackScreenshot(150, 112));

    SaveGameHeader save_header;
    save_header.name = title;
    save_header.locationName = pCurrentMapName;
    save_header.playingTime = pParty->GetPlayingTime();

    serialize(save_header, &lodWriter, tags::via<SaveGame_MM7>);

    // TODO(captainurist): incapsulate this too
    for (size_t i = 0; i < 4; ++i) {  // 4 - players
        Character *player = &pParty->pCharacters[i];
        for (size_t j = 0; j < 5; ++j) {  // 5 - images
            if (j >= player->vBeacons.size()) {
                continue;
            }
            LloydBeacon *beacon = &player->vBeacons[j];
            GraphicsImage *image = beacon->image;
            if ((beacon->uBeaconTime.Valid()) && (image != nullptr)) {
                assert(image->rgba());
                std::string str = fmt::format("lloyd{}{}.pcx", i + 1, j + 1);
                lodWriter.write(str, pcx::encode(image->rgba()));
            }
        }
    }

    Blob uncompressed;
    if (!NotSaveWorld) {  // autosave for change location
        currentLocationTime().last_visit = pParty->GetPlayingTime();
        CompactLayingItemsList();

        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            serialize(*pIndoor, &uncompressed, tags::via<IndoorDelta_MM7>);
        } else {
            assert(uCurrentlyLoadedLevelType == LEVEL_OUTDOOR);
            serialize(*pOutdoor, &uncompressed, tags::via<OutdoorDelta_MM7>);
        }

        std::string file_name = pCurrentMapName;
        size_t pos = file_name.find_last_of(".");
        file_name[pos + 1] = 'd';
        lodWriter.write(file_name, lod::encodeCompressed(uncompressed));
    }

    // Apparently vanilla had two bugs canceling each other out:
    // 1. Broken binary search implementation when looking up LOD entries.
    // 2. Writing additional duplicate entry at the end of a saves LOD file.
    // Our code doesn't support duplicate entries, so we just add a dummy entry
    lodWriter.write("z.bin", Blob::fromString("dummy"));

    lodWriter.close();

    if (IsAutoSAve) {
        std::string src = makeDataPath("data", "new.lod");
        std::string dst = makeDataPath("saves", "autosave.mm7");
        std::error_code ec;
        if (!std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec))
            logger->warning("Copying of autosave.mm7 failed");
    }
    pParty->pos.x = pPositionX;
    pParty->pos.y = pPositionY;
    pParty->pos.z = pPositionZ;
    pParty->uFallStartZ = pPositionZ;
    pParty->_viewYaw = partyViewYaw;
    pParty->_viewPitch = partyViewPitch;

    pSave_LOD->open(makeDataPath("data", "new.lod"), LOD_ALLOW_DUPLICATES);

    return save_header;
}

void DoSavegame(unsigned int uSlot) {
    if (pCurrentMapName != "d05.blv") {  // Not Arena
        pSavegameList->pSavegameHeader[uSlot] = SaveGame(0, 0, pSavegameList->pSavegameHeader[uSlot].name);

        std::string src = makeDataPath("data", "new.lod");
        std::string dst = makeDataPath("saves", fmt::format("save{:03}.mm7", uSlot));
        std::error_code ec;
        if (!std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec))
            Error("Failed to copy: %s", src.c_str());
    }
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

    if (pCurrentMapName == "d05.blv")
        engine->_statusBar->setEvent(LSTR_NO_SAVING_IN_ARENA);

    // TODO(captainurist): This ^v doesn't seem right, need an else block?

    pEventTimer->Resume();
    engine->_statusBar->setEvent(LSTR_GAME_SAVED);
}

void SavegameList::Initialize() {
    pSavegameList->Reset();

    std::string saves_dir = makeDataPath("saves");

    if (std::filesystem::exists(saves_dir)) {
        for (const auto &entry : std::filesystem::directory_iterator(saves_dir)) {
            if (entry.path().extension() == ".mm7") {
                pSavegameList->pFileList[pSavegameList->numSavegameFiles++] = entry.path().filename().string();
                if (pSavegameList->numSavegameFiles == MAX_SAVE_SLOTS) {
                    break;
                }
            }
        }
    } else {
        logger->warning("Couldn't find saves directory!");
    }

    if (pSavegameList->numSavegameFiles) {
        std::sort(pSavegameList->pFileList.begin(), pSavegameList->pFileList.begin() + pSavegameList->numSavegameFiles);
    }
}

SavegameList::SavegameList() { Reset(); }

void SavegameList::Reset() {
    for (int j = 0; j < MAX_SAVE_SLOTS; j++) {
        this->pFileList[j].clear();
    }

    numSavegameFiles = 0;
}

void SaveNewGame() {
    std::string file_path = makeDataPath("data", "new.lod");
    pSave_LOD->close();
    std::filesystem::remove(file_path);

    LodWriter lodWriter(file_path, makeSaveLodInfo());

    // Copy ddm & dlv files.
    for (const std::string &name : pGames_LOD->ls())
        if (name.ends_with(".ddm") || name.ends_with(".dlv"))
            lodWriter.write(name, pGames_LOD->read(name));

    pSavegameList->pSavegameHeader[0].locationName = "out01.odm";

    // TODO(captainurist): encapsulate
    SaveGameHeader_MM7 headerMm7;
    snapshot(pSavegameList->pSavegameHeader[0], &headerMm7);
    lodWriter.write("header.bin", Blob::view(&headerMm7, sizeof(headerMm7)));
    lodWriter.close();

    pParty->lastPos.x = 12552;
    pParty->lastPos.y = 1816;
    pParty->lastPos.z = 193;

    pParty->pos.x = 12552;
    pParty->pos.y = 1816;
    pParty->pos.z = 193;

    pParty->uFallStartZ = 193;

    pParty->_viewPrevPitch = 0;
    pParty->_viewPrevYaw = 512;

    pParty->_viewPitch = 0;
    pParty->_viewYaw = 512;

    SaveGame(1, 1);
}
