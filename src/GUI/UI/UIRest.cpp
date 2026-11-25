#include <memory>
#include <string>
#include <vector>

#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"
#include "Engine/Timer.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIRest.h"

std::shared_ptr<GraphicsImage> rest_ui_btn_4;
std::shared_ptr<GraphicsImage> rest_ui_btn_exit;
std::shared_ptr<GraphicsImage> rest_ui_btn_3;
std::shared_ptr<GraphicsImage> rest_ui_btn_1;
std::shared_ptr<GraphicsImage> rest_ui_btn_2;
std::shared_ptr<GraphicsImage> rest_ui_restmain;

// MM8's option plates. Unlike MM7 - whose restmain bakes the finished plates and whose
// restb1..4/restexit textures only ever flash for one frame via OnButtonClick - MM8's restmain
// bakes only blank recesses, and the original draws the *u frames persistently.
// rest_ui_btn_* above hold the *d click-flash frames for MM8; these hold the persistent look.
std::shared_ptr<GraphicsImage> rest_ui_mm8_plate_1;     // "rdawnu" - Wait Until Dawn
std::shared_ptr<GraphicsImage> rest_ui_mm8_plate_2;     // "r1hu" - Wait One Hour
std::shared_ptr<GraphicsImage> rest_ui_mm8_plate_3;     // "r5mu" - Wait 5 Minutes
std::shared_ptr<GraphicsImage> rest_ui_mm8_plate_4;     // "r8hu" - Rest & Heal 8 Hours
std::shared_ptr<GraphicsImage> rest_ui_mm8_plate_exit;  // "rexitu" - Exit Rest

std::shared_ptr<GraphicsImage> rest_ui_sky_frame_current;

int foodRequiredToRest;
Duration remainingRestTime;
RestType currentRestType;

static void prepareToLoadRestUI() {
    if (current_screen_type != SCREEN_GAME) {
        pGUIWindow_CurrentMenu = nullptr;
        current_screen_type = SCREEN_GAME;
    }
    gameTimer->setPaused(true);
    if (currentRestType != REST_HEAL) {
        // The flash plays at the button's own rect. MM8's command-icon row is in the top bar,
        // see GUIWindow.cpp's UI_Create.
        new OnButtonClick(pBtn_Rest->rect.topLeft(), {0, 0}, pBtn_Rest);
    }
    remainingRestTime = Duration();
    currentRestType = REST_NONE;
}

static void calculateRequiredFood() {
    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        foodRequiredToRest = pOutdoor->getNumFoodRequiredToRestInCurrentPos(pParty->pos);
    } else {
        foodRequiredToRest = 2;
    }

    if (PartyHasDragon())
        ++foodRequiredToRest;

    if (CheckHiredNPCSpeciality(NPC_PROFESSION_PORTER))
        --foodRequiredToRest;
    if (CheckHiredNPCSpeciality(NPC_PROFESSION_QUARTER_MASTER))
        foodRequiredToRest -= 2;
    if (CheckHiredNPCSpeciality(NPC_PROFESSION_GYPSY))
        --foodRequiredToRest;
    if (foodRequiredToRest < 1)
        foodRequiredToRest = 1;
    if (engine->_currentLoadedMapId == MAP_CASTLE_HARMONDALE && pParty->_questBits[QBIT_HARMONDALE_REBUILT])
        foodRequiredToRest = 0;
}

