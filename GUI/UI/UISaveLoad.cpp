#include "GUI/UI/UISaveLoad.h"

#include <io.h>
#include <string>

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/LOD.h"
#include "Engine/SaveLoad.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Viewport.h"

#include "IO/Keyboard.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIMainMenu.h"

#include "Platform/Api.h"


void UI_DrawSaveLoad(bool save);

Image *saveload_ui_ls_saved = nullptr;
Image *saveload_ui_x_d = nullptr;
Image *scrollstop = nullptr;

GUIWindow_Save::GUIWindow_Save() :
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0) {
    memset(&pSavegameUsedSlots, 0, sizeof(pSavegameUsedSlots));
    memset(&pSavegameThumbnails, 0, sizeof(pSavegameThumbnails));

    saveload_ui_loadsave = assets->GetImage_ColorKey("loadsave", 0x7FF);
    saveload_ui_save_up = assets->GetImage_ColorKey("save_up", 0x7FF);
    saveload_ui_saveu = assets->GetImage_ColorKey("LS_saveU", 0x7FF);
    saveload_ui_x_u = assets->GetImage_ColorKey("x_u", 0x7FF);

    pSavegameList->Initialize();

    pSaveListPosition = 0;
    uLoadGameUI_SelectedSlot = 0;

    LOD::File pLODFile;
    for (uint i = 0; i < MAX_SAVE_SLOTS; ++i) {
        String file_name = pSavegameList->pFileList[i];
        if (file_name.empty()) {
            file_name = "1.mm7";
        }

        String str = "saves\\" + file_name;
        str = MakeDataPath(str.c_str());
        if (_access(str.c_str(), 0) || _access(str.c_str(), 6)) {
            pSavegameUsedSlots[i] = 0;
            strcpy(pSavegameHeader[i].pName, localization->GetString(72));  // Empty
        } else {
            pLODFile.Open(str);
            void *data = pLODFile.LoadRaw("header.bin");
            memcpy(&pSavegameHeader[i], data, sizeof(SavegameHeader));

            if (pSavegameHeader[i].pName[0] == '\0') {
                // blank so add something - suspect quicksaves
                String newname = pSavegameList->pFileList[i];
                String test = newname.substr(0, newname.size() - 4);
                strcpy(pSavegameHeader[i].pName, test.c_str());
            }

            pSavegameThumbnails[i] = Image::Create(new PCX_LOD_File_Loader(&pLODFile, "image.pcx"));
            if (pSavegameThumbnails[i]->GetWidth() == 0) {
                pSavegameThumbnails[i]->Release();
                pSavegameThumbnails[i] = nullptr;
            }

            if (pSavegameThumbnails[i] != nullptr) {
                pSavegameUsedSlots[i] = 1;
            } else {
                pSavegameUsedSlots[i] = 0;
            }
        }
    }

    saveload_ui_x_d = assets->GetImage_Alpha("x_d");
    saveload_ui_ls_saved = assets->GetImage_Alpha("LS_saveD");
    ui_ar_up_dn = assets->GetImage_Alpha("ar_up_dn");
    ui_ar_dn_dn = assets->GetImage_Alpha("ar_dn_dn");

    scrollstop = assets->GetImage_ColorKey("con_x", 0x7FF);

    // GUIWindow_Save c-tor --- part
    CreateButton(21, 198, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 0, 0, "");
    CreateButton(21, 218, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 1, 0, "");
    CreateButton(21, 238, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 2, 0, "");
    CreateButton(21, 258, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 3, 0, "");
    CreateButton(21, 278, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 4, 0, "");
    CreateButton(21, 298, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 5, 0, "");
    CreateButton(21, 318, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 6, 0, "");

    pBtnLoadSlot = CreateButton(241, 302, 105, 40, 1, 0, UIMSG_SaveLoadBtn, 0, 0, "", { { saveload_ui_ls_saved } });
    pBtnCancel = CreateButton(350, 302, 105, 40, 1, 0, UIMSG_Cancel, 0, 0, "", { { saveload_ui_x_d } });
    pBtnArrowUp = CreateButton(215, 199, 17, 17, 1, 0, UIMSG_ArrowUp, 0, 0, "", { { ui_ar_up_dn } });
    pBtnDownArrow = CreateButton(215, 323, 17, 17, 1, 0, UIMSG_DownArrow, MAX_SAVE_SLOTS, 0, "", { { ui_ar_dn_dn } });
}

