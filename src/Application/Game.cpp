#include "Game.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "Arcomage/Arcomage.h"

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
#include "Engine/Events/Processor.h"
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
#include "Engine/Components/Trace/EngineTracePlayer.h"
#include "Engine/Components/Trace/EngineTraceRecorder.h"
#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/Components/Trace/EngineTraceComponent.h"
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

using Graphics::IRenderFactory;


void initDataPath(const std::string &dataPath) {
    if (validateDataPath(dataPath)) {
        setDataPath(dataPath);

        std::string savesPath = MakeDataPath("saves");
        if (!std::filesystem::exists(savesPath)) {
            std::filesystem::create_directory(savesPath);
        }

        EngineIocContainer::ResolveLogger()->info("Using MM7 directory: {}", dataPath);
    } else {
        std::string message = fmt::format(
            "Required resources aren't found!\n"
            "You should acquire licensed copy of M&M VII and copy its resources to \n{}\n\n"
            "Additionally you should also copy the content from\n"
            "resources directory from our repository there as well!",
            !dataPath.empty() ? dataPath : "current directory"
        );
        EngineIocContainer::ResolveLogger()->warning("{}", message);
        platform->showMessageBox("CRITICAL ERROR: missing resources", message);
    }
}

Game::Game(PlatformApplication *application, std::shared_ptr<GameConfig> config) {
    _application = application;
    _config = config;
    _log = EngineIocContainer::ResolveLogger();
    _decalBuilder = EngineIocContainer::ResolveDecalBuilder();
    _vis = EngineIocContainer::ResolveVis();
    _menu = GameIocContainer::ResolveGameMenu();

    ::application = application;
    ::platform = application->platform();
    ::eventLoop = application->eventLoop();
    ::window = application->window();
    ::eventHandler = application->eventHandler();
    ::openGLContext = application->openGLContext(); // OK to store into a global even if not yet initialized

    // It doesn't matter where to put control component as it's running the control routine after a call to `SwapBuffers`.
    // But the trace component should go after the deterministic component - deterministic component updates tick count,
    // and then trace component stores the updated value in a recorded `PaintEvent`.
    _windowHandler.reset(GameIocContainer::ResolveGameWindowHandler());
    application->install(_windowHandler.get()); // TODO(captainurist): actually move ownership into PlatformApplication?
    application->install(_windowHandler->KeyboardController()); // TODO(captainurist): do this properly
    application->install(std::make_unique<EngineControlComponent>());
    application->install(std::make_unique<EngineTraceComponent>());
    application->install(std::make_unique<EngineDeterministicComponent>());
    application->install(std::make_unique<EngineTracePlayer>());
    application->install(std::make_unique<EngineTraceRecorder>());
    application->install(std::make_unique<GameTraceHandler>());
}

Game::~Game() {
    _application->remove(_windowHandler->KeyboardController()); // TODO(captainurist): do this properly
    _application->remove(_windowHandler.get());
    if (_nuklearHandler)
        _application->remove(_nuklearHandler.get());
}

int Game::run() {
    IntegrityTest();

    _render = IRenderFactory().Create(_config);
    ::render = _render;

    if (!_render) {
        _log->warning("Render creation failed");
        return -1;
    }

    if (!_render->Initialize()) {
        _log->warning("Render failed to initialize");
        return -1;
    }

    _nuklear = Nuklear::Initialize();
    if (!_nuklear) {
        _log->warning("Nuklear failed to initialize");
    }
    ::nuklear = _nuklear;
    if (_nuklear) {
        _nuklearHandler = std::make_unique<NuklearEventHandler>();
        _application->install(_nuklearHandler.get());
    }

    keyboardActionMapping = std::make_shared<KeyboardActionMapping>(_config);
    ::keyboardActionMapping = keyboardActionMapping;

    keyboardInputHandler = std::make_shared<KeyboardInputHandler>(
        _windowHandler->KeyboardController(),
        keyboardActionMapping
    );

    _mouse = EngineIocContainer::ResolveMouse();
    ::mouse = _mouse;

    EngineFactory engineFactory;
    _engine = engineFactory.CreateEngine(_config);
    ::engine = _engine;

    if (!_engine) {
        _log->warning("Engine creation failed");
        return -1;
    }

    _engine->Initialize();
    /* TODO: We should setup window before render as it is requesting window size upon initialization to setup clipping dimensions, zbuffer, etc.
     * Otherwise starting borderless fullscreen will start with renderer in 640x480 and so be broken.
     * For some reason windows not liking that and hang in SDL_GL_SwapWindow if it was called after changing window position out of primary monitor.
     * And if we try to exclude changing position and set it after render initialization then when game started in fullscreen request will be ignored.
     * Hack below with render reinitialization is a temporary workaround. */
    _windowHandler->UpdateWindowFromConfig(_config.get());
    _render->Reinitialize();
    window->activate();
    ::eventLoop->processMessages(eventHandler);

    ShowMM7IntroVideo_and_LoadingScreen();

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;

    // logger->Warning("MM: entering main loop");
    while (true) {
        GUIWindow_MainMenu::Loop();
        uGameState = GAME_STATE_PLAYING;

        if (!loop()) {
            break;
        }
    }

    if (window) {
        _windowHandler->UpdateConfigFromWindow(_config.get());
        _config->SaveConfiguration();
    }

    if (_engine) {
        _engine->Deinitialize();
        _engine = nullptr;
        ::engine = nullptr;
    }

    if (_render) {
        _render->Release();
        _render = nullptr;
    }

    return 0;
}

