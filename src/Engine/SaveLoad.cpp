#include "Engine/SaveLoad.h"

#include <stdlib.h>
#include <filesystem>
#include <algorithm>
#include <string>

#include "Library/Compression/Compression.h"

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"
#include "Engine/Events.h"

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/PCX.h"
#include "Engine/Graphics/Viewport.h"

#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/SpriteObject.h"

#include "Engine/Serialization/LegacyImages.h"
#include "Engine/Serialization/Serializer.h"
#include "Engine/Serialization/CompositeImages.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UISaveLoad.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

struct SavegameList *pSavegameList = new SavegameList;
// TODO(pskelton): combine these all into savegamelist
unsigned int uNumSavegameFiles;
std::array<unsigned int, MAX_SAVE_SLOTS> pSavegameUsedSlots;
std::array<Image *, MAX_SAVE_SLOTS> pSavegameThumbnails;
std::array<SavegameHeader, MAX_SAVE_SLOTS> pSavegameHeader;

// TODO(pskelton): move to save game list
int pSaveListPosition = 0;
unsigned int uLoadGameUI_SelectedSlot = 0;

void LoadGame(unsigned int uSlot) {
    MapsLongTimers_count = 0;
    if (!pSavegameUsedSlots[uSlot]) {
        pAudioPlayer->playUISound(SOUND_error);
        logger->warning("LoadGame: slot {} is empty", uSlot);
        return;
    }
    uLoadGameUI_SelectedSlot = uSlot;

    pParty->Reset();

    pSave_LOD->CloseWriteFile();
    // uCurrentlyLoadedLevelType = LEVEL_null;

    std::string filename = MakeDataPath("saves", pSavegameList->pFileList[uSlot]);
    std::string to_file_path = MakeDataPath("data", "new.lod");

    std::error_code ec;
    if (!std::filesystem::copy_file(filename, to_file_path, std::filesystem::copy_options::overwrite_existing, ec))
        Error("Failed to copy: %s", filename.c_str());

    pSave_LOD->LoadFile(to_file_path, 0);

    static_assert(sizeof(SavegameHeader) == 100, "Wrong type size");
    Blob headerBlob = pSave_LOD->LoadRaw("header.bin");
    SavegameHeader *header = (SavegameHeader*)headerBlob.data();
    if (header == nullptr) {
        logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 100));
    }

    {
        Blob partyBlob = pSave_LOD->LoadRaw("party.bin");
        Party_MM7 *serialization = (Party_MM7*)partyBlob.data();
        if (serialization == nullptr) {
            logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 101));
        } else {
            Deserialize(*serialization, pParty);

            pParty->bTurnBasedModeOn = false;  // We always start in realtime after loading a game.

            for (size_t i = 0; i < 4; i++) {
                Player *player = &pParty->pPlayers[i];
                for (size_t j = 0; j < 5; j++) {
                    if (j >= player->vBeacons.size()) {
                        continue;
                    }
                    LloydBeacon &beacon = player->vBeacons[j];
                    std::string str = fmt::format("lloyd{}{}.pcx", i + 1, j + 1);
                    //beacon.image = Image::Create(new PCX_LOD_Raw_Loader(pNew_LOD, str));
                    beacon.image = render->CreateTexture_PCXFromLOD(pSave_LOD, str);
                    beacon.image->GetWidth();
                }
            }
        }
    }

    {
        Blob timerBlob = pSave_LOD->LoadRaw("clock.bin");
        Timer_MM7 *serialization = (Timer_MM7*)timerBlob.data();
        if (serialization == nullptr) {
            logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 102));
        } else {
            Deserialize(*serialization, pEventTimer);
        }
    }

    {
        Blob blob = pSave_LOD->LoadRaw("overlay.bin");
        OtherOverlayList_MM7 *serialization = (OtherOverlayList_MM7*)blob.data();
        if (serialization == nullptr) {
            logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 103));
        } else {
            Deserialize(*serialization, pOtherOverlayList);
        }
    }

    {
        Blob blob = pSave_LOD->LoadRaw("npcdata.bin");
        NPCData_MM7 *serialization = (NPCData_MM7*)blob.data();
        if (serialization == nullptr) {
            logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 104));
        } else {
            for (unsigned int i = 0; i < 501; ++i) {
                Deserialize(serialization[i], &pNPCStats->pNewNPCData[i]);
            }
            pNPCStats->OnLoadSetNPC_Names();
        }
    }

    {
        Blob blob = pSave_LOD->LoadRaw("npcgroup.bin");
        const void *npcgroup = blob.data();
        if (npcgroup == nullptr) {
            logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 105));
            __debugbreak();
        } else if (sizeof(pNPCStats->pGroups_copy) != 102) {
            logger->warning("NPCStats: deserialization warning");
        } else {
            memcpy(pNPCStats->pGroups_copy.data(), npcgroup, sizeof(pNPCStats->pGroups_copy));
        }
    }

    pParty->setActiveCharacterIndex(0);
    pParty->setActiveToFirstCanAct();

