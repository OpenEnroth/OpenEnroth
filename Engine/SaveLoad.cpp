#include "Engine/SaveLoad.h"

#include <direct.h>
#include <io.h>
#include <stdlib.h>
#include <vector>

#include "Platform/Api.h"

#include "Engine/Engine.h"
#include "Engine/EngineConfig.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"
#include "Engine/ZlibWrapper.h"
#include "Engine/stru123.h"

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

#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UISaveLoad.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

struct SavegameList *pSavegameList = new SavegameList;
unsigned int uNumSavegameFiles;
std::array<unsigned int, MAX_SAVE_SLOTS> pSavegameUsedSlots;
std::array<Image *, MAX_SAVE_SLOTS> pSavegameThumbnails;
std::array<SavegameHeader, MAX_SAVE_SLOTS> pSavegameHeader;

bool CopyFile(const String &from, const String &to) {
    int file_size = -1;
    int bytes_read = 0;
    int bytes_wrote = 0;

    FILE *copy_from = fopen(from.c_str(), "rb");
    if (copy_from) {
        FILE *copy_to = fopen(to.c_str(), "wb+");
        if (copy_to) {
            fseek(copy_from, 0, SEEK_END);
            file_size = ftell(copy_from);
            fseek(copy_from, 0, SEEK_SET);

            unsigned char *buf = new unsigned char[file_size];
            if (buf) {
                bytes_read = fread(buf, 1, file_size, copy_from);
                if (bytes_read == file_size) {
                    bytes_wrote = fwrite(buf, 1, file_size, copy_to);
                }

                delete[] buf;
            }
            fclose(copy_to);
        }
        fclose(copy_from);
    }

    return file_size != -1 && bytes_read == bytes_wrote;
}