bool Game::loop() {
    while (true) {
        if (uGameState == GAME_FINISHED ||
            GetCurrentMenuID() == MENU_EXIT_GAME) {
            return false;
        } else if (GetCurrentMenuID() == MENU_SAVELOAD) {
            MainMenuLoad_Loop();
            if (GetCurrentMenuID() == MENU_LoadingProcInMainMenu) {
                uGameState = GAME_STATE_PLAYING;
                gameLoop();
            }
            break;
        } else if (GetCurrentMenuID() == MENU_NEWGAME) {
            pOtherOverlayList->Reset();
            if (!PartyCreationUI_Loop()) {
                break;
            }

            pParty->pPickedItem.uItemID = ITEM_NULL;

            pCurrentMapName = _config->gameplay.StartingMap.value();
            bFlashQuestBook = true;
            pMediaPlayer->PlayFullscreenMovie("Intro Post");
            SaveNewGame();
            if (_engine->config->debug.NoMargaret.value()) {
                pParty->_questBits.set(QBIT_EMERALD_ISLAND_MARGARETH_OFF);
            }

            gameLoop();
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
        } else if (GetCurrentMenuID() == MENU_5 || GetCurrentMenuID() == MENU_LoadingProcInMainMenu) {
            uGameState = GAME_STATE_PLAYING;
            gameLoop();
        } else if (GetCurrentMenuID() == MENU_DebugBLVLevel) {
            _mouse->ChangeActivation(0);
            pParty->Reset();
            pParty->createDefaultParty(true);

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
            _mouse->ChangeActivation(1);
            gameLoop();
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
        if (uGameState == GAME_STATE_GAME_QUITTING_TO_MAIN_MENU) {  // from the loaded game
            uGameState = GAME_STATE_PLAYING;
            break;
        }
    }

    return true;
}



void ShowMM7IntroVideo_and_LoadingScreen() {
    bGameoverLoop = true;

    render->PresentBlackScreen();
    if (!engine->config->debug.NoVideo.value()) {
        if (!engine->config->debug.NoLogo.value()) {
            pMediaPlayer->PlayFullscreenMovie("3dologo");
            pMediaPlayer->PlayFullscreenMovie("new world logo");
            pMediaPlayer->PlayFullscreenMovie("jvc");
        }
        if (!engine->config->debug.NoIntro.value()) {
            pMediaPlayer->PlayFullscreenMovie("Intro");
        }
    }

    bGameoverLoop = false;
}




Image *gamma_preview_image = nullptr;  // 506E40

void Game_StartDialogue(unsigned int actor_id) {
    if (pParty->hasActiveCharacter()) {
        pCurrentFrameMessageQueue->Flush();

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

void Game::closeTargetedSpellWindow() {
    if (pGUIWindow_CastTargetedSpell) {
        if (current_screen_type == CURRENT_SCREEN::SCREEN_CHARACTERS) {
            _mouse->SetCursorImage("MICON2");
        } else {
            pGUIWindow_CastTargetedSpell->Release();  // test to fix enchanting issue
            pGUIWindow_CastTargetedSpell = nullptr;  // test to fix enchanting issue
            _mouse->SetCursorImage("MICON1");
            game_ui_status_bar_event_string_time_left = 0;
            IsEnchantingInProgress = false;
            back_to_game();
        }
    }
}

void Game::onEscape() {
    closeTargetedSpellWindow();

    // if ((signed int)pParty->activeCharacterIndex() < 1 || (signed int)pParty->activeCharacterIndex() > 4)

    pParty->switchToNextActiveCharacter();  // always check this - could leave
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

bool IsWindowSwitchable() {
    if (current_screen_type == CURRENT_SCREEN::SCREEN_NPC_DIALOGUE || current_screen_type == CURRENT_SCREEN::SCREEN_HOUSE
        || current_screen_type == CURRENT_SCREEN::SCREEN_INPUT_BLV || current_screen_type == CURRENT_SCREEN::SCREEN_CHANGE_LOCATION) {
        return false;
    }
    return true;
}

void Game::processQueuedMessages() {
    GUIWindow *pWindow2;        // ecx@248
    int v37;                    // eax@341
    int v38;                    // eax@358
    ODMFace *pODMFace;          // ecx@412
    CastSpellInfo *pSpellInfo;  // ecx@415
    int16_t v53;                // ax@431
    int v54;                    // eax@432
    int v55;                    // ecx@432
    int v56;                    // edx@432
    int v57;                    // eax@432
    unsigned int pMapNum;       // eax@445
    Player *pPlayer2;           // ecx@549
    GUIButton *pButton;         // eax@578
    int v91;                    // edx@605
    int v92;                    // eax@605
    int v93;                    // edx@605
    int pPlayerNum;             // edx@611
    int v103;                   // eax@671
    int uMessageParam;            // [sp+18h] [bp-5E4h]@7
    int encounter_index;           // [sp+20h] [bp-5DCh]@23
    unsigned int uNumSeconds;     // [sp+24h] [bp-5D8h]@18
    UIMessageType uMessage;  // [sp+2Ch] [bp-5D0h]@7
    int uMessageParam2;            // [sp+30h] [bp-5CCh]@7
    char pOut[32];                // [sp+BCh] [bp-540h]@370
    int spellbookPages[9] {};                  // [sp+158h] [bp-4A4h]@652
    int currHour;

    dword_50CDC8 = 0;
    if (!pEventTimer->bPaused) {
        pParty->sEyelevel = pParty->uDefaultEyelevel;
        pParty->uPartyHeight = pParty->uDefaultPartyHeight;
    }
    if (bDialogueUI_InitializeActor_NPC_ID) {
        // Actor::Actor(&actor);
        Actor actor = Actor();
        actor.sNPC_ID = bDialogueUI_InitializeActor_NPC_ID;
        GameUI_InitializeDialogue(&actor, false);
        bDialogueUI_InitializeActor_NPC_ID = 0;
    }
    if (!pCurrentFrameMessageQueue->Empty()) {
        while (true) {
            if (pCurrentFrameMessageQueue->Empty()) {
                break;
            }

            pCurrentFrameMessageQueue->PopMessage(&uMessage, &uMessageParam, &uMessageParam2);
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
                        onEscape();
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
                        onEscape();
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
                        onEscape();
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
                        onEscape();
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
                        onEscape();
                        GameUI_StatusBar_Clear();
                    }
                    // open window
                    pGUIWindow_CurrentMenu = new GUIWindow_JournalBook();
                    continue;
                case UIMSG_OpenDebugMenu:
                    pCurrentFrameMessageQueue->Flush();
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_DEBUG) {
                        back_to_game();
                        onEscape();
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
                        case CURRENT_SCREEN::SCREEN_SHOP_INVENTORY:
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
                    _render->ClearZBuffer();
                    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
                        if (!pGUIWindow_CastTargetedSpell) {  // Draw Menu
                            dword_6BE138 = -1;
                            new OnButtonClick2({602, 450}, {0, 0}, pBtn_GameSettings, std::string(), false);

                            pCurrentFrameMessageQueue->Flush();
                            _menu->MenuLoop();
                        } else {
                            pGUIWindow_CastTargetedSpell->Release();
                            pGUIWindow_CastTargetedSpell = 0;
                            _mouse->SetCursorImage("MICON1");
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
                                onEscape();
                                continue;
                            }
                        } else {
                            if (current_screen_type < CURRENT_SCREEN::SCREEN_64) {
                                switch (current_screen_type) {
                                    case CURRENT_SCREEN::SCREEN_CASTING:
                                        if (enchantingActiveCharacter) {
                                            pParty->setActiveCharacterIndex(enchantingActiveCharacter);
                                            pParty->switchToNextActiveCharacter();
                                            enchantingActiveCharacter = 0;
                                            if (pParty->bTurnBasedModeOn) {
                                                pTurnEngine->ApplyPlayerAction();
                                            }
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
                                        onEscape();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_BOOKS:
                                        pEventTimer->Resume();
                                        onEscape();
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
                                        if (currentRestType != REST_NONE) {
                                            Rest(remainingRestTime);
                                            for (Player &player : pParty->pPlayers) {
                                                player.SetAsleep(GameTime(0));
                                            }
                                        }
                                        if (rest_ui_sky_frame_current) {
                                            rest_ui_sky_frame_current->Release();
                                            rest_ui_sky_frame_current = nullptr;
                                        }

                                        if (rest_ui_hourglass_frame_current) {
                                            rest_ui_hourglass_frame_current->Release();
                                            rest_ui_hourglass_frame_current = nullptr;
                                        }

                                        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                                            pOutdoor->UpdateSunlightVectors();
                                            pOutdoor->UpdateFog();
                                        }
                                        remainingRestTime = GameTime();
                                        currentRestType = REST_NONE;
                                        onEscape();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_SHOP_INVENTORY:
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
                                        pAudioPlayer->playHouseSound(SOUND_WoodDoorClosing, false);
                                        pMediaPlayer->Unload();
                                        pGUIWindow_CurrentMenu = window_SpeakInHouse;

                                        onEscape();
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
                                    case CURRENT_SCREEN::SCREEN_CHANGE_LOCATION: // escape
                                        if (pParty->vPosition.x < -22528)
                                            pParty->vPosition.x = -22528;
                                        if (pParty->vPosition.x > 22528)
                                            pParty->vPosition.x = 22528;
                                        if (pParty->vPosition.y < -22528)
                                            pParty->vPosition.y = -22528;
                                        if (pParty->vPosition.y > 22528)
                                            pParty->vPosition.y = 22528;
                                        pMediaPlayer->Unload();
                                        DialogueEnding();
                                        onEscape();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_VIDEO:
                                        pMediaPlayer->Unload();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_CHARACTERS:
                                        CharacterUI_ReleaseButtons();
                                        ReleaseAwardsScrollBar();
                                        onEscape();
                                        continue;
                                    case CURRENT_SCREEN::SCREEN_SPELL_BOOK:
                                        onEscape();
                                        continue;

                                    default:
                                        __debugbreak();  // which GAME_MENU is
                                                         // this?
                                        onEscape();
                                        continue;
                                }
                                __debugbreak();  // which GAME_MENU is this?
                                onEscape();
                                continue;
                            }
                            __debugbreak();  // which GAME_MENU is this?
                            CharacterUI_ReleaseButtons();
                            ReleaseAwardsScrollBar();
                        }
                        // __debugbreak();  // which GAME_MENU is this? debug / fallback
                        onEscape();
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
                    // PID_INVALID was used (exclusive sound)
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);

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
                            pParty->_viewYaw = Party_Teleport_Cam_Yaw;
                        }
                        if (Party_Teleport_Cam_Pitch) {
                            pParty->_viewPitch = Party_Teleport_Cam_Pitch;
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
                            onMapLeave();
                            Transition_StopSound_Autosave(Party_Teleport_Map_Name, MapStartPoint_Party);
                        }
                    } else {
                        eventProcessor(savedEventID, 0, 1, savedEventStep);
                    }
                    if (iequals(s_SavedMapName.data(), "d05.blv"))
                        pParty->GetPlayingTime().AddDays(4);

                    PlayButtonClickSound();
                    DialogueEnding();
                    back_to_game();
                    onEscape();
                    continue;
                case UIMSG_TransitionWindowCloseBtn:
                    PlayButtonClickSound();
                    pMediaPlayer->Unload();
                    DialogueEnding();
                    back_to_game();
                    onEscape();
                    continue;
                case UIMSG_CycleCharacters:
                    pParty->setActiveCharacterIndex(CycleCharacter(keyboardInputHandler->IsAdventurerBackcycleToggled()));
                    continue;
                case UIMSG_OnTravelByFoot:
                    pCurrentFrameMessageQueue->Flush();
                    dword_50CDC8 = 1;

                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    // encounter_index = (NPCData *)getTravelTime();
                    pOutdoor->level_filename = pCurrentMapName;
                    if (!_engine->IsUnderwater() && pParty->bFlying ||
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
                        RestAndHeal(24 * 60 * getTravelTime());
                        if (pParty->GetFood() > 0) {
                            pParty->RestAndHeal();
                            if (pParty->GetFood() < getTravelTime()) {
                                for(Player &player : pParty->pPlayers)
                                    player.SetCondition(Condition_Weak, 0);
                                ++pParty->days_played_without_rest;
                            }
                            pParty->TakeFood(getTravelTime());
                        } else {
                            for (Player &player : pParty->pPlayers)
                                player.SetCondition(Condition_Weak, 0);
                            ++pParty->days_played_without_rest;
                        }
                        pPaletteManager->Reset();
                        pSpriteFrameTable->ResetLoadedFlags();
                        pCurrentMapName = pOut;
                        Level_LoadEvtAndStr(pCurrentMapName.substr(0, pCurrentMapName.rfind('.')));
                        _decalBuilder->Reset(0);
                        LoadLevel_InitializeLevelEvt();
                        uLevelMapStatsID = pMapStats->GetMapInfo(pCurrentMapName);

                        bNoNPCHiring = 0;

                        _engine->SetUnderwater(
                            Is_out15odm_underwater());

                        if (Is_out15odm_underwater() || (pCurrentMapName == "d47.blv"))
                            bNoNPCHiring = 1;
                        PrepareToLoadODM(1u, (ODMRenderParams *)1);
                        bDialogueUI_InitializeActor_NPC_ID = 0;
                        onMapLoad();
                        pOutdoor->SetFog();
                        TeleportToStartingPoint(uLevel_StartingPointType);
                        bool bOnWater = false;
                        pParty->vPosition.z = GetTerrainHeightsAroundParty2(
                            pParty->vPosition.x, pParty->vPosition.y, &bOnWater, 0);
                        pParty->uFallStartZ = pParty->vPosition.z;
                        _engine->_461103_load_level_sub();
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
                case UIMSG_CastSpell_Telekinesis: {
                    int pid = _vis->get_picked_object_zbuf_val().object_pid;
                    ObjectType type = PID_TYPE(pid);
                    int id = PID_ID(pid);
                    bool interactionPossible = false;
                    if (type == OBJECT_Actor) {
                        interactionPossible = pActors[id].uAIState == Dead;
                    }
                    if (type == OBJECT_Item) {
                        int flags = pObjectList->pObjects[pSpriteObjects[id].uObjectDescID].uFlags;
                        interactionPossible = (flags & OBJECT_DESC_UNPICKABLE) != OBJECT_DESC_UNPICKABLE;
                    }
                    if (type == OBJECT_Decoration) {
                        interactionPossible = pLevelDecorations[id].uEventID != 0;
                    }
                    if (type == OBJECT_Face) {
                        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                            ODMFace *pODMFace = &pOutdoor->pBModels[pid >> 9].pFaces[id];
                            interactionPossible = (pODMFace->Clickable() && pODMFace->sCogTriggeredID);
                        } else { // Indoor
                            BLVFace *pBLVFace = &pIndoor->pFaces[id];
                            if (pBLVFace->Clickable()) {
                                interactionPossible = pIndoor->pFaceExtras[pBLVFace->uFaceExtraID].uEventID != 0;
                            }
                        }
                    }
                    if (interactionPossible) {
                        spellTargetPicked(pid, -1);
                        closeTargetedSpellWindow();
                    }
                    continue;
                }
                case UIMSG_CastSpell_TargetCharacter:
                case UIMSG_CastSpell_Hireling:
                    pCurrentFrameMessageQueue->Flush();
                    if (IsEnchantingInProgress) {
                        // Change character while enchanting is active
                        // TODO(Nik-RE-dev): need separate message type
                        pParty->setActiveCharacterIndex(uMessageParam);
                    } else {
                        spellTargetPicked(PID_INVALID, uMessageParam);
                        closeTargetedSpellWindow();
                    }
                    continue;

                case UIMSG_BF:
                    __debugbreak();
                    dword_50CDC8 = 1;
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
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
                    pGUIWindow_CurrentMenu = new GUIWindow_TownPortalBook(uMessageParam);  // (char *)uMessageParam);
                    continue;

                case UIMSG_OnCastLloydsBeacon:
                    pGUIWindow_CurrentMenu = new GUIWindow_LloydsBook();
                    continue;

                case UIMSG_LloydsBeacon_FlippingBtn:
                    bRecallingBeacon = uMessageParam;
                    pAudioPlayer->playUISound(bRecallingBeacon ? SOUND_TurnPage2 : SOUND_TurnPage1);
                    continue;
                case UIMSG_HintBeaconSlot: {
                    if (!pGUIWindow_CurrentMenu) {
                        continue;
                    }
                    Player &player = pParty->pPlayers[lloydsBeaconCasterId];
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
                    Player &player = pParty->pPlayers[lloydsBeaconCasterId];
                    if ((player.vBeacons.size() <= uMessageParam) && bRecallingBeacon) {
                        continue;
                    }

                    isLloydsBeaconBeingInstalled = true;

                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uExpertLevelMana);
                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMasterLevelMana);
                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMagisterLevelMana);
                    player.SpendMana(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana);

                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uExpertLevelRecovery);
                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMasterLevelRecovery);
                    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMagisterLevelRecovery);
                    signed int sRecoveryTime = pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery;
                    if (pParty->bTurnBasedModeOn) {
                        pParty->pTurnBasedPlayerRecoveryTimes[lloydsBeaconCasterId] = sRecoveryTime;
                        player.SetRecoveryTime(sRecoveryTime);
                        pTurnEngine->ApplyPlayerAction();
                    } else {
                        player.SetRecoveryTime(debug_non_combat_recovery_mul * sRecoveryTime * flt_debugrecmod3);
                    }
                    pAudioPlayer->playSpellSound(SPELL_WATER_LLOYDS_BEACON, 0);
                    if (bRecallingBeacon) {
                        if (pCurrentMapName != pGames_LOD->GetSubNodeName(player.vBeacons[uMessageParam].SaveFileID)) {
                            SaveGame(1, 0);
                            onMapLeave();
                            pCurrentMapName = pGames_LOD->GetSubNodeName(player.vBeacons[uMessageParam].SaveFileID);
                            dword_6BE364_game_settings_1 |= GAME_SETTINGS_0001;
                            uGameState = GAME_STATE_CHANGE_LOCATION;
                            Party_Teleport_X_Pos = player.vBeacons[uMessageParam].PartyPos_X;
                            Party_Teleport_Y_Pos = player.vBeacons[uMessageParam].PartyPos_Y;
                            Party_Teleport_Z_Pos = player.vBeacons[uMessageParam].PartyPos_Z;
                            Party_Teleport_Cam_Yaw = player.vBeacons[uMessageParam]._partyViewYaw;
                            Party_Teleport_Cam_Pitch = player.vBeacons[uMessageParam]._partyViewPitch;
                            Start_Party_Teleport_Flag = 1;
                        } else {
                            pParty->vPosition.x = player.vBeacons[uMessageParam].PartyPos_X;
                            pParty->vPosition.y = player.vBeacons[uMessageParam].PartyPos_Y;
                            pParty->vPosition.z = player.vBeacons[uMessageParam].PartyPos_Z;
                            pParty->uFallStartZ = pParty->vPosition.z;
                            pParty->_viewYaw = player.vBeacons[uMessageParam]._partyViewYaw;
                            pParty->_viewPitch = player.vBeacons[uMessageParam]._partyViewPitch;
                        }
                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                        pGUIWindow_CurrentMenu->Release();
                        pGUIWindow_CurrentMenu = 0;
                    } else {
                        player.SetBeacon(uMessageParam, lloydsBeaconSpellDuration);
                    }
                    continue;
                }
                case UIMSG_ClickTownInTP: {
                    //if (uGameState == GAME_STATE_CHANGE_LOCATION) continue;

                    // check if tp location is unlocked
                    if (!pParty->_questBits[townPortalQuestBits[uMessageParam]] && !_engine->config->debug.TownPortal.value()) {
                        continue;
                    }

                    // begin TP
                    SaveGame(1, 0);
                    // if in current map
                    if (pMapStats->GetMapInfo(pCurrentMapName) == TownPortalList[uMessageParam].uMapInfoID) {
                        pParty->vPosition = TownPortalList[uMessageParam].pos;
                        pParty->uFallStartZ = pParty->vPosition.z;
                        pParty->_viewYaw = TownPortalList[uMessageParam]._viewYaw;
                        pParty->_viewPitch = TownPortalList[uMessageParam]._viewPitch;
                    } else {  // if change map
                        onMapLeave();
                        dword_6BE364_game_settings_1 |= GAME_SETTINGS_0001;
                        uGameState = GAME_STATE_CHANGE_LOCATION;
                        pCurrentMapName = pMapStats->pInfos[TownPortalList[uMessageParam].uMapInfoID].pFilename;
                        Start_Party_Teleport_Flag = 1;
                        Party_Teleport_X_Pos = TownPortalList[uMessageParam].pos.x;
                        Party_Teleport_Y_Pos = TownPortalList[uMessageParam].pos.y;
                        Party_Teleport_Z_Pos = TownPortalList[uMessageParam].pos.z;
                        Party_Teleport_Cam_Yaw = TownPortalList[uMessageParam]._viewYaw;
                        Party_Teleport_Cam_Pitch = TownPortalList[uMessageParam]._viewPitch;
                        Actor::InitializeActors();
                    }

                    assert(PID_TYPE(townPortalCasterPid) == OBJECT_Player);

                    int casterId = PID_ID(townPortalCasterPid);
                    if (casterId < pParty->pPlayers.size()) {
                        // Town portal casted by player
                        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelMana == pSpellDatas[SPELL_WATER_TOWN_PORTAL].uExpertLevelMana);
                        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelMana == pSpellDatas[SPELL_WATER_TOWN_PORTAL].uMasterLevelMana);
                        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelMana == pSpellDatas[SPELL_WATER_TOWN_PORTAL].uMagisterLevelMana);
                        pParty->pPlayers[casterId].SpendMana(pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelMana);

                        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_TOWN_PORTAL].uExpertLevelRecovery);
                        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_TOWN_PORTAL].uMasterLevelRecovery);
                        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_TOWN_PORTAL].uMagisterLevelRecovery);
                        signed int sRecoveryTime = pSpellDatas[SPELL_WATER_TOWN_PORTAL].uNormalLevelRecovery;
                        if (pParty->bTurnBasedModeOn) {
                            pParty->pTurnBasedPlayerRecoveryTimes[casterId] = sRecoveryTime;
                            pParty->pPlayers[casterId].SetRecoveryTime(sRecoveryTime);
                            pTurnEngine->ApplyPlayerAction();
                        } else {
                            pParty->pPlayers[casterId].SetRecoveryTime(debug_non_combat_recovery_mul * sRecoveryTime * flt_debugrecmod3);
                        }
                    } else {
                        // Town portal casted by hireling
                        pParty->pHirelings[casterId - pParty->pPlayers.size()].bHasUsedTheAbility = 1;
                    }

                    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                    continue;
                }
                case UIMSG_HintTownPortal: {
                    if (!pParty->_questBits[townPortalQuestBits[uMessageParam]] && !_engine->config->debug.TownPortal.value()) {
                        _render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);
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
                    pAudioPlayer->stopSounds();
                    SaveGame(1, 0);

                    pParty->vPosition.x = -17331;  // respawn point in Harmondale
                    pParty->vPosition.y = 12547;
                    pParty->vPosition.z = 465;
                    pParty->_viewYaw = 0;
                    pParty->uFallStartZ = pParty->vPosition.z;
                    pParty->_viewPitch = 0;
                    pParty->uFallSpeed = 0;
                    pParty->field_6E4_set0_unused = 0;
                    pParty->field_6E0_set0_unused = 0;

                    // change map to Harmondale
                    pCurrentMapName = "out02.odm";
                    Party_Teleport_X_Pos = pParty->vPosition.x;
                    Party_Teleport_Y_Pos = pParty->vPosition.y;
                    Party_Teleport_Z_Pos = pParty->vPosition.z;
                    Party_Teleport_Cam_Yaw = pParty->_viewYaw;
                    Party_Teleport_Cam_Pitch = pParty->_viewPitch;
                    Start_Party_Teleport_Flag = 1;
                    PrepareWorld(1);
                    Actor::InitializeActors();

                    uGameState = GAME_STATE_PLAYING;

                    for (Player &player : pParty->pPlayers) {
                        player.playEmotion(CHARACTER_EXPRESSION_WIDE_SMILE, 0);
                    }

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
                            onMapLeave();
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
                        pAudioPlayer->playUISound(SOUND_error);
                        status_string = "Can't jump to that location!";
                    }
                    GameUI_SetStatusBar(status_string);
                    continue;
                }
                case UIMSG_CastQuickSpell: {
                    if (_engine->IsUnderwater()) {
                        GameUI_SetStatusBar(LSTR_CANT_DO_UNDERWATER);
                        pAudioPlayer->playUISound(SOUND_error);
                        continue;
                    }
                    if (!pParty->hasActiveCharacter() ||
                        (pPlayer2 = pPlayers[pParty->activeCharacterIndex()],
                         pPlayer2->timeToRecovery))
                        continue;
                    pushSpellOrRangedAttack(pPlayer2->uQuickSpell, pParty->activeCharacterIndex() - 1,
                                            0, 0, pParty->activeCharacterIndex());
                    continue;
                }

                case UIMSG_CastSpell_TargetActorBuff:
                case UIMSG_CastSpell_TargetActor: {
                    int pid = _vis->get_picked_object_zbuf_val().object_pid;
                    int depth = _vis->get_picked_object_zbuf_val().depth;
                    if (PID_TYPE(pid) == OBJECT_Actor && depth < _engine->config->gameplay.RangedAttackDepth.value()) {
                        spellTargetPicked(pid, -1);
                        closeTargetedSpellWindow();
                    }
                    continue;
                }
                case UIMSG_1C:
                    __debugbreak();
                    if (!pParty->hasActiveCharacter() || current_screen_type != CURRENT_SCREEN::SCREEN_GAME)
                        continue;
                    __debugbreak();  // ptr_507BC8 = GUIWindow::Create(0, 0,
                                     // window->GetWidth(), window->GetHeight(),
                                     // WINDOW_68, uMessageParam, 0);
                    current_screen_type = CURRENT_SCREEN::SCREEN_19;
                    pEventTimer->Pause();
                    continue;
                case UIMSG_STEALFROMACTOR:
                    if (!pParty->hasActiveCharacter()) continue;
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
                    if (!pParty->hasActiveCharacter()) continue;
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
                    if (currentRestType == REST_HEAL) {
                        GameUI_SetStatusBar(LSTR_ALREADY_RESTING);
                        pAudioPlayer->playUISound(SOUND_error);
                        continue;
                    }
                    new OnButtonClick2({pButton_RestUI_Wait5Minutes->uX, pButton_RestUI_Wait5Minutes->uY}, {0, 0}, pButton_RestUI_Wait5Minutes,
                        localization->GetString(LSTR_WAIT_5_MINUTES));
                    currentRestType = REST_WAIT;
                    remainingRestTime = GameTime::FromMinutes(5);
                    continue;
                case UIMSG_Wait1Hour:
                    if (currentRestType == REST_HEAL) {
                        GameUI_SetStatusBar(LSTR_ALREADY_RESTING);
                        pAudioPlayer->playUISound(SOUND_error);
                        continue;
                    }
                    new OnButtonClick2({pButton_RestUI_Wait1Hour->uX, pButton_RestUI_Wait1Hour->uY}, {0, 0}, pButton_RestUI_Wait1Hour,
                        localization->GetString(LSTR_WAIT_1_HOUR));
                    currentRestType = REST_WAIT;
                    remainingRestTime = GameTime::FromHours(1);
                    continue;
                case UIMSG_RentRoom:
                    pGUIWindow_CurrentMenu = new GUIWindow_Rest();

                    remainingRestTime = GameTime::FromHours(_494820_training_time(pParty->uCurrentHour) + 1) - GameTime::FromMinutes(pParty->uCurrentMinute);
                    if (uMessageParam == 111 || uMessageParam == 114 || uMessageParam == 116) { // 107 = Emerald Isle tavern
                        remainingRestTime = remainingRestTime + GameTime::FromHours(12);
                    }
                    currentRestType = REST_HEAL;
                    pParty->RestAndHeal();
                    pParty->days_played_without_rest = 0;
                    for (Player &player : pParty->pPlayers) {
                        player.SetAsleep(GameTime(1));
                    }
                    continue;
                case UIMSG_RestWindow:
                    pCurrentFrameMessageQueue->Flush();
                    // toggle
                    //if (current_screen_type == CURRENT_SCREEN::SCREEN_REST) {
                    //    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                    //    continue;
                    //}
                    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) continue;
                    closeTargetedSpellWindow();

                    if (CheckActors_proximity()) {
                        if (pParty->bTurnBasedModeOn) {
                            GameUI_SetStatusBar(LSTR_CANT_REST_IN_TURN_BASED);
                            continue;
                        }

                        if (pParty->uFlags & (PARTY_FLAGS_1_AIRBORNE | PARTY_FLAGS_1_STANDING_ON_WATER)) // airbourne or on water
                            GameUI_SetStatusBar(LSTR_CANT_REST_HERE);
                        else
                            GameUI_SetStatusBar(LSTR_HOSTILE_ENEMIES_NEARBY);

                        if (!pParty->hasActiveCharacter()) continue;
                        pParty->activeCharacter().playReaction(SPEECH_CantRestHere);
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
                        if (pParty->uFlags & PARTY_FLAGS_1_AIRBORNE)
                            logger->verbose("Party is airborne");
                        if (pParty->uFlags & PARTY_FLAGS_1_STANDING_ON_WATER)
                            logger->verbose("Party on water");
                    }

                    if (pParty->bTurnBasedModeOn) {
                        GameUI_SetStatusBar(LSTR_CANT_REST_IN_TURN_BASED);
                        continue;
                    }

                    if (pParty->uFlags & (PARTY_FLAGS_1_AIRBORNE | PARTY_FLAGS_1_STANDING_ON_WATER))
                        GameUI_SetStatusBar(LSTR_CANT_REST_HERE);
                    else
                        GameUI_SetStatusBar(LSTR_HOSTILE_ENEMIES_NEARBY);

                    if (!pParty->hasActiveCharacter()) continue;
                    pParty->activeCharacter().playReaction(SPEECH_CantRestHere);
                    continue;
                case UIMSG_Rest8Hour:
                    pCurrentFrameMessageQueue->Clear(); // TODO: sometimes it is called twice, prevent that for now and investigate why later
                    if (currentRestType != REST_NONE) {
                        GameUI_SetStatusBar(LSTR_ALREADY_RESTING);
                        pAudioPlayer->playUISound(SOUND_error);
                        continue;
                    }
                    if (pParty->GetFood() < foodRequiredToRest) {
                        GameUI_SetStatusBar(LSTR_NOT_ENOUGH_FOOD);
                        if (pParty->hasActiveCharacter() && pParty->activeCharacter().CanAct()) {
                            pParty->activeCharacter().playReaction(SPEECH_NotEnoughFood);
                        }
                    } else {
                        for (Player &player : pParty->pPlayers) {
                            player.SetAsleep(pParty->GetPlayingTime());
                        }
                        MAP_TYPE mapIdx = pMapStats->GetMapInfo(pCurrentMapName);
                        if (mapIdx == MAP_INVALID) {
                            mapIdx = static_cast<MAP_TYPE>(grng->random(pMapStats->uNumMaps + 1));
                        }
                        MapInfo *pMapInfo = &pMapStats->pInfos[mapIdx];

                        if (grng->random(100) + 1 <= pMapInfo->Encounter_percent) {
                            v91 = grng->random(100);
                            v92 = pMapInfo->EncM1percent;
                            v93 = v91 + 1;
                            if (v93 > v92)
                                encounter_index = v93 > v92 + pMapInfo->EncM2percent + 2;
                            else
                                encounter_index = 1;

                            if (!SpawnEncounterMonsters(pMapInfo, encounter_index))
                                encounter_index = 0;

                            if (encounter_index) {
                                pPlayerNum = grng->random(4);
                                pParty->pPlayers[pPlayerNum].conditions.Reset(Condition_Sleep);
                                Rest(GameTime::FromHours(1).AddMinutes(grng->random(6)));
                                remainingRestTime = GameTime();
                                currentRestType = REST_NONE;

                                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
                                GameUI_SetStatusBar(LSTR_ENCOUNTER);
                                pAudioPlayer->playUISound(SOUND_encounter);
                                continue;
                            }
                        }
                        pParty->TakeFood(foodRequiredToRest);
                        remainingRestTime = GameTime::FromHours(8);
                        currentRestType = REST_HEAL;
                        pParty->RestAndHeal();
                        pParty->days_played_without_rest = 0;
                        for (Player &player : pParty->pPlayers) {
                            player.SetAsleep(GameTime(1));
                        }
                    }
                    continue;
                case UIMSG_WaitTillDawn:
                    if (currentRestType == REST_HEAL) {
                        GameUI_SetStatusBar(LSTR_ALREADY_RESTING);
                        pAudioPlayer->playUISound(SOUND_error);
                        continue;
                    }
                    new OnButtonClick2({pButton_RestUI_WaitUntilDawn->uX, pButton_RestUI_WaitUntilDawn->uY}, {0, 0}, pButton_RestUI_WaitUntilDawn,
                                       localization->GetString(LSTR_WAIT_UNTIL_DAWN));
                    currentRestType = REST_WAIT;
                    remainingRestTime = GameTime::FromHours(_494820_training_time(pParty->uCurrentHour)) - GameTime::FromMinutes(pParty->uCurrentMinute);
                    continue;

                case UIMSG_HintSelectRemoveQuickSpellBtn: {
                    if (quick_spell_at_page && byte_506550) {
                        GameUI_StatusBar_Set(localization->FormatString(
                            LSTR_FMT_SET_S_AS_READY_SPELL,
                            pSpellStats->pInfos[static_cast<SPELL_TYPE>(quick_spell_at_page +
                                         11 * pParty->activeCharacter().lastOpenedSpellbookPage)].name));
                    } else {
                        if (pParty->activeCharacter().uQuickSpell != SPELL_NONE)
                            GameUI_StatusBar_Set(localization->GetString(LSTR_CLICK_TO_REMOVE_QUICKSPELL));
                        else
                            GameUI_StatusBar_Set(localization->GetString(LSTR_CLICK_TO_SET_QUICKSPELL));
                    }
                    continue;
                }

                case UIMSG_SPellbook_ShowHightlightedSpellInfo: {
                    if (!pParty->hasActiveCharacter())  // || (uNumSeconds = (unsigned
                                            // int)pPlayers[pParty->activeCharacterIndex()],!*(char
                                            // *)(uNumSeconds + 11 * *(char
                                            // *)(uNumSeconds + 6734) +
                                            // uMessageParam + 402)))
                        continue;  // this used to check if player had the spell
                                   // activated - no longer rquired here ??

                    if (sub_4637E0_is_there_popup_onscreen())
                        dword_507B00_spell_info_to_draw_in_popup = uMessageParam + 1;
                    int lastOpened = pParty->activeCharacter().lastOpenedSpellbookPage;
                    if (quick_spell_at_page - 1 == uMessageParam) {
                        GameUI_StatusBar_Set(localization->FormatString(LSTR_CAST_S,
                                    pSpellStats->pInfos[static_cast<SPELL_TYPE>(uMessageParam + 11 * lastOpened + 1)].name));
                    } else {
                        GameUI_StatusBar_Set(localization->FormatString(LSTR_SELECT_S,
                                    pSpellStats->pInfos[static_cast<SPELL_TYPE>(uMessageParam + 11 * lastOpened + 1)].name));
                    }
                    continue;
                }

                case UIMSG_ClickInstallRemoveQuickSpellBtn: {
                    new OnButtonClick2({pBtn_InstallRemoveSpell->uX, pBtn_InstallRemoveSpell->uY}, {0, 0}, pBtn_InstallRemoveSpell);
                    if (!pParty->hasActiveCharacter()) continue;
                    Player *player = &pParty->activeCharacter();
                    if (!byte_506550 || !quick_spell_at_page) {
                        player->uQuickSpell = SPELL_NONE;
                        quick_spell_at_page = 0;
                        pAudioPlayer->playUISound(SOUND_fizzle);
                        continue;
                    }
                    // TODO(captainurist): encapsulate the arithmetic below
                    pParty->activeCharacter().uQuickSpell = static_cast<SPELL_TYPE>(
                        quick_spell_at_page + 11 * pParty->activeCharacter().lastOpenedSpellbookPage);
                    if (pParty->hasActiveCharacter()) {
                        player->playReaction(SPEECH_SetQuickSpell);
                    }
                    byte_506550 = 0;
                    continue;
                }

                case UIMSG_SpellBook_PressTab:  //перелистывание страниц
                                                //клавишей Tab
                {
                    if (!pParty->hasActiveCharacter()) continue;
                    int skill_count = 0;
                    int uAction = 0;
                    int page = 0;
                    for (PLAYER_SKILL_TYPE i : MagicSkills()) {
                        if (pParty->activeCharacter().pActiveSkills[i] || _engine->config->debug.AllMagic.value()) {
                            if (pParty->activeCharacter().lastOpenedSpellbookPage == page)
                                uAction = skill_count;
                            spellbookPages[skill_count++] = page;
                        }

                        page++;
                    }
                    if (!skill_count) {  //нет скиллов
                        pAudioPlayer->playUISound(vrng->randomBool() ? SOUND_TurnPage2 : SOUND_TurnPage1);
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
                        !pParty->hasActiveCharacter() ||
                        uMessageParam == pParty->activeCharacter().lastOpenedSpellbookPage) {
                        continue;
                    }
                    ((GUIWindow_Spellbook *)pGUIWindow_CurrentMenu)->OpenSpellbookPage(uMessageParam);
                    continue;
                case UIMSG_SelectSpell: {
                    if (pTurnEngine->turn_stage == TE_MOVEMENT) {
                        continue;
                    }
                    if (!pParty->hasActiveCharacter()) {
                        continue;
                    }

                    //  uNumSeconds = (unsigned int)pPlayers[pParty->activeCharacterIndex()];
                    Player *player = &pParty->activeCharacter();
                    if (player->spellbook.pChapters[player->lastOpenedSpellbookPage].bIsSpellAvailable[uMessageParam]
                        || _engine->config->debug.AllMagic.value()) {
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
                            dword_50C9E8 + 2] = pParty->activeCharacterIndex() - 1;
                            ++dword_50C9E8;
                            }*/
                            // Processing must happen on next frame because need to close spell book and update
                            // drawing object list which is used to count actors for some spells
                            pNextFrameMessageQueue->AddGUIMessage( UIMSG_CastSpellFromBook, v103, pParty->activeCharacterIndex() - 1);
                            //  pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_CastSpellFromBook,
                            //  v103, pParty->activeCharacterIndex() - 1);
                        } else {
                            byte_506550 = 1;
                            quick_spell_at_page = uMessageParam + 1;
                        }
                    }
                    continue;
                }

                case UIMSG_CastSpellFromBook:
                    if (pTurnEngine->turn_stage != TE_MOVEMENT) {
                        pushSpellOrRangedAttack(static_cast<SPELL_TYPE>(uMessageParam), uMessageParam2, 0, 0, 0);
                    }
                    continue;

                case UIMSG_SpellScrollUse:
                    if (pTurnEngine->turn_stage != TE_MOVEMENT) {
                        pushScrollSpell(static_cast<SPELL_TYPE>(uMessageParam), uMessageParam2);
                    }
                    continue;

                case UIMSG_SpellBookWindow:
                    if (pTurnEngine->turn_stage == TE_MOVEMENT) {
                        continue;
                    }
                    if (_engine->IsUnderwater()) {
                        GameUI_SetStatusBar(LSTR_CANT_DO_UNDERWATER);
                        pAudioPlayer->playUISound(SOUND_error);
                    } else {
                        pCurrentFrameMessageQueue->Flush();
                        if (pParty->hasActiveCharacter()) {
                            if (!pParty->activeCharacter().timeToRecovery) {
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
                                    onEscape();
                                    GameUI_StatusBar_Clear();
                                }
                                // open window
                                new OnButtonClick2({ 476, 450 }, { 0, 0 }, pBtn_CastSpell);
                                pGUIWindow_CurrentMenu = new GUIWindow_Spellbook();
                                continue;
                            }
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
                        onEscape();
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
                    _render->SaveScreenshot("gamma.pcx", 155, 117);
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
                    Pointi pt = _mouse->GetCursorPos();
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
                {
                    PLAYER_SKILL_TYPE skill = static_cast<PLAYER_SKILL_TYPE>(uMessageParam);
                    Player *player = &pParty->activeCharacter();
                    PLAYER_SKILL_LEVEL skill_level = player->GetSkillLevel(skill);
                    const char *statusString;
                    if (player->uSkillPoints < skill_level + 1) {
                        statusString = localization->GetString(LSTR_NOT_ENOUGH_SKILL_POINTS);
                    } else {
                        if (skill_level < skills_max_level[skill]) {
                            player->SetSkillLevel(skill, skill_level + 1);
                            player->uSkillPoints -= skill_level + 1;
                            player->playReaction(SPEECH_SkillIncrease);
                            pAudioPlayer->playUISound(SOUND_quest);
                            continue;
                        }
                        statusString = localization->GetString(LSTR_SKILL_ALREADY_MASTERED);
                    }
                    GameUI_SetStatusBar(statusString);
                    continue;
                }
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
                    bookButtonClicked = true;
                    bookButtonAction = BOOK_BUTTON_ACTION(uMessageParam);
                    switch (bookButtonAction) {
                        case BOOK_PREV_PAGE:
                        case BOOK_ZOOM_IN:
                            pButton = pBtn_Book_1;
                            break;
                        case BOOK_NEXT_PAGE:
                        case BOOK_ZOOM_OUT:
                            pButton = pBtn_Book_2;
                            break;
                        case BOOK_SCROLL_UP:
                        case BOOK_NOTES_POTION:
                            pButton = pBtn_Book_3;
                            break;
                        case BOOK_SCROLL_DOWN:
                        case BOOK_NOTES_FOUNTAIN:
                            pButton = pBtn_Book_4;
                            break;
                        case BOOK_SCROLL_RIGHT:
                        case BOOK_NOTES_OBELISK:
                            pButton = pBtn_Book_5;
                            break;
                        case BOOK_SCROLL_LEFT:
                        case BOOK_NOTES_SEER:
                            pButton = pBtn_Book_6;
                            break;
                        case BOOK_NOTES_MISC:
                            pButton = pBtn_Autonotes_Misc;
                            break;
                        case BOOK_NOTES_INSTRUCTORS:
                            pButton = pBtn_Autonotes_Instructors;
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
                    Player *player = pPlayers[uMessageParam];

                    auto status = NameAndTitle(player->name, player->classType)
                        + ": "
                        + std::string(localization->GetCharacterConditionName(player->GetMajorConditionIdx()));
                    GameUI_StatusBar_Set(status);

                    _mouse->uPointingObjectID =
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
                        pParty->activeCharacter().OnInventoryLeftClick();
                        continue;
                    }
                    Chest::OnChestLeftClick();
                    continue;
                case UIMSG_InventoryLeftClick:
                    pParty->activeCharacter().OnInventoryLeftClick();
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
                    _engine->onGameViewportClick();
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
                        onPressSpace();
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

                case UIMSG_OpenInventory: {
                    if (pParty->hasActiveCharacter()) {
                        pGUIWindow_CurrentMenu = new GUIWindow_CharacterRecord(pParty->activeCharacterIndex(), CURRENT_SCREEN::SCREEN_CHARACTERS);
                        ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->ShowInventoryTab();
                    }
                    break;
                }

                case UIMSG_DebugSpecialItem: {
                    if (!pParty->hasActiveCharacter())
                        continue;

                    for(size_t attempt = 0; attempt < 500; attempt++) {
                        ITEM_TYPE pItemID = grng->randomSample(SpawnableItems());
                        if (pItemTable->pItems[pItemID].uItemID_Rep_St > 6) {
                            if (!pParty->activeCharacter().AddItem(-1, pItemID)) {
                                pAudioPlayer->playUISound(SOUND_error);
                            }
                            break;
                        }
                    }

                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                }
                case UIMSG_DebugGenItem: {
                    if (!pParty->hasActiveCharacter())
                        continue;

                    for (size_t attempt = 0; attempt < 500; attempt++) {
                        ITEM_TYPE pItemID = grng->randomSample(SpawnableItems());
                        // if (pItemTable->pItems[pItemID].uItemID_Rep_St ==
                        //   (item_id - 40015 + 1)) {
                        if (!pParty->activeCharacter().AddItem(-1, pItemID)) {
                            pAudioPlayer->playUISound(SOUND_error);
                        }
                        break;
                        //}
                    }

                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                }
                case UIMSG_DebugKillChar:
                    if (!pParty->hasActiveCharacter())
                        continue;
                    pParty->activeCharacter().SetCondition(Condition_Dead, 0);
                    continue;
                case UIMSG_DebugEradicate:
                    if (!pParty->hasActiveCharacter())
                        continue;
                    pParty->activeCharacter().SetCondition(Condition_Eradicated, 0);
                    continue;
                case UIMSG_DebugFullHeal:
                    if (!pParty->hasActiveCharacter())
                        continue;
                    pParty->activeCharacter().conditions.ResetAll();
                    pParty->activeCharacter().health =
                        pParty->activeCharacter().GetMaxHealth();
                    pParty->activeCharacter().mana =
                        pParty->activeCharacter().GetMaxMana();
                    pAudioPlayer->playUISound(SOUND_heal);
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
                    for (Player &player : pParty->pPlayers) { // loop over players
                        for (PLAYER_SKILL_TYPE ski : AllSkills()) {  // loop over skills
                            // if class can learn this skill
                            if (skillMaxMasteryPerClass[player.classType][ski] > PLAYER_SKILL_MASTERY_NONE) {
                                if (player.GetSkillLevel(ski) == 0) {
                                    player.SetSkillLevel(ski, 1);
                                }
                            }
                        }
                    }
                    continue;
                case UIMSG_DebugGiveSkillP:
                    for (Player &player : pParty->pPlayers) {
                        player.uSkillPoints += 50;
                    }
                    pPlayers[std::max(pParty->activeCharacterIndex(), 1u)]->PlayAwardSound_Anim();
                    continue;
                case UIMSG_DebugGiveEXP:
                    pParty->GivePartyExp(20000);
                    pPlayers[std::max(pParty->activeCharacterIndex(), 1u)]->PlayAwardSound_Anim();
                    continue;
                case UIMSG_DebugGiveGold:
                    pParty->AddGold(10000);
                    continue;
                case UIMSG_DebugTownPortal:
                    _engine->config->debug.TownPortal.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugWizardEye:
                    _engine->config->debug.WizardEye.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugAllMagic:
                    _engine->config->debug.AllMagic.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugTerrain:
                    _engine->config->debug.Terrain.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugLightmap:
                    _engine->config->debug.LightmapDecals.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugTurboSpeed:
                    _engine->config->debug.TurboSpeed.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugNoActors:
                    _engine->config->debug.NoActors.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugFog:
                    _engine->config->graphics.Fog.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugSnow:
                    _engine->config->graphics.Snow.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugNoDamage:
                    _engine->config->debug.NoDamage.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugPortalLines:
                    _engine->config->debug.PortalOutlines.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugPickedFace:
                    _engine->config->debug.ShowPickedFace.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugShowFPS:
                    _engine->config->debug.ShowFPS.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugSeasonsChange:
                    _engine->config->graphics.SeasonsChange.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugVerboseLogging:
                    _engine->config->debug.VerboseLogging.toggle();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                case UIMSG_DebugReloadShader:
                    _render->ReloadShaders();
                    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                    continue;
                default:
                    continue;
            }
        }
    }

    std::swap(pCurrentFrameMessageQueue, pNextFrameMessageQueue);
    assert(pNextFrameMessageQueue->Empty());

    if (AfterEnchClickEventId != UIMSG_0) {
        AfterEnchClickEventTimeout -= pEventTimer->uTimeElapsed;
        if (AfterEnchClickEventTimeout <= 0) {
            pCurrentFrameMessageQueue->AddGUIMessage(AfterEnchClickEventId, AfterEnchClickEventSecondParam, 0);
            AfterEnchClickEventId = UIMSG_0;
            AfterEnchClickEventSecondParam = 0;
            AfterEnchClickEventTimeout = 0;
        }
    }
    CastSpellInfoHelpers::castSpell();
}

