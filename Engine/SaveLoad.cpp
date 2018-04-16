#include "Engine/SaveLoad.h"

#include <direct.h>
#include <io.h>
#include <stdlib.h>
#include <windows.h>
#undef PlaySound
#undef DrawText

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"
#include "Engine/ZlibWrapper.h"
#include "Engine/stru123.h"

#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Overlays.h"
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
std::array<unsigned int, 45> pSavegameUsedSlots;
std::array<Image *, 45> pSavegameThumbnails;
std::array<SavegameHeader, 45> pSavegameHeader;

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

//----- (00411B59) --------------------------------------------------------
void LoadThumbnailLloydTexture(unsigned int uSlot, unsigned int uPlayer) {
    if (pSavegameThumbnails[uSlot]) {
        pSavegameThumbnails[uSlot]->Release();
        pSavegameThumbnails[uSlot] = nullptr;
    }

    String str = StringPrintf("data\\lloyd%d%d.pcx", uPlayer, uSlot + 1);
    str = MakeDataPath(str.c_str());
    pSavegameThumbnails[uSlot] = assets->GetImage_PCXFromFile(str);

    if (!pSavegameThumbnails[uSlot]) {
        String str = StringPrintf("lloyd%d%d.pcx", uPlayer, uSlot + 1);
        str = MakeDataPath(str.c_str());
        pSavegameThumbnails[uSlot] = assets->GetImage_PCXFromNewLOD(str);
    }
}

//----- (0045EE8A) --------------------------------------------------------
void LoadGame(unsigned int uSlot) {
    bool v25;               // esi@62
    bool v26;               // eax@62
    SavegameHeader header;  // [sp+Ch] [bp-E4h]@23
                            //    char Str[123]; // [sp+70h] [bp-80h]@25

    MapsLongTimers_count = 0;
    if (!pSavegameUsedSlots[uSlot]) {
        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
        logger->Warning(L"LoadGame: slot %u is empty", uSlot);
        return;
    }

    for (uint i = 1; i < 5; ++i) {
        for (uint j = 1; j < 6; ++j) {
            String file_path = StringPrintf("data\\lloyd%d%d.pcx", i, j);
            file_path = MakeDataPath(file_path.c_str());
            remove(file_path.c_str());
        }
    }

    pNew_LOD->CloseWriteFile();

    String filename = "saves\\" + pSavegameList->pFileList[uSlot];
    filename = MakeDataPath(filename.c_str());
    String to_file_path = MakeDataPath("data\\new.lod");
    remove(to_file_path.c_str());
    if (!CopyFile(filename, to_file_path)) {
        Error("Failed to copy: %s", filename.c_str());
    }

    pNew_LOD->LoadFile(to_file_path.c_str(), 0);
    FILE *file = pNew_LOD->FindContainer("header.bin", 1);
    if (!file) {
        logger->Warning(L"%S", localization->FormatString(612, 100)
                                   .c_str());  // Savegame damaged! Code=%d
    }
    Assert(sizeof(SavegameHeader) == 100);
    fread(&header, sizeof(SavegameHeader), 1, file);
    {
        file = pNew_LOD->FindContainer("party.bin", 1);
        if (!file) {
            logger->Warning(L"%S", localization->FormatString(612, 101)
                                       .c_str());  // Savegame damaged! Code=%d
        } else {
            Party_Image_MM7 serialization;
            fread(&serialization, sizeof(serialization), 1, file);

            serialization.Deserialize(pParty);
        }
    }

    {
        file = pNew_LOD->FindContainer("clock.bin", 1);
        if (!file) {
            logger->Warning(L"%S", localization->FormatString(612, 102)
                                       .c_str());  // Savegame damaged! Code=%d
        } else {
            Timer_Image_MM7 serialization;
            fread(&serialization, sizeof(serialization), 1, file);

            serialization.Deserialize(pEventTimer);
        }
    }

    {
        file = pNew_LOD->FindContainer("overlay.bin", 1);
        if (!file) {
            logger->Warning(L"%S", localization->FormatString(612, 103)
                                       .c_str());  // Savegame damaged! Code=%d
        } else {
            OtherOverlayList_Image_MM7 serialization;
            fread(&serialization, sizeof(serialization), 1, file);

            serialization.Deserialize(pOtherOverlayList);
        }
    }

    {
        file = pNew_LOD->FindContainer("npcdata.bin", 0);
        if (!file) {
            logger->Warning(L"%S", localization->FormatString(612, 104)
                                       .c_str());  // Savegame damaged! Code=%d
        } else {
            NPCData_Image_MM7 serialization[501];
            fread(serialization, sizeof(serialization), 1, file);

            for (unsigned int i = 0; i < 501; ++i)
                serialization[i].Deserialize(pNPCStats->pNewNPCData + i);

            pNPCStats->_476C60_on_load_game();
        }
    }

    file = pNew_LOD->FindContainer("npcgroup.bin", 0);
    if (!file) {
        logger->Warning(L"%S", localization->FormatString(612, 105)
                                   .c_str());  // Savegame damaged! Code=%d
    }
    if (sizeof(pNPCStats->pGroups_copy) != 0x66)
        logger->Warning(L"NPCStats: deserialization warning");
    fread(pNPCStats->pGroups_copy, sizeof(pNPCStats->pGroups_copy), 1, file);

    uActiveCharacter = 0;
    for (uint i = 0; i < 4; ++i) {
        if (pParty->pPlayers[i].CanAct()) {
            uActiveCharacter = i + 1;
            break;
        }
    }

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

    current_screen_type = SCREEN_GAME;

    viewparams->bRedrawGameUI = true;

    SetUserInterface(pParty->alignment, true);

    pEventTimer->Resume();
    pEventTimer->StopGameTime();

    v25 = pGames_LOD->DoesContainerExist(header.pLocationName);
    v26 = _access(StringPrintf("levels\\%s", header.pLocationName).c_str(),
                  4) != -1;
    if (!v25 && !v26) Error("Unable to find: %s!", header.pLocationName);

    strcpy(pCurrentMapName, header.pLocationName);
    dword_6BE364_game_settings_1 |= GAME_SETTINGS_2000 | GAME_SETTINGS_0001;

    for (uint i = 0; i < uNumSavegameFiles; ++i) {
        if (pSavegameThumbnails[i] != nullptr) {
            pSavegameThumbnails[i]->Release();
            pSavegameThumbnails[i] = nullptr;
        }
    }

    pAudioPlayer->SetMusicVolume(engine_config->music_level);
    pAudioPlayer->SetMasterVolume(engine_config->sound_level);
    if (engine_config->turn_speed > 0)
        pParty->sRotationY = engine_config->turn_speed * pParty->sRotationY / engine_config->turn_speed;
    MM7Initialization();
    bFlashQuestBook = false;
    viewparams->bRedrawGameUI = true;
}