/*
    for (uint i = 0; i < 4; ++i) {
        if (pParty->pPlayers[i].uQuickSpell) {
            AA1058_PartyQuickSpellSound[i].AddPartySpellSound(
                pParty->pPlayers[i].uQuickSpell, i + 1);
        }

        for (uint j = 0; j < 2; ++j) {
            uint uEquipIdx = pParty->pPlayers[i].pEquipment.pIndices[j];
            if (uEquipIdx) {
                int pItemID = pParty->pPlayers[i]
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
    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;

    SetUserInterface(pParty->alignment, true);

    pEventTimer->Resume();
    pEventTimer->StopGameTime();

    if (!pGames_LOD->DoesContainerExist(header->pLocationName)) {
        Error("Unable to find: %s!", header->pLocationName);
    }

    pCurrentMapName = header->pLocationName;

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN | GAME_SETTINGS_0001;

    for (uint i = 0; i < uNumSavegameFiles; ++i) {
        if (pSavegameThumbnails[i] != nullptr) {
            pSavegameThumbnails[i]->Release();
            pSavegameThumbnails[i] = nullptr;
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

    int pPositionX = pParty->vPosition.x;
    int pPositionY = pParty->vPosition.y;
    int pPositionZ = pParty->vPosition.z;
    int partyViewYaw = pParty->_viewYaw;
    int partyViewPitch = pParty->_viewPitch;
    pParty->vPosition.x = pParty->vPrevPosition.x;
    pParty->vPosition.z = pParty->vPrevPosition.z;
    pParty->vPosition.y = pParty->vPrevPosition.y;

    pParty->uFallStartZ = pParty->vPrevPosition.z;

    pParty->_viewYaw = pParty->_viewPrevYaw;
    pParty->_viewPitch = pParty->_viewPrevPitch;
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        pIndoor->stru1.last_visit = pParty->GetPlayingTime();
    else
        pOutdoor->loc_time.last_visit = pParty->GetPlayingTime();

    // saving - please wait

    // if (current_screen_type == SCREEN_SAVEGAME) {
    //    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f,
    //        saveload_ui_loadsave);
    //    render->DrawTextureNew(18 / 640.0f, 141 / 480.0f,
    //        saveload_ui_loadsave);
    //    int text_pos = pFontSmallnum->AlignText_Center(186, localization->GetString(190));
    //    pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, text_pos + 25, 219, 0,
    //        localization->GetString(190), 0, 0,
    //        0);  // Сохранение
    //    text_pos = pFontSmallnum->AlignText_Center(
    //        186, pSavegameHeader[uLoadGameUI_SelectedSlot].pName);
    //    pGUIWindow_CurrentMenu->DrawTextInRect(
    //        pFontSmallnum, text_pos + 25, 259, 0,
    //        pSavegameHeader[uLoadGameUI_SelectedSlot].pName, 185, 0);
    //    text_pos =
    //        pFontSmallnum->AlignText_Center(186, localization->GetString(LSTR_PLEASE_WAIT));
    //    pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, text_pos + 25, 299, 0,
    //        localization->GetString(LSTR_PLEASE_WAIT), 0, 0,
    //        0);  // Пожалуйста, подождите
    //    render->Present();
    //}

    Blob packedScreenshot{ render->PackScreenshot(150, 112) };  // создание скриншота
    if (pSave_LOD->Write("image.pcx", packedScreenshot.data(), packedScreenshot.size(), 0)) {
        logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 200));
    }

    static_assert(sizeof(SavegameHeader) == 100, "Wrong type size");
    SavegameHeader save_header;
    memset(save_header.pName, 0, 20);
    memset(save_header.pLocationName, 0, 20);
    memset(save_header.field_30, 0, 52);
    strcpy(save_header.pLocationName, pCurrentMapName.c_str());
    save_header.playing_time = pParty->GetPlayingTime();
    if (pSave_LOD->Write("header.bin", &save_header, sizeof(SavegameHeader), 0)) {
        logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 201));
    }

    {
        Party_MM7 serialization;
        Serialize(*pParty, &serialization);

        if (pSave_LOD->Write("party.bin", &serialization, sizeof(serialization), 0)) {
            logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 202));
        }
    }

    {
        Timer_MM7 serialization;
        Serialize(*pEventTimer, &serialization);

        if (pSave_LOD->Write("clock.bin", &serialization, sizeof(serialization), 0)) {
            logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 203));
        }
    }

    {
        OtherOverlayList_MM7 serialization;
        Serialize(*pOtherOverlayList, &serialization);

        if (pSave_LOD->Write("overlay.bin", &serialization, sizeof(serialization), 0)) {
            logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 204));
        }
    }

    {
        NPCData_MM7 serialization[501];
        for (unsigned int i = 0; i < 501; ++i) {
            Serialize(pNPCStats->pNewNPCData[i], &serialization[i]);
        }

        if (pSave_LOD->Write("npcdata.bin", serialization, sizeof(serialization), 0)) {
            logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 205));
        }
    }

    if (pSave_LOD->Write("npcgroup.bin", pNPCStats->pGroups_copy.data(), sizeof(pNPCStats->pGroups_copy), 0)) {
        logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 206));
    }

    for (size_t i = 0; i < 4; ++i) {  // 4 - players
        Player *player = &pParty->pPlayers[i];
        for (size_t j = 0; j < 5; ++j) {  // 5 - images
            if (j >= player->vBeacons.size()) {
                continue;
            }
            LloydBeacon *beacon = &player->vBeacons[j];
            Image *image = beacon->image;
            if ((beacon->uBeaconTime.Valid()) && (image != nullptr)) {
                const void *pixels = image->GetPixels(IMAGE_FORMAT_A8B8G8R8);
                if (!pixels)
                    __debugbreak();

                Blob packedPCX{ PCX::Encode(pixels, image->GetWidth(), image->GetHeight()) };
                std::string str = fmt::format("lloyd{}{}.pcx", i + 1, j + 1);
                if (pSave_LOD->Write(str, packedPCX.data(), packedPCX.size(), 0)) {
                    logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 207));
                }
            }
        }
    }

    Blob uncompressed;
    if (!NotSaveWorld) {  // autosave for change location
        CompactLayingItemsList();

        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            pIndoor->dlv.uNumFacesInBModels = pIndoor->pFaces.size();
            pIndoor->dlv.uNumBModels = 0;
            pIndoor->dlv.uNumDecorations = pLevelDecorations.size();

            IndoorSave_MM7 save;
            Serialize(*pIndoor, &save);
            Serialize(save, &uncompressed);
        } else {  // for Outdoor
            BlobSerializer stream;

            pOutdoor->ddm.uNumFacesInBModels = 0;
            for (BSPModel &model : pOutdoor->pBModels) {
                pOutdoor->ddm.uNumFacesInBModels += model.pFaces.size();
            }
            pOutdoor->ddm.uNumBModels = pOutdoor->pBModels.size();
            pOutdoor->ddm.uNumDecorations = pLevelDecorations.size();

            stream.WriteRaw(&pOutdoor->ddm);
            stream.WriteRaw(&pOutdoor->uFullyRevealedCellOnMap);
            stream.WriteRaw(&pOutdoor->uPartiallyRevealedCellOnMap);

            for (BSPModel &model : pOutdoor->pBModels) {
                for (ODMFace &face : model.pFaces) {
                    stream.WriteRaw(&face.uAttributes);
                    static_assert(sizeof(face.uAttributes) == 4);
                }
            }

            for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
                stream.WriteRaw(&pLevelDecorations[i].uFlags);
                static_assert(sizeof(pLevelDecorations[i].uFlags) == 2);
            }

            stream.WriteLegacyVector<Actor_MM7>(pActors);
            stream.WriteLegacyVector<SpriteObject_MM7>(pSpriteObjects);
            stream.WriteLegacyVector<Chest_MM7>(vChests);
            stream.WriteLegacy<MapEventVariables_MM7>(&mapEventVariables);
            stream.WriteLegacy<LocationTime_MM7>(&pOutdoor->loc_time);

            uncompressed = stream.Close();
        }

        ODMHeader odm_data;
        odm_data.uVersion = 91969;
        odm_data.pMagic[0] = 'm';
        odm_data.pMagic[1] = 'v';
        odm_data.pMagic[2] = 'i';
        odm_data.pMagic[3] = 'i';

        Blob compressed = zlib::Compress(uncompressed);

        odm_data.uCompressedSize = compressed.size();
        odm_data.uDecompressedSize = uncompressed.size();

        Blob mapBlob = Blob::concat(Blob::view(&odm_data, sizeof(ODMHeader)), compressed);

        std::string file_name = pCurrentMapName;
        size_t pos = file_name.find_last_of(".");
        file_name[pos + 1] = 'd';
        if (pSave_LOD->Write(file_name, mapBlob.data(), mapBlob.size(), 0)) {
            logger->warning("{}", localization->FormatString(LSTR_FMT_SAVEGAME_CORRUPTED, 208));
        }
    }

    if (IsAutoSAve) {
        std::string src = MakeDataPath("data", "new.lod");
        std::string dst = MakeDataPath("saves", "autosave.mm7");
        std::error_code ec;
        if (!std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec))
            logger->warning("Copying of autosave.mm7 failed");
    }
    pParty->vPosition.x = pPositionX;
    pParty->vPosition.y = pPositionY;
    pParty->vPosition.z = pPositionZ;
    pParty->uFallStartZ = pPositionZ;
    pParty->_viewYaw = partyViewYaw;
    pParty->_viewPitch = partyViewPitch;
}

void DoSavegame(unsigned int uSlot) {
    if (pCurrentMapName != "d05.blv") {  // Not Arena(не Арена)
        SaveGame(0, 0);
        strcpy(pSavegameHeader[uSlot].pLocationName, pCurrentMapName.c_str());
        pSavegameHeader[uSlot].playing_time = pParty->GetPlayingTime();
        pSave_LOD->Write("header.bin", &pSavegameHeader[uSlot], sizeof(SavegameHeader), 0);
        pSave_LOD->CloseWriteFile();  //закрыть
        std::string src = MakeDataPath("data", "new.lod");
        std::string dst = MakeDataPath("saves", fmt::format("save{:03}.mm7", uSlot));
        std::error_code ec;
        if (!std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec))
            Error("Failed to copy: %s", src.c_str());
    }
    uLoadGameUI_SelectedSlot = uSlot;

    GUI_UpdateWindows();
    pGUIWindow_CurrentMenu->Release();
    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;

    for (uint i = 0; i < MAX_SAVE_SLOTS; i++) {
        if (pSavegameThumbnails[i] != nullptr) {
            pSavegameThumbnails[i]->Release();
            pSavegameThumbnails[i] = nullptr;
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
    uNumSavegameFiles = 0;

    std::string saves_dir = MakeDataPath("saves");

    if (std::filesystem::exists(saves_dir)) {
        for (const auto &entry : std::filesystem::directory_iterator(saves_dir)) {
            if (entry.path().extension() == ".mm7") {
                pSavegameList->pFileList[uNumSavegameFiles++] = entry.path().filename().string();
                if (uNumSavegameFiles == MAX_SAVE_SLOTS) break;
            }
        }
    } else {
        logger->warning("Couldn't find saves directory!");
    }

    if (uNumSavegameFiles)
        std::sort(pSavegameList->pFileList.begin(), pSavegameList->pFileList.begin() + uNumSavegameFiles);
}

SavegameList::SavegameList() { Reset(); }

void SavegameList::Reset() {
    for (int j = 0; j < MAX_SAVE_SLOTS; j++) {
        this->pFileList[j].clear();
    }
}

void SaveNewGame() {
    if (pSave_LOD != nullptr) {
        pSave_LOD->CloseWriteFile();
    }

    std::string file_path = MakeDataPath("data", "new.lod");
    remove(file_path.c_str());  // удалить new.lod

    LOD::FileHeader header;  // заголовок
    strcpy(header.LodVersion, "MMVII");
    strcpy(header.LodDescription, "newmaps for MMVII");
    header.LODSize = 100;
    header.dword_0000A8 = 0;

    pSave_LOD->CreateNewLod(&header, "current", file_path);  // создаётся new.lod в дирректории
    if (pSave_LOD->LoadFile(file_path, false)) {  // загрузить файл new.lod(isFileOpened = true)
        pSave_LOD->CreateTempFile();  // создаётся временный файл OutputFileHandle
        pSave_LOD->ClearSubNodes();

        for (size_t i = pGames_LOD->GetSubNodesCount() / 2; i < pGames_LOD->GetSubNodesCount(); ++i) {  // копирование файлов с 76 по 151
            std::string name = pGames_LOD->GetSubNodeName(i);
            Blob data = pGames_LOD->LoadRaw(name);
            pSave_LOD->AppendDirectory(name, data.data(), data.size());
        }

        strcpy(pSavegameHeader[0].pLocationName, "out01.odm");
        pSave_LOD->AppendDirectory("header.bin", &pSavegameHeader[0], sizeof(SavegameHeader));

        pSave_LOD->FixDirectoryOffsets();

        pParty->vPrevPosition.x = 12552;
        pParty->vPrevPosition.y = 1816;
        pParty->vPrevPosition.z = 193;

        pParty->vPosition.x = 12552;
        pParty->vPosition.y = 1816;
        pParty->vPosition.z = 193;

        pParty->uFallStartZ = 193;

        pParty->_viewPrevPitch = 0;
        pParty->_viewPrevYaw = 512;

        pParty->_viewPitch = 0;
        pParty->_viewYaw = 512;

        SaveGame(1, 1);
    }
}
