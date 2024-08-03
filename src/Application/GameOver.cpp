#include <string>
#include <memory>
#include <utility>

#include "GameOver.h"

#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time/Time.h"
#include "Engine/mm7_data.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIHouses.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Image/PCX.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/DataPath.h"


//----- (004BF91E) --------------------------------------------------------
void GameOver_Loop(int v15) {
    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_4000;
    bGameoverLoop = true;
    pAudioPlayer->stopSounds();

    CreateWinnerCertificate();

    // break out of house and dialogue
    if (window_SpeakInHouse) window_SpeakInHouse->Release();
    window_SpeakInHouse = nullptr;
    if (pDialogueWindow) pDialogueWindow->Release();
    window_SpeakInHouse = nullptr;
}

void CreateWinnerCertificate() {
    render->Present();
    render->BeginScene2D();
    GraphicsImage *background = assets->getImage_PCXFromIconsLOD("winbg.pcx");
    render->DrawTextureNew(0, 0, background);

    GUIWindow *tempwindow_SpeakInHouse = new GUIWindow(WINDOW_Unknown, { 0, 0 }, render->GetRenderDimensions());
    GUIWindow pWindow;
    pWindow.uFrameX = 75;
    pWindow.uFrameY = 60;
    pWindow.uFrameWidth = 469;
    pWindow.uFrameHeight = 338;
    pWindow.uFrameZ = 543;
    pWindow.uFrameW = 397;
    std::unique_ptr<GUIFont> pFont = GUIFont::LoadFont("endgame.fnt", "FONTPAL");

    std::string pInString;
    if (pParty->isPartyGood())
        pInString = localization->GetString(LSTR_GOOD_ENDING);
    else if (pParty->isPartyEvil())
        pInString = localization->GetString(LSTR_EVIL_ENDING);
    else
        assert(false);

    // TODO(captainurist): Introduce a constant here, for game start time.
    Duration play_time = pParty->GetPlayingTime() - Time() -  Duration::fromHours(9); // game begins at 9 am

    int totalDays = play_time.days();
    if (!totalDays) totalDays = 1;

    LongCivilDuration duration = play_time.toLongCivilDuration();
    int years = duration.years;
    int months = duration.months;
    int days = duration.days;

    pWindow.DrawTitleText(
        pFont.get(), 1, 0x23, colorTable.Black, localization->GetString(LSTR_CONGRATULATIONS), 3
    );
    uint64_t v23 = 0ull;
    int v20 = 0;
    for (int i = 0; i < 4; i++) {
        pWindow.DrawTitleText(
            pFont.get(), 1,
            i * (pFont->GetHeight() - 2) + pFont->GetHeight() + 46,
            colorTable.Black,
            localization->FormatString(
                LSTR_FMT_S_THE_LEVEL_D_S,
                pParty->pCharacters[i].name,
                pParty->pCharacters[i].GetBaseLevel(),
                localization->GetClassName(pParty->pCharacters[i].classType)),
            3);
        v23 += pParty->pCharacters[i].experience;
    }
    v23 = (int64_t)v23 / totalDays;
    std::string v6 = pFont->FitTextInAWindow(pInString, pWindow.uFrameWidth, 12);
    pWindow.DrawTitleText(pFont.get(), 1, 5 * (pFont->GetHeight() + 11), colorTable.Black, v6, 0);

    std::string v7 = localization->GetString(LSTR_DAY_CAPITALIZED);
    if (days != 1) v7 = localization->GetString(LSTR_DAYS);

    std::string v8 = localization->GetString(LSTR_MONTH);
    if (months != 1) v8 = localization->GetString(LSTR_MONTHS);

    std::string v9 = localization->GetString(LSTR_YEAR);
    if (years != 1) v9 = localization->GetString(LSTR_YEARS);

    pWindow.DrawTitleText(
        pFont.get(), 1, pWindow.uFrameHeight - 2 * pFont->GetHeight() - 5, colorTable.Black,
        fmt::format("{} {} {}, {} {}, {} {} ", localization->GetString(LSTR_TOTAL_TIME), years, v9, months, v8, days, v7), 3);

    pWindow.DrawTitleText(pFont.get(), 1, pWindow.uFrameHeight, colorTable.Black,
        localization->FormatString(LSTR_FMT_YOUR_SCORE_D, v23), 3);

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;

    // flush draw buffer so cert is drawn
    render->DrawTwodVerts();
    render->EndLines2D();
    render->EndTextNew();

    RgbaImage pixels = render->MakeFullScreenshot();
    FileOutputStream(makeDataPath("MM7_Win.Pcx")).write(pcx::encode(pixels).string_view());
    assets->winnerCert = GraphicsImage::Create(std::move(pixels));

    background->Release();
    background = nullptr;
    tempwindow_SpeakInHouse->Release();
}