void LoadGame(unsigned int uSlot) {
    MapsLongTimers_count = 0;
    if (!pSavegameUsedSlots[uSlot]) {
        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
        logger->Warning(L"LoadGame: slot %u is empty", uSlot);
        return;
    }

    pNew_LOD->CloseWriteFile();

    String filename = "saves\\" + pSavegameList->pFileList[uSlot];
    filename = MakeDataPath(filename.c_str());
    String to_file_path = MakeDataPath("data\\new.lod");
    remove(to_file_path.c_str());
    if (!CopyFile(filename, to_file_path)) {
        Error("Failed to copy: %s", filename.c_str());
    }

    pNew_LOD->LoadFile(to_file_path, 0);

    static_assert(sizeof(SavegameHeader) == 100, "Wrong type size");
    SavegameHeader *header = (SavegameHeader*)pNew_LOD->LoadRaw("header.bin");
    if (header == nullptr) {
        logger->Warning(L"%S", localization->FormatString(612, 100).c_str());  // Savegame damaged! Code=%d
    }

    {
        Party_Image_MM7 *serialization = (Party_Image_MM7*)pNew_LOD->LoadRaw("party.bin");
        if (serialization == nullptr) {
            logger->Warning(L"%S", localization->FormatString(612, 101).c_str());  // Savegame damaged! Code=%d
        } else {
            serialization->Deserialize(pParty);
            free(serialization);

            for (size_t i = 0; i < 4; i++) {
                Player *player = &pParty->pPlayers[i];
                for (size_t j = 0; j < 5; j++) {
                    if (j >= player->vBeacons.size()) {
                        continue;
                    }
                    LloydBeacon &beacon = player->vBeacons[j];
                    String str = StringPrintf("lloyd%d%d.pcx", i + 1, j + 1);
                    beacon.image = Image::Create(new PCX_LOD_File_Loader(pNew_LOD, str));
                    beacon.image->GetWidth();
                }
            }
        }
    }

    {
        Timer_Image_MM7 *serialization = (Timer_Image_MM7*)pNew_LOD->LoadRaw("clock.bin");
        if (serialization == nullptr) {
            logger->Warning(L"%S", localization->FormatString(612, 102).c_str());  // Savegame damaged! Code=%d
        } else {
            serialization->Deserialize(pEventTimer);
            free(serialization);
        }
    }

    {
        OtherOverlayList_Image_MM7 *serialization = (OtherOverlayList_Image_MM7*)pNew_LOD->LoadRaw("overlay.bin");
        if (serialization == nullptr) {
            logger->Warning(L"%S", localization->FormatString(612, 103).c_str());  // Savegame damaged! Code=%d
        } else {
            serialization->Deserialize(pOtherOverlayList);
            free(serialization);
        }
    }

    {
        NPCData_Image_MM7 *serialization = (NPCData_Image_MM7*)pNew_LOD->LoadRaw("npcdata.bin");
        if (serialization == nullptr) {
            logger->Warning(L"%S", localization->FormatString(612, 104).c_str());  // Savegame damaged! Code=%d
        } else {
            for (unsigned int i = 0; i < 501; ++i) {
                serialization[i].Deserialize(pNPCStats->pNewNPCData + i);
            }
            pNPCStats->_476C60_on_load_game();
            free(serialization);
        }
    }

    {
        void *npcgroup = pNew_LOD->LoadRaw("npcgroup.bin");
        if (npcgroup == nullptr) {
            logger->Warning(L"%S", localization->FormatString(612, 105).c_str());  // Savegame damaged! Code=%d
        }
        if (sizeof(pNPCStats->pGroups_copy) != 102) {
            logger->Warning(L"NPCStats: deserialization warning");
        }
        memcpy(pNPCStats->pGroups_copy, npcgroup, sizeof(pNPCStats->pGroups_copy));
        free(npcgroup);
    }

    uActiveCharacter = 0;
    for (uint i = 0; i < 4; ++i) {
        if (pParty->pPlayers[i].CanAct()) {
            uActiveCharacter = i + 1;
            break;
        }
    }
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
                if (pItemsTable->pItems[pItemID].uEquipType == EQUIP_WAND &&
                    pItemID) {       // жезл
                    __debugbreak();  // looks like offset in player's inventory
                                     // and wand_lut much like case in 0042ECB5
                    stru_A750F8[i].AddPartySpellSound(
                        wand_spell_ids[pItemID - ITEM_WAND_FIRE], i + 9);
                }
            }
        }
    }
*/
    current_screen_type = SCREEN_GAME;

    viewparams->bRedrawGameUI = true;

    SetUserInterface(pParty->alignment, true);

    pEventTimer->Resume();
    pEventTimer->StopGameTime();

    if (!pGames_LOD->DoesContainerExist(header->pLocationName)) {
        Error("Unable to find: %s!", header->pLocationName);
    }

    pCurrentMapName = header->pLocationName;
    free(header);

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_2000 | GAME_SETTINGS_0001;

    for (uint i = 0; i < uNumSavegameFiles; ++i) {
        if (pSavegameThumbnails[i] != nullptr) {
            pSavegameThumbnails[i]->Release();
            pSavegameThumbnails[i] = nullptr;
        }
    }

    pAudioPlayer->SetMusicVolume(engine->config->music_level);
    pAudioPlayer->SetMasterVolume(engine->config->sound_level);
    if (engine->config->turn_speed > 0) {
        pParty->sRotationY = engine->config->turn_speed * pParty->sRotationY / engine->config->turn_speed;
    }
    MM7Initialization();
    bFlashQuestBook = false;
    viewparams->bRedrawGameUI = true;
}

