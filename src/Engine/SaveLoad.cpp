#include "SaveLoad.h"

#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <string>

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

#include "Utility/DataPath.h"

struct SavegameList *pSavegameList = new SavegameList;

void LoadGame(unsigned int uSlot) {
    if (!pSavegameList->pSavegameUsedSlots[uSlot]) {
        pAudioPlayer->playUISound(SOUND_error);
        logger->warning("LoadGame: slot {} is empty", uSlot);
        return;
    }
    pSavegameList->selectedSlot = uSlot;
    pSavegameList->lastLoadedSave = pSavegameList->pFileList[uSlot];

    pParty->Reset();

    pSave_LOD->CloseWriteFile();
    // uCurrentlyLoadedLevelType = LEVEL_NULL;

    std::string filename = makeDataPath("saves", pSavegameList->pFileList[uSlot]);
    std::string to_file_path = makeDataPath("data", "new.lod");

    std::error_code ec;
    if (!std::filesystem::copy_file(filename, to_file_path, std::filesystem::copy_options::overwrite_existing, ec))
        Error("Failed to copy: %s", filename.c_str());

    pSave_LOD->LoadFile(to_file_path, 0);

    SaveGameHeader header;
    deserialize(*pSave_LOD, &header, tags::via<SaveGame_MM7>);

    // TODO(captainurist): incapsulate this too
    pParty->bTurnBasedModeOn = false;  // We always start in realtime after loading a game.
    for (size_t i = 0; i < 4; i++) {
        Character *player = &pParty->pCharacters[i];
        for (size_t j = 0; j < 5; j++) {
            if (j >= player->vBeacons.size()) {
                continue;
            }
            LloydBeacon &beacon = player->vBeacons[j];
            std::string str = fmt::format("lloyd{}{}.pcx", i + 1, j + 1);
            //beacon.image = Image::Create(new PCX_LOD_Raw_Loader(pNew_LOD, str));
            beacon.image = render->CreateTexture_PCXFromLOD(pSave_LOD, str);
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
                if (pItemTable->pItems[pItemID].uEquipType == EQUIP_WAND &&
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

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN | GAME_SETTINGS_0001;

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

void SaveGame(bool IsAutoSAve, bool NotSaveWorld) {
    s_SavedMapName = pCurrentMapName;
    if (pCurrentMapName == "d05.blv") {  // arena
        return;
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

    Blob packedScreenshot{ render->PackScreenshot(150, 112) };  // создание скриншота
    if (pSave_LOD->Write("image.pcx", packedScreenshot.data(), packedScreenshot.size(), 0)) {
        logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 200));
    }

    SaveGameHeader save_header;
    save_header.locationName = pCurrentMapName;
    save_header.playingTime = pParty->GetPlayingTime();

    serialize(save_header, pSave_LOD, tags::via<SaveGame_MM7>);

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
                Blob packedPCX = PCX::Encode(image->rgba());
                std::string str = fmt::format("lloyd{}{}.pcx", i + 1, j + 1);
                if (pSave_LOD->Write(str, packedPCX.data(), packedPCX.size(), 0)) {
                    logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 207));
                }
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

        LOD::CompressedHeader odm_data;
        odm_data.uVersion = 91969;
        odm_data.pMagic[0] = 'm';
        odm_data.pMagic[1] = 'v';
        odm_data.pMagic[2] = 'i';
        odm_data.pMagic[3] = 'i';

        Blob compressed = zlib::Compress(uncompressed);

        odm_data.uCompressedSize = compressed.size();
        odm_data.uDecompressedSize = uncompressed.size();

        Blob mapBlob = Blob::concat(Blob::view(&odm_data, sizeof(odm_data)), compressed);

        std::string file_name = pCurrentMapName;
        size_t pos = file_name.find_last_of(".");
        file_name[pos + 1] = 'd';
        if (pSave_LOD->Write(file_name, mapBlob.data(), mapBlob.size(), 0)) {
            logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 208));
        }
    }

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
}

void DoSavegame(unsigned int uSlot) {
    if (pCurrentMapName != "d05.blv") {  // Not Arena(не Арена)
        SaveGame(0, 0);

        // TODO(captainurist): this code doesn't belong here.
        pSavegameList->pSavegameHeader[uSlot].locationName = pCurrentMapName;
        pSavegameList->pSavegameHeader[uSlot].playingTime = pParty->GetPlayingTime();

        SaveGameHeader_MM7 headerMm7;
        snapshot(pSavegameList->pSavegameHeader[uSlot], &headerMm7);

        pSave_LOD->Write("header.bin", &headerMm7, sizeof(headerMm7), 0);
        pSave_LOD->CloseWriteFile();  //закрыть
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

    if (pCurrentMapName != "d05.blv")
        pSave_LOD->_4621A7();
    else
        GameUI_SetStatusBar(LSTR_NO_SAVING_IN_ARENA);

    pEventTimer->Resume();
    GameUI_SetStatusBar(LSTR_GAME_SAVED);
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
    if (pSave_LOD != nullptr) {
        pSave_LOD->CloseWriteFile();
    }

    std::string file_path = makeDataPath("data", "new.lod");
    std::filesystem::remove(file_path);  // удалить new.lod

    LOD::FileHeader header;  // заголовок
    strcpy(header.LodVersion, "MMVII");
    strcpy(header.LodDescription, "newmaps for MMVII");
    header.LODSize = 100;
    header.dword_0000A8 = 0;

    pSave_LOD->CreateNewLod(&header, "current", file_path);  // создаётся new.lod в дирректории
    if (pSave_LOD->LoadFile(file_path, false)) {  // загрузить файл new.lod(isFileOpened = true)
        pSave_LOD->CreateTempFile();  // создаётся временный файл OutputFileHandle
        pSave_LOD->ClearSubNodes();

        // Copy ddm & dlv files, can actually just filter by extension instead.
        for (const std::string &name : pGames_LOD->ls()) {
            if (!name.ends_with(".ddm") && !name.ends_with(".dlv"))
                continue;

            Blob data = pGames_LOD->readRaw(name);
            pSave_LOD->AppendDirectory(name, data.data(), data.size());
        }

        pSavegameList->pSavegameHeader[0].locationName = "out01.odm";

        // TODO(captainurist): encapsulate
        SaveGameHeader_MM7 headerMm7;
        snapshot(pSavegameList->pSavegameHeader[0], &headerMm7);

        pSave_LOD->AppendDirectory("header.bin", &headerMm7, sizeof(headerMm7));

        pSave_LOD->FixDirectoryOffsets();

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
}
