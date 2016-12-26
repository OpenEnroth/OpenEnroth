#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/SaveLoad.h"
#include "Engine/LOD.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"

#include "IO/Keyboard.h"

#include "GUI/UI/UISaveLoad.h"

#include "Game/MainMenu.h"
#include "Game/MainMenuLoad.h"

#include "Platform/Api.h"

void MainMenuLoad_EventLoop()
{
    while (pMessageQueue_50CBD0->uNumMessages)
    {
        UIMessageType msg;
        int param, param2;
        pMessageQueue_50CBD0->PopMessage(&msg, &param, &param2);

        switch (msg)
        {
            case UIMSG_LoadGame:
            {
                if (!pSavegameUsedSlots[uLoadGameUI_SelectedSlot])
                    break;
                SetCurrentMenuID(MENU_LoadingProcInMainMenu);
            }
            break;

            case UIMSG_SelectLoadSlot:
            {
                //main menu save/load wnd   clicking on savegame lines
                if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 == WINDOW_INPUT_IN_PROGRESS)
                    pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_NONE);
                if (current_screen_type != SCREEN_SAVEGAME || uLoadGameUI_SelectedSlot != param + pSaveListPosition)
                {
                    //load clicked line
                    int v26 = param + pSaveListPosition;
                    if (dword_6BE138 == v26)
                    {
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_SaveLoadBtn, 0, 0);
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_LoadGame, 0, 0);
                    }
                    uLoadGameUI_SelectedSlot = v26;
                    dword_6BE138 = v26;
                }
                else
                {
                    //typing in the line
                    pKeyActionMap->EnterText(0, 19, pGUIWindow_CurrentMenu);
                    strcpy(pKeyActionMap->pPressedKeysBuffer, pSavegameHeader[uLoadGameUI_SelectedSlot].pName);
                    pKeyActionMap->uNumKeysPressed = strlen(pKeyActionMap->pPressedKeysBuffer);
                }
            }
            break;

            case UIMSG_SaveLoadBtn:
                new OnSaveLoad(pGUIWindow_CurrentMenu->uFrameX + 241, pGUIWindow_CurrentMenu->uFrameY + 302, 61, 28, (int)pBtnLoadSlot, 0);
            break;


            case UIMSG_DownArrow:
            {
                ++pSaveListPosition;
                if (pSaveListPosition >= param)
                    pSaveListPosition = param - 1;
                if (pSaveListPosition < 1)
                    pSaveListPosition = 0;
                new OnButtonClick2(pGUIWindow_CurrentMenu->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 323, 0, 0, (int)pBtnDownArrow, 0);
            }
            break;


            case UIMSG_ArrowUp:
            {
                --pSaveListPosition;
                if (pSaveListPosition < 0)
                    pSaveListPosition = 0;
                new OnButtonClick2(pGUIWindow_CurrentMenu->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 197, 0, 0, (int)pBtnArrowUp, 0);
            }
            break;

            case UIMSG_Cancel:
                new OnCancel3(pGUIWindow_CurrentMenu->uFrameX + 350, pGUIWindow_CurrentMenu->uFrameY + 302, 61, 28, (int)pBtnCancel, 0);
            break;


            case UIMSG_Escape:
            {
                viewparams->field_48 = 1;
                //if (current_screen_type == SCREEN_LOADGAME)
                {
                    //crt_deconstruct_ptr_6A0118();

                    if (main_menu_background)
                    {
                        main_menu_background->Release();
                        main_menu_background = nullptr;
                    }
                    main_menu_background = assets->GetImage_PCXFromIconsLOD("title.pcx");

                    SetCurrentMenuID(MENU_MAIN);
                    current_screen_type = SCREEN_GAME;
                    pEventTimer->Resume();
                    viewparams->bRedrawGameUI = true;
                    break;
                }
            }
            break;
        }
    }
}

void MainMenuLoad_Loop()
{
    current_screen_type = SCREEN_LOADGAME;

    if (main_menu_background)
    {
        main_menu_background->Release();
        main_menu_background = nullptr;
    }
    main_menu_background = assets->GetImage_PCXFromIconsLOD("lsave640.pcx");

        //LoadUI_Load(0);
    pGUIWindow_CurrentMenu = new GUIWindow_Load(false);


    while (GetCurrentMenuID() == MENU_SAVELOAD && current_screen_type == SCREEN_LOADGAME)
    {
        OS_PeekMessageLoop();
        if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE)
        {
            OS_WaitMessage();
            continue;
        }

        render->BeginScene();
        render->DrawTextureNew(0, 0, main_menu_background);

        MainMenuLoad_EventLoop();
        GUI_UpdateWindows();

        render->EndScene();
        render->Present();
    }

    pGUIWindow_CurrentMenu->Release();
    pGUIWindow_CurrentMenu = nullptr;
}