void GUIWindow_Save::Update() {
    if (GetCurrentMenuID() != MENU_SAVELOAD && GetCurrentMenuID() != MENU_LoadingProcInMainMenu) {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, saveload_ui_loadsave);
        render->DrawTextureAlphaNew(241 / 640.0f, 302 / 480.0f, saveload_ui_saveu);
        render->DrawTextureAlphaNew(18 / 640.0f, 139 / 480.0f, saveload_ui_save_up);
        render->DrawTextureAlphaNew(351 / 640.0f, 302 / 480.0f, saveload_ui_x_u);
    }
    UI_DrawSaveLoad(true);
}

GUIWindow_Load::GUIWindow_Load(bool ingame) :
    GUIWindow(0, 0, 0, 0, 0) {
    current_screen_type = SCREEN_LOADGAME;

    dword_6BE138 = -1;
    pIcons_LOD->_inlined_sub2();

    memset(pSavegameUsedSlots.data(), 0, sizeof(pSavegameUsedSlots));
    memset(pSavegameThumbnails.data(), 0, MAX_SAVE_SLOTS * sizeof(Image *));

    saveload_ui_loadsave = assets->GetImage_ColorKey("loadsave", 0x7FF);
    saveload_ui_load_up = assets->GetImage_ColorKey("load_up", 0x7FF);
    saveload_ui_loadu = assets->GetImage_ColorKey("LS_loadU", 0x7FF);
    saveload_ui_x_u = assets->GetImage_ColorKey("x_u", 0x7FF);

    main_menu_background = nullptr;
    if (!ingame) {
        main_menu_background = assets->GetImage_PCXFromIconsLOD("lsave640.pcx");
    }

    // GUIWindow::GUIWindow
    this->uFrameX = saveload_dlg_xs[ingame ? 1 : 0];
    this->uFrameY = saveload_dlg_ys[ingame ? 1 : 0];
    this->uFrameWidth = saveload_dlg_zs[ingame ? 1 : 0];
    this->uFrameHeight = saveload_dlg_ws[ingame ? 1 : 0];
    this->uFrameZ = uFrameX + uFrameWidth - 1;
    this->uFrameW = uFrameY + uFrameHeight - 1;

    DrawText(pFontSmallnum, 25, 199, 0, localization->GetString(505), 0, 0, 0);  // "Reading..."
    render->Present();

    pSavegameList->Initialize();
    // if (pSaveListPosition > (int)uNumSavegameFiles) {
        pSaveListPosition = 0;
        uLoadGameUI_SelectedSlot = 0;
    //}
    LOD::File pLODFile;
    Assert(sizeof(SavegameHeader) == 100);
    for (uint i = 0; i < uNumSavegameFiles; ++i) {
        String str = "saves\\" + pSavegameList->pFileList[i];
        str = MakeDataPath(str.c_str());
        if (_access(str.c_str(), 6)) {
            pSavegameUsedSlots[i] = 0;
            strcpy(pSavegameHeader[i].pName, localization->GetString(72));  // "Empty"
            continue;
        }

        if (!pLODFile.Open(str)) __debugbreak();
        void *data = pLODFile.LoadRaw("header.bin");
        memcpy(&pSavegameHeader[i], data, sizeof(SavegameHeader));
        if (!_stricmp(pSavegameList->pFileList[i].c_str(), localization->GetString(613))) {  // "AutoSave.MM7"
            strcpy(pSavegameHeader[i].pName, localization->GetString(16));  // "Autosave"
        }

        if (pSavegameHeader[i].pName[0] == '\0') {
            // blank so add something - suspect quicksaves
            String newname = pSavegameList->pFileList[i];
            String test = newname.substr(0, newname.size() - 4);
            strcpy(pSavegameHeader[i].pName, test.c_str());
        }

        pSavegameThumbnails[i] = Image::Create(new PCX_LOD_File_Loader(&pLODFile, "image.pcx"));
        if (pSavegameThumbnails[i]->GetWidth() == 0) {
            pSavegameThumbnails[i]->Release();
            pSavegameThumbnails[i] = nullptr;
        }

        if (pSavegameThumbnails[i] != nullptr) {
            pSavegameUsedSlots[i] = 1;
        } else {
            pSavegameUsedSlots[i] = 0;
            pSavegameList->pFileList[i].clear();
        }
    }

    saveload_ui_x_d = assets->GetImage_Alpha("x_d");
    saveload_ui_ls_saved = assets->GetImage_Alpha("LS_loadD");
    ui_ar_up_dn = assets->GetImage_Alpha("AR_UP_DN");
    ui_ar_dn_dn = assets->GetImage_Alpha("AR_DN_DN");

    scrollstop = assets->GetImage_ColorKey("con_x", 0x7FF);

    CreateButton(21, 198, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 0, 0, "");
    CreateButton(21, 219, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 1, 0, "");
    CreateButton(21, 240, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 2, 0, "");
    CreateButton(21, 261, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 3, 0, "");
    CreateButton(21, 282, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 4, 0, "");
    CreateButton(21, 303, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 5, 0, "");
    CreateButton(21, 324, 191, 18, 1, 0, UIMSG_SelectLoadSlot, 6, 0, "");

    pBtnLoadSlot = CreateButton(241, 302, 105, 40, 1, 0, UIMSG_SaveLoadBtn, 0, 0, "", { { saveload_ui_ls_saved } });
    pBtnCancel = CreateButton(350, 302, 105, 40, 1, 0, UIMSG_Cancel, 0, 0, "", { { saveload_ui_x_d } });
    pBtnArrowUp = CreateButton(215, 199, 17, 17, 1, 0, UIMSG_ArrowUp, 0, 0, "", { { ui_ar_up_dn } });
    pBtnDownArrow = CreateButton(215, 323, 17, 17, 1, 0, UIMSG_DownArrow, uNumSavegameFiles, 0, "", { { ui_ar_dn_dn } });
}