void SaveGame(bool IsAutoSAve, bool NotSaveWorld) {
    s_SavedMapName = pCurrentMapName;
    if (pCurrentMapName == "d05.blv") {  // arena
        return;
    }

    char *uncompressed_buff = (char *)malloc(1000000);

    int pPositionX = pParty->vPosition.x;
    int pPositionY = pParty->vPosition.y;
    int pPositionZ = pParty->vPosition.z;
    int sPRotationY = pParty->sRotationY;
    int sPRotationX = pParty->sRotationX;
    pParty->vPosition.x = pParty->vPrevPosition.x;
    pParty->vPosition.z = pParty->vPrevPosition.z;
    pParty->vPosition.y = pParty->vPrevPosition.y;

    pParty->uFallStartY = pParty->vPrevPosition.z;

    pParty->sRotationY = pParty->sPrevRotationY;
    pParty->sRotationX = pParty->sPrevRotationX;
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        pIndoor->stru1.last_visit = pParty->GetPlayingTime();
    else
        pOutdoor->loc_time.last_visit = pParty->GetPlayingTime();

    unsigned int buf_size = 0;
    render->PackScreenshot(150, 112, uncompressed_buff, 1000000, &buf_size);  // создание скриншота

    // saving - please wait

    // if (current_screen_type == SCREEN_SAVEGAME) {
    //    render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f,
    //        saveload_ui_loadsave);
    //    render->DrawTextureAlphaNew(18 / 640.0f, 141 / 480.0f,
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
    //        pFontSmallnum->AlignText_Center(186, localization->GetString(165));
    //    pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, text_pos + 25, 299, 0,
    //        localization->GetString(165), 0, 0,
    //        0);  // Пожалуйста, подождите
    //    render->Present();
    //}

    if (pNew_LOD->Write("image.pcx", uncompressed_buff, buf_size, 0)) {
        auto error_message = localization->FormatString(612, 200);  // Savegame damaged! Code=%d
        logger->Warning(L"%S", error_message.c_str());
    }

    static_assert(sizeof(SavegameHeader) == 100, "Wrong type size");
    SavegameHeader save_header;
    memset(save_header.pName, 0, 20);
    memset(save_header.pLocationName, 0, 20);
    memset(save_header.field_30, 0, 52);
    strcpy(save_header.pLocationName, pCurrentMapName.c_str());
    save_header.playing_time = pParty->GetPlayingTime();
    if (pNew_LOD->Write("header.bin", &save_header, sizeof(SavegameHeader), 0)) {
        auto error_message = localization->FormatString(612, 201);
        logger->Warning(L"%S", error_message.c_str());
    }

    {
        Party_Image_MM7 serialization;
        serialization.Serialize(pParty);

        if (pNew_LOD->Write("party.bin", &serialization, sizeof(serialization), 0)) {
            auto error_message = localization->FormatString(612, 202);
            logger->Warning(L"%S", error_message.c_str());
        }
    }

    {
        Timer_Image_MM7 serialization;
        serialization.Serialize(pEventTimer);

        if (pNew_LOD->Write("clock.bin", &serialization, sizeof(serialization), 0)) {
            auto error_message = localization->FormatString(612, 203);
            logger->Warning(L"%S", error_message.c_str());
        }
    }

    {
        OtherOverlayList_Image_MM7 serialization;
        serialization.Serialize(pOtherOverlayList);

        if (pNew_LOD->Write("overlay.bin", &serialization, sizeof(serialization), 0)) {
            auto error_message = localization->FormatString(612, 204);
            logger->Warning(L"%S", error_message.c_str());
        }
    }

    {
        NPCData_Image_MM7 serialization[501];
        for (unsigned int i = 0; i < 501; ++i) {
            serialization[i].Serialize(pNPCStats->pNewNPCData + i);
        }

        if (pNew_LOD->Write("npcdata.bin", serialization, sizeof(serialization), 0)) {
            auto error_message = localization->FormatString(612, 205);
            logger->Warning(L"%S", error_message.c_str());
        }
    }

    if (pNew_LOD->Write("npcgroup.bin", pNPCStats->pGroups_copy, sizeof(pNPCStats->pGroups_copy), 0)) {
        auto error_message = localization->FormatString(612, 206);
        logger->Warning(L"%S", error_message.c_str());
    }

    for (size_t i = 0; i < 4; ++i) {  // 4 - players
        Player *player = &pParty->pPlayers[i];
        for (size_t j = 0; j < 5; ++j) {  // 5 - images
            if (j >= player->vBeacons.size()) {
                continue;
            }
            LloydBeacon *beacon = &player->vBeacons[j];
            Image *image = beacon->image;
            if ((beacon->uBeaconTime != 0) && (image != nullptr)) {
                const void *pixels = image->GetPixels(IMAGE_FORMAT_R5G6B5);
                unsigned int pcx_data_size = 30000;
                void *pcx_data = malloc(pcx_data_size);
                PCX::Encode16(pixels, image->GetWidth(), image->GetHeight(),
                              pcx_data, pcx_data_size, &pcx_data_size);
                String str = StringPrintf("lloyd%d%d.pcx", i + 1, j + 1);
                if (pNew_LOD->Write(str, pcx_data, pcx_data_size, 0)) {
                    auto error_message = localization->FormatString(612, 207);
                    logger->Warning(L"%S", error_message.c_str());
                }
                free(pcx_data);
            }
        }
    }

    if (!NotSaveWorld) {  // autosave for change location
        CompactLayingItemsList();
        char *compressed_buf = (char *)malloc(1000000);
        ODMHeader *odm_data = (ODMHeader*)compressed_buf;
        odm_data->uVersion = 91969;
        odm_data->pMagic[0] = 'm';
        odm_data->pMagic[1] = 'v';
        odm_data->pMagic[2] = 'i';
        odm_data->pMagic[3] = 'i';
        odm_data->uCompressedSize = 0;
        odm_data->uDecompressedSize = 0;

        char *data_write_pos = uncompressed_buff;
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            pIndoor->dlv.uNumFacesInBModels = pIndoor->uNumFaces;
            pIndoor->dlv.uNumBModels = 0;
            pIndoor->dlv.uNumDecorations = uNumLevelDecorations;
            memcpy(data_write_pos, &pIndoor->dlv, sizeof(DDM_DLV_Header));  // 0x28
            data_write_pos += sizeof(DDM_DLV_Header);
            memcpy(data_write_pos, pIndoor->_visible_outlines, 0x36B);
            data_write_pos += 875;
            for (int i = 0; i < (signed int)pIndoor->uNumFaces; ++i) {
                memcpy(data_write_pos, &pIndoor->pFaces[i].uAttributes, 4);
                data_write_pos += 4;
            }

            for (int i = 0; i < (signed int)uNumLevelDecorations; ++i) {
                memcpy(data_write_pos, &pLevelDecorations[i].uFlags, 2);
                data_write_pos += 2;
            }
            memcpy(data_write_pos, &uNumActors, 4);
            data_write_pos += 4;
            memcpy(data_write_pos, &pActors, uNumActors * sizeof(Actor));
            data_write_pos += uNumActors * sizeof(Actor);
            memcpy(data_write_pos, &uNumSpriteObjects, 4);
            data_write_pos += 4;
            memcpy(data_write_pos, pSpriteObjects.data(),
                   112 * uNumSpriteObjects);
            data_write_pos += 112 * uNumSpriteObjects;

            data_write_pos = ChestsSerialize(data_write_pos);

            memcpy(data_write_pos, pIndoor->pDoors, sizeof(BLVDoor) * 200);
            data_write_pos += 16000;
            memcpy(data_write_pos, pIndoor->ptr_0002B4_doors_ddata,
                   pIndoor->blv.uDoors_ddata_Size);
            data_write_pos += pIndoor->blv.uDoors_ddata_Size;
            memcpy(data_write_pos, &stru_5E4C90_MapPersistVars, 0xC8);
            data_write_pos += 200;
            memcpy(data_write_pos, &pIndoor->stru1, 0x38);
            data_write_pos += 56;

        } else {  // for Outdoor
            pOutdoor->ddm.uNumFacesInBModels = 0;
            for (BSPModel &model : pOutdoor->pBModels) {
                pOutdoor->ddm.uNumFacesInBModels += model.pFaces.size();
            }
            pOutdoor->ddm.uNumBModels = pOutdoor->pBModels.size();
            pOutdoor->ddm.uNumDecorations = uNumLevelDecorations;
            memcpy(data_write_pos, &pOutdoor->ddm,
                   sizeof(DDM_DLV_Header));  // 0x28
            data_write_pos += sizeof(DDM_DLV_Header);
            memcpy(data_write_pos, pOutdoor->uFullyRevealedCellOnMap, 0x3C8);
            data_write_pos += 968;
            memcpy(data_write_pos, pOutdoor->uPartiallyRevealedCellOnMap,
                   0x3C8);
            data_write_pos += 968;
            for (BSPModel &model : pOutdoor->pBModels) {
                for (ODMFace &face : model.pFaces) {
                    memcpy(data_write_pos, &(face.uAttributes), 4);
                    data_write_pos += 4;
                }
            }

            for (size_t i = 0; i < uNumLevelDecorations; ++i) {
                memcpy(data_write_pos, &pLevelDecorations[i].uFlags, 2);
                data_write_pos += 2;
            }
            memcpy(data_write_pos, &uNumActors, 4);
            data_write_pos += 4;
            memcpy(data_write_pos, &pActors, uNumActors * sizeof(Actor));
            data_write_pos += uNumActors * sizeof(Actor);
            memcpy(data_write_pos, &uNumSpriteObjects, 4);
            data_write_pos += 4;
            memcpy(data_write_pos, &pSpriteObjects,
                   uNumSpriteObjects * sizeof(SpriteObject));
            data_write_pos += uNumSpriteObjects * sizeof(SpriteObject);

            data_write_pos = ChestsSerialize(data_write_pos);

            memcpy(data_write_pos, &stru_5E4C90_MapPersistVars, 0xC8);
            data_write_pos += 200;
            memcpy(data_write_pos, &pOutdoor->loc_time, 0x38);
            data_write_pos += 56;
        }

        unsigned int compressed_block_size = 1000000 - sizeof(ODMHeader);
        size_t Size = data_write_pos - uncompressed_buff;
        int res = zlib::Compress(compressed_buf + sizeof(ODMHeader), &compressed_block_size, uncompressed_buff, Size);
        if (res || (compressed_block_size > Size)) {
            memcpy((void *)(compressed_buf + sizeof(ODMHeader)), uncompressed_buff, Size);
            compressed_block_size = Size;
        }

        odm_data->uCompressedSize = compressed_block_size;
        odm_data->uDecompressedSize = Size;

        String file_name = pCurrentMapName;
        size_t pos = file_name.find_last_of(".");
        file_name[pos + 1] = 'd';
        if (pNew_LOD->Write(file_name, compressed_buf, compressed_block_size + sizeof(ODMHeader), 0)) {
            auto error_message = localization->FormatString(612, 208);
            logger->Warning(L"%S", error_message.c_str());
        }
        free(compressed_buf);
    }
    free(uncompressed_buff);
    if (IsAutoSAve) {
        if (!CopyFile(MakeDataPath("data\\new.lod"),
                      MakeDataPath("saves\\autosave.mm7"))) {
            logger->Warning(L"Copy autosave.mm7 failed");
        }
    }
    pParty->vPosition.x = pPositionX;
    pParty->vPosition.y = pPositionY;
    pParty->vPosition.z = pPositionZ;
    pParty->uFallStartY = pPositionZ;
    pParty->sRotationY = sPRotationY;
    pParty->sRotationX = sPRotationX;
}