//----- (0046A14B) --------------------------------------------------------
void Game::onPressSpace() {
    _engine->PickKeyboard(_engine->config->gameplay.KeyboardInteractionDepth.value(),
                          keyboardInputHandler->IsKeyboardPickingOutlineToggled(),
                          &vis_sprite_filter_3, &vis_door_filter);

    uint16_t pid = _vis->get_picked_object_zbuf_val().object_pid;
    if (pid != PID_INVALID) {
        DoInteractionWithTopmostZObject(pid);
    }
}

void Game::gameLoop() {
    const char *pLocationName;  // [sp-4h] [bp-68h]@74
    bool bLoading;              // [sp+10h] [bp-54h]@1
    char Source[64];            // [sp+44h] [bp-20h]@76

    bLoading = sCurrentMenuID == MENU_LoadingProcInMainMenu;
    SetCurrentMenuID((MENU_STATE)-1);
    if (bLoading) {
        uGameState = GAME_STATE_PLAYING;
        LoadGame(uLoadGameUI_SelectedSlot);
    }

    extern bool use_music_folder;
    GameUI_LoadPlayerPortraintsAndVoices();
    pIcons_LOD->_inlined_sub1();
    // pAudioPlayer->SetMusicVolume(engine->config->music_level);

    while (true) {
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
        _vis->_4C1A02();
        _engine->Draw();

        bool game_finished = false;
        do {
            MessageLoopWithWait();

            _engine->_44EEA7();  // pop up . mouse picking
            GameUI_WritePointedObjectStatusString();
            keyboardInputHandler->GenerateInputActions();
            processQueuedMessages();
            if (pArcomageGame->bGameInProgress) {
                ArcomageGame::Loop();
                _render->Present();
                continue;
            }

            pMediaPlayer->HouseMovieLoop();

            pEventTimer->Update();
            pMiscTimer->Update();

            onTimer();
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
            if (game_ui_status_bar_event_string_time_left != 0 && game_ui_status_bar_event_string_time_left < platform->tickCount()) {
                 GameUI_StatusBar_Clear();
            }
            if (uGameState == GAME_STATE_PLAYING) {
                _engine->Draw();
                continue;
            }
            if (uGameState == GAME_FINISHED) {
                game_finished = true;
                continue;
            }


            if (uGameState == GAME_STATE_CHANGE_LOCATION) {  // смена локации
                pAudioPlayer->stopSounds();
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
                _render->BeginScene2D();
                GUI_UpdateWindows();
                _render->Present();
                continue;
            }
            if (uGameState != GAME_STATE_PARTY_DIED) {
                _engine->Draw();
                continue;
            }
            if (uGameState == GAME_STATE_PARTY_DIED) {
                pAudioPlayer->stopSounds();
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
                for (Player &player : pParty->pPlayers) {
                    player.SetVariable(VAR_Award, Award_Deaths);
                }
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
                    player.health = 1;
                }
                pParty->setActiveCharacterIndex(1);

                if (pParty->_questBits[QBIT_ESCAPED_EMERALD_ISLE]) {
                    pParty->vPosition.x = -17331;  // respawn in harmondale
                    pParty->vPosition.y = 12547;
                    pParty->vPosition.z = 465;
                    pParty->_viewYaw = 0;
                    pLocationName = "out02.odm";
                } else {
                    pParty->vPosition.x = 12552;  // respawn on emerald isle
                    pParty->vPosition.y = 1816;
                    pParty->vPosition.z = 193;
                    pParty->_viewYaw = 512;
                    pLocationName = _config->gameplay.StartingMap.value().c_str();
                }
                strcpy(Source, pLocationName);
                pParty->uFallStartZ = pParty->vPosition.z;
                pParty->_viewPitch = 0;
                pParty->uFallSpeed = 0;
                pParty->field_6E4_set0_unused = 0;
                pParty->field_6E0_set0_unused = 0;
                // change map
                if (pCurrentMapName != Source) {
                    pCurrentMapName = Source;
                    Party_Teleport_X_Pos = pParty->vPosition.x;
                    Party_Teleport_Y_Pos = pParty->vPosition.y;
                    Party_Teleport_Z_Pos = pParty->vPosition.z;
                    Party_Teleport_Cam_Yaw = pParty->_viewYaw;
                    Party_Teleport_Cam_Pitch = pParty->_viewPitch;
                    Start_Party_Teleport_Flag = 1;
                    PrepareWorld(1);
                }
                Actor::InitializeActors();

                int playerId = pParty->getRandomActiveCharacterId(vrng.get());

                if (playerId != -1) {
                    pParty->pPlayers[playerId].playReaction(SPEECH_CheatedDeath);
                }

                GameUI_SetStatusBar(LSTR_CHEATED_THE_DEATH);
                uGameState = GAME_STATE_PLAYING;

                // need to flush messages here??
            }
        } while (!game_finished);

        pEventTimer->Pause();
        _engine->ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
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
