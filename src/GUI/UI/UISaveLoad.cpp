#include "GUI/UI/UISaveLoad.h"

#include <string>
#include <filesystem>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/MapInfo.h"
#include "Engine/SaveLoad.h"

#include "Io/KeyboardInputHandler.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIMainMenu.h"



using Io::TextInputType;

static void UI_DrawSaveLoad(bool save);

Image *saveload_ui_ls_saved = nullptr;
Image *saveload_ui_x_d = nullptr;
Image *scrollstop = nullptr;

GUIWindow_Save::GUIWindow_Save() :
    GUIWindow(WINDOW_Save, {0, 0}, render->GetRenderDimensions(), 0) {
    memset(&pSavegameUsedSlots, 0, sizeof(pSavegameUsedSlots));
    memset(&pSavegameThumbnails, 0, sizeof(pSavegameThumbnails));

    saveload_ui_loadsave = assets->GetImage_ColorKey("loadsave");
    saveload_ui_save_up = assets->GetImage_ColorKey("save_up");
    saveload_ui_saveu = assets->GetImage_ColorKey("LS_saveU");
    saveload_ui_x_u = assets->GetImage_ColorKey("x_u");

    pSavegameList->Initialize();

    pSaveListPosition = 0;
    uLoadGameUI_SelectedSlot = 0;

    LOD::File pLODFile;
    for (uint i = 0; i < MAX_SAVE_SLOTS; ++i) {
        // std::string file_name = pSavegameList->pFileList[i];
        std::string file_name = fmt::format("save{:03}.mm7", i);
        if (file_name.empty()) {
            file_name = "1.mm7";
        }

        std::string str = MakeDataPath("saves", file_name);
        if (!std::filesystem::exists(str)) {
            pSavegameUsedSlots[i] = 0;
            strcpy(pSavegameHeader[i].pName, localization->GetString(LSTR_EMPTY_SAVESLOT));
        } else {
            pLODFile.Open(str);
            Blob data = pLODFile.LoadRaw("header.bin");
            memcpy(&pSavegameHeader[i], data.data(), sizeof(SavegameHeader));

            if (pSavegameHeader[i].pName[0] == '\0') {
                // blank so add something - suspect quicksaves
                std::string newname = pSavegameList->pFileList[i];
                std::string test = newname.substr(0, newname.size() - 4);
                strcpy(pSavegameHeader[i].pName, test.c_str());
            }

            pSavegameThumbnails[i] = render->CreateTexture_PCXFromLOD(&pLODFile, "image.pcx");
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

    scrollstop = assets->GetImage_ColorKey("con_x");

    // GUIWindow_Save c-tor --- part
    CreateButton({21, 198}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 0);
    CreateButton({21, 218}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 1);
    CreateButton({21, 238}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 2);
    CreateButton({21, 258}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 3);
    CreateButton({21, 278}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 4);
    CreateButton({21, 298}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 5);
    CreateButton({21, 318}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 6);

    pBtnLoadSlot = CreateButton({241, 302}, {105, 40}, 1, 0, UIMSG_SaveLoadBtn, 0, InputAction::Invalid, "", {saveload_ui_ls_saved});
    pBtnCancel = CreateButton({350, 302}, {105, 40}, 1, 0, UIMSG_Cancel, 0, InputAction::Invalid, "", {saveload_ui_x_d});
    pBtnArrowUp = CreateButton({215, 199}, {17, 17}, 1, 0, UIMSG_ArrowUp, 0, InputAction::Invalid, "", {ui_ar_up_dn});
    pBtnDownArrow = CreateButton({215, 323}, {17, 17}, 1, 0, UIMSG_DownArrow, MAX_SAVE_SLOTS, InputAction::Invalid, "", {ui_ar_dn_dn});

    CreateButton({215, 216}, {17, 107}, 1, 0, UIMSG_SaveLoadScroll, MAX_SAVE_SLOTS);
}

void GUIWindow_Save::Update() {
    if (GetCurrentMenuID() != MENU_SAVELOAD && GetCurrentMenuID() != MENU_LoadingProcInMainMenu) {
        render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, saveload_ui_loadsave);
        render->DrawTextureNew(241 / 640.0f, 302 / 480.0f, saveload_ui_saveu);
        render->DrawTextureNew(18 / 640.0f, 139 / 480.0f, saveload_ui_save_up);
        render->DrawTextureNew(351 / 640.0f, 302 / 480.0f, saveload_ui_x_u);
    }
    UI_DrawSaveLoad(true);
}

GUIWindow_Load::GUIWindow_Load(bool ingame) :
    GUIWindow(WINDOW_Load, {0, 0}, {0, 0}, 0) {
    current_screen_type = CURRENT_SCREEN::SCREEN_LOADGAME;

    dword_6BE138 = -1;
    pIcons_LOD->_inlined_sub2();

    memset(pSavegameUsedSlots.data(), 0, sizeof(pSavegameUsedSlots));
    memset(pSavegameThumbnails.data(), 0, MAX_SAVE_SLOTS * sizeof(Image *));

    saveload_ui_loadsave = assets->GetImage_ColorKey("loadsave");
    saveload_ui_load_up = assets->GetImage_ColorKey("load_up");
    saveload_ui_loadu = assets->GetImage_ColorKey("LS_loadU");
    saveload_ui_x_u = assets->GetImage_ColorKey("x_u");

    main_menu_background = nullptr;
    if (!ingame) {
        main_menu_background = assets->GetImage_PCXFromIconsLOD("lsave640.pcx");
        render->DrawTextureNew(0, 0, main_menu_background);
    } else {
        render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, saveload_ui_loadsave);
        render->DrawTextureNew(241 / 640.0f, 302 / 480.0f, saveload_ui_loadu);
        render->DrawTextureNew(18 / 640.0f, 139 / 480.0f, saveload_ui_load_up);
        render->DrawTextureNew(351 / 640.0f, 302 / 480.0f, saveload_ui_x_u);
    }

    // GUIWindow::GUIWindow
    this->uFrameX = saveload_dlg_xs[ingame ? 1 : 0];
    this->uFrameY = saveload_dlg_ys[ingame ? 1 : 0];
    this->uFrameWidth = saveload_dlg_zs[ingame ? 1 : 0];
    this->uFrameHeight = saveload_dlg_ws[ingame ? 1 : 0];
    this->uFrameZ = uFrameX + uFrameWidth - 1;
    this->uFrameW = uFrameY + uFrameHeight - 1;

    DrawText(pFontSmallnum, {25, 199}, 0, localization->GetString(LSTR_READING), 0, 0, 0);
    render->Present();

    pSavegameList->Initialize();
    // if (pSaveListPosition > (int)uNumSavegameFiles) {
        pSaveListPosition = 0;
        uLoadGameUI_SelectedSlot = 0;
    //}
    LOD::File pLODFile;
    Assert(sizeof(SavegameHeader) == 100);
    for (uint i = 0; i < uNumSavegameFiles; ++i) {
        std::string str = MakeDataPath("saves", pSavegameList->pFileList[i]);
        if (!std::filesystem::exists(str)) {
            pSavegameUsedSlots[i] = 0;
            strcpy(pSavegameHeader[i].pName, localization->GetString(LSTR_EMPTY_SAVESLOT));
            continue;
        }

        if (!pLODFile.Open(str)) __debugbreak();
        Blob data = pLODFile.LoadRaw("header.bin");
        memcpy(&pSavegameHeader[i], data.data(), sizeof(SavegameHeader));
        if (iequals(pSavegameList->pFileList[i], localization->GetString(LSTR_AUTOSAVE_MM7))) {
            strcpy(pSavegameHeader[i].pName, localization->GetString(LSTR_AUTOSAVE));
        }

        if (pSavegameHeader[i].pName[0] == '\0') {
            // blank so add something - suspect quicksaves
            std::string newname = pSavegameList->pFileList[i];
            std::string test = newname.substr(0, newname.size() - 4);
            strcpy(pSavegameHeader[i].pName, test.c_str());
        }

        // pSavegameThumbnails[i] = Image::Create(new PCX_LOD_Raw_Loader(&pLODFile, "image.pcx"));
        pSavegameThumbnails[i] = render->CreateTexture_PCXFromLOD(&pLODFile, "image.pcx");

        if (pSavegameThumbnails[i]->GetWidth() == 0) {
            pSavegameThumbnails[i]->Release();
            pSavegameThumbnails[i] = nullptr;
        }

        pSavegameUsedSlots[i] = 1;
        if (pSavegameThumbnails[i] != nullptr) {
           // pSavegameUsedSlots[i] = 1;
        } else {
            // pSavegameUsedSlots[i] = 0;
            // pSavegameList->pFileList[i].clear();
        }
    }

    saveload_ui_x_d = assets->GetImage_Alpha("x_d");
    saveload_ui_ls_saved = assets->GetImage_Alpha("LS_loadD");
    ui_ar_up_dn = assets->GetImage_Alpha("AR_UP_DN");
    ui_ar_dn_dn = assets->GetImage_Alpha("AR_DN_DN");

    scrollstop = assets->GetImage_ColorKey("con_x");

    CreateButton("LoadMenu_Slot0", {21, 198}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 0);
    CreateButton("LoadMenu_Slot1", {21, 219}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 1);
    CreateButton("LoadMenu_Slot2", {21, 240}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 2);
    CreateButton("LoadMenu_Slot3", {21, 261}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 3);
    CreateButton("LoadMenu_Slot4", {21, 282}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 4);
    CreateButton("LoadMenu_Slot5", {21, 303}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 5);
    CreateButton("LoadMenu_Slot6", {21, 324}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 6);

    pBtnLoadSlot = CreateButton("LoadMenu_Load", {241, 302}, {105, 40}, 1, 0, UIMSG_SaveLoadBtn, 0, InputAction::Invalid, "", {saveload_ui_ls_saved});
    pBtnCancel = CreateButton({350, 302}, {105, 40}, 1, 0, UIMSG_Cancel, 0, InputAction::Invalid, "", {saveload_ui_x_d});
    pBtnArrowUp = CreateButton({215, 199}, {17, 17}, 1, 0, UIMSG_ArrowUp, 0, InputAction::Invalid, "", {ui_ar_up_dn});
    pBtnDownArrow = CreateButton({215, 323}, {17, 17}, 1, 0, UIMSG_DownArrow, MAX_SAVE_SLOTS, InputAction::Invalid, "", {ui_ar_dn_dn});

    CreateButton({215, 216}, {17, 107}, 1, 0, UIMSG_SaveLoadScroll, uNumSavegameFiles);
}

