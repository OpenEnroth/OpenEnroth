#include "Game.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "Arcomage/Arcomage.h"

#include "GameFactory.h"
#include "GameMenu.h"

#include "Application/GameWindowHandler.h"
#include "Application/GamePathResolver.h"
#include "Application/GameTraceHandler.h"

#include "Engine/AssetsManager.h"
#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/EngineFactory.h"
#include "Engine/Events.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/IRenderFactory.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/NuklearEventHandler.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Plugins/EngineTracer.h"
#include "Engine/Plugins/EngineControlPlugin.h"
#include "Engine/Plugins/EngineDeterministicPlugin.h"
#include "Engine/Plugins/EngineTracePlugin.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Tables/FrameTableInc.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/Books/AutonotesBook.h"
#include "GUI/UI/Books/CalendarBook.h"
#include "GUI/UI/Books/JournalBook.h"
#include "GUI/UI/Books/LloydsBook.h"
#include "GUI/UI/Books/MapBook.h"
#include "GUI/UI/Books/QuestBook.h"
#include "GUI/UI/Books/TownPortalBook.h"
#include "GUI/UI/Spellbook.h"
#include "GUI/UI/UIBooks.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UICredits.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIMainMenu.h"
#include "GUI/UI/UIGameOver.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UIQuickReference.h"
#include "GUI/UI/UIRest.h"
#include "GUI/UI/UISaveLoad.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"
#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Library/Application/PlatformApplication.h"

#include "Platform/Platform.h"
#include "Platform/Filters/FilteringEventHandler.h"

#include "Utility/Format.h"
#include "Library/Random/Random.h"


void ShowMM7IntroVideo_and_LoadingScreen();
void IntegrityTest();

using Application::Game;
using Application::GameConfig;
using Application::GameFactory;
using Engine_::EngineFactory;
using Graphics::IRenderFactory;


void Application::AutoInitDataPath(Platform *platform) {
    // TODO (captainurist): we should consider reading Unicode (utf8) strings from win32 registry, as it might contain paths
    // curretnly we convert all strings out of registry into CP_ACP (default windows ansi)
    // it is later on passed to std::filesystem that should be ascii on windows as well
    // this means we will can't handle win32 unicode paths at the time
    std::string mm7dir = resolveMm7Path(platform);

#ifdef __ANDROID__
    if (mm7dir.empty()) {
        platform->ShowMessageBox("Device currently unsupported", "Your device doesn't have any storage so it is unsupported!");
        return;
    }
#endif

    if (validateDataPath(mm7dir)) {
        SetDataPath(mm7dir);

        std::string savesPath = MakeDataPath("saves");
        if (!std::filesystem::exists(savesPath)) {
            std::filesystem::create_directory(savesPath);
        }

        EngineIoc::ResolveLogger()->Info("Using MM7 directory: {}", mm7dir);
    } else {
        std::string message = fmt::format(
            "Required resources aren't found!\n"
            "You should acquire licensed copy of M&M VII and copy its resources to \n{}\n\n"
            "Additionally you should also copy the content from\n"
            "resources directory from our repository there as well!",
            !mm7dir.empty() ? mm7dir : "current directory"
        );
        EngineIoc::ResolveLogger()->Warning("{}", message);
        platform->showMessageBox("CRITICAL ERROR: missing resources", message);
    }
}

Game::Game(PlatformApplication *app) {
    this->app = app;
    this->log = EngineIoc::ResolveLogger();
    this->decal_builder = EngineIoc::ResolveDecalBuilder();
    this->vis = EngineIoc::ResolveVis();
    this->menu = GameIoc::ResolveGameMenu();

    ::application = app;
    ::platform = app->platform();
    ::eventLoop = app->eventLoop();
    ::window = app->window();
    ::eventHandler = app->eventHandler();
    ::openGLContext = app->openGLContext(); // OK to store into a global even if not yet initialized

    // It doesn't matter where to put control plugin as it's running the control routine after a call to `SwapBuffers`.
    // But the trace plugin should go after the deterministic plugin - deterministic plugin updates tick count, and then
    // trace plugin stores the updated value in a recorded `PaintEvent`.
    windowHandler.reset(Application::IocContainer::ResolveGameWindowHandler());
    app->install(windowHandler.get()); // TODO(captainurist): actually move ownership into PlatformApplication?
    app->install(windowHandler->KeyboardController()); // TODO(captainurist): do this properly
    app->install(std::make_unique<EngineControlPlugin>());
    app->install(std::make_unique<EngineTracePlugin>());
    app->install(std::make_unique<EngineDeterministicPlugin>());
    app->install(std::make_unique<EngineTracer>(EngineTracer::ENABLE_RECORDING | EngineTracer::ENABLE_PLAYBACK)); // TODO(captainurist): make configurable
    app->install(std::make_unique<GameTraceHandler>(app->get<EngineTracer>())); // TODO(captainurist): get() call not needed.
}

Game::~Game() {
    app->remove(windowHandler->KeyboardController()); // TODO(captainurist): do this properly
    app->remove(windowHandler.get());
    if (nuklearHandler)
        app->remove(nuklearHandler.get());
}

int Game::Run() {
    IntegrityTest();

    render = IRenderFactory().Create(config);
    ::render = render;

    if (!render) {
        log->Warning("Render creation failed");
        return -1;
    }

    if (!render->Initialize()) {
        log->Warning("Render failed to initialize");
        return -1;
    }

    nuklear = Nuklear::Initialize();
    if (!nuklear) {
        log->Warning("Nuklear failed to initialize");
    }
    ::nuklear = nuklear;
    if (nuklear) {
        nuklearHandler = std::make_unique<NuklearEventHandler>();
        app->install(nuklearHandler.get());
    }

    keyboardActionMapping = std::make_shared<KeyboardActionMapping>(config);
    ::keyboardActionMapping = keyboardActionMapping;

    keyboardInputHandler = std::make_shared<KeyboardInputHandler>(
        windowHandler->KeyboardController(),
        keyboardActionMapping
    );

    mouse = EngineIoc::ResolveMouse();
    ::mouse = mouse;

    EngineFactory engineFactory;
    engine = engineFactory.CreateEngine(config);
    ::engine = engine;

    if (!engine) {
        log->Warning("Engine creation failed");
        return -1;
    }

    engine->Initialize();
    /* TODO: We should setup window before render as it is requesting window size upon initialization to setup clipping dimensions, zbuffer, etc.
     * Otherwise starting borderless fullscreen will start with renderer in 640x480 and so be broken.
     * For some reason windows not liking that and hang in SDL_GL_SwapWindow if it was called after changing window position out of primary monitor.
     * And if we try to exclude changing position and set it after render initialization then when game started in fullscreen request will be ignored.
     * Hack below with render reinitialization is a temporary workaround. */
    windowHandler->UpdateWindowFromConfig(config.get());
    render->Reinitialize();
    window->activate();
    eventLoop->processMessages(eventHandler);

    ShowMM7IntroVideo_and_LoadingScreen();

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;

    // logger->Warning("MM: entering main loop");
    while (true) {
        GUIWindow_MainMenu::Loop();
        uGameState = GAME_STATE_PLAYING;

        if (!Loop()) {
            break;
        }
    }

    if (window) {
        windowHandler->UpdateConfigFromWindow(config.get());
        config->SaveConfiguration();
    }

    if (engine) {
        engine->Deinitialize();
        engine = nullptr;
        ::engine = nullptr;
    }

    if (render) {
        render->Release();
        render = nullptr;
    }

    return 0;
}

bool Game::Loop() {
    while (1) {
        if (uGameState == GAME_FINISHED ||
            GetCurrentMenuID() == MENU_EXIT_GAME) {
            return false;
        } else if (GetCurrentMenuID() == MENU_SAVELOAD) {
            MainMenuLoad_Loop();
            if (GetCurrentMenuID() == MENU_LoadingProcInMainMenu) {
                uGameState = GAME_STATE_PLAYING;
                GameLoop();
            }
            break;
        } else if (GetCurrentMenuID() == MENU_NEWGAME) {
            pOtherOverlayList->Reset();
            if (!PartyCreationUI_Loop()) {
                break;
            }

            pParty->pPickedItem.uItemID = ITEM_NULL;

            pCurrentMapName = config->gameplay.StartingMap.Get();
            bFlashQuestBook = true;
            pMediaPlayer->PlayFullscreenMovie("Intro Post");
            SaveNewGame();
            if (engine->config->debug.NoMargaret.Get()) {
                _449B7E_toggle_bit(pParty->_quest_bits, QBIT_EMERALD_ISLAND_MARGARETH_OFF, 1);
            }

            GameLoop();
            if (uGameState == GAME_STATE_NEWGAME_OUT_GAMEMENU) {
                SetCurrentMenuID(MENU_NEWGAME);
                uGameState = GAME_STATE_PLAYING;
                continue;
            } else if (uGameState == GAME_STATE_GAME_QUITTING_TO_MAIN_MENU) {
                break;
            }
            assert(false && "Invalid game state");
        } else if (GetCurrentMenuID() == MENU_CREDITS) {
            pAudioPlayer->MusicStop();
            GUICredits::ExecuteCredits();
            break;
        } else if (GetCurrentMenuID() == MENU_5 ||
            GetCurrentMenuID() == MENU_LoadingProcInMainMenu) {
            uGameState = GAME_STATE_PLAYING;
            GameLoop();
        } else if (GetCurrentMenuID() == MENU_DebugBLVLevel) {
            mouse->ChangeActivation(0);
            pParty->Reset();
            pParty->CreateDefaultParty(true);

            __debugbreak();
            /*extern void CreateDefaultBLVLevel();
            CreateDefaultBLVLevel();

            OPENFILENAMEA ofn;
            if ( !GetOpenFileNameA((LPOPENFILENAMEA)&ofn) )
            {
            pMouse->ChangeActivation(1);
            break;
            }
            _chdir("..\\");
            strcpy(pCurrentMapName, ofn.lpstrFileTitle);*/
            mouse->ChangeActivation(1);
            GameLoop();
        }
        if (uGameState == GAME_STATE_LOADING_GAME) {
            SetCurrentMenuID(MENU_5);
            uGameState = GAME_STATE_PLAYING;
            continue;
        }
        if (uGameState == GAME_STATE_NEWGAME_OUT_GAMEMENU) {
            SetCurrentMenuID(MENU_NEWGAME);
            uGameState = GAME_STATE_PLAYING;
            continue;
        }
        if (uGameState ==
            GAME_STATE_GAME_QUITTING_TO_MAIN_MENU) {  // from the loaded game
            pAudioPlayer->PauseSounds(-1);
            uGameState = GAME_STATE_PLAYING;
            break;
        }
    }

    return true;
}



void ShowMM7IntroVideo_and_LoadingScreen() {
    bGameoverLoop = true;

    render->PresentBlackScreen();
    if (!engine->config->debug.NoVideo.Get()) {
        if (!engine->config->debug.NoLogo.Get()) {
            pMediaPlayer->PlayFullscreenMovie("3dologo");
            pMediaPlayer->PlayFullscreenMovie("new world logo");
            pMediaPlayer->PlayFullscreenMovie("jvc");
        }
        if (!engine->config->debug.NoIntro.Get()) {
            pMediaPlayer->PlayFullscreenMovie("Intro");
        }
    }

    bGameoverLoop = false;
}




Image *gamma_preview_image = nullptr;  // 506E40

void Game_StartDialogue(unsigned int actor_id) {
    if (uActiveCharacter) {
        pCurrentFrameMessageQueue->Flush();

        dword_5B65D0_dialogue_actor_npc_id = pActors[actor_id].sNPC_ID;
        GameUI_InitializeDialogue(&pActors[actor_id], true);
    }
}

void Game_StartHirelingDialogue(unsigned int hireling_id) {
    if (bNoNPCHiring || current_screen_type != CURRENT_SCREEN::SCREEN_GAME) return;

    pCurrentFrameMessageQueue->Flush();

    FlatHirelings buf;
    buf.Prepare();

    if ((signed int)hireling_id + (signed int)pParty->hirelingScrollPosition < buf.Size()) {
        Actor actor;
        actor.sNPC_ID += -1 - pParty->hirelingScrollPosition - hireling_id;
        GameUI_InitializeDialogue(&actor, true);
    }
}

void Game::CloseTargetedSpellWindow() {
    if (pGUIWindow_CastTargetedSpell) {
        if (current_screen_type == CURRENT_SCREEN::SCREEN_CHARACTERS) {
            mouse->SetCursorImage("MICON2");
        } else {
            pGUIWindow_CastTargetedSpell->Release();  // test to fix enchanting issue
            pGUIWindow_CastTargetedSpell = nullptr;  // test to fix enchanting issue
            mouse->SetCursorImage("MICON1");
            game_ui_status_bar_event_string_time_left = 0;
            IsEnchantingInProgress = false;
            back_to_game();
        }
    }
}

void Game::OnEscape() {
    CloseTargetedSpellWindow();

    // if ((signed int)uActiveCharacter < 1 || (signed int)uActiveCharacter > 4)

    uActiveCharacter =
        pParty->GetNextActiveCharacter();  // always check this - could leave
                                           // shops with characters who couldnt
                                           // act sctive

    if (pGUIWindow_CurrentMenu == window_SpeakInHouse) {
        window_SpeakInHouse = nullptr;
    }
    if (pGUIWindow_CurrentMenu != nullptr) {
        pGUIWindow_CurrentMenu->Release();  // check this
        pGUIWindow_CurrentMenu = nullptr;
    }
    pEventTimer->Resume();
    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
}

extern bool _506360_installing_beacon;

bool IsWindowSwitchable() {
    if (current_screen_type == CURRENT_SCREEN::SCREEN_NPC_DIALOGUE || current_screen_type == CURRENT_SCREEN::SCREEN_HOUSE
        || current_screen_type == CURRENT_SCREEN::SCREEN_INPUT_BLV || current_screen_type == CURRENT_SCREEN::SCREEN_CHANGE_LOCATION) {
        return false;
    }
    return true;
}