//----- (0045F469) --------------------------------------------------------
void SaveGame(bool IsAutoSAve, bool NotSaveWorld) {
    int text_pos;                // eax@6
    FILE *pLLoidFile;            // edi@24
    char *compressed_buf;        // edi@30
    char *data_write_pos;        // esi@41
                                 //    char Buffer[128]; // [sp+Ch] [bp-264h]@59
    char Dir[255];               // [sp+8Ch] [bp-1E4h]@51
    char Drive[255];             // [sp+ACh] [bp-1C4h]@51
    SavegameHeader save_header;  // [sp+CCh] [bp-1A4h]@10
    char Filename[255];          // [sp+130h] [bp-140h]@51
    char Ext[255];               // [sp+150h] [bp-120h]@51
    char Source[32];             // [sp+170h] [bp-100h]@51
    int pPositionY;              // [sp+208h] [bp-68h]@2
    int pPositionX;              // [sp+20Ch] [bp-64h]@2
    int sPRotationY;             // [sp+210h] [bp-60h]@2
    int sPRotationX;             // [sp+214h] [bp-5Ch]@2
    ODMHeader odm_data;          // [sp+218h] [bp-58h]@30
    int res;                     // [sp+224h] [bp-4Ch]@30
    int pPositionZ;              // [sp+228h] [bp-48h]@2
    size_t Size;                 // [sp+250h] [bp-20h]@26
    char *uncompressed_buff;     // [sp+258h] [bp-18h]@2
    unsigned int compressed_block_size;  // [sp+260h] [bp-10h]@23

    // v66 = a2;
    s_SavedMapName = pCurrentMapName;
    if (!_stricmp(pCurrentMapName, "d05.blv"))  // arena
        return;

    uncompressed_buff = (char *)malloc(1000000);

    LOD::Directory pLodDirectory;  // [sp+22Ch] [bp-44h]@2
    pPositionX = pParty->vPosition.x;
    pPositionY = pParty->vPosition.y;
    pPositionZ = pParty->vPosition.z;
    sPRotationY = pParty->sRotationY;
    sPRotationX = pParty->sRotationX;
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

    render->PackScreenshot(150, 112, uncompressed_buff, 1000000,
                           &pLodDirectory.uDataSize);  //создание скриншота
    strcpy(pLodDirectory.pFilename, "image.pcx");

    if (current_screen_type == SCREEN_SAVEGAME) {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f,
                                    saveload_ui_loadsave);
        render->DrawTextureAlphaNew(18 / 640.0f, 141 / 480.0f,
                                    saveload_ui_loadsave);
        text_pos =
            pFontSmallnum->AlignText_Center(186, localization->GetString(190));
        pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, text_pos + 25, 219, 0,
                                         localization->GetString(190), 0, 0,
                                         0);  //Сохранение
        text_pos = pFontSmallnum->AlignText_Center(
            186, pSavegameHeader[uLoadGameUI_SelectedSlot].pName);
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontSmallnum, text_pos + 25, 259, 0,
            pSavegameHeader[uLoadGameUI_SelectedSlot].pName, 185, 0);
        text_pos =
            pFontSmallnum->AlignText_Center(186, localization->GetString(165));
        pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, text_pos + 25, 299, 0,
                                         localization->GetString(165), 0, 0,
                                         0);  //Пожалуйста, подождите
        render->Present();
    }

    if (pNew_LOD->Write(&pLodDirectory, uncompressed_buff, 0)) {
        auto error_message =
            localization->FormatString(612, 200);  // Savegame damaged! Code=%d
        logger->Warning(L"%S", error_message.c_str());
    }

    Assert(sizeof(SavegameHeader) == 100);
    memset(save_header.pName, 0, 20);
    memset(save_header.pLocationName, 0, 20);
    memset(save_header.field_30, 0, 52);
    strcpy(save_header.pLocationName, pCurrentMapName);
    save_header.playing_time = pParty->GetPlayingTime();
    strcpy(pLodDirectory.pFilename, "header.bin");
    pLodDirectory.uDataSize = sizeof(SavegameHeader);
    if (pNew_LOD->Write(&pLodDirectory, &save_header, 0)) {
        auto error_message = localization->FormatString(612, 201);
        logger->Warning(L"%S", error_message.c_str());
    }

    {
        Party_Image_MM7 serialization;
        serialization.Serialize(pParty);

        pLodDirectory.uDataSize = sizeof(serialization);
        strcpy(pLodDirectory.pFilename, "party.bin");
        if (pNew_LOD->Write(&pLodDirectory, &serialization, 0)) {
            auto error_message = localization->FormatString(612, 202);
            logger->Warning(L"%S", error_message.c_str());
        }
    }

    {
        Timer_Image_MM7 serialization;
        serialization.Serialize(pEventTimer);

        pLodDirectory.uDataSize = sizeof(serialization);
        strcpy(pLodDirectory.pFilename, "clock.bin");
        if (pNew_LOD->Write(&pLodDirectory, &serialization, 0)) {
            auto error_message = localization->FormatString(612, 203);
            logger->Warning(L"%S", error_message.c_str());
        }
    }

    {
        OtherOverlayList_Image_MM7 serialization;
        serialization.Serialize(pOtherOverlayList);

        pLodDirectory.uDataSize = sizeof(serialization);
        strcpy(pLodDirectory.pFilename, "overlay.bin");
        if (pNew_LOD->Write(&pLodDirectory, &serialization, 0)) {
            auto error_message = localization->FormatString(612, 204);
            logger->Warning(L"%S", error_message.c_str());
        }
    }

    {
        NPCData_Image_MM7 serialization[501];
        for (unsigned int i = 0; i < 501; ++i)
            serialization[i].Serialize(pNPCStats->pNewNPCData + i);

        pLodDirectory.uDataSize = sizeof(serialization);
        strcpy(pLodDirectory.pFilename, "npcdata.bin");
        if (pNew_LOD->Write(&pLodDirectory, serialization, 0)) {
            auto error_message = localization->FormatString(612, 205);
            logger->Warning(L"%S", error_message.c_str());
        }
    }

    strcpy(pLodDirectory.pFilename, "npcgroup.bin");
    pLodDirectory.uDataSize = 102;
    if (pNew_LOD->Write(&pLodDirectory, pNPCStats->pGroups_copy, 0)) {
        auto error_message = localization->FormatString(612, 206);
        logger->Warning(L"%S", error_message.c_str());
    }

    for (int i = 1; i <= 4; ++i) {  // 4 - players
        for (int j = 1; j <= 5; ++j) {  // 5 - images
            char work_string[120];
            sprintf(work_string, "data\\lloyd%d%d.pcx", i, j);
            String file_path = MakeDataPath(work_string);
            pLLoidFile = fopen(file_path.c_str(), "rb");
            if (pLLoidFile) {
                __debugbreak();
                sprintf(work_string, "lloyd%d%d.pcx", i, j);
                fseek(pLLoidFile, 0, SEEK_END);
                pLodDirectory.uDataSize = ftell(pLLoidFile);
                rewind(pLLoidFile);
                fread(uncompressed_buff, pLodDirectory.uDataSize, 1,
                      pLLoidFile);
                strcpy(pLodDirectory.pFilename, work_string);
                fclose(pLLoidFile);
                remove(work_string);
                if (pNew_LOD->Write(&pLodDirectory, uncompressed_buff, 0)) {
                    auto error_message = localization->FormatString(612, 207);
                    logger->Warning(L"%S", error_message.c_str());
                }
            }
        }
    }

    if (!NotSaveWorld) {  // autosave for change location
        // __debugbreak();
        CompactLayingItemsList();
        compressed_buf = (char *)malloc(1000000);
        odm_data.uVersion = 91969;
        odm_data.pMagic[0] = 'm';
        odm_data.pMagic[1] = 'v';
        odm_data.pMagic[2] = 'i';
        odm_data.pMagic[3] = 'i';
        odm_data.uCompressedSize = 0;
        odm_data.uDecompressedSize = 0;
        data_write_pos = uncompressed_buff;
        memcpy((void *)compressed_buf, &odm_data, 0x10);
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            pIndoor->dlv.uNumFacesInBModels = pIndoor->uNumFaces;
            pIndoor->dlv.uNumBModels = 0;
            pIndoor->dlv.uNumDecorations = uNumLevelDecorations;
            memcpy(data_write_pos, &pIndoor->dlv,
                   sizeof(DDM_DLV_Header));  // 0x28
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
            memcpy(data_write_pos, &uNumChests, 4);
            data_write_pos += 4;
            memcpy(data_write_pos, pChests.data(),
                   sizeof(Chest) * uNumChests);  // 5324 *
            data_write_pos += sizeof(Chest) * uNumChests;
            memcpy(data_write_pos, pIndoor->pDoors, 0x3E80);
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
            memcpy(data_write_pos, &uNumChests, 4);
            data_write_pos += 4;
            memcpy(data_write_pos, pChests.data(), sizeof(Chest) * uNumChests);
            data_write_pos += sizeof(Chest) * uNumChests;
            memcpy(data_write_pos, &stru_5E4C90_MapPersistVars, 0xC8);
            data_write_pos += 200;
            memcpy(data_write_pos, &pOutdoor->loc_time, 0x38);
            data_write_pos += 56;
        }
        strcpy(Source, pCurrentMapName);
        _splitpath(Source, Drive, Dir, Filename, Ext);
        Ext[1] = 'd';

        Size = (int)data_write_pos - (int)uncompressed_buff;
        compressed_block_size = 999984;
        res = zlib::Compress(compressed_buf + 16, &compressed_block_size,
                             uncompressed_buff, Size);
        if (res || (signed int)compressed_block_size > (signed int)Size) {
            memcpy((void *)(compressed_buf + 16), uncompressed_buff, Size);
            compressed_block_size = Size;
        }
        compressed_block_size += 16;
        memcpy(&((ODMHeader *)compressed_buf)->uCompressedSize,
               &compressed_block_size, 4);
        memcpy(&((ODMHeader *)compressed_buf)->uDecompressedSize, &Size, 4);
        sprintf(Source, "%s%s", Filename, Ext);
        strcpy(pLodDirectory.pFilename, Source);
        pLodDirectory.uDataSize = compressed_block_size;
        if (pNew_LOD->Write(&pLodDirectory, (const void *)compressed_buf, 0)) {
            auto error_message = localization->FormatString(612, 208);
            logger->Warning(L"%S", error_message.c_str());
        }
        free((void *)compressed_buf);
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

