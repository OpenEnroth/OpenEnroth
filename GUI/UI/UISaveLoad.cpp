#include <io.h>

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/LOD.h"
#include "Engine/SaveLoad.h"
#include "Engine/Graphics/ImageLoader.h"

#include "IO/Keyboard.h"

#include "GUI/GUIFont.h"
#include "GUI/UI/UISaveLoad.h"

#include "Game/MainMenu.h"

#include "Platform/Api.h"


void UI_DrawSaveLoad(bool save);


Image *saveload_ui_save_up = nullptr;
Image *saveload_ui_load_up = nullptr;
Image *saveload_ui_loadsave = nullptr;
Image *saveload_ui_saveu = nullptr;
Image *saveload_ui_loadu = nullptr;
Image *saveload_ui_x_u = nullptr;
Image *saveload_ui_ls_saved = nullptr;
Image *saveload_ui_x_d = nullptr;

GUIWindow_Save::GUIWindow_Save() :
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0, nullptr)
{
    // ------------------------------------------------
    // 0045E93E SaveUI_Load(enum CURRENT_SCREEN screen)
    std::string file_name; // eax@7
    LODWriteableFile pLODFile; // [sp+1Ch] [bp-248h]@1

    memset(&pSavegameUsedSlots, 0, sizeof(pSavegameUsedSlots));
    memset(&pSavegameThumbnails, 0, sizeof(pSavegameThumbnails));
    saveload_ui_loadsave = assets->GetImage_16BitColorKey("loadsave", 0x7FF);
    saveload_ui_save_up = assets->GetImage_16BitColorKey("save_up", 0x7FF);
    saveload_ui_load_up = assets->GetImage_16BitColorKey("load_up", 0x7FF);
    saveload_ui_saveu = assets->GetImage_16BitColorKey("LS_saveU", 0x7FF);
    saveload_ui_loadu = assets->GetImage_16BitColorKey("LS_loadU", 0x7FF);
    saveload_ui_x_u = assets->GetImage_16BitColorKey("x_u", 0x7FF);

    render->DrawTextureAlphaNew(8/640.0f, 8/480.0f, saveload_ui_loadsave);

    render->DrawTextureAlphaNew(241/640.0f, 302/480.0f, saveload_ui_saveu);
    render->DrawTextureAlphaNew(351/640.0f, 302/480.0f, saveload_ui_x_u);
    render->DrawTextureAlphaNew(18/640.0f, 141/480.0f, saveload_ui_save_up);

    render->Present();
    pSavegameList->Initialize();
    pLODFile.AllocSubIndicesAndIO(300, 0);
    for (uint i = 0; i < 40; ++i) {
        file_name = pSavegameList->pFileList[i];
        if (file_name.empty()) {
            file_name = "1.mm7";
        }

        std::string str = "saves\\" + file_name;
        str = MakeDataPath(str.c_str());
        if (_access(str.c_str(), 0) || _access(str.c_str(), 6))
        {
            pSavegameUsedSlots[i] = 0;
            strcpy(pSavegameHeader[i].pName, localization->GetString(72)); // Empty
        }
        else
        {
            pLODFile.LoadFile(str.c_str(), 1);
            fread(&pSavegameHeader[i], 100, 1, pLODFile.FindContainer("header.bin", 1)); // problesm new maps mm7
            if (pLODFile.FindContainer("image.pcx", 1))
            {
                //pSavegameThumbnails[i].LoadFromFILE(pLODFile.FindContainer("image.pcx", 1), 0, 1);
                pLODFile.CloseWriteFile();
                pSavegameUsedSlots[i] = 1;
            }
            else
                pSavegameUsedSlots[i] = 0;
        }
    }
    pLODFile.FreeSubIndexAndIO();

    if (!saveload_ui_x_d)
        saveload_ui_x_d = assets->GetImage_16BitAlpha("x_d");
    if (!saveload_ui_ls_saved)
        saveload_ui_ls_saved = assets->GetImage_16BitAlpha("LS_saveD");
    if (!ui_ar_up_dn)
        ui_ar_up_dn = assets->GetImage_16BitAlpha("ar_up_dn");
    if (!ui_ar_dn_dn)
        ui_ar_dn_dn = assets->GetImage_16BitAlpha("ar_dn_dn");


// -----------------------------
// GUIWindow_Save c-tor --- part
    CreateButton(21, 198, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 0, 0, "", 0);
    CreateButton(21, 218, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 1, 0, "", 0);
    CreateButton(21, 238, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 2, 0, "", 0);
    CreateButton(21, 258, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 3, 0, "", 0);
    CreateButton(21, 278, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 4, 0, "", 0);
    CreateButton(21, 298, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 5, 0, "", 0);
    CreateButton(21, 318, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 6, 0, "", 0);

    pBtnLoadSlot = CreateButton(241, 302, 105, 40, 1, 0, UIMSG_SaveLoadBtn, 0, 0, "", saveload_ui_ls_saved, 0);
    pBtnCancel = CreateButton(350, 302, 105, 40, 1, 0, UIMSG_Cancel, 0, 0, "", saveload_ui_x_d, 0);
    pBtnArrowUp = CreateButton(215, 199, 17, 17, 1, 0, UIMSG_ArrowUp, 0, 0, "", ui_ar_up_dn, 0);
    pBtnDownArrow = CreateButton(215, 323, 17, 17, 1, 0, UIMSG_DownArrow, 34, 0, "", ui_ar_dn_dn, 0);
}