void Game::EventLoop() {
    unsigned int v2 {};            // edx@7
    GUIWindow *pWindow2;        // ecx@248
    int v37;                    // eax@341
    int v38;                    // eax@358
    int v42 {};                    // eax@396
    signed int v44;             // eax@398
    ObjectType v45;                    // edx@398
    signed int v46;             // ecx@398
    char v47;                   // zf@399
    char v48;                   // zf@405
    BLVFace *pBLVFace;          // ecx@410
    ODMFace *pODMFace;          // ecx@412
    CastSpellInfo *pSpellInfo;  // ecx@415
    int16_t v53;                // ax@431
    int v54;                    // eax@432
    int v55;                    // ecx@432
    int v56;                    // edx@432
    int v57;                    // eax@432
    unsigned int pMapNum;       // eax@445
    int16_t v63;                // dx@479
    unsigned int v64;           // eax@486
    int v65;                    // ecx@486
    int v66;                    // eax@488
    Player *pPlayer2;           // ecx@549
    Vis_PIDAndDepth v83;             // ecx@554
    signed int v84;             // ecx@554
    GUIButton *pButton;         // eax@578
    unsigned int v86;           // eax@583
    const char *v87;            // ecx@595
    unsigned int v90;           // eax@602
    int v91;                    // edx@605
    int v92;                    // eax@605
    int v93;                    // edx@605
    int pPlayerNum;             // edx@611
    int v95;                    // eax@611
    unsigned int v97;           // eax@624
    int v98;                    // eax@636
    int v103;                   // eax@671
    Player *pPlayer4;           // ecx@718
    short *v105;                   // eax@718
    Player *pPlayer5;           // ST78_4@758
    MapInfo *pMapInfo;            // [sp+14h] [bp-5E8h]@604
    Player *pPlayer10;            // [sp+14h] [bp-5E8h]@641
    int uMessageParam;            // [sp+18h] [bp-5E4h]@7
    int uAction;                  // [sp+1Ch] [bp-5E0h]@18
    int encounter_index;           // [sp+20h] [bp-5DCh]@23
    unsigned int uNumSeconds;     // [sp+24h] [bp-5D8h]@18
    UIMessageType uMessage;  // [sp+2Ch] [bp-5D0h]@7
    unsigned int v199 {};            // [sp+30h] [bp-5CCh]@7
    char *v200 = nullptr;                   // [sp+34h] [bp-5C8h]@518
    char pOut[32];                // [sp+BCh] [bp-540h]@370
    int spellbookPages[9] {};                  // [sp+158h] [bp-4A4h]@652
    Actor actor;                  // [sp+2B8h] [bp-344h]@4
    int currHour;
    PLAYER_SKILL_TYPE skill;
    PLAYER_SKILL_LEVEL skill_level;


    dword_50CDC8 = 0;
    if (!pEventTimer->bPaused) {
        pParty->sEyelevel = pParty->uDefaultEyelevel;
        pParty->uPartyHeight = pParty->uDefaultPartyHeight;
    }
    if (bDialogueUI_InitializeActor_NPC_ID) {
        // Actor::Actor(&actor);
        actor = Actor();
        dword_5B65D0_dialogue_actor_npc_id = bDialogueUI_InitializeActor_NPC_ID;
        actor.sNPC_ID = bDialogueUI_InitializeActor_NPC_ID;
        GameUI_InitializeDialogue(&actor, false);
        bDialogueUI_InitializeActor_NPC_ID = 0;
    }
    if (!pCurrentFrameMessageQueue->Empty()) {
        // v1 = "";
        while (2) {
            if (pCurrentFrameMessageQueue->Empty()) {
                break;
            }

            pCurrentFrameMessageQueue->PopMessage(&uMessage, &uMessageParam,
                                             (int *)&v199);
            switch (uMessage) {
                case UIMSG_ChangeGameState:
                    uGameState = GAME_FINISHED;
                    continue;
                case UIMSG_PlayArcomage:
                    BackToHouseMenu();
                    pArcomageGame->bGameInProgress = 1;
                    ArcomageGame::PrepareArcomage();
                    continue;

                case UIMSG_StartNPCDialogue:
                    Game_StartDialogue(uMessageParam);
                    continue;
                case UIMSG_StartHireling1Dialogue:
                case UIMSG_StartHireling2Dialogue:
                    Game_StartHirelingDialogue(uMessage - UIMSG_StartHireling1Dialogue);
                    continue;
                case UIMSG_BuyInShop_Identify_Repair:
                    UIShop_Buy_Identify_Repair();
                    continue;
                case UIMSG_ClickNPCTopic:
                    ClickNPCTopic((DIALOGUE_TYPE)uMessageParam);
                    continue;
                case UIMSG_SelectShopDialogueOption:
                    OnSelectShopDialogueOption((DIALOGUE_TYPE)uMessageParam);
                    continue;
                case UIMSG_SelectNPCDialogueOption:
                    OnSelectNPCDialogueOption((DIALOGUE_TYPE)uMessageParam);
                    continue;
                case UIMSG_ClickHouseNPCPortrait:
                    _4B4224_UpdateNPCTopics(uMessageParam);
                    continue;
                    // case UIMSG_StartNewGame:
                    // Game_StartNewGameWhilePlaying(uMessageParam); continue;
                    // case UIMSG_Game_OpenLoadGameDialog:
                    // Game_OpenLoadGameDialog(); continue; case UIMSG_Quit:
                    // Game_QuitGameWhilePlaying(uMessageParam); continue;
                case UIMSG_80:
                    __debugbreak();
                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = CURRENT_SCREEN::SCREEN_OPTIONS;
                    __debugbreak();  // pGUIWindow_CurrentMenu =
                                     // GUIWindow::Create(0, 0,
                                     // window->GetWidth(), window->GetHeight(),
                                     // WINDOW_8, 0, 0);
                    continue;
                case UIMSG_Cancel:
                    new OnCancel({350, 302}, {106, 42}, pBtnCancel);
                    continue;
                case UIMSG_OpenQuestBook:
                    pCurrentFrameMessageQueue->Flush();
                    // toggle
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_BOOKS && pGUIWindow_CurrentMenu->eWindowType == WindowType::WINDOW_QuestBook) {
                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                        continue;
                    }
                    // cant open screen - talking or in shop or map transition
                    if (!IsWindowSwitchable()) {
                        continue;
                    } else {
                        // close out current window
                        back_to_game();
                        OnEscape();
                        GameUI_StatusBar_Clear();
                    }
                    // open window
                    pGUIWindow_CurrentMenu = new GUIWindow_QuestBook();
                    continue;
                case UIMSG_OpenAutonotes:
                    pCurrentFrameMessageQueue->Flush();
                    // toggle
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_BOOKS && pGUIWindow_CurrentMenu->eWindowType == WindowType::WINDOW_AutonotesBook) {
                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                        continue;
                    }
                    // cant open screen - talking or in shop or map transition
                    if (!IsWindowSwitchable()) {
                        continue;
                    } else {
                        // close out current window
                        back_to_game();
                        OnEscape();
                        GameUI_StatusBar_Clear();
                    }
                    // open window
                    pGUIWindow_CurrentMenu = new GUIWindow_AutonotesBook();
                    continue;
                case UIMSG_OpenMapBook:
                    pCurrentFrameMessageQueue->Flush();
                    // toggle
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_BOOKS && pGUIWindow_CurrentMenu->eWindowType == WindowType::WINDOW_MapsBook) {
                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                        continue;
                    }
                    // cant open screen - talking or in shop or map transition
                    if (!IsWindowSwitchable()) {
                        continue;
                    } else {
                        // close out current window
                        back_to_game();
                        OnEscape();
                        GameUI_StatusBar_Clear();
                    }
                    // open window;
                    pGUIWindow_CurrentMenu = new GUIWindow_MapBook();
                    continue;
                case UIMSG_OpenCalendar:
                    pCurrentFrameMessageQueue->Flush();
                    // toggle
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_BOOKS && pGUIWindow_CurrentMenu->eWindowType == WindowType::WINDOW_CalendarBook) {
                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                        continue;
                    }
                    // cant open screen - talking or in shop or map transition
                    if (!IsWindowSwitchable()) {
                        continue;
                    } else {
                        // close out current window
                        back_to_game();
                        OnEscape();
                        GameUI_StatusBar_Clear();
                    }
                    // open window
                    pGUIWindow_CurrentMenu = new GUIWindow_CalendarBook();
                    continue;
                case UIMSG_OpenHistoryBook:
                    pCurrentFrameMessageQueue->Flush();
                    // toggle
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_BOOKS && pGUIWindow_CurrentMenu->eWindowType == WindowType::WINDOW_JournalBook) {
                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                        continue;
                    }
                    // cant open screen - talking or in shop or map transition
                    if (!IsWindowSwitchable()) {
                        continue;
                    } else {
                        // close out current window
                        back_to_game();
                        OnEscape();
                        GameUI_StatusBar_Clear();
                    }
                    // open window
                    pGUIWindow_CurrentMenu = new GUIWindow_JournalBook();
                    continue;
                case UIMSG_OpenDebugMenu:
                    pCurrentFrameMessageQueue->Flush();
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_DEBUG) {
                        back_to_game();
                        OnEscape();
                        GameUI_StatusBar_Clear();
                        break;
                    }
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) continue;
                        // pGUIWindow_CurrentMenu->Release();
                    pGUIWindow_CurrentMenu = new GUIWindow_DebugMenu();
                    current_screen_type = CURRENT_SCREEN::SCREEN_DEBUG;
                    continue;
                case UIMSG_Escape:  // нажатие Escape and return to game
                    back_to_game();
                    pCurrentFrameMessageQueue->Flush();
                    switch (current_screen_type) {
                        case CURRENT_SCREEN::SCREEN_E:
                            __debugbreak();
                        case CURRENT_SCREEN::SCREEN_NPC_DIALOGUE:
                        case CURRENT_SCREEN::SCREEN_CHEST:
                        case CURRENT_SCREEN::SCREEN_CHEST_INVENTORY:
                        case CURRENT_SCREEN::SCREEN_CHANGE_LOCATION:
                        case CURRENT_SCREEN::SCREEN_INPUT_BLV:
                        case CURRENT_SCREEN::SCREEN_QUICK_REFERENCE:
                            if (dword_50CDC8) break;
                            PlayButtonClickSound();
                            uMessageParam = 1;
                            break;
                        case CURRENT_SCREEN::SCREEN_HOUSE:
                            if (!dword_50CDC8) {
                                PlayButtonClickSound();
                                uMessageParam = 1;
                                break;
                            }
                            break;
                        default:
                            break;
                    }
                    if (pGameOverWindow) {
                        if (bGameOverWindowCheckExit) {
                            pGameOverWindow->Release();
                            pGameOverWindow = nullptr;
                            continue;
                        } else {
                            bGameOverWindowCheckExit = true;
                            continue;
                        }
                    }
                    render->ClearZBuffer();
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                        if (!pGUIWindow_CastTargetedSpell) {  // Draw Menu
                            dword_6BE138 = -1;
                            new OnButtonClick2({602, 450}, {0, 0}, pBtn_GameSettings, std::string(), false);

                            pCurrentFrameMessageQueue->Flush();
                            menu->MenuLoop();
                        } else {
                            pGUIWindow_CastTargetedSpell->Release();
                            pGUIWindow_CastTargetedSpell = 0;
                            mouse->SetCursorImage("MICON1");
                            game_ui_status_bar_event_string_time_left = 0;
                            IsEnchantingInProgress = false;
                            back_to_game();
                        }
                        continue;
                    } else if (current_screen_type == CURRENT_SCREEN::SCREEN_MENU) {
                        break;
                    } else if (
                        current_screen_type == CURRENT_SCREEN::SCREEN_SAVEGAME ||
                        current_screen_type == CURRENT_SCREEN::SCREEN_LOADGAME) {
                        break;
                    } else if (current_screen_type == CURRENT_SCREEN::SCREEN_OPTIONS) {
                        break;
                    } else if (current_screen_type == CURRENT_SCREEN::SCREEN_VIDEO_OPTIONS) {
                        break;
                    } else if (current_screen_type == CURRENT_SCREEN::SCREEN_KEYBOARD_OPTIONS) {
                        break;
                    } else {
                        if (current_screen_type > CURRENT_SCREEN::SCREEN_67) {
                            if (current_screen_type == CURRENT_SCREEN::SCREEN_QUICK_REFERENCE) {
                                OnEscape();
                                continue;
                            }
                        } else {
                            if (current_screen_type < CURRENT_SCREEN::SCREEN_64) {
                                switch (current_screen_type) {
                                    case CURRENT_SCREEN::SCREEN_CASTING:
                                        if (some_active_character) {
                                            uActiveCharacter =
                                                some_active_character;
                                            uActiveCharacter =
                                                pParty
                                                    ->GetNextActiveCharacter();
                                            some_active_character = 0;
                                            if (pParty->bTurnBasedModeOn)
                                                pTurnEngine
                                                    ->ApplyPlayerAction();
                                            AfterEnchClickEventId = UIMSG_0;
                                            AfterEnchClickEventSecondParam = 0;
                                            AfterEnchClickEventTimeout = 0;
                                        }
                                        if (ptr_50C9A4_ItemToEnchant &&
                                            ptr_50C9A4_ItemToEnchant->uItemID != ITEM_NULL) {
                                            ptr_50C9A4_ItemToEnchant->uAttributes &= ~ITEM_ENCHANT_ANIMATION_MASK;
                                            ItemEnchantmentTimer = 0;
                                            ptr_50C9A4_ItemToEnchant = nullptr;
                                        }
                                        OnEscape();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_BOOKS:
                                        pEventTimer->Resume();
                                        OnEscape();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_CHEST_INVENTORY:
                                        current_screen_type = CURRENT_SCREEN::SCREEN_CHEST;
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_CHEST:
                                        pWindow2 = pGUIWindow_CurrentMenu;
                                        pWindow2->Release();
                                        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                                        pEventTimer->Resume();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_19:
                                        __debugbreak();
                                        pWindow2 = ptr_507BC8;
                                        pWindow2->Release();
                                        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                                        pEventTimer->Resume();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_REST:  // close rest screen
                                        if (_506F14_resting_stage) {
                                            Rest(_506F18_num_minutes_to_sleep);
                                            pParty->pPlayers[3].SetAsleep(
                                                GameTime(0));
                                            pParty->pPlayers[2].SetAsleep(
                                                GameTime(0));
                                            pParty->pPlayers[1].SetAsleep(
                                                GameTime(0));
                                            pParty->pPlayers[0].SetAsleep(
                                                GameTime(0));
                                        }
                                        if (rest_ui_sky_frame_current) {
                                            rest_ui_sky_frame_current
                                                ->Release();
                                            rest_ui_sky_frame_current = nullptr;
                                        }

                                        if (rest_ui_hourglass_frame_current) {
                                            rest_ui_hourglass_frame_current
                                                ->Release();
                                            rest_ui_hourglass_frame_current =
                                                nullptr;
                                        }

                                        if (uCurrentlyLoadedLevelType ==
                                            LEVEL_Outdoor) {
                                            pOutdoor->UpdateSunlightVectors();
                                            pOutdoor->UpdateFog();
                                        }
                                        _506F18_num_minutes_to_sleep = 0;
                                        _506F14_resting_stage = 0;
                                        OnEscape();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_E:
                                        __debugbreak();
                                        pGUIWindow_CurrentMenu->Release();
                                        current_screen_type = CURRENT_SCREEN::SCREEN_HOUSE;
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_HOUSE:
                                        if (uDialogueType != DIALOGUE_NULL) {
                                            uDialogueType = DIALOGUE_NULL;
                                        }
                                        if (uGameState == GAME_STATE_CHANGE_LOCATION) {
                                            while (HouseDialogPressCloseBtn()) {}
                                        } else {
                                            if (HouseDialogPressCloseBtn())
                                                continue;
                                        }
                                        GetHouseGoodbyeSpeech();
                                        pAudioPlayer->PlaySound(
                                            SOUND_WoodDoorClosing, 814, 0, -1, 0, 0);
                                        pMediaPlayer->Unload();
                                        pGUIWindow_CurrentMenu = window_SpeakInHouse;

                                        OnEscape();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_INPUT_BLV:  // click escape
                                        if (uCurrentHouse_Animation == 153)
                                            PlayHouseSound(0x99u, HouseSound_Greeting_2);
                                        pMediaPlayer->Unload();
                                        if (npcIdToDismissAfterDialogue) {
                                            pParty->hirelingScrollPosition = 0;
                                            pNPCStats
                                                ->pNewNPCData[npcIdToDismissAfterDialogue]
                                                .uFlags &= 0xFFFFFF7F;
                                            pParty->CountHirelings();
                                            npcIdToDismissAfterDialogue = 0;
                                        }
                                        DialogueEnding();
                                        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_NPC_DIALOGUE:  // click escape
                                        if (npcIdToDismissAfterDialogue) {
                                            pParty->hirelingScrollPosition = 0;
                                            pNPCStats
                                                ->pNewNPCData[npcIdToDismissAfterDialogue]
                                                .uFlags &= 0xFFFFFF7F;
                                            pParty->CountHirelings();
                                            npcIdToDismissAfterDialogue = 0;
                                        }
                                        // goto LABEL_317;
                                        DialogueEnding();
                                        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_BRANCHLESS_NPC_DIALOG:  // click
                                                                        // escape
                                        GameUI_StatusBar_ClearEventString();

                                        ReleaseBranchlessDialogue();
                                        DialogueEnding();
                                        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_CHANGE_LOCATION:  // click
                                                                  // escape
                                        if (pParty->vPosition.x < -22528)
                                            pParty->vPosition.x = -22528;
                                        if (pParty->vPosition.x > 22528)
                                            pParty->vPosition.x = 22528;
                                        if (pParty->vPosition.y < -22528)
                                            pParty->vPosition.y = -22528;
                                        if (pParty->vPosition.y > 22528)
                                            pParty->vPosition.y = 22528;
                                        ReleaseBranchlessDialogue();
                                        DialogueEnding();
                                        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_VIDEO:
                                        pMediaPlayer->Unload();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_CHARACTERS:
                                        CharacterUI_ReleaseButtons();
                                        ReleaseAwardsScrollBar();
                                        OnEscape();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_SPELL_BOOK:
                                        OnEscape();
                                        continue;

                                    default:
                                        __debugbreak();  // which GAME_MENU is
                                                         // this?
                                        OnEscape();
                                        continue;
                                }
                                __debugbreak();  // which GAME_MENU is this?
                                OnEscape();
                                continue;
                            }
                            __debugbreak();  // which GAME_MENU is this?
                            CharacterUI_ReleaseButtons();
                            ReleaseAwardsScrollBar();
                        }
                        // __debugbreak();  // which GAME_MENU is this? debug / fallback
                        OnEscape();
                        continue;
                    }
                    continue;

                case UIMSG_ScrollNPCPanel:  // Right and Left button for
                                            // NPCPanel
                    if (uMessageParam) {
                        new OnButtonClick2({626, 179}, {0, 0}, pBtn_NPCRight);
                        v37 = (!pParty->pHirelings[0].pName.empty()) +
                              (!pParty->pHirelings[1].pName.empty()) +
                              (uint8_t)pParty->cNonHireFollowers - 2;
                        // v37 is max scroll position
                        if (pParty->hirelingScrollPosition < v37) {
                            ++pParty->hirelingScrollPosition;
                        }
                    } else {
                        new OnButtonClick2({469, 179}, {0, 0}, pBtn_NPCLeft);
                        if (pParty->hirelingScrollPosition > 0) {
                            --pParty->hirelingScrollPosition;
                        }
                    }
                    GameUI_DrawHiredNPCs();
                    continue;

                case UIMSG_TransitionUI_Confirm:
                    pCurrentFrameMessageQueue->Flush();
                    dword_50CDC8 = 1;
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, PID_INVALID, 0, -1, 0, 0);

                    // PlayHouseSound(  // this is wrong - what is it meant to do??
                    //    uCurrentHouse_Animation,
                    //    HouseSound_NotEnoughMoney);

                    if (pMovie_Track) pMediaPlayer->Unload();
                    DialogueEnding();

                    if (Party_Teleport_X_Pos | Party_Teleport_Y_Pos |
                        Party_Teleport_Z_Pos |
                        Party_Teleport_Cam_Yaw |
                        Party_Teleport_Cam_Pitch |
                        Party_Teleport_Z_Speed) {
                        if (Party_Teleport_X_Pos) {
                            pParty->vPosition.x = Party_Teleport_X_Pos;
                        }
                        if (Party_Teleport_Y_Pos) {
                            pParty->vPosition.y = Party_Teleport_Y_Pos;
                        }
                        if (Party_Teleport_Z_Pos) {
                            pParty->vPosition.z = Party_Teleport_Z_Pos;
                            pParty->uFallStartZ = Party_Teleport_Z_Pos;
                        }
                        if (Party_Teleport_Cam_Yaw) {
                            pParty->sRotationZ = Party_Teleport_Cam_Yaw;
                        }
                        if (Party_Teleport_Cam_Pitch) {
                            pParty->sRotationY = Party_Teleport_Cam_Pitch;
                            v38 = Party_Teleport_Z_Speed;
                            pParty->uFallSpeed = Party_Teleport_Z_Speed;
                        } else {
                            v38 = Party_Teleport_Z_Speed;
                        }
                        if (*Party_Teleport_Map_Name != 48) {
                            //pGameLoadingUI_ProgressBar->Initialize(GUIProgressBar::TYPE_Box);
                            Start_Party_Teleport_Flag =
                                Party_Teleport_X_Pos |
                                Party_Teleport_Y_Pos |
                                Party_Teleport_Z_Pos |
                                Party_Teleport_Cam_Yaw |
                                Party_Teleport_Cam_Pitch | v38;
                            OnMapLeave();
                            Transition_StopSound_Autosave(
                                Party_Teleport_Map_Name,
                                MapStartPoint_Party);
                        }
                    } else {
                        EventProcessor(dword_5C3418, 0, 1, dword_5C341C);
                    }
                    if (iequals(s_SavedMapName.data(), "d05.blv"))
                        pParty->GetPlayingTime().AddDays(4);

                    PlayButtonClickSound();
                    DialogueEnding();
                    back_to_game();
                    OnEscape();
                    continue;
                case UIMSG_TransitionWindowCloseBtn:
                    PlayButtonClickSound();
                    pMediaPlayer->Unload();
                    DialogueEnding();
                    back_to_game();
                    OnEscape();
                    continue;
                case UIMSG_CycleCharacters:
                    uActiveCharacter = CycleCharacter(keyboardInputHandler->IsAdventurerBackcycleToggled());
                    continue;
                case UIMSG_OnTravelByFoot:
                    pCurrentFrameMessageQueue->Flush();
                    dword_50CDC8 = 1;

                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    // encounter_index = (NPCData *)GetTravelTime();
                    pOutdoor->level_filename = pCurrentMapName;
                    if (!engine->IsUnderwater() && pParty->bFlying ||
                        pOutdoor->GetTravelDestination(pParty->vPosition.x,
                                                       pParty->vPosition.y,
                                                       pOut, 20) != 1) {
                        PlayButtonClickSound();
                        if (pParty->vPosition.x < -22528)
                            pParty->vPosition.x = -22528;
                        if (pParty->vPosition.x > 22528)
                            pParty->vPosition.x = 22528;
                        if (pParty->vPosition.y < -22528)
                            pParty->vPosition.y = -22528;
                        if (pParty->vPosition.y > 22528)
                            pParty->vPosition.y = 22528;
                        DialogueEnding();
                        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                    } else {
                        pParty->field_6E4_set0_unused = 0;
                        pParty->field_6E0_set0_unused = 0;
                        CastSpellInfoHelpers::cancelSpellCastInProgress();
                        DialogueEnding();
                        pEventTimer->Pause();
                        pGameLoadingUI_ProgressBar->Initialize(GUIProgressBar::TYPE_Box);
                        pGameLoadingUI_ProgressBar->Progress();
                        SaveGame(1, 0);
                        pGameLoadingUI_ProgressBar->Progress();
                        RestAndHeal(24 * 60 * GetTravelTime());
                        if (pParty->GetFood() > 0) {
                            pParty->RestAndHeal();
                            if (pParty->GetFood() < GetTravelTime()) {
                                for(Player &player : pParty->pPlayers)
                                    player.SetCondition(Condition_Weak, 0);
                                ++pParty->days_played_without_rest;
                            }
                            pParty->TakeFood(GetTravelTime());
                        } else {
                            for (Player &player : pParty->pPlayers)
                                player.SetCondition(Condition_Weak, 0);
                            ++pParty->days_played_without_rest;
                        }
                        pPaletteManager->Reset();
                        pSpriteFrameTable->ResetLoadedFlags();
                        pCurrentMapName = pOut;
                        Level_LoadEvtAndStr(pCurrentMapName.substr(0, pCurrentMapName.rfind('.')));
                        decal_builder->Reset(0);
                        LoadLevel_InitializeLevelEvt();
                        uLevelMapStatsID = pMapStats->GetMapInfo(pCurrentMapName);

                        bNoNPCHiring = 0;

                        engine->SetUnderwater(
                            Is_out15odm_underwater());

                        if (Is_out15odm_underwater() || (pCurrentMapName == "d47.blv"))
                            bNoNPCHiring = 1;
                        PrepareToLoadODM(1u, (ODMRenderParams *)1);
                        bDialogueUI_InitializeActor_NPC_ID = 0;
                        OnMapLoad();
                        pOutdoor->SetFog();
                        TeleportToStartingPoint(uLevel_StartingPointType);
                        bool bOnWater = false;
                        pParty->vPosition.z = GetTerrainHeightsAroundParty2(
                            pParty->vPosition.x, pParty->vPosition.y, &bOnWater, 0);
                        pParty->uFallStartZ = pParty->vPosition.z;
                        engine->_461103_load_level_sub();
                        pEventTimer->Resume();
                        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                        pGameLoadingUI_ProgressBar->Release();
                    }
                    continue;
                case UIMSG_CHANGE_LOCATION_ClickCancelBtn:
                    PlayButtonClickSound();
                    if (pParty->vPosition.x < -22528)
                        pParty->vPosition.x = -22528;
                    if (pParty->vPosition.x > 22528)
                        pParty->vPosition.x = 22528;
                    if (pParty->vPosition.y < -22528)
                        pParty->vPosition.y = -22528;
                    if (pParty->vPosition.y > 22528)
                        pParty->vPosition.y = 22528;
                    DialogueEnding();
                    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                    continue;
                case UIMSG_CastSpell_Telekinesis:
                    HEXRAYS_LOWORD(v42) = vis->get_picked_object_zbuf_val().object_pid;
                    v44 = (uint16_t)v42;
                    v45 = PID_TYPE(v44);
                    uNumSeconds = v44;
                    v46 = PID_ID(v44);
                    if (v45 == OBJECT_Actor) {
                        v47 = pActors[v46].uAIState == Dead;
                        if (!v47) continue;
                        pSpellInfo = static_cast<CastSpellInfo *>(pGUIWindow_CastTargetedSpell->wData.ptr);
                        pSpellInfo->uFlags &= ~ON_CAST_Telekenesis;
                        pSpellInfo->uPlayerID_2 = uMessageParam;
                        pSpellInfo->spell_target_pid = v44;
                        pParty->pPlayers[pSpellInfo->uPlayerID].SetRecoveryTime(
                            300);
                        pGUIWindow_CastTargetedSpell->Release();
                        pGUIWindow_CastTargetedSpell = 0;
                        mouse->SetCursorImage("MICON1");
                        game_ui_status_bar_event_string_time_left = 0;
                        IsEnchantingInProgress = false;
                        back_to_game();
                        continue;
                    }
                    if (v45 == OBJECT_Item) {
                        v47 = (pObjectList
                                   ->pObjects[pSpriteObjects[v46].uObjectDescID]
                                   .uFlags &
                               0x10) == 0;
                        if (!v47) continue;
                        pSpellInfo = static_cast<CastSpellInfo *>(pGUIWindow_CastTargetedSpell->wData.ptr);
                        pSpellInfo->uFlags &= ~ON_CAST_Telekenesis;
                        pSpellInfo->uPlayerID_2 = uMessageParam;
                        pSpellInfo->spell_target_pid = v44;
                        pParty->pPlayers[pSpellInfo->uPlayerID].SetRecoveryTime(
                            300);
                        pGUIWindow_CastTargetedSpell->Release();
                        pGUIWindow_CastTargetedSpell = 0;
                        mouse->SetCursorImage("MICON1");
                        game_ui_status_bar_event_string_time_left = 0;
                        IsEnchantingInProgress = false;
                        back_to_game();
                        continue;
                    }
                    if (v45 == OBJECT_Decoration) {
                        v48 = pLevelDecorations[v46].uEventID == 0;
                    } else {
                        if (v45 != OBJECT_Face) continue;
                        if (uCurrentlyLoadedLevelType != LEVEL_Indoor) {
                            pODMFace = &pOutdoor->pBModels[v44 >> 9].pFaces[v46 & 0x3F];
                            if (!pODMFace->Clickable() ||
                                !pODMFace->sCogTriggeredID)
                                continue;
                            v44 = uNumSeconds;
                            pSpellInfo = static_cast<CastSpellInfo *>(pGUIWindow_CastTargetedSpell->wData.ptr);
                            pSpellInfo->uFlags &= ~ON_CAST_Telekenesis;
                            pSpellInfo->uPlayerID_2 = uMessageParam;
                            pSpellInfo->spell_target_pid = v44;
                            pParty->pPlayers[pSpellInfo->uPlayerID].SetRecoveryTime(300);
                            pGUIWindow_CastTargetedSpell->Release();
                            pGUIWindow_CastTargetedSpell = 0;
                            mouse->SetCursorImage("MICON1");
                            game_ui_status_bar_event_string_time_left = 0;
                            IsEnchantingInProgress = false;
                            back_to_game();
                            continue;
                        }
                        pBLVFace = &pIndoor->pFaces[v46];
                        if (!pBLVFace->Clickable())
                            continue;
                        v48 = pIndoor->pFaceExtras[pBLVFace->uFaceExtraID]
                                  .uEventID == 0;
                    }
                    if (v48) continue;
                    pSpellInfo = static_cast<CastSpellInfo *>(pGUIWindow_CastTargetedSpell->wData.ptr);
                    pSpellInfo->uFlags &= ~ON_CAST_Telekenesis;
                    pSpellInfo->uPlayerID_2 = uMessageParam;
                    pSpellInfo->spell_target_pid = v44;
                    pParty->pPlayers[pSpellInfo->uPlayerID].SetRecoveryTime(
                        300);
                    pGUIWindow_CastTargetedSpell->Release();
                    pGUIWindow_CastTargetedSpell = 0;
                    mouse->SetCursorImage("MICON1");
                    game_ui_status_bar_event_string_time_left = 0;
                    IsEnchantingInProgress = false;
                    back_to_game();
                    continue;
                case UIMSG_CastSpell_Character_Big_Improvement:  // Preservation
                                                                 // and blessing,
                                                                 // treatment
                                                                 // paralysis,
                                                                 // hand
                                                                 // hammers(individual
                                                                 // upgrade)
                case UIMSG_CastSpell_Character_Small_Improvement:  // Fate, cure
                case UIMSG_HiredNPC_CastSpell:
                    pCurrentFrameMessageQueue->Flush();
                    if (IsEnchantingInProgress) {
                        uActiveCharacter = uMessageParam;
                    } else {
                        if (pGUIWindow_CastTargetedSpell) {
                            pSpellInfo = static_cast<CastSpellInfo *>(pGUIWindow_CastTargetedSpell->wData.ptr);
                            switch (uMessage) {
                                case UIMSG_CastSpell_Character_Big_Improvement:
                                    pSpellInfo->uFlags &= ~ON_CAST_SinglePlayer_BigImprovementAnim;
                                    break;
                                case UIMSG_CastSpell_Character_Small_Improvement:
                                    pSpellInfo->uFlags &= ~ON_CAST_MonsterSparkles;
                                    break;
                                case UIMSG_HiredNPC_CastSpell:
                                    pSpellInfo->uFlags &= ~ON_CAST_DarkSacrifice;
                                    break;
                                default:
                                    break;
                            }
                            pSpellInfo->uPlayerID_2 = uMessageParam;
                            pParty->pPlayers[pSpellInfo->uPlayerID]
                                .SetRecoveryTime(300);
                            pGUIWindow_CastTargetedSpell->Release();
                            pGUIWindow_CastTargetedSpell = 0;
                            pEventTimer->Resume();
                            mouse->SetCursorImage("MICON1");
                            game_ui_status_bar_event_string_time_left = 0;
                            IsEnchantingInProgress = false;
                        }
                    }
                    continue;

                case UIMSG_BF:
                    __debugbreak();
                    dword_50CDC8 = 1;
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    SaveGame(1, 0);
                    pCurrentMapName = pMapStats->pInfos[uHouse_ExitPic].pFilename;
                    dword_6BE364_game_settings_1 |= GAME_SETTINGS_0001;
                    uGameState = GAME_STATE_CHANGE_LOCATION;
                    // v53 = p2DEvents_minus1_::30[26 * (unsigned
                    // int)ptr_507BC0->ptr_1C];
                    v53 = p2DEvents[window_SpeakInHouse->wData.val - 1]._quest_bit;
                    if (v53 < 0) {
                        v54 = abs(v53) - 1;
                        Party_Teleport_Cam_Pitch = 0;
                        Party_Teleport_Z_Speed = 0;
                        v55 = dword_4E4560[v54];
                        Party_Teleport_Y_Pos = dword_4E4578[v54];
                        v56 = dword_4E4590[v54];
                        v57 = dword_4E45A8[v54];
                        Party_Teleport_X_Pos = v55;
                        Party_Teleport_Cam_Yaw = v57;
                        Party_Teleport_Z_Pos = v56;
                        Start_Party_Teleport_Flag =
                            v55 | Party_Teleport_Y_Pos | v56 | v57;
                    }
                    HouseDialogPressCloseBtn();
                    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                    continue;

                case UIMSG_OnCastTownPortal:
                    pAudioPlayer->PauseSounds(-1);
                    pGUIWindow_CurrentMenu = new GUIWindow_TownPortalBook();  // (char *)uMessageParam);
                    continue;

                case UIMSG_OnCastLloydsBeacon:
                    pAudioPlayer->PauseSounds(-1);
                    pGUIWindow_CurrentMenu = new GUIWindow_LloydsBook();
                    continue;

                case UIMSG_LloydsBeacon_FlippingBtn:
                    bRecallingBeacon = uMessageParam;
                    pAudioPlayer->PlaySound(bRecallingBeacon ? SOUND_TurnPage2 : SOUND_TurnPage1, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_HintBeaconSlot: {
                    if (!pGUIWindow_CurrentMenu) {
                        continue;
                    }
                    Player &player = pParty->pPlayers[CurrentLloydPlayerID];
                    if (uMessageParam >= player.vBeacons.size()) {
                        continue;
                    }
                    LloydBeacon &beacon = player.vBeacons[uMessageParam];
                    if (bRecallingBeacon) {
                        if (beacon.uBeaconTime) {
                            std::string pMapName = pMapStats->pInfos[pMapStats->sub_410D99_get_map_index(beacon.SaveFileID)].pName;
                            GameUI_StatusBar_Set(localization->FormatString(LSTR_FMT_RECALL_TO_S, pMapName.c_str()));
                        }
                        continue;
                    }
                    pMapNum = pMapStats->GetMapInfo(pCurrentMapName);
                    std::string pMapName = "Not in Map Stats";
                    if (pMapNum) {
                        pMapName = pMapStats->pInfos[pMapNum].pName;
                    }

                    if (beacon.uBeaconTime) {
                        std::string pMapName2 = pMapStats->pInfos[pMapStats->sub_410D99_get_map_index(beacon.SaveFileID)].pName;
                        GameUI_StatusBar_Set(localization->FormatString(LSTR_FMT_SET_S_OVER_S, pMapName.c_str(), pMapName2.c_str()));
                    } else {
                        GameUI_StatusBar_Set(localization->FormatString(LSTR_FMT_SET_S_TO_S, pMapName.c_str()));
                    }
                    continue;
                }
                case UIMSG_CloseAfterInstallBeacon:
                    dword_50CDC8 = 1;
                    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                    continue;
                case UIMSG_InstallBeacon: {
                    Player &player = pParty->pPlayers[CurrentLloydPlayerID];
                    if ((player.vBeacons.size() <= uMessageParam) && bRecallingBeacon) {
                        continue;
                    }

                    _506360_installing_beacon = true;

                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uExpertLevelMana);
                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMasterLevelMana);
                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMagisterLevelMana);
                    player.SpendMana(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana);

                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uExpertLevelRecovery);
                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMasterLevelRecovery);
                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMagisterLevelRecovery);
                    signed int sRecoveryTime = pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery;
                    if (pParty->bTurnBasedModeOn) {
                        pParty->pTurnBasedPlayerRecoveryTimes[CurrentLloydPlayerID] = sRecoveryTime;
                        player.SetRecoveryTime(sRecoveryTime);
                        pTurnEngine->ApplyPlayerAction();
                    } else {
                        player.SetRecoveryTime(debug_non_combat_recovery_mul * sRecoveryTime * flt_debugrecmod3);
                    }
                    pAudioPlayer->PlaySpellSound(SPELL_WATER_LLOYDS_BEACON, 0);
                    if (bRecallingBeacon) {
                        if (pCurrentMapName != pGames_LOD->GetSubNodeName(player.vBeacons[uMessageParam].SaveFileID)) {
                            SaveGame(1, 0);
                            OnMapLeave();
                            pCurrentMapName = pGames_LOD->GetSubNodeName(player.vBeacons[uMessageParam].SaveFileID);
                            dword_6BE364_game_settings_1 |= GAME_SETTINGS_0001;
                            uGameState = GAME_STATE_CHANGE_LOCATION;
                            Party_Teleport_X_Pos = player.vBeacons[uMessageParam].PartyPos_X;
                            Party_Teleport_Y_Pos = player.vBeacons[uMessageParam].PartyPos_Y;
                            Party_Teleport_Z_Pos = player.vBeacons[uMessageParam].PartyPos_Z;
                            Party_Teleport_Cam_Yaw = player.vBeacons[uMessageParam].PartyRot_X;
                            Party_Teleport_Cam_Pitch = player.vBeacons[uMessageParam].PartyRot_Y;
                            Start_Party_Teleport_Flag = 1;
                        } else {
                            pParty->vPosition.x = player.vBeacons[uMessageParam].PartyPos_X;
                            pParty->vPosition.y = player.vBeacons[uMessageParam].PartyPos_Y;
                            pParty->vPosition.z = player.vBeacons[uMessageParam].PartyPos_Z;
                            pParty->uFallStartZ = pParty->vPosition.z;
                            pParty->sRotationZ = player.vBeacons[uMessageParam].PartyRot_X;
                            pParty->sRotationY = player.vBeacons[uMessageParam].PartyRot_Y;
                        }
                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                        pGUIWindow_CurrentMenu->Release();
                        pGUIWindow_CurrentMenu = 0;
                    } else {
                        player.SetBeacon(uMessageParam, LloydsBeaconSpellDuration);
                    }
                    continue;
                }
                case UIMSG_ClickTownInTP: {
                    //if (uGameState == GAME_STATE_CHANGE_LOCATION) continue;
                    int16_t fountainBit;
                    switch (uMessageParam) {
                        case 0:
                            fountainBit = QBIT_FOUNTAIN_IN_HARMONDALE_ACTIVATED;
                            break;
                        case 1:
                            fountainBit = QBIT_FOUNTAIN_IN_PIERPONT_ACTIVATED;
                            break;
                        case 2:
                            fountainBit = QBIT_FOUNTAIN_IN_MOUNT_NIGHON_ACTIVATED;
                            break;
                        case 3:
                            fountainBit = QBIT_FOUNTAIN_IN_EVENMORN_ISLE_ACTIVATED;
                            break;
                        case 4:
                            fountainBit = QBIT_FOUNTAIN_IN_CELESTIA_ACTIVATED;
                            break;
                        case 5:
                            fountainBit = QBIT_FOUNTAIN_IN_THE_PIT_ACTIVATED;
                            break;
                        default:
                            Assert(false && "Bad TP param");
                            break;
                    }

                    // check if tp location is unlocked
                    if (!_449B57_test_bit(pParty->_quest_bits, fountainBit) && !engine->config->debug.TownPortal.Get())
                        continue;

                    // begin TP
                    SaveGame(1, 0);
                    v64 = pMapStats->GetMapInfo(pCurrentMapName);
                    v65 = uMessageParam;
                    // if in current map
                    if (v64 == TownPortalList[uMessageParam].uMapInfoID) {
                        pParty->vPosition.x = TownPortalList[v65].pos.x;
                        pParty->vPosition.y = TownPortalList[v65].pos.y;
                        pParty->vPosition.z = TownPortalList[v65].pos.z;
                        pParty->uFallStartZ = pParty->vPosition.z;
                        pParty->sRotationZ = TownPortalList[v65].rot_y;
                        pParty->sRotationY = TownPortalList[v65].rot_x;
                    } else {  // if change map
                        OnMapLeave();
                        dword_6BE364_game_settings_1 |= GAME_SETTINGS_0001;
                        uGameState = GAME_STATE_CHANGE_LOCATION;
                        pCurrentMapName = pMapStats->pInfos[TownPortalList[uMessageParam].uMapInfoID].pFilename;
                        Start_Party_Teleport_Flag = 1;
                        Party_Teleport_X_Pos = TownPortalList[uMessageParam].pos.x;
                        Party_Teleport_Y_Pos = TownPortalList[uMessageParam].pos.y;
                        Party_Teleport_Z_Pos = TownPortalList[uMessageParam].pos.z;
                        Party_Teleport_Cam_Yaw = TownPortalList[uMessageParam].rot_y;
                        Party_Teleport_Cam_Pitch = TownPortalList[uMessageParam].rot_x;
                        Actor::InitializeActors();
                    }

                    assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelMana == pSpellDatas[SPELL_WATER_TOWN_PORTAL].uExpertLevelMana);
                    assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelMana == pSpellDatas[SPELL_WATER_TOWN_PORTAL].uMasterLevelMana);
                    assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelMana == pSpellDatas[SPELL_WATER_TOWN_PORTAL].uMagisterLevelMana);
                    pParty->pPlayers[TownPortalCasterId].SpendMana(pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelMana);
                    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                    continue;
                }
                case UIMSG_HintTownPortal: {
                    int16_t fountainBit;
                    switch (uMessageParam) {
                        case 0:
                            fountainBit = QBIT_FOUNTAIN_IN_HARMONDALE_ACTIVATED;
                            break;
                        case 1:
                            fountainBit = QBIT_FOUNTAIN_IN_PIERPONT_ACTIVATED;
                            break;
                        case 2:
                            fountainBit = QBIT_FOUNTAIN_IN_MOUNT_NIGHON_ACTIVATED;
                            break;
                        case 3:
                            fountainBit = QBIT_FOUNTAIN_IN_EVENMORN_ISLE_ACTIVATED;
                            break;
                        case 4:
                            fountainBit = QBIT_FOUNTAIN_IN_CELESTIA_ACTIVATED;
                            break;
                        case 5:
                            fountainBit = QBIT_FOUNTAIN_IN_THE_PIT_ACTIVATED;
                            break;
                        default:
                            Assert(false && "Bad TP param");
                            break;
                    }

                    if (!_449B57_test_bit(pParty->_quest_bits, fountainBit) && !engine->config->debug.TownPortal.Get()) {
                        render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);
                        continue;
                    }
                    // LABEL_506:
                    std::string townName = "";
                    switch (uMessageParam) {
                        case 0:
                            townName = pMapStats->pInfos[21].pName;
                            break;
                        case 1:
                            townName = pMapStats->pInfos[4].pName;
                            break;
                        case 2:
                            townName = pMapStats->pInfos[3].pName;
                            break;
                        case 3:
                            townName = pMapStats->pInfos[10].pName;
                            break;
                        case 4:
                            townName = pMapStats->pInfos[7].pName;
                            break;
                        case 5:
                            townName = pMapStats->pInfos[8].pName;
                            break;
                        default:
                            Assert(false && "Bad TP param");
                            break;
                    }
                    GameUI_StatusBar_Set(localization->FormatString(LSTR_TOWN_PORTAL_TO_S, townName.c_str()));
                    continue;
                }
                case UIMSG_ShowGameOverWindow: {
                    pGameOverWindow = new GUIWindow_GameOver();
                    uGameState = GAME_STATE_FINAL_WINDOW;
                    continue;
                }
                case UIMSG_OnGameOverWindowClose:
                    pAudioPlayer->PauseSounds(-1);
                    SaveGame(1, 0);

                    pParty->vPosition.x = -17331;  // respawn point in Harmondale
                    pParty->vPosition.y = 12547;
                    pParty->vPosition.z = 465;
                    pParty->sRotationZ = 0;
                    pParty->uFallStartZ = pParty->vPosition.z;
                    pParty->sRotationY = 0;
                    pParty->uFallSpeed = 0;
                    pParty->field_6E4_set0_unused = 0;
                    pParty->field_6E0_set0_unused = 0;

                    // change map to Harmondale
                    pCurrentMapName = "out02.odm";
                    Party_Teleport_X_Pos = pParty->vPosition.x;
                    Party_Teleport_Y_Pos = pParty->vPosition.y;
                    Party_Teleport_Z_Pos = pParty->vPosition.z;
                    Party_Teleport_Cam_Yaw = pParty->sRotationZ;
                    Party_Teleport_Cam_Pitch = pParty->sRotationY;
                    Start_Party_Teleport_Flag = 1;
                    PrepareWorld(1);
                    Actor::InitializeActors();

                    uGameState = GAME_STATE_PLAYING;

                    for (int i = 0; i < 4; ++i)
                        pParty->pPlayers[i].PlayEmotion(CHARACTER_EXPRESSION_SMILE, 0);

                    // strcpy((char *)userInputHandler->pPressedKeysBuffer, "2");
                    // __debugbreak();  // missed break/continue?
                    continue;

                case UIMSG_DD: {
                    __debugbreak();
                    // sprintf(tmp_str.data(), "%s",
                    // pKeyActionMap->pPressedKeysBuffer);
                    FrameTableTxtLine frameTableTxtLine;
                    txt_file_frametable_parser(keyboardInputHandler->GetTextInput().c_str(), &frameTableTxtLine);
                    std::string status_string;
                    if (frameTableTxtLine.uPropCount == 1) {
                        size_t map_index = atoi(frameTableTxtLine.pProperties[0]);
                        if (map_index <= 0 || map_index >= 77) continue;
                        std::string map_name = pMapStats->pInfos[map_index].pFilename;
                        if (pGames_LOD->GetSubNodeIndex(map_name) < (pGames_LOD->GetSubNodesCount() / 2)) {
                            pCurrentMapName = map_name;
                            dword_6BE364_game_settings_1 |= GAME_SETTINGS_0001;
                            uGameState = GAME_STATE_CHANGE_LOCATION;
                            OnMapLeave();
                            continue;
                        }
                        status_string = fmt::format("No map found for {}", pMapStats->pInfos[map_index].pName);
                    } else {
                        if (frameTableTxtLine.uPropCount != 3) continue;
                        int x = atoi(frameTableTxtLine.pProperties[0]);
                        int y = atoi(frameTableTxtLine.pProperties[1]);
                        int z = atoi(frameTableTxtLine.pProperties[2]);
                        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                            if (pIndoor->GetSector(x, y, z)) {
                                pParty->vPosition.x = x;
                                pParty->vPosition.y = y;
                                pParty->vPosition.z = z;
                                pParty->uFallStartZ = z;
                                continue;
                            }
                        } else {
                            if ((x > -32768) && (x < 32768) && (y > -32768) && (y < 32768) && (z >= 0) && (z < 10000)) {
                                pParty->vPosition.x = x;
                                pParty->vPosition.y = y;
                                pParty->vPosition.z = z;
                                pParty->uFallStartZ = z;
                                continue;
                            }
                        }
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                        status_string = "Can't jump to that location!";
                    }
                    GameUI_SetStatusBar(status_string);
                    continue;
                }
                case UIMSG_CastQuickSpell: {
                    if (engine->IsUnderwater()) {
                        GameUI_SetStatusBar(LSTR_CANT_DO_UNDERWATER);
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                        continue;
                    }
                    if (!uActiveCharacter ||
                        (pPlayer2 = pPlayers[uActiveCharacter],
                         pPlayer2->uTimeToRecovery))
                        continue;
                    pushSpellOrRangedAttack(pPlayer2->uQuickSpell, uActiveCharacter - 1,
                                            0, 0, uActiveCharacter);
                    continue;
                }

                case UIMSG_CastSpell_Monster_Improvement:
                case UIMSG_CastSpell_Shoot_Monster:  // FireBlow, Lightning, Ice
                                                     // Lightning, Swarm,
                    v83 = vis->get_picked_object_zbuf_val();
                    v44 = v83.object_pid;
                    v84 = v83.depth;
                    if (PID_TYPE(v44) != OBJECT_Actor || v84 >= engine->config->gameplay.RangedAttackDepth.Get())
                        continue;
                    pSpellInfo = static_cast<CastSpellInfo *>(pGUIWindow_CastTargetedSpell->wData.ptr);
                    if (uMessage == UIMSG_CastSpell_Shoot_Monster) {
                        pSpellInfo->uFlags &= ~ON_CAST_TargetCrosshair;
                    } else {
                        if (uMessage == UIMSG_CastSpell_Monster_Improvement)
                            pSpellInfo->uFlags &= ~ON_CAST_MonsterSparkles;
                        else
                            pSpellInfo->uFlags &= ~ON_CAST_DarkSacrifice;
                    }
                    pSpellInfo->uPlayerID_2 = uMessageParam;
                    pSpellInfo->spell_target_pid = v44;
                    pParty->pPlayers[pSpellInfo->uPlayerID].SetRecoveryTime(300);
                    pGUIWindow_CastTargetedSpell->Release();
                    pGUIWindow_CastTargetedSpell = 0;
                    mouse->SetCursorImage("MICON1");
                    game_ui_status_bar_event_string_time_left = 0;
                    IsEnchantingInProgress = false;
                    back_to_game();
                    continue;
                case UIMSG_1C:
                    __debugbreak();
                    if (!uActiveCharacter || current_screen_type != CURRENT_SCREEN::SCREEN_GAME)
                        continue;
                    __debugbreak();  // ptr_507BC8 = GUIWindow::Create(0, 0,
                                     // window->GetWidth(), window->GetHeight(),
                                     // WINDOW_68, uMessageParam, 0);
                    current_screen_type = CURRENT_SCREEN::SCREEN_19;
                    pEventTimer->Pause();
                    continue;
                case UIMSG_STEALFROMACTOR:
                    if (!uActiveCharacter) continue;
                    if (!pParty->bTurnBasedModeOn) {
                        if (pActors[uMessageParam].uAIState == AIState::Dead)
                            pActors[uMessageParam].LootActor();
                        else
                            Actor::StealFrom(uMessageParam);
                        continue;
                    }
                    if (pTurnEngine->turn_stage == TE_WAIT ||
                        pTurnEngine->turn_stage == TE_MOVEMENT)
                        continue;
                    if (!(pTurnEngine->flags & TE_HAVE_PENDING_ACTIONS)) {
                        if (pActors[uMessageParam].uAIState == AIState::Dead)
                            pActors[uMessageParam].LootActor();
                        else
                            Actor::StealFrom(uMessageParam);
                    }
                    continue;

                case UIMSG_Attack:
                    if (!uActiveCharacter) continue;
                    if (!pParty->bTurnBasedModeOn) {
                        Player::_42ECB5_PlayerAttacksActor();
                        continue;
                    }
                    if (pTurnEngine->turn_stage == TE_WAIT ||
                        pTurnEngine->turn_stage == TE_MOVEMENT)
                        continue;
                    if (!(pTurnEngine->flags & TE_HAVE_PENDING_ACTIONS))
                        Player::_42ECB5_PlayerAttacksActor();
                    continue;
                case UIMSG_ExitRest:
                    new OnCancel({pButton_RestUI_Exit->uX, pButton_RestUI_Exit->uY}, {0, 0}, pButton_RestUI_Exit, localization->GetString(LSTR_EXIT_REST));
                    continue;
                case UIMSG_Wait5Minutes:
                    if (_506F14_resting_stage == 2) {
                        GameUI_SetStatusBar(LSTR_ALREADY_RESTING);
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                        continue;
                    }
                    new OnButtonClick2({pButton_RestUI_Wait5Minutes->uX, pButton_RestUI_Wait5Minutes->uY}, {0, 0}, pButton_RestUI_Wait5Minutes,
                        localization->GetString(LSTR_WAIT_5_MINUTES));
                    _506F14_resting_stage = 1;
                    _506F18_num_minutes_to_sleep = 5;
                    continue;
                case UIMSG_Wait1Hour:
                    if (_506F14_resting_stage == 2) {
                        GameUI_SetStatusBar(LSTR_ALREADY_RESTING);
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                        continue;
                    }
                    new OnButtonClick2({pButton_RestUI_Wait1Hour->uX, pButton_RestUI_Wait1Hour->uY}, {0, 0}, pButton_RestUI_Wait1Hour,
                        localization->GetString(LSTR_WAIT_1_HOUR));
                    _506F14_resting_stage = 1;
                    _506F18_num_minutes_to_sleep = 60;
                    continue;
                case UIMSG_RentRoom:
                    _506F14_resting_stage = 2;

                    pGUIWindow_CurrentMenu = new GUIWindow_Rest();

                    v86 =
                        60 * (_494820_training_time(pParty->uCurrentHour) + 1) -
                        pParty->uCurrentMinute;
                    _506F18_num_minutes_to_sleep = v86;
                    if (uMessageParam == 111 || uMessageParam == 114 ||
                        uMessageParam == 116)  // 107 = Emerald Isle tavern
                        _506F18_num_minutes_to_sleep = v86 + 12 * 60;
                    _506F14_resting_stage = 2;
                    pParty->RestAndHeal();
                    pParty->days_played_without_rest = 0;
                    pParty->pPlayers[3].SetAsleep(GameTime(1));
                    pParty->pPlayers[2].SetAsleep(GameTime(1));
                    pParty->pPlayers[1].SetAsleep(GameTime(1));
                    pParty->pPlayers[0].SetAsleep(GameTime(1));
                    continue;
                case UIMSG_RestWindow:
                    pCurrentFrameMessageQueue->Flush();
                    // toggle
                    //if (current_screen_type == CURRENT_SCREEN::SCREEN_REST) {
                    //    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                    //    continue;
                    //}
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) continue;
                    CloseTargetedSpellWindow();

                    if (CheckActors_proximity()) {
                        if (pParty->bTurnBasedModeOn) {
                            GameUI_SetStatusBar(LSTR_CANT_REST_IN_TURN_BASED);
                            continue;
                        }

                        if (pParty->uFlags & (PARTY_FLAGS_1_AIRBORNE | PARTY_FLAGS_1_STANDING_ON_WATER)) // airbourne or on water
                            GameUI_SetStatusBar(LSTR_CANT_REST_HERE);
                        else
                            GameUI_SetStatusBar(LSTR_HOSTILE_ENEMIES_NEARBY);

                        if (!uActiveCharacter) continue;
                        pPlayers[uActiveCharacter]->PlaySound(SPEECH_CantRestHere, 0);
                        continue;
                    }
                    if (pParty->bTurnBasedModeOn) {
                        GameUI_SetStatusBar(LSTR_CANT_REST_IN_TURN_BASED);
                        continue;
                    }

                    if (!(pParty->uFlags & (PARTY_FLAGS_1_AIRBORNE | PARTY_FLAGS_1_STANDING_ON_WATER))) {
                        pGUIWindow_CurrentMenu = new GUIWindow_Rest();
                        continue;
                    } else {
                        if (engine->config->debug.VerboseLogging.Get()) {
                            if (pParty->uFlags & PARTY_FLAGS_1_AIRBORNE)
                                logger->Info("Party is airborne");
                            if (pParty->uFlags & PARTY_FLAGS_1_STANDING_ON_WATER)
                                logger->Info("Party on water");
                        }
                    }

                    if (pParty->bTurnBasedModeOn) {
                        GameUI_SetStatusBar(LSTR_CANT_REST_IN_TURN_BASED);
                        continue;
                    }

                    if (pParty->uFlags & (PARTY_FLAGS_1_AIRBORNE | PARTY_FLAGS_1_STANDING_ON_WATER))
                        GameUI_SetStatusBar(LSTR_CANT_REST_HERE);
                    else
                        GameUI_SetStatusBar(LSTR_HOSTILE_ENEMIES_NEARBY);

                    if (!uActiveCharacter) continue;
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_CantRestHere, 0);
                    continue;
                case UIMSG_Rest8Hour:
                    pCurrentFrameMessageQueue->Clear(); // TODO: sometimes it is called twice, prevent that for now and investigate why later
                    if (_506F14_resting_stage != 0) {
                        GameUI_SetStatusBar(LSTR_ALREADY_RESTING);
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                        continue;
                    }
                    if (pParty->GetFood() < uRestUI_FoodRequiredToRest) {
                        GameUI_SetStatusBar(LSTR_NOT_ENOUGH_FOOD);
                        if (uActiveCharacter &&
                            pPlayers[uActiveCharacter]->CanAct())
                            pPlayers[uActiveCharacter]->PlaySound(SPEECH_NotEnoughFood, 0);
                    } else {
                        pParty->pPlayers[3].conditions.Set(Condition_Sleep, pParty->GetPlayingTime());
                        pParty->pPlayers[2].conditions.Set(Condition_Sleep, pParty->GetPlayingTime());
                        pParty->pPlayers[1].conditions.Set(Condition_Sleep, pParty->GetPlayingTime());
                        pParty->pPlayers[0].conditions.Set(Condition_Sleep, pParty->GetPlayingTime());
                        v90 = pMapStats->GetMapInfo(pCurrentMapName);
                        if (!v90)
                            v90 = grng->Random(pMapStats->uNumMaps + 1);
                        pMapInfo = &pMapStats->pInfos[v90];

                        if (grng->Random(100) + 1 <= pMapInfo->Encounter_percent) {
                            v91 = grng->Random(100);
                            v92 = pMapInfo->EncM1percent;
                            v93 = v91 + 1;
                            if (v93 > v92)
                                encounter_index = v93 > v92 + pMapInfo->EncM2percent + 2;
                            else
                                encounter_index = 1;

                            if (!SpawnEncounterMonsters(pMapInfo, encounter_index))
                                encounter_index = 0;

                            if (encounter_index) {
                                pPlayerNum = grng->Random(4);
                                pParty->pPlayers[pPlayerNum].conditions.Reset(Condition_Sleep);
                                Rest(grng->Random(6) + 60);
                                _506F18_num_minutes_to_sleep = 0;
                                _506F14_resting_stage = 0;

                                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                                GameUI_SetStatusBar(LSTR_ENCOUNTER);
                                pAudioPlayer->PlaySound(SOUND_encounter, 0, 0, -1, 0, 0);
                                continue;
                            }
                        }
                        pParty->TakeFood(uRestUI_FoodRequiredToRest);
                        _506F18_num_minutes_to_sleep = 480;
                        _506F14_resting_stage = 2;
                        pParty->RestAndHeal();
                        pParty->days_played_without_rest = 0;
                        pParty->pPlayers[3].SetAsleep(GameTime(1));
                        pParty->pPlayers[2].SetAsleep(GameTime(1));
                        pParty->pPlayers[1].SetAsleep(GameTime(1));
                        pParty->pPlayers[0].SetAsleep(GameTime(1));
                    }
                    continue;
                case UIMSG_WaitTillDawn:
                    if (_506F14_resting_stage == 2) {
                        GameUI_SetStatusBar(LSTR_ALREADY_RESTING);
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                        continue;
                    }
                    new OnButtonClick2({pButton_RestUI_WaitUntilDawn->uX, pButton_RestUI_WaitUntilDawn->uY}, {0, 0}, pButton_RestUI_WaitUntilDawn,
                        localization->GetString(LSTR_WAIT_UNTIL_DAWN));
                    v97 = _494820_training_time(pParty->uCurrentHour);
                    _506F14_resting_stage = 1;
                    _506F18_num_minutes_to_sleep =
                        60 * v97 - pParty->uCurrentMinute;
                    continue;

                case UIMSG_HintSelectRemoveQuickSpellBtn: {
                    if (quick_spell_at_page && byte_506550) {
                        GameUI_StatusBar_Set(localization->FormatString(
                            LSTR_FMT_SET_S_AS_READY_SPELL,
                            pSpellStats->pInfos[quick_spell_at_page +
                                         11 * pPlayers[uActiveCharacter]
                                                  ->lastOpenedSpellbookPage].pName));
                    } else {
                        if (pPlayers[uActiveCharacter]->uQuickSpell)
                            GameUI_StatusBar_Set(
                                localization->GetString(LSTR_CLICK_TO_REMOVE_QUICKSPELL));
                        else
                            GameUI_StatusBar_Set(
                                localization->GetString(LSTR_CLICK_TO_SET_QUICKSPELL));
                    }
                    continue;
                }

                case UIMSG_SPellbook_ShowHightlightedSpellInfo: {
                    if (!uActiveCharacter)  // || (uNumSeconds = (unsigned
                                            // int)pPlayers[uActiveCharacter],!*(char
                                            // *)(uNumSeconds + 11 * *(char
                                            // *)(uNumSeconds + 6734) +
                                            // uMessageParam + 402)))
                        continue;  // this used to check if player had the spell
                                   // activated - no longer rquired here ??

                    if (sub_4637E0_is_there_popup_onscreen())
                        dword_507B00_spell_info_to_draw_in_popup =
                            uMessageParam + 1;
                    v98 = pPlayers[uActiveCharacter]->lastOpenedSpellbookPage;
                    if (quick_spell_at_page - 1 == uMessageParam) {
                        GameUI_StatusBar_Set(localization->FormatString(
                            LSTR_CAST_S,
                            pSpellStats->pInfos[uMessageParam + 11 * v98 + 1].pName));
                    } else {
                        GameUI_StatusBar_Set(localization->FormatString(
                            LSTR_SELECT_S,
                            pSpellStats->pInfos[uMessageParam + 11 * v98 + 1].pName));
                    }
                    continue;
                }

                case UIMSG_ClickInstallRemoveQuickSpellBtn: {
                    new OnButtonClick2({pBtn_InstallRemoveSpell->uX, pBtn_InstallRemoveSpell->uY}, {0, 0}, pBtn_InstallRemoveSpell);
                    if (!uActiveCharacter) continue;
                    pPlayer10 = pPlayers[uActiveCharacter];
                    if (!byte_506550 || !quick_spell_at_page) {
                        pPlayer10->uQuickSpell = SPELL_NONE;
                        quick_spell_at_page = 0;
                        pAudioPlayer->PlaySound(SOUND_fizzle, 0, 0, -1, 0, 0);
                        continue;
                    }
                    // TODO(captainurist): encapsulate the arithmetic below
                    pPlayers[uActiveCharacter]->uQuickSpell = static_cast<SPELL_TYPE>(
                        quick_spell_at_page + 11 * pPlayers[uActiveCharacter]->lastOpenedSpellbookPage);
                    if (uActiveCharacter) pPlayer10->PlaySound(SPEECH_SetQuickSpell, 0);
                    byte_506550 = 0;
                    continue;
                }

                case UIMSG_SpellBook_PressTab:  //перелистывание страниц
                                                //клавишей Tab
                {
                    if (!uActiveCharacter) continue;
                    int skill_count = 0;
                    uAction = 0;
                    int page = 0;
                    for (PLAYER_SKILL_TYPE i : MagicSkills()) {
                        if (pPlayers[uActiveCharacter]->pActiveSkills[i] || engine->config->debug.AllMagic.Get()) {
                            if (pPlayers[uActiveCharacter]->lastOpenedSpellbookPage == page)
                                uAction = skill_count;
                            spellbookPages[skill_count++] = page;
                        }

                        page++;
                    }
                    if (!skill_count) {  //нет скиллов
                        pAudioPlayer->PlaySound(vrng->RandomBool() ? SOUND_TurnPage2 : SOUND_TurnPage1, 0, 0, -1, 0, 0);
                    } else {
                        if (keyboardInputHandler->IsSpellBackcycleToggled()) {
                            --uAction;
                            if (uAction < 0)
                                uAction = skill_count - 1;
                        } else {
                            ++uAction;
                            if (uAction >= skill_count)
                                uAction = 0;
                        }
                        ((GUIWindow_Spellbook *)pGUIWindow_CurrentMenu)->OpenSpellbookPage(spellbookPages[uAction]);
                    }
                    continue;
                }
                case UIMSG_OpenSpellbookPage:
                    if (pTurnEngine->turn_stage == TE_MOVEMENT ||
                        !uActiveCharacter ||
                        uMessageParam ==
                            pPlayers[uActiveCharacter]->lastOpenedSpellbookPage)
                        continue;
                    ((GUIWindow_Spellbook *)pGUIWindow_CurrentMenu)->OpenSpellbookPage(uMessageParam);
                    continue;
                case UIMSG_SelectSpell: {
                    if (pTurnEngine->turn_stage == TE_MOVEMENT) continue;
                    if (!uActiveCharacter) continue;

                    //  uNumSeconds = (unsigned int)pPlayers[uActiveCharacter];
                    Player *player = pPlayers[uActiveCharacter];
                    if (player->spellbook.pChapters[player->lastOpenedSpellbookPage].bIsSpellAvailable[uMessageParam]
                        || engine->config->debug.AllMagic.Get()) {
                        if (quick_spell_at_page - 1 == uMessageParam) {
                            pGUIWindow_CurrentMenu->Release();  // spellbook close
                            pEventTimer->Resume();
                            current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                            v103 = quick_spell_at_page + 11 * player->lastOpenedSpellbookPage;
                            /*if ( dword_50C9E8 < 40 )
                            {
                            dword_50C9EC[3 * dword_50C9E8] =
                            UIMSG_CastSpellFromBook; dword_50C9EC[3 *
                            dword_50C9E8 + 1] = v103; dword_50C9EC[3 *
                            dword_50C9E8 + 2] = uActiveCharacter - 1;
                            ++dword_50C9E8;
                            }*/
                            // Processing must happen on next frame because need to close spell book and update
                            // drawing object list which is used to count actors for some spells
                            pNextFrameMessageQueue->AddGUIMessage( UIMSG_CastSpellFromBook, v103, uActiveCharacter - 1);
                            //  pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_CastSpellFromBook,
                            //  v103, uActiveCharacter - 1);
                        } else {
                            byte_506550 = 1;
                            quick_spell_at_page = uMessageParam + 1;
                        }
                    }
                    continue;
                }

                case UIMSG_CastSpellFromBook:
                    if (pTurnEngine->turn_stage != TE_MOVEMENT) {
                        pushSpellOrRangedAttack(static_cast<SPELL_TYPE>(uMessageParam), v199, 0, 0, 0);
                    }
                    continue;

                case UIMSG_SpellScrollUse:
                    if (pTurnEngine->turn_stage != TE_MOVEMENT) {
                        pushScrollSpell(static_cast<SPELL_TYPE>(uMessageParam), v199);
                    }
                    continue;
                case UIMSG_SpellBookWindow:
                    if (pTurnEngine->turn_stage == TE_MOVEMENT) continue;
                    if (engine->IsUnderwater()) {
                        GameUI_SetStatusBar(LSTR_CANT_DO_UNDERWATER);
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                    } else {
                        pCurrentFrameMessageQueue->Flush();
                        if (uActiveCharacter && !pPlayers[uActiveCharacter]->uTimeToRecovery) {
                            // toggle
                            if (current_screen_type == CURRENT_SCREEN::SCREEN_SPELL_BOOK) {
                                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                                continue;
                            }
                            // cant open screen - talking or in shop or map transition
                            if (!IsWindowSwitchable()) {
                                continue;
                            } else {
                                // close out current window
                                back_to_game();
                                OnEscape();
                                GameUI_StatusBar_Clear();
                            }
                            // open window
                            new OnButtonClick2({476, 450}, {0, 0}, pBtn_CastSpell);
                            pGUIWindow_CurrentMenu = new GUIWindow_Spellbook();
                            continue;
                        }
                    }
                    continue;
                case UIMSG_QuickReference:
                    pCurrentFrameMessageQueue->Flush();
                    // toggle
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_QUICK_REFERENCE) {
                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                        continue;
                    }
                    // cant open screen - talking or in shop or map transition
                    if (!IsWindowSwitchable()) {
                        continue;
                    } else {
                        // close out current window
                        back_to_game();
                        OnEscape();
                        GameUI_StatusBar_Clear();
                    }
                    // open window
                    new OnButtonClick2({560, 450}, {0, 0}, pBtn_QuickReference);
                    pGUIWindow_CurrentMenu = new GUIWindow_QuickReference();
                    continue;
                case UIMSG_GameMenuButton:
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) {
                        pGUIWindow_CurrentMenu->Release();
                        pEventTimer->Resume();
                        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                    }

                    if (gamma_preview_image) {
                        gamma_preview_image->Release();
                        gamma_preview_image = nullptr;
                    }
                    render->SaveScreenshot("gamma.pcx", 155, 117);
                    gamma_preview_image = assets->GetImage_PCXFromFile("gamma.pcx");

                    new OnButtonClick({602, 450}, {0, 0}, pBtn_GameSettings);
                    // LABEL_453:
                    /*if ( (signed int)pCurrentFrameMessageQueue->uNumMessages >= 40
                    ) continue;
                    pCurrentFrameMessageQueue->pMessages[pCurrentFrameMessageQueue->uNumMessages].eType
                    = UIMSG_Escape;
                    //goto LABEL_770;
                    pCurrentFrameMessageQueue->pMessages[pCurrentFrameMessageQueue->uNumMessages].param
                    = 0;
                    *(&pCurrentFrameMessageQueue->uNumMessages + 3 *
                    pCurrentFrameMessageQueue->uNumMessages + 3) = 0;
                    ++pCurrentFrameMessageQueue->uNumMessages;*/
                    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                    continue;
                case UIMSG_ClickAwardScrollBar: {
                    books_page_number = 1;
                    Pointi pt = mouse->GetCursorPos();
                    if (pt.y > 178) books_page_number = -1;
                    continue;
                }
                case UIMSG_ClickAwardsUpBtn:
                    new OnButtonClick3(WINDOW_CharacterWindow_Awards, {pBtn_Up->uX, pBtn_Up->uY}, {0, 0}, pBtn_Up);
                    BtnUp_flag = 1;
                    continue;
                case UIMSG_ClickAwardsDownBtn:
                    new OnButtonClick3(WINDOW_CharacterWindow_Awards, {pBtn_Down->uX, pBtn_Down->uY}, {0, 0}, pBtn_Down);
                    BtnDown_flag = 1;
                    continue;
                case UIMSG_ChangeDetaliz:
                    ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->ToggleRingsOverlay();
                    continue;
                case UIMSG_ClickPaperdoll:
                    OnPaperdollLeftClick();
                    continue;
                case UIMSG_SkillUp:
                    skill = static_cast<PLAYER_SKILL_TYPE>(uMessageParam);
                    pPlayer4 = pPlayers[uActiveCharacter];
                    skill_level = pPlayer4->GetSkillLevel(skill);
                    if (pPlayer4->uSkillPoints < skill_level + 1) {
                        v87 = localization->GetString(LSTR_NOT_ENOUGH_SKILL_POINTS);
                    } else {
                        if (skill_level < skills_max_level[skill]) {
                            pPlayer4->SetSkillLevel(skill, skill_level + 1);
                            pPlayer4->uSkillPoints -= skill_level + 1;
                            pPlayer4->PlaySound(SPEECH_SkillIncrease, 0);
                            pAudioPlayer->PlaySound((SoundID)SOUND_quest, 0, 0, -1, 0, 0);
                            continue;
                        }
                        v87 = localization->GetString(LSTR_SKILL_ALREADY_MASTERED);
                    }
                    GameUI_SetStatusBar(v87);
                    continue;
                case UIMSG_ClickStatsBtn:
                    ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->ShowStatsTab();
                    continue;
                case UIMSG_ClickSkillsBtn:
                    ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->ShowSkillsTab();
                    continue;
                case UIMSG_ClickInventoryBtn:
                    ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->ShowInventoryTab();
                    continue;
                case UIMSG_ClickAwardsBtn:
                    ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->ShowAwardsTab();
                    continue;
                case UIMSG_ClickExitCharacterWindowBtn:
                    new OnCancel2({pCharacterScreen_ExitBtn->uX, pCharacterScreen_ExitBtn->uY}, {0, 0}, pCharacterScreen_ExitBtn);
                    continue;
                case UIMSG_ClickBooksBtn:
                    switch (uMessageParam) {
                        case 11:  // Page UP
                            BtnUp_flag = 1;
                            pButton = pBtn_Book_2;
                            break;
                        case 10:  // Page DOWN
                            BtnDown_flag = 1;
                            pButton = pBtn_Book_1;
                            break;
                        case 0:  // Zoom plus
                            pButton = pBtn_Book_1;
                            BtnDown_flag = 1;
                            break;
                        case 1:  // Zoom minus
                            pButton = pBtn_Book_2;
                            BtnUp_flag = 1;
                            break;
                        case 2:  // Potions
                            Book_PageBtn3_flag = 1;
                            if (MapBookOpen) continue;
                            pButton = pBtn_Book_3;
                            break;
                        case 3:  // fountains
                            Book_PageBtn4_flag = 1;
                            if (MapBookOpen) continue;
                            pButton = pBtn_Book_4;
                            break;
                        case 4:  // obelisks
                            Book_PageBtn5_flag =
                                1;  // Autonotes_Obelisks_page_flag
                            if (MapBookOpen) continue;
                            pButton = pBtn_Book_5;
                            break;
                        case 5:                      // seer
                            Book_PageBtn6_flag = 1;  // Autonotes_Seer_page_flag
                            if (MapBookOpen) continue;
                            pButton = pBtn_Book_6;
                            break;
                        case 6:  // misc
                            pButton = pBtn_Autonotes_Misc;
                            Autonotes_Misc_page_flag = 1;
                            break;
                        case 7:  // Instructors
                            pButton = pBtn_Autonotes_Instructors;
                            Autonotes_Instructors_page_flag = 1;
                            break;
                        default:
                            continue;
                    }
                    new OnButtonClick({pButton->uX, pButton->uY}, {0, 0}, pButton, std::string(), false);
                    continue;
                case UIMSG_SelectCharacter:
                    pCurrentFrameMessageQueue->Flush();
                    GameUI_OnPlayerPortraitLeftClick(uMessageParam);
                    continue;
                case UIMSG_ShowStatus_Funds: {
                    GameUI_StatusBar_Set(localization->FormatString(
                        LSTR_FMT_D_TOTAL_GOLD_D_IN_BANK,
                        pParty->GetGold() + pParty->uNumGoldInBank,
                        pParty->uNumGoldInBank));
                    continue;
                }
                case UIMSG_ShowStatus_DateTime:
                    currHour = pParty->uCurrentHour;
                    uNumSeconds = 1;
                    if (pParty->uCurrentHour > 12) {
                        if (pParty->uCurrentHour >= 24) uNumSeconds = 0;
                        currHour = (currHour - 12);
                    } else {
                        if (pParty->uCurrentHour < 12)  // 12:00 is PM
                            uNumSeconds = 0;
                        if (pParty->uCurrentHour == 0) currHour = 12;
                    }
                    GameUI_StatusBar_Set(fmt::format(
                        "{}:{:02}{} {} {} {} {}", currHour,
                        pParty->uCurrentMinute,
                        localization->GetAmPm(uNumSeconds),
                        localization->GetDayName(pParty->uCurrentDayOfMonth % 7),
                        7 * pParty->uCurrentMonthWeek + pParty->uCurrentDayOfMonth % 7 + 1,
                        localization->GetMonthName(pParty->uCurrentMonth),
                        pParty->uCurrentYear));
                    continue;

                case UIMSG_ShowStatus_Food: {
                    GameUI_StatusBar_Set(localization->FormatString(
                        LSTR_FMT_YOU_HAVE_D_FOOD, pParty->GetFood()));
                    continue;
                }

                case UIMSG_ShowStatus_Player: {
                    pPlayer5 = pPlayers[uMessageParam];

                    auto status = NameAndTitle(pPlayer5->pName, pPlayer5->classType);
                        + ": "
                        + std::string(localization->GetCharacterConditionName(pPlayer5->GetMajorConditionIdx()));
                    GameUI_StatusBar_Set(status);

                    mouse->uPointingObjectID =
                        PID(OBJECT_Player,
                            (unsigned char)(8 * uMessageParam - 8) | 4);
                    continue;
                }

                case UIMSG_ShowStatus_ManaHP: {
                    GameUI_StatusBar_Set(
                        fmt::format("{} / {} {}    {} / {} {}",
                                     pPlayers[uMessageParam]->GetHealth(),
                                     pPlayers[uMessageParam]->GetMaxHealth(),
                                     localization->GetString(LSTR_HIT_POINTS),
                                     pPlayers[uMessageParam]->GetMana(),
                                     pPlayers[uMessageParam]->GetMaxMana(),
                                     localization->GetString(LSTR_SPELL_POINTS)));
                    continue;
                }

                case UIMSG_CHEST_ClickItem:
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_CHEST_INVENTORY) {
                        pPlayers[uActiveCharacter]->OnInventoryLeftClick();
                        continue;
                    }
                    Chest::OnChestLeftClick();
                    continue;
                case UIMSG_InventoryLeftClick:
                    pPlayers[uActiveCharacter]->OnInventoryLeftClick();
                    continue;
                case UIMSG_MouseLeftClickInGame:
                    pCurrentFrameMessageQueue->Flush();
                    pCurrentFrameMessageQueue->AddGUIMessage(
                        UIMSG_MouseLeftClickInScreen, 0, 0);
                    continue;
                case UIMSG_MouseLeftClickInScreen:  // срабатывает при нажатии на
                                                    // правую кнопку мыши после
                                                    // UIMSG_MouseLeftClickInGame
                    pCurrentFrameMessageQueue->Flush();
                    engine->OnGameViewportClick();
                    continue;
                case UIMSG_F:  // what event?
                    __debugbreak();
                    //pButton2 = (GUIButton *)(uint16_t)vis->get_picked_object_zbuf_val().object_pid;
                    __debugbreak();  // GUIWindow::Create(0, 0, 0, 0, WINDOW_F, (int)pButton2, 0);
                    continue;
                case UIMSG_54:  // what event?
                    __debugbreak();
                    //pButton2 = (GUIButton *)uMessageParam;
                    __debugbreak();  // GUIWindow::Create(0, 0, 0, 0, WINDOW_22, (int)pButton2, 0);
                    continue;
                case UIMSG_Game_Action:
                    pCurrentFrameMessageQueue->Flush();
                    // if currently in a chest
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_CHEST) {
                        Chest::GrabItem(keyboardInputHandler->IsTakeAllToggled());
                    } else {
                        OnPressSpace();
                    }
                    continue;
                case UIMSG_ClickZoomInBtn:
                    if (!(current_screen_type == CURRENT_SCREEN::SCREEN_GAME)) continue;
                    pParty->uFlags |= 2u;
                    new OnButtonClick2({519, 136}, {0, 0}, pBtn_ZoomIn);
                    uNumSeconds = 131072;

                    ++viewparams->field_28;
                    viewparams->uMinimapZoom *= 2;

                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        if (viewparams->uMinimapZoom > 4096) {
                            viewparams->uMinimapZoom = 4096;
                            viewparams->field_28 = 12;
                        }
                    } else {
                        if (viewparams->uMinimapZoom > 2048) {
                            viewparams->uMinimapZoom = 2048;
                            viewparams->field_28 = 11;
                        }
                    }

                    break;
                case UIMSG_ClickZoomOutBtn:
                    if (!(current_screen_type == CURRENT_SCREEN::SCREEN_GAME)) continue;
                    pParty->uFlags |= 2;
                    new OnButtonClick2({574, 136}, {0, 0}, pBtn_ZoomOut);
                    uNumSeconds = 32768;

                    --viewparams->field_28;
                    viewparams->uMinimapZoom /= 2;

                    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                        if (viewparams->uMinimapZoom < 512) {
                            viewparams->uMinimapZoom = 512;
                            viewparams->field_28 = 9;
                        }
                    } else {
                        if (viewparams->uMinimapZoom < 256) {
                            viewparams->uMinimapZoom = 256;
                            viewparams->field_28 = 8;
                        }
                    }

                    break;
                case UIMSG_DebugSpecialItem: {
                    if (uActiveCharacter == 0)
                        continue;

                    for(size_t attempt = 0; attempt < 500; attempt++) {
                        ITEM_TYPE pItemID = grng->RandomSample(SpawnableItems());
                        if (pItemTable->pItems[pItemID].uItemID_Rep_St > 6) {
                            pPlayers[uActiveCharacter]->AddItem(-1, pItemID);
                            break;
                        }
                    }

                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                }
                case UIMSG_DebugGenItem: {
                    if (uActiveCharacter == 0)
                        continue;

                    for (size_t attempt = 0; attempt < 500; attempt++) {
                        ITEM_TYPE pItemID = grng->RandomSample(SpawnableItems());
                        // if (pItemTable->pItems[pItemID].uItemID_Rep_St ==
                        //   (item_id - 40015 + 1)) {
                        pPlayers[uActiveCharacter]->AddItem(-1, pItemID);
                        break;
                        //}
                    }

                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                }
                case UIMSG_DebugKillChar:
                    if (uActiveCharacter == 0)
                        continue;
                    pPlayers[uActiveCharacter]->SetCondition(Condition_Dead, 0);
                    continue;
                case UIMSG_DebugEradicate:
                    if (uActiveCharacter == 0)
                        continue;
                    pPlayers[uActiveCharacter]->SetCondition(Condition_Eradicated, 0);
                    continue;
                case UIMSG_DebugFullHeal:
                    if (uActiveCharacter == 0)
                        continue;
                    pPlayers[uActiveCharacter]->conditions.ResetAll();
                    pPlayers[uActiveCharacter]->sHealth =
                        pPlayers[uActiveCharacter]->GetMaxHealth();
                    pPlayers[uActiveCharacter]->sMana =
                        pPlayers[uActiveCharacter]->GetMaxMana();
                    pAudioPlayer->PlaySound(SOUND_heal, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugCycleAlign:
                    if (pParty->alignment == PartyAlignment::PartyAlignment_Good) pParty->alignment = PartyAlignment::PartyAlignment_Neutral;
                    else if (pParty->alignment == PartyAlignment::PartyAlignment_Neutral) pParty->alignment = PartyAlignment::PartyAlignment_Evil;
                    else if (pParty->alignment == PartyAlignment::PartyAlignment_Evil) pParty->alignment = PartyAlignment::PartyAlignment_Good;
                    SetUserInterface(pParty->alignment, true);
                    continue;
                case UIMSG_DebugTakeFood:
                    pParty->SetFood(0);
                    continue;
                case UIMSG_DebugGiveFood:
                    pParty->GiveFood(20);
                    continue;
                case UIMSG_DebugTakeGold:
                    pParty->SetGold(0);
                    continue;
                case UIMSG_DebugLearnSkills:
                    for (uint i = 0; i < 4; ++i) {            // loop over players
                        for (PLAYER_SKILL_TYPE ski : AllSkills()) {  // loop over skills
                            // if class can learn this skill
                            if (byte_4ED970_skill_learn_ability_by_class_table[pParty->pPlayers[i].classType][ski] > PLAYER_SKILL_MASTERY_NONE) {
                                if (pParty->pPlayers[i].GetSkillLevel(ski) == 0)
                                    pParty->pPlayers[i].SetSkillLevel(ski, 1);
                            }
                        }
                    }
                    continue;
                case UIMSG_DebugGiveSkillP:
                    for (uint i = 0; i < 4; ++i)
                        pParty->pPlayers[i].uSkillPoints += 50;
                    pPlayers[std::max(uActiveCharacter, 1u)]->PlayAwardSound_Anim();
                    continue;
                case UIMSG_DebugGiveEXP:
                    pParty->GivePartyExp(20000);
                    pPlayers[std::max(uActiveCharacter, 1u)]->PlayAwardSound_Anim();
                    continue;
                case UIMSG_DebugGiveGold:
                    pParty->AddGold(10000);
                    continue;
                case UIMSG_DebugTownPortal:
                    engine->config->debug.TownPortal.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugWizardEye:
                    engine->config->debug.WizardEye.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugAllMagic:
                    engine->config->debug.AllMagic.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugTerrain:
                    engine->config->debug.Terrain.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugLightmap:
                    engine->config->debug.LightmapDecals.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugTurboSpeed:
                    engine->config->debug.TurboSpeed.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugNoActors:
                    engine->config->debug.NoActors.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugFog:
                    engine->config->graphics.Fog.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugSnow:
                    engine->config->graphics.Snow.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugNoDamage:
                    engine->config->debug.NoDamage.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugPortalLines:
                    engine->config->debug.PortalOutlines.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugPickedFace:
                    engine->config->debug.ShowPickedFace.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugShowFPS:
                    engine->config->debug.ShowFPS.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugSeasonsChange:
                    engine->config->graphics.SeasonsChange.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugVerboseLogging:
                    engine->config->debug.VerboseLogging.Toggle();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                case UIMSG_DebugReloadShader:
                    render->ReloadShaders();
                    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
                    continue;
                default:
                    continue;
            }
        }
    }

    std::swap(pCurrentFrameMessageQueue, pNextFrameMessageQueue);
    assert(pNextFrameMessageQueue->Empty());

    if (GateMasterEventId != UIMSG_0) {
        pCurrentFrameMessageQueue->AddGUIMessage(GateMasterEventId, (int64_t)GateMasterNPCData, 0);
        GateMasterEventId = UIMSG_0;
    } else {
        if (AfterEnchClickEventId != UIMSG_0) {
            AfterEnchClickEventTimeout -= pEventTimer->uTimeElapsed;
            if (AfterEnchClickEventTimeout <= 0) {
                pCurrentFrameMessageQueue->AddGUIMessage(AfterEnchClickEventId, AfterEnchClickEventSecondParam, 0);
                AfterEnchClickEventId = UIMSG_0;
                AfterEnchClickEventSecondParam = 0;
                AfterEnchClickEventTimeout = 0;
            }
        }
    }
    CastSpellInfoHelpers::castSpell();
}