//----- (00460078) --------------------------------------------------------
void DoSavegame(unsigned int uSlot) {
    if (_stricmp(pCurrentMapName, "d05.blv")) {  // Not Arena(не Арена)
        LOD::Directory pDir;  // [sp+Ch] [bp-28h]@2
        SaveGame(0, 0);
        strcpy(pSavegameHeader[uSlot].pLocationName, pCurrentMapName);
        pSavegameHeader[uSlot].playing_time = pParty->GetPlayingTime();
        strcpy(pDir.pFilename, "header.bin");
        pDir.uDataSize = 100;
        pNew_LOD->Write(&pDir, &pSavegameHeader[uSlot], 0);
        pNew_LOD->CloseWriteFile();  //закрыть
        String file_path = StringPrintf("saves\\save%03d.mm7", uSlot);
        file_path = MakeDataPath(file_path.c_str());
        CopyFile(MakeDataPath("data\\new.lod"), file_path);
    }
    GUI_UpdateWindows();
    pGUIWindow_CurrentMenu->Release();
    current_screen_type = SCREEN_GAME;

    viewparams->bRedrawGameUI = true;
    for (uint i = 0; i < 45; i++) {
        if (pSavegameThumbnails[i] != nullptr) {
            pSavegameThumbnails[i]->Release();
            pSavegameThumbnails[i] = nullptr;
        }
    }

    if (_stricmp(pCurrentMapName, "d05.blv"))
        pNew_LOD->_4621A7();
    else
        GameUI_StatusBar_OnEvent(localization->GetString(583),
                                 2);  // "No saving in the Arena"

    pEventTimer->Resume();
    GameUI_StatusBar_OnEvent(localization->GetString(656), 2);  // "Game Saved!"
    viewparams->bRedrawGameUI = true;
}