void DoSavegame(unsigned int uSlot) {
    if (pCurrentMapName != "d05.blv") {  // Not Arena(не Арена)
        SaveGame(0, 0);
        strcpy(pSavegameHeader[uSlot].pLocationName, pCurrentMapName.c_str());
        pSavegameHeader[uSlot].playing_time = pParty->GetPlayingTime();
        pNew_LOD->Write("header.bin", &pSavegameHeader[uSlot], sizeof(SavegameHeader), 0);
        pNew_LOD->CloseWriteFile();  //закрыть
        String file_path = StringPrintf("saves\\save%03d.mm7", uSlot);
        file_path = MakeDataPath(file_path.c_str());
        CopyFile(MakeDataPath("data\\new.lod"), file_path);
    }
    GUI_UpdateWindows();
    pGUIWindow_CurrentMenu->Release();
    current_screen_type = SCREEN_GAME;

    viewparams->bRedrawGameUI = true;
    for (uint i = 0; i < MAX_SAVE_SLOTS; i++) {
        if (pSavegameThumbnails[i] != nullptr) {
            pSavegameThumbnails[i]->Release();
            pSavegameThumbnails[i] = nullptr;
        }
    }

    if (pCurrentMapName != "d05.blv")
        pNew_LOD->_4621A7();
    else
        GameUI_StatusBar_OnEvent(localization->GetString(583), 2);  // "No saving in the Arena"

    pEventTimer->Resume();
    GameUI_StatusBar_OnEvent(localization->GetString(656), 2);  // "Game Saved!"
    viewparams->bRedrawGameUI = true;
}