GUIWindow_Rest::GUIWindow_Rest()
    : GUIWindow(WINDOW_REST, {0, 0}, render->GetRenderDimensions()) {
    prepareToLoadRestUI();
    calculateRequiredFood();

    current_screen_type = SCREEN_REST;

    hourglassLoopTimer = 0_ticks;

    // MM8's rest screen art is in EnglishD.lod under rest-derived names:
    //   "restmain" -> EnglishD.lod "restmain" (640x366 green-stone panel; bakes the
    //       "Wait Without Healing" plate and blank recesses for the sky strip, the option plates
    //       and the hourglass/clock block)
    //   "restb4"/Rest & heal 8 hours -> EnglishD.lod "r8hu/r8hh/r8hd" (272x42)
    //   "restb1"/Wait Until Dawn     -> EnglishD.lod "rdawnu/rdawnh/rdawnd" (188x30)
    //   "restb2"/Wait One Hour       -> EnglishD.lod "r1hu/r1hh/r1hd" (188x30)
    //   "restb3"/Wait 5 Minutes      -> EnglishD.lod "r5mu/r5mh/r5md" (188x30)
    //   "restexit"                   -> EnglishD.lod "rexitu/rexith/rexitd" (185x43)
    // All labels are baked into the plate art, so unlike MM7 no text is drawn over the buttons.
    if (gameType() == GAME_TYPE_MM8) {
        rest_ui_restmain = assets->getImage_ColorKeyFromEnglishDLOD("restmain");
        rest_ui_btn_1 = assets->getImage_ColorKeyFromEnglishDLOD("rdawnd");
        rest_ui_btn_2 = assets->getImage_ColorKeyFromEnglishDLOD("r1hd");
        rest_ui_btn_3 = assets->getImage_ColorKeyFromEnglishDLOD("r5md");
        rest_ui_btn_4 = assets->getImage_ColorKeyFromEnglishDLOD("r8hd");
        rest_ui_btn_exit = assets->getImage_ColorKeyFromEnglishDLOD("rexitd");
        rest_ui_mm8_plate_1 = assets->getImage_ColorKeyFromEnglishDLOD("rdawnu");
        rest_ui_mm8_plate_2 = assets->getImage_ColorKeyFromEnglishDLOD("r1hu");
        rest_ui_mm8_plate_3 = assets->getImage_ColorKeyFromEnglishDLOD("r5mu");
        rest_ui_mm8_plate_4 = assets->getImage_ColorKeyFromEnglishDLOD("r8hu");
        rest_ui_mm8_plate_exit = assets->getImage_ColorKeyFromEnglishDLOD("rexitu");
    } else {
        rest_ui_restmain = assets->getImage_Alpha("restmain");
        rest_ui_btn_1 = assets->getImage_Alpha("restb1");
        rest_ui_btn_2 = assets->getImage_Alpha("restb2");
        rest_ui_btn_3 = assets->getImage_Alpha("restb3");
        rest_ui_btn_4 = assets->getImage_Alpha("restb4");
        rest_ui_btn_exit = assets->getImage_Alpha("restexit");
    }

    OutdoorLocation::LoadActualSkyFrame();

    if (gameType() == GAME_TYPE_MM8) {
        // Geometry measured from the original game's screen: plates sit on
        // restmain's baked recesses, Exit Rest under the clock block. MM8's panel is full-bleed -
        // see Update().
        pButton_RestUI_Exit = CreateButton({362, 300}, {185, 43}, BUTTON_TYPE_NORMAL, 0, UIMSG_ExitRest, 0, INPUT_ACTION_INVALID, "",
            rest_ui_btn_exit ? std::vector<std::shared_ptr<GraphicsImage>>{rest_ui_btn_exit} : std::vector<std::shared_ptr<GraphicsImage>>{});
        pButton_RestUI_Main = CreateButton("Rest_RestAndHeal", {60, 156}, {272, 42}, BUTTON_TYPE_NORMAL, 0, UIMSG_Rest8Hour, 0, INPUT_ACTION_REST_HEAL, "",
            rest_ui_btn_4 ? std::vector<std::shared_ptr<GraphicsImage>>{rest_ui_btn_4} : std::vector<std::shared_ptr<GraphicsImage>>{});
        pButton_RestUI_WaitUntilDawn = CreateButton("Rest_WaitTillDawn", {99, 240}, {188, 30}, BUTTON_TYPE_NORMAL, 0, UIMSG_WaitTillDawn, 0, INPUT_ACTION_REST_WAIT_TILL_DAWN, "",
            rest_ui_btn_1 ? std::vector<std::shared_ptr<GraphicsImage>>{rest_ui_btn_1} : std::vector<std::shared_ptr<GraphicsImage>>{});
        pButton_RestUI_Wait1Hour = CreateButton("Rest_Wait1Hour", {99, 272}, {188, 30}, BUTTON_TYPE_NORMAL, 0, UIMSG_Wait1Hour, 0, INPUT_ACTION_REST_WAIT_1_HOUR, "",
            rest_ui_btn_2 ? std::vector<std::shared_ptr<GraphicsImage>>{rest_ui_btn_2} : std::vector<std::shared_ptr<GraphicsImage>>{});
        pButton_RestUI_Wait5Minutes = CreateButton({99, 304}, {188, 30}, BUTTON_TYPE_NORMAL, 0, UIMSG_Wait5Minutes, 0, INPUT_ACTION_REST_WAIT_5_MINUTES, "",
            rest_ui_btn_3 ? std::vector<std::shared_ptr<GraphicsImage>>{rest_ui_btn_3} : std::vector<std::shared_ptr<GraphicsImage>>{});
    } else {
        pButton_RestUI_Exit = CreateButton({280, 297}, {154, 37}, BUTTON_TYPE_NORMAL, 0, UIMSG_ExitRest, 0, INPUT_ACTION_INVALID, "",
            rest_ui_btn_exit ? std::vector<std::shared_ptr<GraphicsImage>>{rest_ui_btn_exit} : std::vector<std::shared_ptr<GraphicsImage>>{});
        pButton_RestUI_Main = CreateButton("Rest_RestAndHeal", {24, 154}, {225, 37}, BUTTON_TYPE_NORMAL, 0, UIMSG_Rest8Hour, 0, INPUT_ACTION_REST_HEAL, "",
            rest_ui_btn_4 ? std::vector<std::shared_ptr<GraphicsImage>>{rest_ui_btn_4} : std::vector<std::shared_ptr<GraphicsImage>>{});
        pButton_RestUI_WaitUntilDawn = CreateButton("Rest_WaitTillDawn", {61, 232}, {154, 33}, BUTTON_TYPE_NORMAL, 0, UIMSG_WaitTillDawn, 0, INPUT_ACTION_REST_WAIT_TILL_DAWN, "",
            rest_ui_btn_1 ? std::vector<std::shared_ptr<GraphicsImage>>{rest_ui_btn_1} : std::vector<std::shared_ptr<GraphicsImage>>{});
        pButton_RestUI_Wait1Hour = CreateButton("Rest_Wait1Hour", {61, 264}, {154, 33}, BUTTON_TYPE_NORMAL, 0, UIMSG_Wait1Hour, 0, INPUT_ACTION_REST_WAIT_1_HOUR, "",
            rest_ui_btn_2 ? std::vector<std::shared_ptr<GraphicsImage>>{rest_ui_btn_2} : std::vector<std::shared_ptr<GraphicsImage>>{});
        pButton_RestUI_Wait5Minutes = CreateButton({61, 296}, {154, 33}, BUTTON_TYPE_NORMAL, 0, UIMSG_Wait5Minutes, 0, INPUT_ACTION_REST_WAIT_5_MINUTES, "",
            rest_ui_btn_3 ? std::vector<std::shared_ptr<GraphicsImage>>{rest_ui_btn_3} : std::vector<std::shared_ptr<GraphicsImage>>{});
    }
}