//----- (0045E297) --------------------------------------------------------
void SavegameList::Initialize() {
    pSavegameList->Reset();
    uNumSavegameFiles = 0;

    String saves_dir = MakeDataPath("Saves\\*.mm7");

    WIN32_FIND_DATAA ffd = {0};
    HANDLE hFind = FindFirstFileA(saves_dir.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {
        return;
    }

    do {
        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            pSavegameList->pFileList[uNumSavegameFiles++] = ffd.cFileName;
        }
    } while (FindNextFileA(hFind, &ffd) != 0);

    FindClose(hFind);
}

SavegameList::SavegameList() { Reset(); }

void SavegameList::Reset() {
    for (int j = 0; j < 45; j++) {
        this->pFileList[j].clear();
    }
}

void SaveNewGame() {
    String file_path = MakeDataPath("data\\new.lod");

    void *pSave = malloc(1000000);
    if (pNew_LOD != nullptr) {
        pNew_LOD->CloseWriteFile();
    }
    remove(file_path.c_str());  // удалить new.lod

    LOD::FileHeader header;  // заголовок
    strcpy(header.LodVersion, "MMVII");
    strcpy(header.LodDescription, "newmaps for MMVII");
    header.LODSize = 100;
    header.dword_0000A8 = 0;

    LOD::Directory a3;
    a3.dword_000018 = 0;
    a3.word_00001E = 0;
    strcpy(a3.pFilename, "current");
    pNew_LOD->CreateNewLod(
        &header, &a3, file_path.c_str());  //создаётся new.lod в дирректории
    if (pNew_LOD->LoadFile(
            file_path.c_str(),
            false)) {  //загрузить файл new.lod(isFileOpened = true)
        pNew_LOD->CreateTempFile();  //создаётся временный файл OutputFileHandle
        pNew_LOD->uNumSubDirs = 0;

        LOD::Directory pDir;  // [sp+10Ch] [bp-6Ch]@4
        for (int i = pGames_LOD->uNumSubDirs / 2; i < pGames_LOD->uNumSubDirs;
             ++i) {  // копирование файлов с 76 по 151
            memcpy(&pDir, &pGames_LOD->pSubIndices[i],
                   sizeof(pDir));  //копирование текущего файла в pDir
            FILE *file = pGames_LOD->FindContainer(
                pGames_LOD->pSubIndices[i].pFilename, 1);
            fread(pSave, pGames_LOD->pSubIndices[i].uDataSize, 1, file);
            pNew_LOD->AppendDirectory(&pDir, pSave);
        }

        LOD::Directory save_game_dir;  // [sp+12Ch] [bp-4Ch]@9
        strcpy(pSavegameHeader[0].pLocationName, "out01.odm");
        strcpy(save_game_dir.pFilename, "header.bin");
        save_game_dir.uDataSize = sizeof(SavegameHeader);
        pNew_LOD->AppendDirectory(&save_game_dir, &pSavegameHeader[0]);

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
    free(pSave);
}