//----- (0046A14B) --------------------------------------------------------
void Game::OnPressSpace() {
    engine->PickKeyboard(engine->config->gameplay.KeyboardInteractionDepth.Get(),
                         keyboardInputHandler->IsKeyboardPickingOutlineToggled(),
                         &vis_sprite_filter_3, &vis_door_filter);

    uint16_t pid = vis->get_picked_object_zbuf_val().object_pid;
    if (pid != PID_INVALID) {
        // wasn't there, but we decided to deny interactions where there are no active character
        if (uActiveCharacter == 0) {
            GameUI_SetStatusBar(localization->GetString(LSTR_NOBODY_IS_IN_CONDITION));
            return;
        }
        DoInteractionWithTopmostZObject(pid);
    }
}

void Game::GameLoop() {
    const char *pLocationName;  // [sp-4h] [bp-68h]@74
    bool bLoading;              // [sp+10h] [bp-54h]@1
    char Source[64];            // [sp+44h] [bp-20h]@76

    bLoading = sCurrentMenuID == MENU_LoadingProcInMainMenu;
    SetCurrentMenuID((MENU_STATE)-1);
    if (bLoading) {
        pParty->Reset();
        uGameState = GAME_STATE_PLAYING;
        LoadGame(uLoadGameUI_SelectedSlot);
    }

    extern bool use_music_folder;
    GameUI_LoadPlayerPortraintsAndVoices();
    pIcons_LOD->_inlined_sub1();
    // pAudioPlayer->SetMusicVolume(engine->config->music_level);

    while (2) {
        pCurrentFrameMessageQueue->Flush();

        pPartyActionQueue->uNumActions = 0;

        pTurnEngine->End(false);
        pParty->bTurnBasedModeOn = false;  // Make sure turn engine and party turn based mode flag are in sync.

        DoPrepareWorld(bLoading, 1);
        pEventTimer->Resume();
        dword_6BE364_game_settings_1 |=
            GAME_SETTINGS_0080_SKIP_USER_INPUT_THIS_FRAME;
        // uGame_if_0_else_ui_id__11_save__else_load__8_drawSpellInfoPopup__22_final_window__26_keymapOptions__2_options__28_videoOptions
        // = 0;
        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;

        // if ( render->pRenderD3D )
        vis->_4C1A02();
        engine->Draw();

        bool game_finished = false;
        do {
            MessageLoopWithWait();

            engine->_44EEA7();  // pop up . mouse picking
            GameUI_WritePointedObjectStatusString();
            keyboardInputHandler->GenerateInputActions();
            EventLoop();
            if (pArcomageGame->bGameInProgress) {
                ArcomageGame::Loop();
                render->Present();
                continue;
            }

            pMediaPlayer->HouseMovieLoop();

            pEventTimer->Update();
            pMiscTimer->Update();

            OnTimer(0);
            GameUI_StatusBar_Update();
            if (pMiscTimer->bPaused && !pEventTimer->bPaused)
                pMiscTimer->Resume();
            if (pEventTimer->bTackGameTime && !pParty->bTurnBasedModeOn)
                pEventTimer->bTackGameTime = 0;
            if (!pEventTimer->bPaused && uGameState == GAME_STATE_PLAYING) {
                if (!pEventTimer->bTackGameTime)
                    _494035_timed_effects__water_walking_damage__etc();

                if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0001) {
                    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0001;
                } else {
                    Actor::UpdateActorAI();
                    UpdateUserInput_and_MapSpecificStuff();
                }
            }
            pAudioPlayer->UpdateSounds();
            // expire timed status messages
            // TODO(pskelton): check tickcount usage here
            if (game_ui_status_bar_event_string_time_left != 0 && game_ui_status_bar_event_string_time_left < platform->tickCount()) {
                 GameUI_StatusBar_Clear();
            }
            if (uGameState == GAME_STATE_PLAYING) {
                engine->Draw();
                continue;
            }
            if (uGameState == GAME_FINISHED) {
                game_finished = true;
                continue;
            }


            if (uGameState == GAME_STATE_CHANGE_LOCATION) {  // смена локации
                pAudioPlayer->PauseSounds(-1);
                PrepareWorld(0);
                uGameState = GAME_STATE_PLAYING;
                continue;
            }

            // if ((signed int)uGameState <= GAME_STATE_5 || uGameState ==
            // GAME_STATE_GAME_QUITTING_TO_MAIN_MENU)//GAME_STATE_NEWGAME_OUT_GAMEMENU,
            // GAME_STATE_LOADING_GAME
            if (uGameState == GAME_STATE_LOADING_GAME ||
                uGameState == GAME_STATE_NEWGAME_OUT_GAMEMENU ||
                uGameState == GAME_STATE_5 ||
                uGameState == GAME_STATE_GAME_QUITTING_TO_MAIN_MENU) {
                game_finished = true;
                continue;
            }
            if (uGameState == GAME_STATE_FINAL_WINDOW) {
                render->BeginScene2D();
                GUI_UpdateWindows();
                render->Present();
                continue;
            }
            if (uGameState != GAME_STATE_PARTY_DIED) {
                engine->Draw();
                continue;
            }
            if (uGameState == GAME_STATE_PARTY_DIED) {
                pAudioPlayer->PauseSounds(-1);
                pParty->pHirelings[0] = NPCData();
                pParty->pHirelings[1] = NPCData();
                for (int i = 0; i < (signed int)pNPCStats->uNumNewNPCs; ++i) {
                    if (pNPCStats->pNewNPCData[i].field_24)
                        pNPCStats->pNewNPCData[i].uFlags &= 0xFFFFFF7Fu;
                }
                pMediaPlayer->PlayFullscreenMovie("losegame");
                if (pMovie_Track) pMediaPlayer->Unload();
                SaveGame(0, 0);
                ++pParty->uNumDeaths;
                for (uint i = 0; i < 4; ++i)
                    pParty->pPlayers[i].SetVariable(VAR_Award, Award_Deaths);
                pParty->days_played_without_rest = 0;
                pParty->GetPlayingTime().AddDays(7);  // += 2580480
                HEXRAYS_LOWORD(pParty->uFlags) &= ~0x204;
                pParty->SetGold(0);
                pOtherOverlayList->Reset();
                pParty->pPartyBuffs.fill(SpellBuff());

                if (pParty->bTurnBasedModeOn) {
                    pTurnEngine->End(true);
                    pParty->bTurnBasedModeOn = false;
                }
                for (Player &player : pParty->pPlayers) {
                    player.conditions.ResetAll();
                    player.pPlayerBuffs.fill(
                        SpellBuff());  // ???
                                       // memset(pParty->pPlayers[i].conditions_times.data(),
                                       // 0, 0xA0u);//(pConditions, 0, 160)
                                       // memset(pParty->pPlayers[i].pPlayerBuffs.data(),
                                       // 0, 0x180u);//(pPlayerBuffs[0], 0, 384)
                    player.sHealth = 1;
                    uActiveCharacter = 1;
                }
                if (_449B57_test_bit(pParty->_quest_bits, QBIT_ESCAPED_EMERALD_ISLE)) {
                    pParty->vPosition.x = -17331;  // respawn in harmondale
                    pParty->vPosition.y = 12547;
                    pParty->vPosition.z = 465;
                    pParty->sRotationZ = 0;
                    pLocationName = "out02.odm";
                } else {
                    pParty->vPosition.x = 12552;  // respawn on emerald isle
                    pParty->vPosition.y = 1816;
                    pParty->vPosition.z = 193;
                    pParty->sRotationZ = 512;
                    pLocationName = config->gameplay.StartingMap.Get().c_str();
                }
                strcpy(Source, pLocationName);
                pParty->uFallStartZ = pParty->vPosition.z;
                pParty->sRotationY = 0;
                pParty->uFallSpeed = 0;
                pParty->field_6E4_set0_unused = 0;
                pParty->field_6E0_set0_unused = 0;
                // change map
                if (pCurrentMapName != Source) {
                    pCurrentMapName = Source;
                    Party_Teleport_X_Pos = pParty->vPosition.x;
                    Party_Teleport_Y_Pos = pParty->vPosition.y;
                    Party_Teleport_Z_Pos = pParty->vPosition.z;
                    Party_Teleport_Cam_Yaw = pParty->sRotationZ;
                    Party_Teleport_Cam_Pitch = pParty->sRotationY;
                    Start_Party_Teleport_Flag = 1;
                    PrepareWorld(1);
                }
                Actor::InitializeActors();

                int num_conscious_players = 0;
                int conscious_players_ids[4] = {-1, -1, -1, -1};
                for (int i = 0; i < 4; ++i) {
                    if (pParty->pPlayers[i].CanAct())
                        conscious_players_ids[num_conscious_players++] = i;
                }
                if (num_conscious_players) {
                    int idx =
                        conscious_players_ids[vrng->Random(num_conscious_players)];
                    Assert(idx >= 0);
                    pParty->pPlayers[idx].PlaySound(SPEECH_CheatedDeath, 0);
                }

                GameUI_SetStatusBar(LSTR_CHEATED_THE_DEATH);
                uGameState = GAME_STATE_PLAYING;

                // need to flush messages here??
            }
        } while (!game_finished);

        pEventTimer->Pause();
        engine->ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
        if (uGameState == GAME_STATE_LOADING_GAME) {
            GameUI_LoadPlayerPortraintsAndVoices();
            uGameState = GAME_STATE_PLAYING;
            bLoading = true;
            continue;
        }
        break;
    }
    current_screen_type = CURRENT_SCREEN::SCREEN_VIDEO;
}