void GUIWindow_Load::Update() {
    if (main_menu_background != nullptr) {
        render->DrawTextureNew(0, 0, main_menu_background);
    }
    if (GetCurrentMenuID() != MENU_SAVELOAD && GetCurrentMenuID() != MENU_LoadingProcInMainMenu) {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, saveload_ui_loadsave);
        render->DrawTextureAlphaNew(241 / 640.0f, 302 / 480.0f, saveload_ui_loadu);
        render->DrawTextureAlphaNew(18 / 640.0f, 139 / 480.0f, saveload_ui_load_up);
        render->DrawTextureAlphaNew(351 / 640.0f, 302 / 480.0f, saveload_ui_x_u);
    }
    UI_DrawSaveLoad(false);
}

static void UI_DrawSaveLoad(bool save) {
    GUIWindow save_load_window;
    unsigned int pSaveFiles;

    if (pSavegameUsedSlots[uLoadGameUI_SelectedSlot]) {
        memset(&save_load_window, 0, 0x54);
        save_load_window.uFrameX = pGUIWindow_CurrentMenu->uFrameX + 240;
        save_load_window.uFrameWidth = 220;
        save_load_window.uFrameY = (pGUIWindow_CurrentMenu->uFrameY - pFontSmallnum->GetHeight()) + 157;
        save_load_window.uFrameZ = save_load_window.uFrameX + 219;
        save_load_window.uFrameHeight = pFontSmallnum->GetHeight();
        save_load_window.uFrameW = pFontSmallnum->GetHeight() + save_load_window.uFrameY - 1;
        if (pSavegameThumbnails[uLoadGameUI_SelectedSlot])
            render->DrawTextureNew((pGUIWindow_CurrentMenu->uFrameX + 276) / 640.0f, (pGUIWindow_CurrentMenu->uFrameY + 171) / 480.0f, pSavegameThumbnails[uLoadGameUI_SelectedSlot]);
        // Draw map name
        save_load_window.DrawTitleText(pFontSmallnum, 0, 0, 0, pMapStats->pInfos[pMapStats->GetMapInfo(pSavegameHeader[uLoadGameUI_SelectedSlot].pLocationName)].pName, 3);

        // Draw date
        GameTime savegame_time = pSavegameHeader[uLoadGameUI_SelectedSlot].playing_time;
        auto savegame_hour = savegame_time.GetHoursOfDay();

        save_load_window.uFrameY = pGUIWindow_CurrentMenu->uFrameY + 261;
        int am;
        if (savegame_hour >= 12) {
            savegame_hour -= 12;
            if (!savegame_hour) {
                savegame_hour = 12;
            }
            am = 1;
        } else {
            am = 0;
        }

        auto str = StringPrintf(
            "%s %d:%02d %s\n%d %s %d",
            localization->GetDayName(savegame_time.GetDaysOfWeek()),
            savegame_hour,
            savegame_time.GetMinutesFraction(),
            localization->GetAmPm(am),
            savegame_time.GetDaysOfMonth() + 1,
            localization->GetMonthName(savegame_time.GetMonthsOfYear()),
            savegame_time.GetYears() + game_starting_year);
        save_load_window.DrawTitleText(pFontSmallnum, 0, 0, 0, str, 3);
    }

    if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 == WINDOW_INPUT_CONFIRMED) {
        pGUIWindow_CurrentMenu->receives_keyboard_input_2 = WINDOW_INPUT_NONE;
        strcpy((char *)&pSavegameHeader + 100 * uLoadGameUI_SelectedSlot, pKeyActionMap->pPressedKeysBuffer);
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_SaveGame, 0, 0);
    } else {
        if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 == WINDOW_INPUT_CANCELLED)
            pGUIWindow_CurrentMenu->receives_keyboard_input_2 = WINDOW_INPUT_NONE;
    }

    if (GetCurrentMenuID() == MENU_LoadingProcInMainMenu) {
        pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, pFontSmallnum->AlignText_Center(186, localization->GetString(135)) + 25,
            220, 0, localization->GetString(135), 0, 0, 0);  // Çàãðóçêà
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontSmallnum, pFontSmallnum->AlignText_Center(186,
            pSavegameHeader[uLoadGameUI_SelectedSlot].pName) + 25, 0x106, 0, pSavegameHeader[uLoadGameUI_SelectedSlot].pName, 185, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontSmallnum, pFontSmallnum->AlignText_Center(186, localization->GetString(165)) + 25,
            304, 0, localization->GetString(165), 0, 0, 0);  // Ïîæàëóéñòà, ïîæîæäèòå
    } else {
        if (save) {
            pSaveFiles = MAX_SAVE_SLOTS;

            // ingame save scroll bar
            float ypos3 = (float(pSaveListPosition) / (pSaveFiles - 7)) * 89.f;
            render->DrawTextureAlphaNew(216 / 640.f, (217 + ypos3) / 480.f, scrollstop);
        } else {
            pSaveFiles = uNumSavegameFiles;

            // load scroll bar
            float ypos = (float(pSaveListPosition) / (pSaveFiles - 7)) * 89.f;
            render->DrawTextureAlphaNew((216+ pGUIWindow_CurrentMenu->uFrameX) / 640.f, (217 + pGUIWindow_CurrentMenu->uFrameY + ypos) / 480.f, scrollstop);
        }

        int slot_Y = 199;
        for (uint i = pSaveListPosition; i < pSaveFiles; ++i) {
            if (slot_Y >= 346) {
                break;
            }
            if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 != WINDOW_INPUT_IN_PROGRESS || i != uLoadGameUI_SelectedSlot) {
                pGUIWindow_CurrentMenu->DrawTextInRect(pFontSmallnum, 27, slot_Y, i == uLoadGameUI_SelectedSlot ? Color16(0xFF, 0xFF, 0x64) : 0, pSavegameHeader[i].pName, 185, 0);
            } else {
                pGUIWindow_CurrentMenu->DrawFlashingInputCursor(pGUIWindow_CurrentMenu->DrawTextInRect(pFontSmallnum, 27, slot_Y, i == uLoadGameUI_SelectedSlot ? Color16(0xFF, 0xFF, 0x64) : 0, (const char *)pKeyActionMap->pPressedKeysBuffer, 175, 1) + 27,
                    slot_Y, pFontSmallnum);
            }
            slot_Y += 21;
        }
    }
}