void GUIWindow_Save::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
// {
//     SaveUI_Draw();
// }

// ----- (004606FE) --------------------------------------------------------
// void SaveUI_Draw()
// {
    UI_DrawSaveLoad(true);
}




GUIWindow_Load::GUIWindow_Load(bool ingame) :
    GUIWindow(0, 0, 0, 0, 0, nullptr)
{
    current_screen_type = SCREEN_LOADGAME;

    dword_6BE138 = -1;
    pIcons_LOD->_inlined_sub2();

    memset(pSavegameUsedSlots.data(), 0, sizeof(pSavegameUsedSlots));
    memset(pSavegameThumbnails.data(), 0, 45 * sizeof(Image *));
    saveload_ui_loadsave = assets->GetImage_16BitColorKey("loadsave", 0x7FF);
    saveload_ui_save_up = assets->GetImage_16BitColorKey("save_up", 0x7FF);
    saveload_ui_load_up = assets->GetImage_16BitColorKey("load_up", 0x7FF);
    saveload_ui_saveu = assets->GetImage_16BitColorKey("LS_saveU", 0x7FF);
    saveload_ui_loadu = assets->GetImage_16BitColorKey("LS_loadU", 0x7FF);
    saveload_ui_x_u = assets->GetImage_16BitColorKey("x_u", 0x7FF);

    if (ingame)
    {
        render->DrawTextureAlphaNew(8/640.0f, 8/480.0f, saveload_ui_loadsave);
        if (current_screen_type == SCREEN_SAVEGAME)
        {
            render->DrawTextureAlphaNew(241/640.0f, 302/480.0f, saveload_ui_saveu);
            render->DrawTextureAlphaNew(18 / 640.0f, 141 / 480.0f, saveload_ui_save_up);
        }
        else
        {
            render->DrawTextureAlphaNew(241 / 640.0f, 302 / 480.0f, saveload_ui_loadu);
            render->DrawTextureAlphaNew(18 / 640.0f, 141 / 480.0f, saveload_ui_load_up);
        }
        render->DrawTextureAlphaNew(351 / 640.0f, 302 / 480.0f, saveload_ui_x_u);
    }
    else
        render->DrawTextureNew(0, 0, main_menu_background);


    /*pGUIWindow_CurrentMenu = new GUIWindow_Load(
        saveload_dlg_xs[uDialogueType],
        saveload_dlg_ys[uDialogueType],
        saveload_dlg_zs[uDialogueType],
        saveload_dlg_ws[uDialogueType], 0, 0);*/

    // GUIWindow::GUIWindow
    this->uFrameX = saveload_dlg_xs[ingame ? 1 : 0];
    this->uFrameY = saveload_dlg_ys[ingame ? 1 : 0];
    this->uFrameWidth = saveload_dlg_zs[ingame ? 1 : 0];
    this->uFrameHeight = saveload_dlg_ws[ingame ? 1 : 0];
    this->uFrameZ = uFrameX + uFrameWidth - 1;
    this->uFrameW = uFrameY + uFrameHeight - 1;



    DrawText(pFontSmallnum, 25, 199, 0, localization->GetString(505), 0, 0, 0);// "Reading..."
    render->Present();
    pSavegameList->Initialize();
    if (pSaveListPosition > (signed int)uNumSavegameFiles)
    {
        pSaveListPosition = 0;
        uLoadGameUI_SelectedSlot = 0;
    }
    LODWriteableFile pLODFile;
    pLODFile.AllocSubIndicesAndIO(300, 0);
    Assert(sizeof(SavegameHeader) == 100);
    for (uint i = 0; i < uNumSavegameFiles; ++i)
    {
        String str = "saves\\" + pSavegameList->pFileList[i];
        str = MakeDataPath(str.c_str());
        if (_access(str.c_str(), 6))
        {
            pSavegameUsedSlots[i] = 0;
            strcpy(pSavegameHeader[i].pName, localization->GetString(72)); // "Empty"
            continue;
        }
        pLODFile.LoadFile(str.c_str(), 1);
        if (pLODFile.FindContainer("header.bin", true))
            fread(&pSavegameHeader[i], 100, 1, pLODFile.FindContainer("header.bin", true));
        if (!_stricmp(pSavegameList->pFileList[i].c_str(), localization->GetString(613))) {  // "AutoSave.MM7"
            strcpy(pSavegameHeader[i].pName, localization->GetString(16));  // "Autosave"
        }
        if (!pLODFile.FindContainer("image.pcx", true)) {
            pSavegameUsedSlots[i] = 0;
            pSavegameList->pFileList[i].clear();
        } else {
            pSavegameThumbnails[i] = Image::Create(new PCX_LOD_File_Loader(&pLODFile, "image.pcx"));
            if (pSavegameThumbnails[i]->GetWidth() == 0) {
                pSavegameThumbnails[i]->Release();
                pSavegameThumbnails[i] = nullptr;
            }
            pLODFile.CloseWriteFile();
            pSavegameUsedSlots[i] = 1;
        }
    }

    pLODFile.FreeSubIndexAndIO();

    saveload_ui_x_d = assets->GetImage_16BitAlpha("x_d");

    if (saveload_ui_ls_saved)
    {
        saveload_ui_ls_saved->Release();
        saveload_ui_ls_saved = nullptr;
    }
    if (current_screen_type == SCREEN_SAVEGAME)
        saveload_ui_ls_saved = assets->GetImage_16BitAlpha("LS_saveD");
    else
        saveload_ui_ls_saved = assets->GetImage_16BitAlpha("LS_loadD");

    if (!ui_ar_up_dn)
        ui_ar_up_dn = assets->GetImage_16BitAlpha("AR_UP_DN");
    if (!ui_ar_dn_dn)
        ui_ar_dn_dn = assets->GetImage_16BitAlpha("AR_DN_DN");

    CreateButton(21, 198, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 0, 0, "", 0);
    CreateButton(21, 219, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 1, 0, "", 0);
    CreateButton(21, 240, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 2, 0, "", 0);
    CreateButton(21, 261, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 3, 0, "", 0);
    CreateButton(21, 282, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 4, 0, "", 0);
    CreateButton(21, 303, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 5, 0, "", 0);
    CreateButton(21, 324, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 6, 0, "", 0);

    pBtnLoadSlot = CreateButton(241, 302, 105, 40, 1, 0, UIMSG_SaveLoadBtn, 0, 0, "", saveload_ui_ls_saved, 0);
    pBtnCancel = CreateButton(350, 302, 105, 40, 1, 0, UIMSG_Cancel, 0, 0, "", saveload_ui_x_d, 0);
    pBtnArrowUp = CreateButton(215, 199, 17, 17, 1, 0, UIMSG_ArrowUp, 0, 0, "", ui_ar_up_dn, 0);
    pBtnDownArrow = CreateButton(215, 323, 17, 17, 1, 0, UIMSG_DownArrow, uNumSavegameFiles, 0, "", ui_ar_dn_dn, 0);
}