void SavegameList::Initialize() {
    pSavegameList->Reset();
    uNumSavegameFiles = 0;

    String saves_dir = MakeDataPath("Saves");
    std::vector<String> files = OS_FindFiles(saves_dir, "*.mm7");
    for (const String &path : files) {
        pSavegameList->pFileList[uNumSavegameFiles++] = path;
    }
}

SavegameList::SavegameList() { Reset(); }

void SavegameList::Reset() {
    for (int j = 0; j < MAX_SAVE_SLOTS; j++) {
        this->pFileList[j].clear();
    }
}

void SaveNewGame() {
    if (pNew_LOD != nullptr) {
        pNew_LOD->CloseWriteFile();
    }

    String file_path = MakeDataPath("data\\new.lod");
    remove(file_path.c_str());  // удалить new.lod

    LOD::FileHeader header;  // заголовок
    strcpy(header.LodVersion, "MMVII");
    strcpy(header.LodDescription, "newmaps for MMVII");
    header.LODSize = 100;
    header.dword_0000A8 = 0;

    pNew_LOD->CreateNewLod(&header, "current", file_path);  // создаётся new.lod в дирректории
    if (pNew_LOD->LoadFile(file_path, false)) {  // загрузить файл new.lod(isFileOpened = true)
        pNew_LOD->CreateTempFile();  // создаётся временный файл OutputFileHandle
        pNew_LOD->ClearSubNodes();

        for (size_t i = pGames_LOD->GetSubNodesCount() / 2; i < pGames_LOD->GetSubNodesCount(); ++i) {  // копирование файлов с 76 по 151
            String name = pGames_LOD->GetSubNodeName(i);
            size_t size = 0;
            void *data = pGames_LOD->LoadRaw(name, &size);
            pNew_LOD->AppendDirectory(name, data, size);
            free(data);
        }

        strcpy(pSavegameHeader[0].pLocationName, "out01.odm");
        pNew_LOD->AppendDirectory("header.bin", &pSavegameHeader[0], sizeof(SavegameHeader));

        pNew_LOD->FixDirectoryOffsets();

        pParty->vPrevPosition.x = 12552;
        pParty->vPrevPosition.y = 1816;
        pParty->vPrevPosition.z = 0;

        pParty->vPosition.x = 12552;
        pParty->vPosition.y = 1816;
        pParty->vPosition.z = 0;

        pParty->uFallStartY = 0;

        pParty->sPrevRotationX = 0;
        pParty->sPrevRotationY = 512;

        pParty->sRotationX = 0;
        pParty->sRotationY = 512;

        SaveGame(1, 1);
    }
}