void MainMenuLoad_EventLoop() {
    while (!pMessageQueue_50CBD0->Empty()) {
        UIMessageType msg;
        int param, param2;
        pMessageQueue_50CBD0->PopMessage(&msg, &param, &param2);

        switch (msg) {
        case UIMSG_LoadGame: {
            if (!pSavegameUsedSlots[uLoadGameUI_SelectedSlot])
                break;
            SetCurrentMenuID(MENU_LoadingProcInMainMenu);
            break;
        }
        case UIMSG_SelectLoadSlot: {
            // main menu save/load wnd   clicking on savegame lines
            if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 == WINDOW_INPUT_IN_PROGRESS)
                pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_NONE);
            if (current_screen_type != SCREEN_SAVEGAME || uLoadGameUI_SelectedSlot != param + pSaveListPosition) {
                // load clicked line
                int v26 = param + pSaveListPosition;
                if (dword_6BE138 == v26) {
                    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_SaveLoadBtn, 0, 0);
                    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_LoadGame, 0, 0);
                }
                uLoadGameUI_SelectedSlot = v26;
                dword_6BE138 = v26;
            } else {
                // typing in the line
                pKeyActionMap->EnterText(0, 19, pGUIWindow_CurrentMenu);
                strcpy(pKeyActionMap->pPressedKeysBuffer, pSavegameHeader[uLoadGameUI_SelectedSlot].pName);
                pKeyActionMap->uNumKeysPressed = strlen(pKeyActionMap->pPressedKeysBuffer);
            }
            break;
        }
        case UIMSG_SaveLoadBtn: {
            new OnSaveLoad(pGUIWindow_CurrentMenu->uFrameX + 241, pGUIWindow_CurrentMenu->uFrameY + 302, 61, 28, (int)pBtnLoadSlot);
            break;
        }
        case UIMSG_DownArrow: {
            ++pSaveListPosition;
            if (pSaveListPosition > (param - 7))
                pSaveListPosition = (param - 7);
            // if (pSaveListPosition < 1)
             //   pSaveListPosition = 0;
            new OnButtonClick2(pGUIWindow_CurrentMenu->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 323, 0, 0, (int)pBtnDownArrow);
            break;
        }
        case UIMSG_ArrowUp: {
            --pSaveListPosition;
            if (pSaveListPosition < 0)
                pSaveListPosition = 0;
            new OnButtonClick2(pGUIWindow_CurrentMenu->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 197, 0, 0, (int)pBtnArrowUp);
            break;
        }
        case UIMSG_Cancel: {
            new OnCancel3(pGUIWindow_CurrentMenu->uFrameX + 350, pGUIWindow_CurrentMenu->uFrameY + 302, 61, 28, (int)pBtnCancel);
            break;
        }
        case UIMSG_Escape: {
            viewparams->field_48 = 1;
            // if (current_screen_type == SCREEN_LOADGAME)
            {
                // crt_deconstruct_ptr_6A0118();

                SetCurrentMenuID(MENU_MAIN);
                current_screen_type = SCREEN_GAME;
                pEventTimer->Resume();
                viewparams->bRedrawGameUI = true;
                break;
            }
            break;
        }
        }
    }
}

void MainMenuLoad_Loop() {
    current_screen_type = SCREEN_LOADGAME;
    pGUIWindow_CurrentMenu = new GUIWindow_Load(false);

    while (GetCurrentMenuID() == MENU_SAVELOAD && current_screen_type == SCREEN_LOADGAME) {
        window->PeekMessageLoop();
        if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE) {
            OS_WaitMessage();
            continue;
        }

        render->BeginScene();
        GUI_UpdateWindows();
        MainMenuLoad_EventLoop();
        render->Present();
    }

    pGUIWindow_CurrentMenu->Release();
    pGUIWindow_CurrentMenu = nullptr;
}