void GUIWindow_Load::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
// {
//     LoadUI_Draw();
// }

// ----- (004606F7) --------------------------------------------------------
// void LoadUI_Draw()
// {
    UI_DrawSaveLoad(false);
}


//----- (004601B7) --------------------------------------------------------
static void UI_DrawSaveLoad(bool save)
{
    //  const char *pSlotName; // edi@36
    GUIWindow save_load_window; // [sp+Ch] [bp-78h]@8
    unsigned int pSaveFiles; // [sp+70h] [bp-14h]@10

    render->BeginScene();
    if (GetCurrentMenuID() != MENU_SAVELOAD && GetCurrentMenuID() != MENU_LoadingProcInMainMenu)
    {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, saveload_ui_loadsave);
        if (save)
        {
            render->DrawTextureAlphaNew(241 / 640.0f, 302 / 480.0f, saveload_ui_saveu);
            render->DrawTextureAlphaNew(18 / 640.0f, 139 / 480.0f, saveload_ui_save_up);
        }
        else
        {
            render->DrawTextureAlphaNew(241 / 640.0f, 302 / 480.0f, saveload_ui_loadu);
            render->DrawTextureAlphaNew(18 / 640.0f, 139 / 480.0f, saveload_ui_load_up);
        }
        render->DrawTextureAlphaNew(351 / 640.0f, 302 / 480.0f, saveload_ui_x_u);
    }

    if (pSavegameUsedSlots[uLoadGameUI_SelectedSlot])
    {
        memset(&save_load_window, 0, 0x54);
        save_load_window.uFrameX = pGUIWindow_CurrentMenu->uFrameX + 240;
        save_load_window.uFrameWidth = 220;
        save_load_window.uFrameY = (pGUIWindow_CurrentMenu->uFrameY - pFontSmallnum->GetHeight()) + 157;
        save_load_window.uFrameZ = save_load_window.uFrameX + 219;
        save_load_window.uFrameHeight = pFontSmallnum->GetHeight();
        save_load_window.uFrameW = pFontSmallnum->GetHeight() + save_load_window.uFrameY - 1;
        if (pSavegameThumbnails[uLoadGameUI_SelectedSlot])
            render->DrawTextureNew((pGUIWindow_CurrentMenu->uFrameX + 276) / 640.0f, (pGUIWindow_CurrentMenu->uFrameY + 171) / 480.0f, pSavegameThumbnails[uLoadGameUI_SelectedSlot]);
        //Draw map name
        save_load_window.DrawTitleText(pFontSmallnum, 0, 0, 0, pMapStats->pInfos[pMapStats->GetMapInfo(pSavegameHeader[uLoadGameUI_SelectedSlot].pLocationName)].pName, 3);

        //Draw date
        auto savegame_time = pSavegameHeader[uLoadGameUI_SelectedSlot].playing_time;
        auto savegame_hour = savegame_time.GetHoursOfDay();

        save_load_window.uFrameY = pGUIWindow_CurrentMenu->uFrameY + 261;
        int am;
        if (savegame_hour >= 12)
        {
            savegame_hour -= 12;
            if (!savegame_hour)
                savegame_hour = 12;
            am = 1;
        }
        else
            am = 0;

        auto str = StringPrintf(
            "%s %d:%02d %s\n%d %s %d",
            localization->GetDayName(savegame_time.GetDaysOfWeek()),
            savegame_hour,
            savegame_time.GetMinutesFraction(),
            localization->GetAmPm(am),
            savegame_time.GetDaysOfMonth() + 1,
            localization->GetMonthName(savegame_time.GetMonthsOfYear()),
            savegame_time.GetYears() + game_starting_year
        );
        save_load_window.DrawTitleText(pFontSmallnum, 0, 0, 0, str, 3);
    }
    if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 == WINDOW_INPUT_CONFIRMED)
    {
        pGUIWindow_CurrentMenu->receives_keyboard_input_2 = WINDOW_INPUT_NONE;
        strcpy((char *)&pSavegameHeader + 100 * uLoadGameUI_SelectedSlot, pKeyActionMap->pPressedKeysBuffer);
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_SaveGame, 0, 0);
    }
    else
    {
        if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 == WINDOW_INPUT_CANCELLED)
            pGUIWindow_CurrentMenu->receives_keyboard_input_2 = WINDOW_INPUT_NONE;
    }
    if (GetCurrentMenuID() == MENU_LoadingProcInMainMenu)
    {
        pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, pFontSmallnum->AlignText_Center(186, localization->GetString(135)) + 25,
            220, 0, localization->GetString(135), 0, 0, 0); // Загрузка
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontSmallnum, pFontSmallnum->AlignText_Center(186,
            pSavegameHeader[uLoadGameUI_SelectedSlot].pName) + 25, 0x106, 0, pSavegameHeader[uLoadGameUI_SelectedSlot].pName, 185, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, pFontSmallnum->AlignText_Center(186, localization->GetString(165)) + 25,
            304, 0, localization->GetString(165), 0, 0, 0); // Пожалуйста, пожождите
    }
    else
    {
        if (save)
            pSaveFiles = 40;
        else
            pSaveFiles = uNumSavegameFiles;

        int slot_Y = 199;
        for (uint i = pSaveListPosition; i < pSaveFiles; ++i)
        {
            if (slot_Y >= 346)
                break;
            if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 != WINDOW_INPUT_IN_PROGRESS || i != uLoadGameUI_SelectedSlot)
                pGUIWindow_CurrentMenu->DrawTextInRect(pFontSmallnum, 27, slot_Y, i == uLoadGameUI_SelectedSlot ? Color16(0xFF, 0xFF, 0x64) : 0, pSavegameHeader[i].pName, 185, 0);
            else
                pGUIWindow_CurrentMenu->DrawFlashingInputCursor(pGUIWindow_CurrentMenu->DrawTextInRect(pFontSmallnum, 27, slot_Y, i == uLoadGameUI_SelectedSlot ? Color16(0xFF, 0xFF, 0x64) : 0, (const char *)pKeyActionMap->pPressedKeysBuffer, 175, 1) + 27,
                    slot_Y, pFontSmallnum);
            slot_Y += 21;
        }
    }
    render->EndScene();
}