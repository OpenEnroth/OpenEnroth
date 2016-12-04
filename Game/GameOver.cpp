#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Timer.h"
#include "Engine/texts.h"
#include "Engine/Party.h"
#include "Engine/LOD.h"
#include "Engine/SaveLoad.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"

#include "IO/Mouse.h"
#include "IO/Keyboard.h"

#include "Media/Audio/AudioPlayer.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIPartyCreation.h"


//----- (004BF91E) --------------------------------------------------------
void GameOver_Loop(int v15)
{
    const char *v1; // eax@2
    unsigned int result; // eax@3
    const char *v6; // eax@10
    const char *v7; // edx@10
    const char *v8; // ecx@12
    const char *v9; // eax@14
    unsigned int v10; // eax@25
    GUIWindow pWindow; // [sp+34h] [bp-9Ch]@1
    unsigned int v14; // [sp+A4h] [bp-2Ch]@5
    const char *pInString; // [sp+ACh] [bp-24h]@5
    unsigned int v17; // [sp+B0h] [bp-20h]@5
    unsigned int v18; // [sp+B4h] [bp-1Ch]@5
    unsigned int v19; // [sp+B8h] [bp-18h]@5
    int v20; // [sp+BCh] [bp-14h]@7
    GUIFont *pFont; // [sp+C4h] [bp-Ch]@1
    unsigned __int64 v23; // [sp+C8h] [bp-8h]@5
    MSG msg;

    //RGBTexture _this; // [sp+Ch] [bp-C4h]@1
    //RGBTexture::RGBTexture(&this);

    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_4000;
    bGameoverLoop = true;
    pMediaPlayer->bStopBeforeSchedule = 0;
    pAudioPlayer->StopChannels(-1, -1);
    pRenderer->BeginScene();
    pRenderer->ClearBlack();
    pRenderer->EndScene();
    pRenderer->Present();
    //pMediaPlayer->pResetflag = 0;
    _449B57_test_bit(pParty->_quest_bits, 99);


    Image *background = assets->GetImage_PCXFromIconsLOD(L"winbg.pcx");
    {
        pRenderer->BeginScene();
        pRenderer->DrawTextureNew(0, 0, background);
        pRenderer->EndScene();
    }
    background->Release();
    background = nullptr;

    window_SpeakInHouse = new GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0, 0);
    pWindow.uFrameX = 75;
    pWindow.uFrameY = 60;
    pWindow.uFrameWidth = 469;
    pWindow.uFrameHeight = 338;
    pWindow.uFrameZ = 543;
    pWindow.uFrameW = 397;
    pFont = LoadFont("endgame.fnt", "FONTPAL", NULL);
    if (pParty->IsPartyGood())
        v1 = pGlobalTXT_LocalizationStrings[675];//"Splendid job!  With the activation of the Gate, a thousand worlds lie at your feet.  Perhaps on one of them you will find the Ancients themselves, and return with the fruits their great civilization has to offer your world and your kingdom."
    else if (pParty->IsPartyEvil())
        v1 = pGlobalTXT_LocalizationStrings[676];//"Brilliant!  The completion of the Heavenly Forge has provided enough Ancient weapons to crush all resistance to your plans.  Soon the world will bow to your every whim!  Still, you can't help but wonder what was beyond the Gate the other side was trying so hard to build."
    else return;
    pInString = v1;
    v23 = pParty->uTimePlayed - 138240;
    v19 = (unsigned int)((signed __int64)((double)(pParty->uTimePlayed - 138240) * 0.234375) / 60 / 60) / 24;
    v14 = (unsigned int)((signed __int64)((double)(pParty->uTimePlayed - 138240) * 0.234375) / 60 / 60) / 24 / 0x1C / 0xC;
    v18 = (unsigned int)((signed __int64)((double)(pParty->uTimePlayed - 138240) * 0.234375) / 60 / 60) / 24 / 0x1C % 0xC;
    v17 = v19 % 0x1C;
    if (!v19)
        v19 = 1;
    pRenderer->BeginScene();
    pWindow.DrawTitleText(pFont, 1, 0x23, 1, pGlobalTXT_LocalizationStrings[9], 3);//Congratulations!
    v23 = 0i64;
    v20 = 0;
    for (uint i = 0; i < 4; i++)
    {
        sprintf(pTmpBuf.data(), pGlobalTXT_LocalizationStrings[129], pParty->pPlayers[i].pName, pParty->pPlayers[i].GetBaseLevel(), pClassNames[pParty->pPlayers[i].classType]);//%s the Level %u %s
        pWindow.DrawTitleText(pFont, 1, i * (LOBYTE(pFont->uFontHeight) - 2) + LOBYTE(pFont->uFontHeight) + 46, 1, pTmpBuf.data(), 3);
        v23 += pParty->pPlayers[i].uExperience;//__PAIR__(*(int *)(i - 4), *(int *)(i - 8));
    }
    v23 = (signed __int64)v23 / v19;
    v6 = FitTextInAWindow(pInString, pFont, &pWindow, 0xC, 0);
    pWindow.DrawTitleText(pFont, 1, 5 * (LOBYTE(pFont->uFontHeight) + 11), 1, v6, 0);
    strcpy(pTmpBuf.data(), pGlobalTXT_LocalizationStrings[37]);//Total Time:

    v7 = pGlobalTXT_LocalizationStrings[56];
    if (v17 != 1)
        v7 = pGlobalTXT_LocalizationStrings[57];

    v8 = pGlobalTXT_LocalizationStrings[146];//Month
    if (v18 != 1)
        v8 = pGlobalTXT_LocalizationStrings[148];//Months

    v9 = pGlobalTXT_LocalizationStrings[245];
    if (v14 != 1)
        v9 = pGlobalTXT_LocalizationStrings[132];

    sprintf(pTmpBuf2.data(), " %lu %s, %lu %s, %lu %s ", v14, v9, v18, v8, v17, v7);
    strcat(pTmpBuf.data(), pTmpBuf2.data());
    pWindow.DrawTitleText(pFont, 1, pWindow.uFrameHeight - 2 * LOBYTE(pFont->uFontHeight) - 5, 1, pTmpBuf.data(), 3);
    sprintf(pTmpBuf.data(), pGlobalTXT_LocalizationStrings[94], v23);
    pWindow.DrawTitleText(pFont, 1, pWindow.uFrameHeight, 1, pTmpBuf.data(), 3);
    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;
    pRenderer->EndScene();
    pRenderer->Present();
    //if ( pRenderer->pRenderD3D )
    pRenderer->pBeforePresentFunction();
    pRenderer->SaveWinnersCertificate("MM7_Win.Pcx");
    free(pFont);
    window_SpeakInHouse->Release();
    window_SpeakInHouse = 0;
    if (v15 == 2)
        result = pMessageQueue_50CBD0->uNumMessages;
    else
    {
        LODWORD(v23) = GetTickCount() + 5000;
        while ((unsigned int)v23 > GetTickCount())
            ;
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                Engine_DeinitializeAndTerminate(0);
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (pMessageQueue_50CBD0->uNumMessages)
        {
            LOBYTE(v10) = pMessageQueue_50CBD0->pMessages[0].field_8 != 0;
            pMessageQueue_50CBD0->uNumMessages = v10;
        }
        pKeyActionMap->ResetKeys();
        pKeyActionMap->uLastKeyPressed = 0;
        do
        {
            while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                    Engine_DeinitializeAndTerminate(0);
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } while (!pKeyActionMap->uLastKeyPressed);
        if (pMessageQueue_50CBD0->uNumMessages)
        {
            pMessageQueue_50CBD0->uNumMessages = pMessageQueue_50CBD0->pMessages[0].field_8 != 0;
        }
    }
    if (v15)
    {
        /*if ( (signed int)result < 40 )
        {
        pMessageQueue_50CBD0->pMessages[result].eType = UIMSG_Quit;
        }*/
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Quit, 1, 0);
    }
    else
    {
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_ShowFinalWindow, 1, 0);
        /*if ( (signed int)result < 40 )
        {
        pMessageQueue_50CBD0->pMessages[result].eType = UIMSG_C5;
        pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].param = 1;
        result = 3 * pMessageQueue_50CBD0->uNumMessages + 3;
        *(&pMessageQueue_50CBD0->uNumMessages + result) = 0;
        ++pMessageQueue_50CBD0->uNumMessages;
        }*/
    }
    bGameoverLoop = false;
}