void GUIWindow_Load::Update() {
    if (main_menu_background != nullptr) {
        render->DrawTextureNew(0, 0, main_menu_background);
    }
    if (GetCurrentMenuID() != MENU_SAVELOAD && GetCurrentMenuID() != MENU_LoadingProcInMainMenu) {
        render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, saveload_ui_loadsave);
        render->DrawTextureNew(241 / 640.0f, 302 / 480.0f, saveload_ui_loadu);
        render->DrawTextureNew(18 / 640.0f, 139 / 480.0f, saveload_ui_load_up);
        render->DrawTextureNew(351 / 640.0f, 302 / 480.0f, saveload_ui_x_u);
    }
    UI_DrawSaveLoad(false);
}

static void UI_DrawSaveLoad(bool save) {
    GUIWindow save_load_window;
    unsigned int pSaveFiles;

    if (pSavegameUsedSlots[uLoadGameUI_SelectedSlot]) {
        save_load_window.Init();
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

        auto str = fmt::format(
            "{} {}:{:02} {}\n{} {} {}",
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

    if (pGUIWindow_CurrentMenu->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CONFIRMED) {
        pGUIWindow_CurrentMenu->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
        strcpy(pSavegameHeader[uLoadGameUI_SelectedSlot].pName, keyboardInputHandler->GetTextInput().c_str());
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_SaveGame, 0, 0);
    } else {
        if (pGUIWindow_CurrentMenu->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CANCELLED)
            pGUIWindow_CurrentMenu->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
    }

    if (GetCurrentMenuID() == MENU_LoadingProcInMainMenu) {
        pGUIWindow_CurrentMenu->DrawText(pFontSmallnum,
            {pFontSmallnum->AlignText_Center(186, localization->GetString(LSTR_LOADING)) + 25, 220}, 0,
            localization->GetString(LSTR_LOADING), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontSmallnum,
            {pFontSmallnum->AlignText_Center(186, pSavegameHeader[uLoadGameUI_SelectedSlot].pName) + 25, 262}, 0,
            pSavegameHeader[uLoadGameUI_SelectedSlot].pName, 185, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontSmallnum,
            {pFontSmallnum->AlignText_Center(186, localization->GetString(LSTR_PLEASE_WAIT)) + 25, 304}, 0,
            localization->GetString(LSTR_PLEASE_WAIT), 0, 0, 0);
    } else {
        if (save) {
            pSaveFiles = MAX_SAVE_SLOTS;

            // ingame save scroll bar
            float ypos3 = (float(pSaveListPosition) / (pSaveFiles - 7)) * 89.f;
            render->DrawTextureNew(216 / 640.f, (217 + ypos3) / 480.f, scrollstop);
        } else {
            pSaveFiles = uNumSavegameFiles;

            // load scroll bar
            float ypos = (float(pSaveListPosition) / (pSaveFiles - 7)) * 89.f;
            render->DrawTextureNew((216+ pGUIWindow_CurrentMenu->uFrameX) / 640.f, (217 + pGUIWindow_CurrentMenu->uFrameY + ypos) / 480.f, scrollstop);
        }

        int slot_Y = 199;
        for (uint i = pSaveListPosition; i < pSaveFiles; ++i) {
            if (slot_Y >= 346) {
                break;
            }
            if (pGUIWindow_CurrentMenu->keyboard_input_status != WindowInputStatus::WINDOW_INPUT_IN_PROGRESS || i != uLoadGameUI_SelectedSlot) {
                pGUIWindow_CurrentMenu->DrawTextInRect(pFontSmallnum, {27, slot_Y}, i == uLoadGameUI_SelectedSlot ? colorTable.LaserLemon.C16() : 0, pSavegameHeader[i].pName, 185, 0);
            } else {
                pGUIWindow_CurrentMenu->DrawFlashingInputCursor(pGUIWindow_CurrentMenu->DrawTextInRect(pFontSmallnum, {27, slot_Y},
                    i == uLoadGameUI_SelectedSlot ? colorTable.LaserLemon.C16() : 0, keyboardInputHandler->GetTextInput().c_str(), 175, 1) + 27, slot_Y, pFontSmallnum);
            }
            slot_Y += 21;
        }
    }
}

void MainMenuLoad_EventLoop() {
    while (!pCurrentFrameMessageQueue->Empty()) {
        UIMessageType msg;
        int param, param2;
        pCurrentFrameMessageQueue->PopMessage(&msg, &param, &param2);

        switch (msg) {
        case UIMSG_LoadGame: {
            if (!pSavegameUsedSlots[uLoadGameUI_SelectedSlot])
                break;
            SetCurrentMenuID(MENU_LoadingProcInMainMenu);
            break;
        }
        case UIMSG_SelectLoadSlot: {
            // main menu save/load wnd   clicking on savegame lines
            if (pGUIWindow_CurrentMenu->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_IN_PROGRESS)
                keyboardInputHandler->SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_NONE);
            if (current_screen_type != CURRENT_SCREEN::SCREEN_SAVEGAME || uLoadGameUI_SelectedSlot != param + pSaveListPosition) {
                // load clicked line
                int v26 = param + pSaveListPosition;
                if (dword_6BE138 == v26) {
                    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_SaveLoadBtn, 0, 0);
                    // Breaks UI interaction after save load
                    // pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_LoadGame, 0, 0);
                }
                uLoadGameUI_SelectedSlot = v26;
                dword_6BE138 = v26;
            } else {
                // typing in the line
                keyboardInputHandler->StartTextInput(TextInputType::Text, 19, pGUIWindow_CurrentMenu);
                keyboardInputHandler->SetTextInput(pSavegameHeader[uLoadGameUI_SelectedSlot].pName);
            }
            break;
        }
        case UIMSG_SaveLoadBtn: {
            new OnSaveLoad({pGUIWindow_CurrentMenu->uFrameX + 241, pGUIWindow_CurrentMenu->uFrameY + 302}, {61, 28}, pBtnLoadSlot);
            break;
        }
        case UIMSG_DownArrow: {
            ++pSaveListPosition;
            if (pSaveListPosition > (param - 7))
                pSaveListPosition = (param - 7);
            // if (pSaveListPosition < 1)
             //   pSaveListPosition = 0;
            new OnButtonClick2({pGUIWindow_CurrentMenu->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 323}, {0, 0}, pBtnDownArrow);
            break;
        }
        case UIMSG_ArrowUp: {
            --pSaveListPosition;
            if (pSaveListPosition < 0)
                pSaveListPosition = 0;
            new OnButtonClick2({pGUIWindow_CurrentMenu->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 197}, {0, 0}, pBtnArrowUp);
            break;
        }
        case UIMSG_Cancel: {
            new OnCancel3({pGUIWindow_CurrentMenu->uFrameX + 350, pGUIWindow_CurrentMenu->uFrameY + 302}, {61, 28}, pBtnCancel);
            break;
        }
        case UIMSG_Escape: {
            // if (current_screen_type == SCREEN_LOADGAME)
            {
                // crt_deconstruct_ptr_6A0118();

                SetCurrentMenuID(MENU_MAIN);
                current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                pEventTimer->Resume();
                break;
            }
            break;
        }
        case UIMSG_SaveLoadScroll: {
            // pskelton add for scroll click
            int pSaveFiles{ static_cast<int>(uNumSavegameFiles) };
            int mx{}, my{};
            mouse->GetClickPos(&mx, &my);
            // 276 is offset down from top (216 + 60 frame)
            my -= 276;
            // 107 is total height of bar
            float fmy = static_cast<float>(my) / 107.0f;
            int newlistpost = std::round((param - 7) * fmy);
            newlistpost = std::clamp(newlistpost, 0, (param - 7));
            pSaveListPosition = newlistpost;
            new OnButtonClick2({pGUIWindow_CurrentMenu->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 197}, {0, 0}, pBtnArrowUp);
            break;
        }
        default:
            break;
        }
    }
}

void MainMenuLoad_Loop() {
    current_screen_type = CURRENT_SCREEN::SCREEN_LOADGAME;
    pGUIWindow_CurrentMenu = new GUIWindow_Load(false);

    while (GetCurrentMenuID() == MENU_SAVELOAD && current_screen_type == CURRENT_SCREEN::SCREEN_LOADGAME) {
        MessageLoopWithWait();

        render->BeginScene2D();
        GUI_UpdateWindows();
        MainMenuLoad_EventLoop();
        render->Present();
    }

    pGUIWindow_CurrentMenu->Release();
    pGUIWindow_CurrentMenu = nullptr;
}