void GUIWindow_Rest::Update() {
    GUIButton tmp_button;

    bool mm8 = gameType() == GAME_TYPE_MM8;

    int liveCharacters = 0;
    for (Character &player : pParty->pCharacters)
        if (!player.IsDead() && !player.IsEradicated() && player.health > 0)
            ++liveCharacters;

    if (liveCharacters) {
        // Positions measured from the original game's screen. MM8's panel is full-bleed at
        // (0,0) - its bottom edge lands on the portrait bar's usual strip - and its sky
        // recess/hourglass sit at different offsets than MM7's.
        render->DrawQuad2D(rest_ui_restmain, mm8 ? Pointi(0, 0) : Pointi(8, 8));
        render->DrawQuad2D(rest_ui_sky_frame_current, mm8 ? Pointi(96, 25) : Pointi(16, 26));

        hourglassLoopTimer += gameTimer->dt();
        if (hourglassLoopTimer >= Duration::fromRealtimeSeconds(4))
            hourglassLoopTimer = 0_ticks;

        int hourglass_icon_idx = (int)floorf(((double)hourglassLoopTimer.ticks() / 512.0 * 120.0) + 0.5f) % 256 + 1;
        if (hourglass_icon_idx >= 120)
            hourglass_icon_idx = 1;

        render->DrawQuad2D(assets->getImage_ColorKey(fmt::format("hglas{:03}", hourglass_icon_idx)), mm8 ? Pointi(384, 164) : Pointi(267, 159));

        if (mm8) {
            // The plates are engine-drawn on top of restmain's blank recesses; their labels are
            // baked into the art, so - unlike MM7 - no text goes over them.
            render->DrawQuad2D(rest_ui_mm8_plate_4, {60, 156});
            render->DrawQuad2D(rest_ui_mm8_plate_1, {99, 240});
            render->DrawQuad2D(rest_ui_mm8_plate_2, {99, 272});
            render->DrawQuad2D(rest_ui_mm8_plate_3, {99, 304});
            render->DrawQuad2D(rest_ui_mm8_plate_exit, {362, 300});
        } else {
            tmp_button.rect = Recti(24, 154, 171, 37);
            tmp_button.pParent = pButton_RestUI_WaitUntilDawn->pParent;
            tmp_button.DrawLabel(localization->str(LSTR_REST_HEAL_8_HOURS), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
            tmp_button.pParent = 0;
        }

        // Food counter: MM8 recesses it into the r8h plate's own box (right edge x=313), on the
        // same text row as the clock block; MM7 right-justifies it to x~223 at y=164.
        auto str1 = mm8 ? fmt::format("\r326{}", foodRequiredToRest) : fmt::format("\r408{}", foodRequiredToRest);
        GUIWindow::DrawText(assets->pFontCreate.get(), {0, mm8 ? 168 : 164}, colorTable.Diesel, str1, pGUIWindow_CurrentMenu->frameRect, 0, colorTable.StarkWhite);

        if (!mm8) {
            pButton_RestUI_WaitUntilDawn->DrawLabel(localization->str(LSTR_WAIT_UNTIL_DAWN), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
            pButton_RestUI_Wait1Hour->DrawLabel(localization->str(LSTR_WAIT_1_HOUR), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
            pButton_RestUI_Wait5Minutes->DrawLabel(localization->str(LSTR_WAIT_5_MINUTES), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
            pButton_RestUI_Exit->DrawLabel(localization->str(LSTR_EXIT_REST), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
            tmp_button.rect = Recti(45, 199, 185, 30);

            tmp_button.pParent = pButton_RestUI_WaitUntilDawn->pParent;
            tmp_button.DrawLabel(localization->str(LSTR_WAIT_WITHOUT_HEALING), assets->pFontCreate.get(), colorTable.Diesel, colorTable.StarkWhite);
            tmp_button.pParent = 0;
        }

        CivilTime time = pParty->GetPlayingTime().toCivilTime();

        // Clock block, measured from the original game's screen - row tops are MM7's
        // (168/190/222/254), but labels left-align at x=464 with values right-justified to x=549,
        // and the readout itself right-justifies to x=543.
        std::string str2 = mm8 ? fmt::format("\r096{}:{:02} {}", time.hourAmPm, time.minute, localization->amPm(time.isPm))
                               : fmt::format("{}:{:02} {}", time.hourAmPm, time.minute, localization->amPm(time.isPm));
        DrawText(assets->pFontCreate.get(), mm8 ? Pointi(0, 168) : Pointi(368, 168), colorTable.Diesel, str2, pGUIWindow_CurrentMenu->frameRect, 0, colorTable.StarkWhite);
        std::string str3 = mm8 ? fmt::format("{}\r090{}", localization->str(LSTR_DAY_CAPITALIZED), time.day)
                               : fmt::format("{}\r190{}", localization->str(LSTR_DAY_CAPITALIZED), time.day);
        DrawText(assets->pFontCreate.get(), mm8 ? Pointi(464, 190) : Pointi(350, 190), colorTable.Diesel, str3, pGUIWindow_CurrentMenu->frameRect, 0, colorTable.StarkWhite);
        std::string str4 = mm8 ? fmt::format("{}\r090{}", localization->str(LSTR_MONTH), time.month)
                               : fmt::format("{}\r190{}", localization->str(LSTR_MONTH), time.month);
        DrawText(assets->pFontCreate.get(), mm8 ? Pointi(464, 222) : Pointi(350, 222), colorTable.Diesel, str4, pGUIWindow_CurrentMenu->frameRect, 0, colorTable.StarkWhite);
        std::string str5 = mm8 ? fmt::format("{}\r090{}", localization->str(LSTR_YEAR), time.year)
                               : fmt::format("{}\r190{}", localization->str(LSTR_YEAR), time.year);
        DrawText(assets->pFontCreate.get(), mm8 ? Pointi(464, 254) : Pointi(350, 254), colorTable.Diesel, str5, pGUIWindow_CurrentMenu->frameRect, 0, colorTable.StarkWhite);
        if (currentRestType != REST_NONE)
            Party::restOneFrame();
    } else {
        new OnCancel(pButton_RestUI_Exit->rect.topLeft(), {0, 0}, pButton_RestUI_Exit, localization->str(LSTR_EXIT_REST));
    }
}
