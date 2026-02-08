#include "Game.h"

#include <algorithm>
#include <string>
#include <utility>
#include <memory>

#include "Arcomage/Arcomage.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Data/AwardEnums.h"
#include "Engine/Data/HouseEnumFunctions.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/TurnBasedOverlay.h"
#include "Engine/Localization.h"
#include "Engine/Resources/LodTextureCache.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/Random/Random.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Spells/SpellEnumFunctions.h"
#include "Engine/Tables/FrameTableInc.h"
#include "Engine/Time/Timer.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/MapInfo.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/Books/AutonotesBook.h"
#include "GUI/UI/Books/CalendarBook.h"
#include "GUI/UI/Books/JournalBook.h"
#include "GUI/UI/Books/LloydsBook.h"
#include "GUI/UI/Books/MapBook.h"
#include "GUI/UI/Books/QuestBook.h"
#include "GUI/UI/Books/TownPortalBook.h"
#include "GUI/UI/UISpellbook.h"
#include "GUI/UI/UIBooks.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIBranchlessDialogue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIMainMenu.h"
#include "GUI/UI/UIGameOver.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UIQuickReference.h"
#include "GUI/UI/UIRest.h"
#include "GUI/UI/UISaveLoad.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UISpell.h"

#include "Io/Mouse.h"
#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Logger/Logger.h"
#include "Library/Fsm/Fsm.h"

#include "Utility/String/Format.h"
#include "Utility/ScopeGuard.h"

#include "GameWindowHandler.h"
#include "GameMenu.h"
#include "GameStates/GameFsmBuilder.h"

Game::Game(PlatformApplication *application, std::shared_ptr<GameConfig> config) {
    _application = application;
    _config = config;
    _menu = std::make_unique<Menu>();
    _decalBuilder = EngineIocContainer::ResolveDecalBuilder();
}

Game::~Game() = default;

int Game::run() {
    window->activate();
    ::eventLoop->processMessages(eventHandler);

    std::string_view startingState = "Start";
    // Need to have this do/while external loop till we remove entirely all the states
    do {
        {
            Fsm *fsm = application->installComponent(GameFsmBuilder::buildFsm(startingState));
            MM_AT_SCOPE_EXIT(application->removeComponent<Fsm>());
            while (!fsm->hasReachedExitState()) {
                render->ClearTarget(colorTable.Black);
                render->BeginScene2D();

                fsm->update();

                // This method should be interpreted as a future RetainedUISystem::update()
                // It does update all the GUIWindow alive + it does various hacks
                GUI_UpdateWindows();
                render->flushAndScale();
                engine->drawOverlay();
                render->swapBuffers();

                MessageLoopWithWait();
            }
        }

        // Here we're still running the rest of the loops as usual.
        uGameState = GAME_STATE_PLAYING;
        if (!loop()) {
            break;
        } else {
            startingState = "MainMenu";
        }
    } while (true);

    return 0;
}

bool Game::loop() {
    while (true) {
        if (uGameState == GAME_FINISHED ||
            GetCurrentMenuID() == MENU_EXIT_GAME) {
            return false;
        } else if (GetCurrentMenuID() == MENU_LoadingProcInMainMenu) {
            uGameState = GAME_STATE_PLAYING;
            gameLoop();
            if (uGameState == GAME_STATE_NEWGAME_OUT_GAMEMENU) {
                SetCurrentMenuID(MENU_NEWGAME);
                uGameState = GAME_STATE_PLAYING;
                continue;
            }
            break;
        } else if (GetCurrentMenuID() == MENU_NEWGAME) {
            pActiveOverlayList->Reset();
            if (!PartyCreationUI_Loop()) {
                break;
            }

            pParty->pPickedItem.itemId = ITEM_NULL;

            engine->_transitionMapId = pMapStats->GetMapInfo(_config->gameplay.StartingMap.value());

            // TODO(Nik-RE-dev): should not be an assert but an exception or error message.
            assert(engine->_transitionMapId != MAP_INVALID);

            bFlashQuestBook = true;
            pMediaPlayer->PlayFullscreenMovie("Intro Post");
            saveNewGame();
            if (engine->config->debug.NoMargaret.value()) {
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
        } else if (GetCurrentMenuID() == MENU_5 || GetCurrentMenuID() == MENU_LoadingProcInMainMenu) {
            uGameState = GAME_STATE_PLAYING;
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

GraphicsImage *gamma_preview_image = nullptr;  // 506E40

void Game_StartDialogue(int actor_id) {
    if (pParty->hasActiveCharacter()) {
        engine->_messageQueue->clear();

        initializeNPCDialogue(pActors[actor_id].npcId, true, &pActors[actor_id]);
    }
}

void Game_StartHirelingDialogue(int hireling_id) {
    assert(hireling_id == 0 || hireling_id == 1);

    if (isHirelingsBlockedOnMap(engine->_currentLoadedMapId) || current_screen_type != SCREEN_GAME)
        return;

    engine->_messageQueue->clear();

    FlatHirelings buf;
    buf.Prepare();

    int index = hireling_id + pParty->hirelingScrollPosition;
    if (index < buf.Size()) {
        if (buf.GetSacrificeStatus(index) && buf.GetSacrificeStatus(index)->inProgress)
            return; // Hireling is being dark sacrificed.

        initializeNPCDialogue(-1 - pParty->hirelingScrollPosition - hireling_id, true);
    }
}

void Game::closeTargetedSpellWindow() {
    if (pGUIWindow_CastTargetedSpell) {
        if (current_screen_type == SCREEN_CHARACTERS) {
            mouse->SetCursorImage("MICON2");
        } else {
            pGUIWindow_CastTargetedSpell->Release();  // test to fix enchanting issue
            pGUIWindow_CastTargetedSpell = nullptr;  // test to fix enchanting issue
            mouse->SetCursorImage("MICON1");
            engine->_statusBar->clearEvent();
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
        delete pGUIWindow_CurrentMenu;
        pGUIWindow_CurrentMenu = nullptr;
    }
    pEventTimer->setPaused(false);
    current_screen_type = SCREEN_GAME;
}

bool IsWindowSwitchable() {
    if (current_screen_type == SCREEN_NPC_DIALOGUE || current_screen_type == SCREEN_HOUSE
        || current_screen_type == SCREEN_INPUT_BLV || current_screen_type == SCREEN_CHANGE_LOCATION) {
        return false;
    }
    return true;
}

void Game::processQueuedMessages() {
    GUIWindow *pWindow2;        // ecx@248
    int v37;                    // eax@341
    ODMFace *pODMFace;          // ecx@412
    CastSpellInfo *pSpellInfo;  // ecx@415
    int16_t v53;                // ax@431
    int v54;                    // eax@432
    GUIButton *pButton;         // eax@578
    int v91;                    // edx@605
    int v92;                    // eax@605
    int v93;                    // edx@605
    int pPlayerNum;             // edx@611
    int uMessageParam;            // [sp+18h] [bp-5E4h]@7
    int encounter_index;           // [sp+20h] [bp-5DCh]@23
    int uNumSeconds;     // [sp+24h] [bp-5D8h]@18
    UIMessageType uMessage;  // [sp+2Ch] [bp-5D0h]@7
    int uMessageParam2;            // [sp+30h] [bp-5CCh]@7
    MapId travelMapId;
    std::array<MagicSchool, 9> spellbookPages = {};                  // [sp+158h] [bp-4A4h]@652
    int currHour;
    bool playButtonSoundOnEscape = true;

    if (bDialogueUI_InitializeActor_NPC_ID) {
        initializeNPCDialogue(bDialogueUI_InitializeActor_NPC_ID, false);
        bDialogueUI_InitializeActor_NPC_ID = 0;
    }

    while (engine->_messageQueue->haveMessages()) {
        engine->_messageQueue->popMessage(&uMessage, &uMessageParam, &uMessageParam2);
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
                Game_StartHirelingDialogue(uMessage == UIMSG_StartHireling1Dialogue ? 0 : 1);
                continue;
            case UIMSG_HouseScreenClick:
                if (window_SpeakInHouse) {
                    window_SpeakInHouse->houseScreenClick();
                }
                continue;
            case UIMSG_SelectHouseNPCDialogueOption:
                selectHouseNPCDialogueOption((DialogueId)uMessageParam);
                continue;
            case UIMSG_SelectProprietorDialogueOption:
                selectProprietorDialogueOption((DialogueId)uMessageParam);
                continue;
            case UIMSG_SelectNPCDialogueOption:
                selectNPCDialogueOption((DialogueId)uMessageParam);
                continue;
            case UIMSG_ClickHouseNPCPortrait:
                updateHouseNPCTopics(uMessageParam);
                continue;
                // case UIMSG_StartNewGame:
                // Game_StartNewGameWhilePlaying(uMessageParam); continue;
                // case UIMSG_Game_OpenLoadGameDialog:
                // Game_OpenLoadGameDialog(); continue; case UIMSG_Quit:
                // Game_QuitGameWhilePlaying(uMessageParam); continue;
            case UIMSG_80:
                assert(false);
                pGUIWindow_CurrentMenu->Release();
                current_screen_type = SCREEN_OPTIONS;
                // pGUIWindow_CurrentMenu =
                // GUIWindow::Create(0, 0,
                // window->GetWidth(), window->GetHeight(),
                // WINDOW_8, 0, 0);
                continue;
            case UIMSG_Cancel:
                new OnCancel({350, 302}, {106, 42}, pBtnCancel);
                continue;
            case UIMSG_OpenQuestBook:
                engine->_messageQueue->clear();
                // toggle
                if (current_screen_type == SCREEN_BOOKS && pGUIWindow_CurrentMenu->eWindowType == WindowType::WINDOW_QuestBook) {
                    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
                    continue;
                }
                // cant open screen - talking or in shop or map transition
                if (!IsWindowSwitchable()) {
                    continue;
                } else {
                    // close out current window
                    back_to_game();
                    onEscape();
                    engine->_statusBar->clearAll();
                }
                // open window
                pGUIWindow_CurrentMenu = new GUIWindow_QuestBook();
                continue;
            case UIMSG_OpenAutonotes:
                engine->_messageQueue->clear();
                // toggle
                if (current_screen_type == SCREEN_BOOKS && pGUIWindow_CurrentMenu->eWindowType == WindowType::WINDOW_AutonotesBook) {
                    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
                    continue;
                }
                // cant open screen - talking or in shop or map transition
                if (!IsWindowSwitchable()) {
                    continue;
                } else {
                    // close out current window
                    back_to_game();
                    onEscape();
                    engine->_statusBar->clearAll();
                }
                // open window
                pGUIWindow_CurrentMenu = new GUIWindow_AutonotesBook();
                continue;
            case UIMSG_OpenMapBook:
                engine->_messageQueue->clear();
                // toggle
                if (current_screen_type == SCREEN_BOOKS && pGUIWindow_CurrentMenu->eWindowType == WindowType::WINDOW_MapsBook) {
                    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
                    continue;
                }
                // cant open screen - talking or in shop or map transition
                if (!IsWindowSwitchable()) {
                    continue;
                } else {
                    // close out current window
                    back_to_game();
                    onEscape();
                    engine->_statusBar->clearAll();
                }
                // open window;
                pGUIWindow_CurrentMenu = new GUIWindow_MapBook();
                continue;
            case UIMSG_OpenCalendar:
                engine->_messageQueue->clear();
                // toggle
                if (current_screen_type == SCREEN_BOOKS && pGUIWindow_CurrentMenu->eWindowType == WindowType::WINDOW_CalendarBook) {
                    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
                    continue;
                }
                // cant open screen - talking or in shop or map transition
                if (!IsWindowSwitchable()) {
                    continue;
                } else {
                    // close out current window
                    back_to_game();
                    onEscape();
                    engine->_statusBar->clearAll();
                }
                // open window
                pGUIWindow_CurrentMenu = new GUIWindow_CalendarBook();
                continue;
            case UIMSG_OpenHistoryBook:
                engine->_messageQueue->clear();
                // toggle
                if (current_screen_type == SCREEN_BOOKS && pGUIWindow_CurrentMenu->eWindowType == WindowType::WINDOW_JournalBook) {
                    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
                    continue;
                }
                // cant open screen - talking or in shop or map transition
                if (!IsWindowSwitchable()) {
                    continue;
                } else {
                    // close out current window
                    back_to_game();
                    onEscape();
                    engine->_statusBar->clearAll();
                }
                // open window
                pGUIWindow_CurrentMenu = new GUIWindow_JournalBook();
                continue;
            case UIMSG_Escape:  // нажатие Escape and return to game
                back_to_game();
                engine->_messageQueue->clear();
                switch (current_screen_type) {
                    case SCREEN_SHOP_INVENTORY:
                    case SCREEN_NPC_DIALOGUE:
                    case SCREEN_CHEST:
                    case SCREEN_CHEST_INVENTORY:
                    case SCREEN_CHANGE_LOCATION:
                    case SCREEN_INPUT_BLV:
                    case SCREEN_QUICK_REFERENCE:
                        if (playButtonSoundOnEscape) {
                            PlayButtonClickSound();
                            uMessageParam = 1;
                        }
                        break;
                    case SCREEN_HOUSE:
                        if (playButtonSoundOnEscape) {
                            PlayButtonClickSound();
                            uMessageParam = 1;
                            break;
                        }
                        break;
                    default:
                        break;
                }

                if (pGameOverWindow) {
                    if (pGameOverWindow->toggleAndTestFinished()) {
                        pGameOverWindow->Release();
                        delete pGameOverWindow;
                        pGameOverWindow = nullptr;
                    }
                    continue;
                }

                if (current_screen_type == SCREEN_GAME) {
                    if (!pGUIWindow_CastTargetedSpell) {  // Draw Menu
                        new OnButtonClick2({602, 450}, {0, 0}, pBtn_GameSettings, std::string(), false);

                        engine->_messageQueue->clear();
                        _menu->MenuLoop();
                    } else {
                        pGUIWindow_CastTargetedSpell->Release();
                        pGUIWindow_CastTargetedSpell = 0;
                        mouse->SetCursorImage("MICON1");
                        engine->_statusBar->clearEvent();
                        IsEnchantingInProgress = false;
                        back_to_game();
                    }
                    continue;
                } else if (current_screen_type == SCREEN_MENU) {
                    break;
                } else if (
                    current_screen_type == SCREEN_SAVEGAME ||
                    current_screen_type == SCREEN_LOADGAME) {
                    break;
                } else if (current_screen_type == SCREEN_OPTIONS) {
                    break;
                } else if (current_screen_type == SCREEN_VIDEO_OPTIONS) {
                    break;
                } else if (current_screen_type == SCREEN_KEYBOARD_OPTIONS) {
                    break;
                } else {
                    if (current_screen_type > SCREEN_67) {
                        if (current_screen_type == SCREEN_QUICK_REFERENCE) {
                            onEscape();
                            continue;
                        }
                    } else {
                        if (current_screen_type < SCREEN_64) {
                            switch (current_screen_type) {
                                case SCREEN_CASTING:
                                    if (enchantingActiveCharacter) {
                                        pParty->setActiveCharacterIndex(enchantingActiveCharacter);
                                        pParty->switchToNextActiveCharacter();
                                        enchantingActiveCharacter = 0;
                                        if (pParty->bTurnBasedModeOn) {
                                            pTurnEngine->ApplyPlayerAction();
                                        }
                                        AfterEnchClickEventId = UIMSG_0;
                                        AfterEnchClickEventSecondParam = 0;
                                        AfterEnchClickEventTimeout = 0_ticks;
                                    }
                                    if (ptr_50C9A4_ItemToEnchant &&
                                        ptr_50C9A4_ItemToEnchant->itemId != ITEM_NULL) {
                                        ptr_50C9A4_ItemToEnchant->flags &= ~ITEM_ENCHANT_ANIMATION_MASK;
                                        ItemEnchantmentTimer = 0_ticks;
                                        ptr_50C9A4_ItemToEnchant = nullptr;
                                    }
                                    onEscape();
                                    continue;
                                case SCREEN_BOOKS:
                                    pEventTimer->setPaused(false);
                                    onEscape();
                                    continue;
                                case SCREEN_CHEST_INVENTORY:
                                    current_screen_type = SCREEN_CHEST;
                                    continue;
                                case SCREEN_CHEST:
                                    pWindow2 = pGUIWindow_CurrentMenu;
                                    pWindow2->Release();
                                    current_screen_type = SCREEN_GAME;
                                    pEventTimer->setPaused(false);
                                    continue;
                                case SCREEN_19:
                                    assert(false);
                                    pWindow2 = ptr_507BC8;
                                    pWindow2->Release();
                                    current_screen_type = SCREEN_GAME;
                                    pEventTimer->setPaused(false);
                                    continue;
                                case SCREEN_REST:  // close rest screen
                                    if (currentRestType != REST_NONE) {
                                        Rest(remainingRestTime);
                                        for (Character &character : pParty->pCharacters) {
                                            character.conditions.reset(CONDITION_SLEEP);
                                        }
                                    }
                                    if (rest_ui_sky_frame_current) {
                                        rest_ui_sky_frame_current->release();
                                        rest_ui_sky_frame_current = nullptr;
                                    }

                                    if (rest_ui_hourglass_frame_current) {
                                        rest_ui_hourglass_frame_current->release();
                                        rest_ui_hourglass_frame_current = nullptr;
                                    }

                                    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                                        pOutdoor->UpdateSunlightVectors();
                                        pOutdoor->UpdateFog();
                                    }
                                    remainingRestTime = Duration();
                                    currentRestType = REST_NONE;
                                    onEscape();
                                    continue;
                                case SCREEN_SHOP_INVENTORY:
                                    pGUIWindow_CurrentMenu->Release();
                                    current_screen_type = SCREEN_HOUSE;
                                    continue;
                                case SCREEN_HOUSE:
                                    if (uGameState == GAME_STATE_CHANGE_LOCATION) {
                                        while (houseDialogPressEscape()) {}
                                    } else {
                                        if (houseDialogPressEscape())
                                            continue;
                                    }
                                    window_SpeakInHouse->playHouseGoodbyeSpeech();
                                    pAudioPlayer->playHouseSound(SOUND_WoodDoorClosing, false);
                                    pMediaPlayer->Unload();
                                    pGUIWindow_CurrentMenu = window_SpeakInHouse;

                                    onEscape();
                                    continue;
                                case SCREEN_INPUT_BLV:  // click escape
                                    if (uCurrentHouse_Animation == 153) // TODO(Nik-RE-dev): what is this? Btw, 153 == HOUSE_EARTH_GUILD_STONE_CITY.
                                        playHouseSound(HOUSE_EARTH_GUILD_STONE_CITY, HOUSE_SOUND_MAGIC_GUILD_MEMBERS_ONLY);
                                    pMediaPlayer->Unload();
                                    if (npcIdToDismissAfterDialogue) {
                                        pParty->hirelingScrollPosition = 0;
                                        pNPCStats->pNPCData[npcIdToDismissAfterDialogue].flags &= ~NPC_HIRED;
                                        pParty->CountHirelings();
                                        npcIdToDismissAfterDialogue = 0;
                                    }
                                    DialogueEnding();
                                    current_screen_type = SCREEN_GAME;
                                    continue;
                                case SCREEN_NPC_DIALOGUE:  // click escape
                                    if (npcIdToDismissAfterDialogue) {
                                        pParty->hirelingScrollPosition = 0;
                                        pNPCStats->pNPCData[npcIdToDismissAfterDialogue].flags &= ~NPC_HIRED;
                                        pParty->CountHirelings();
                                        npcIdToDismissAfterDialogue = 0;
                                    }
                                    // goto LABEL_317;
                                    DialogueEnding();
                                    current_screen_type = SCREEN_GAME;
                                    continue;
                                case SCREEN_BRANCHLESS_NPC_DIALOG:  // click escape
                                    engine->_statusBar->clearEvent();

                                    releaseBranchlessDialogue();
                                    DialogueEnding();
                                    current_screen_type = SCREEN_GAME;
                                    continue;
                                case SCREEN_CHANGE_LOCATION: // escape
                                    pParty->pos.x = std::clamp(pParty->pos.x, -maxPartyAxisDistance, maxPartyAxisDistance);
                                    pParty->pos.y = std::clamp(pParty->pos.y, -maxPartyAxisDistance, maxPartyAxisDistance);
                                    pMediaPlayer->Unload();
                                    DialogueEnding();
                                    onEscape();
                                    continue;
                                case SCREEN_VIDEO:
                                    pMediaPlayer->Unload();
                                    continue;
                                case SCREEN_CHARACTERS:
                                    CharacterUI_ReleaseButtons();
                                    ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->releaseAwardsScrollBar();
                                    onEscape();
                                    continue;
                                case SCREEN_SPELL_BOOK:
                                    onEscape();
                                    continue;

                                default:
                                    assert(false);  // which GAME_MENU is this?
                                    onEscape();
                                    continue;
                            }
                            assert(false);  // which GAME_MENU is this?
                            onEscape();
                            continue;
                        }
                        assert(false);  // which GAME_MENU is this?
                        CharacterUI_ReleaseButtons();
                        //ReleaseAwardsScrollBar();
                    }
                    // assert(false);  // which GAME_MENU is this? debug / fallback
                    onEscape();
                    continue;
                }
                continue;

            case UIMSG_ScrollNPCPanel:  // Right and Left button for
                                        // NPCPanel
                if (uMessageParam) {
                    new OnButtonClick2({626, 179 + 2 * (pParty->alignment == PartyAlignment::PartyAlignment_Evil) }, {0, 0}, pBtn_NPCRight);
                    v37 = (!pParty->pHirelings[0].name.empty()) +
                          (!pParty->pHirelings[1].name.empty()) +
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

            case UIMSG_OnIndoorEntryExit:
                engine->_messageQueue->clear();
                playButtonSoundOnEscape = false;
                // PID_INVALID was used (exclusive sound)
                pAudioPlayer->playUISound(SOUND_StartMainChoice02);

                // PlayHouseSound(  // this is wrong - what is it meant to do??
                //    uCurrentHouse_Animation,
                //    HouseSound_NotEnoughMoney);

                if (pMovie_Track)
                    pMediaPlayer->Unload();
                DialogueEnding();

                if (engine->_teleportPoint.isValid()) {
                    if (!engine->_teleportPoint.getTeleportMap().starts_with('0')) { // '0' means teleportation within the current map.
                        //pGameLoadingUI_ProgressBar->Initialize(GUIProgressBar::TYPE_Box);
                        bool leavingArena = engine->_currentLoadedMapId == MAP_ARENA;
                        onMapLeave();
                        Transition_StopSound_Autosave(engine->_teleportPoint.getTeleportMap(), MAP_START_POINT_PARTY);
                        if (leavingArena)
                            pParty->GetPlayingTime() += Duration::fromDays(4);
                    } else {
                        engine->_teleportPoint.doTeleport(true);
                        engine->_teleportPoint.invalidate();
                    }
                } else {
                    eventProcessor(savedEventID, Pid(), 1, savedEventStep);
                }

                PlayButtonClickSound();
                DialogueEnding();
                back_to_game();
                onEscape();
                continue;
            case UIMSG_CancelIndoorEntryExit:
                PlayButtonClickSound();
                pMediaPlayer->Unload();
                engine->_teleportPoint.invalidate();
                DialogueEnding();
                back_to_game();
                onEscape();
                continue;
            case UIMSG_CycleCharacters:
                if (pParty->hasActiveCharacter()) {
                    pParty->setActiveCharacterIndex(cycleCharacter(keyboardInputHandler->IsAdventurerBackcycleToggled()));
                }
                continue;
            case UIMSG_OnTravelByFoot:
                engine->_messageQueue->clear();
                playButtonSoundOnEscape = false;

                pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                // encounter_index = (NPCData *)getTravelTime();
                travelMapId = pOutdoor->getTravelDestination(pParty->pos.x, pParty->pos.y);
                if (!engine->IsUnderwater() && pParty->bFlying || travelMapId == MAP_INVALID) {
                    PlayButtonClickSound();
                    pParty->pos.x = std::clamp(pParty->pos.x, -maxPartyAxisDistance, maxPartyAxisDistance);
                    pParty->pos.y = std::clamp(pParty->pos.y, -maxPartyAxisDistance, maxPartyAxisDistance);;
                    DialogueEnding();
                    current_screen_type = SCREEN_GAME;
                } else {
                    DialogueEnding();
                    pAudioPlayer->stopSounds();
                    pEventTimer->setPaused(true);
                    autoSave();
                    uGameState = GAME_STATE_CHANGE_LOCATION;
                    engine->_transitionMapId = travelMapId;
                    // TODO(Nik-RE-dev): rest and heal uncoditionally even if party does not have food?
                    restAndHeal(Duration::fromDays(getTravelTime()));
                    if (pParty->GetFood() > 0) {
                        pParty->restAndHeal();
                        if (pParty->GetFood() < getTravelTime()) {
                            for(Character &character : pParty->pCharacters)
                                character.SetCondition(CONDITION_WEAK, 0);
                            ++pParty->days_played_without_rest;
                        }
                        pParty->TakeFood(getTravelTime());
                    } else {
                        for (Character &character : pParty->pCharacters)
                            character.SetCondition(CONDITION_WEAK, 0);
                        ++pParty->days_played_without_rest;
                    }
                }
                continue;
            case UIMSG_CancelTravelByFoot:
                PlayButtonClickSound();
                pParty->pos.x = std::clamp(pParty->pos.x, -maxPartyAxisDistance, maxPartyAxisDistance);
                pParty->pos.y = std::clamp(pParty->pos.y, -maxPartyAxisDistance, maxPartyAxisDistance);
                DialogueEnding();
                current_screen_type = SCREEN_GAME;
                continue;
            case UIMSG_CastSpell_Telekinesis: {
                Pid pid = engine->PickMouseTarget().pid;
                ObjectType type = pid.type();
                int id = pid.id();
                bool interactionPossible = false;
                if (type == OBJECT_Actor) {
                    interactionPossible = pActors[id].aiState == Dead;
                }
                if (type == OBJECT_Sprite) {
                    interactionPossible = !(pObjectList->pObjects[pSpriteObjects[id].uObjectDescID].uFlags & OBJECT_DESC_UNPICKABLE);
                }
                if (type == OBJECT_Decoration) {
                    interactionPossible = pLevelDecorations[id].uEventID != 0;
                }
                if (type == OBJECT_Face) {
                    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                        ODMFace *pODMFace = &pOutdoor->pBModels[id >> 6].faces[id & 0x3F];
                        interactionPossible = (pODMFace->Clickable() && pODMFace->eventId);
                    } else { // Indoor
                        BLVFace *pBLVFace = &pIndoor->faces[id];
                        if (pBLVFace->Clickable()) {
                            interactionPossible = pIndoor->faceExtras[pBLVFace->faceExtraId].eventId != 0;
                        }
                    }
                }
                if (interactionPossible) {
                    spellTargetPicked(pid, -1);
                    closeTargetedSpellWindow();
                }
                continue;
            }
            case UIMSG_CastSpell_Hireling: {
                FlatHirelings buf;
                buf.Prepare();
                int flatHirelingId = pParty->hirelingScrollPosition + uMessageParam;
                if (flatHirelingId >= buf.Size())
                    continue; // Can't cast sacrifice on an empty slot.

                engine->_messageQueue->clear();
                spellTargetPicked(Pid(), uMessageParam);
                closeTargetedSpellWindow();
                continue;
            }
            case UIMSG_CastSpell_TargetCharacter:
                engine->_messageQueue->clear();
                if (IsEnchantingInProgress) {
                    // Change character while enchanting is active
                    // TODO(Nik-RE-dev): need separate message type
                    pParty->setActiveCharacterIndex(uMessageParam);
                } else {
                    spellTargetPicked(Pid(), uMessageParam);
                    closeTargetedSpellWindow();
                }
                continue;

            case UIMSG_HouseTransitionConfirmation:
                assert(false);
                playButtonSoundOnEscape = false;
                pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                autoSave();
                engine->_transitionMapId = houseNpcs[currentHouseNpc].targetMapID;
                dword_6BE364_game_settings_1 |= GAME_SETTINGS_SKIP_WORLD_UPDATE;
                uGameState = GAME_STATE_CHANGE_LOCATION;
                // v53 = buildingTable_minus1_::30[26 * (unsigned
                // int)ptr_507BC0->ptr_1C];
                v53 = std::to_underlying(houseTable[window_SpeakInHouse->houseId()]._quest_bit); // TODO(captainurist): what's going on here?
                if (v53 < 0) {
                    v54 = std::abs(v53) - 1;
                    engine->_teleportPoint.setTeleportTarget(Vec3f(teleportX[v54], teleportY[v54], teleportZ[v54]), teleportYaw[v54], 0, 0);
                }
                houseDialogPressEscape();
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
                continue;

            case UIMSG_OnCastTownPortal:
                pGUIWindow_CurrentMenu = new GUIWindow_TownPortalBook(Pid::fromPacked(uMessageParam), static_cast<SpellCastFlags>(uMessageParam2));
                continue;

            case UIMSG_OnCastLloydsBeacon:
                pGUIWindow_CurrentMenu = new GUIWindow_LloydsBook(Pid::fromPacked(uMessageParam), static_cast<SpellCastFlags>(uMessageParam2));
                continue;

            case UIMSG_LloydBookFlipButton:
                if (pGUIWindow_CurrentMenu) {
                    ((GUIWindow_LloydsBook *)pGUIWindow_CurrentMenu)->flipButtonClicked(uMessageParam != 0);
                }
                continue;

            case UIMSG_CloseAfterInstallBeacon:
                playButtonSoundOnEscape = false;
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
                continue;

            case UIMSG_InstallOrRecallBeacon:
                if (pGUIWindow_CurrentMenu) {
                    ((GUIWindow_LloydsBook *)pGUIWindow_CurrentMenu)->installOrRecallBeacon(uMessageParam);
                }
                continue;

            case UIMSG_ClickTownInTP:
                if (pGUIWindow_CurrentMenu) {
                    ((GUIWindow_TownPortalBook *)pGUIWindow_CurrentMenu)->clickTown(uMessageParam);
                }
                continue;

            case UIMSG_ShowGameOverWindow: {
                pGameOverWindow = new GUIWindow_GameOver();
                uGameState = GAME_STATE_FINAL_WINDOW;
                continue;
            }
            case UIMSG_OnGameOverWindowClose:
                pAudioPlayer->stopSounds();
                autoSave();

                pParty->pos = Vec3f(-17331, 12547, 465); // respawn point in Harmondale
                pParty->velocity = Vec3f();
                pParty->_viewYaw = 0;
                pParty->uFallStartZ = pParty->pos.z;
                pParty->_viewPitch = 0;

                // change map to Harmondale
                engine->_transitionMapId = MAP_HARMONDALE;
                engine->_teleportPoint.setTeleportTarget(pParty->pos, pParty->_viewYaw, pParty->_viewPitch, 0);
                PrepareWorld(1);
                Actor::InitializeActors();

                uGameState = GAME_STATE_PLAYING;

                for (Character &character : pParty->pCharacters) {
                    character.playEmotion(PORTRAIT_WIDE_SMILE, 0_ticks);
                }

                // strcpy((char *)userInputHandler->pPressedKeysBuffer, "2");
                // assert(false);  // missed break/continue?
                continue;

            case UIMSG_DD: {
                assert(false);
                // sprintf(tmp_str.data(), "%s",
                // pKeyActionMap->pPressedKeysBuffer);
                FrameTableTxtLine frameTableTxtLine;
                txt_file_frametable_parser(keyboardInputHandler->GetTextInput().c_str(), &frameTableTxtLine);
                std::string status_string;
                if (frameTableTxtLine.uPropCount == 1) {
                    MapId map_index = static_cast<MapId>(atoi(frameTableTxtLine.pProperties[0]));
                    if (map_index < MAP_FIRST || map_index > MAP_LAST)
                        continue;
                    engine->_transitionMapId = map_index;
                    dword_6BE364_game_settings_1 |= GAME_SETTINGS_SKIP_WORLD_UPDATE;
                    uGameState = GAME_STATE_CHANGE_LOCATION;
                    onMapLeave();
                    continue;
                } else {
                    if (frameTableTxtLine.uPropCount != 3) continue;
                    int x = atoi(frameTableTxtLine.pProperties[0]);
                    int y = atoi(frameTableTxtLine.pProperties[1]);
                    int z = atoi(frameTableTxtLine.pProperties[2]);
                    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                        if (pIndoor->GetSector(x, y, z)) {
                            pParty->pos = Vec3f(x, y, z);
                            pParty->uFallStartZ = z;
                            continue;
                        }
                    } else {
                        if ((x > -32768) && (x < 32768) && (y > -32768) && (y < 32768) && (z >= 0) && (z < 10000)) {
                            pParty->pos = Vec3f(x, y, z);
                            pParty->uFallStartZ = z;
                            continue;
                        }
                    }
                    pAudioPlayer->playUISound(SOUND_error);
                    status_string = "Can't jump to that location!";
                }
                engine->_statusBar->setEvent(status_string);
                continue;
            }
            case UIMSG_CastQuickSpell: {
                if (engine->IsUnderwater()) {
                    engine->_statusBar->setEvent(LSTR_YOU_CAN_NOT_DO_THAT_WHILE_YOU_ARE);
                    pAudioPlayer->playUISound(SOUND_error);
                    continue;
                }
                if (!pParty->hasActiveCharacter() || pParty->activeCharacter().timeToRecovery) {
                    continue;
                }
                pushSpellOrRangedAttack(pParty->activeCharacter().uQuickSpell, pParty->activeCharacterIndex() - 1,
                                        CombinedSkillValue::none(), 0, pParty->activeCharacterIndex());
                continue;
            }

            case UIMSG_CastSpell_TargetActorBuff:
            case UIMSG_CastSpell_TargetActor: {
                Vis_PIDAndDepth object = engine->PickMouseTarget();
                Pid pid = object.pid;
                int depth = object.depth;
                if (pid.type() == OBJECT_Actor && depth < engine->config->gameplay.RangedAttackDepth.value()) {
                    spellTargetPicked(pid, -1);
                    closeTargetedSpellWindow();
                }
                continue;
            }
            case UIMSG_1C:
                assert(false);
                if (!pParty->hasActiveCharacter() || current_screen_type != SCREEN_GAME)
                    continue;
                assert(false);  // ptr_507BC8 = GUIWindow::Create(0, 0,
                                // window->GetWidth(), window->GetHeight(),
                                // WINDOW_68, uMessageParam, 0);
                current_screen_type = SCREEN_19;
                pEventTimer->setPaused(true);
                continue;
            case UIMSG_STEALFROMACTOR:
                if (!pParty->hasActiveCharacter()) continue;
                if (!pParty->bTurnBasedModeOn) {
                    if (pActors[uMessageParam].aiState == AIState::Dead)
                        pActors[uMessageParam].LootActor();
                    else
                        Actor::StealFrom(uMessageParam);
                    continue;
                }
                if (pTurnEngine->turn_stage == TE_WAIT ||
                    pTurnEngine->turn_stage == TE_MOVEMENT)
                    continue;
                if (!(pTurnEngine->flags & TE_HAVE_PENDING_ACTIONS)) {
                    if (pActors[uMessageParam].aiState == AIState::Dead)
                        pActors[uMessageParam].LootActor();
                    else
                        Actor::StealFrom(uMessageParam);
                }
                continue;

            case UIMSG_Attack:
                if (!pParty->hasActiveCharacter()) continue;
                if (!pParty->bTurnBasedModeOn) {
                    Character::_42ECB5_CharacterAttacksActor();
                    continue;
                }
                if (pTurnEngine->turn_stage == TE_WAIT ||
                    pTurnEngine->turn_stage == TE_MOVEMENT)
                    continue;
                if (!(pTurnEngine->flags & TE_HAVE_PENDING_ACTIONS))
                    Character::_42ECB5_CharacterAttacksActor();
                continue;
            case UIMSG_ExitRest:
                new OnCancel(pButton_RestUI_Exit->rect.topLeft(), {0, 0}, pButton_RestUI_Exit, localization->str(LSTR_EXIT_REST));
                continue;
            case UIMSG_Wait5Minutes:
                if (currentRestType == REST_HEAL) {
                    engine->_statusBar->setEvent(LSTR_YOU_ARE_ALREADY_RESTING);
                    pAudioPlayer->playUISound(SOUND_error);
                    continue;
                }
                new OnButtonClick2(pButton_RestUI_Wait5Minutes->rect.topLeft(), {0, 0}, pButton_RestUI_Wait5Minutes,
                    localization->str(LSTR_WAIT_5_MINUTES));
                currentRestType = REST_WAIT;
                remainingRestTime = Duration::fromMinutes(5);
                continue;
            case UIMSG_Wait1Hour:
                if (currentRestType == REST_HEAL) {
                    engine->_statusBar->setEvent(LSTR_YOU_ARE_ALREADY_RESTING);
                    pAudioPlayer->playUISound(SOUND_error);
                    continue;
                }
                new OnButtonClick2(pButton_RestUI_Wait1Hour->rect.topLeft(), {0, 0}, pButton_RestUI_Wait1Hour,
                    localization->str(LSTR_WAIT_1_HOUR));
                currentRestType = REST_WAIT;
                remainingRestTime = Duration::fromHours(1);
                continue;
            case UIMSG_RentRoom: {
                HouseId tavern = static_cast<HouseId>(uMessageParam);
                assert(isTavern(tavern));

                pGUIWindow_CurrentMenu = new GUIWindow_Rest();

                remainingRestTime = timeUntilDawn() + Duration::fromHours(1);
                if (tavern == HOUSE_TAVERN_DEYJA || tavern == HOUSE_TAVERN_PIT || tavern == HOUSE_TAVERN_MOUNT_NIGHON) {
                    remainingRestTime = remainingRestTime + Duration::fromHours(12);
                }
                currentRestType = REST_HEAL;
                pParty->restAndHeal();
                pParty->days_played_without_rest = 0;
                for (Character &character : pParty->pCharacters) {
                    character.conditions.set(CONDITION_SLEEP, Time::fromTicks(1));
                }
                continue;
            }
            case UIMSG_RestWindow:
                engine->_messageQueue->clear();
                // toggle
                //if (current_screen_type == SCREEN_REST) {
                //    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
                //    continue;
                //}
                if (current_screen_type != SCREEN_GAME) continue;
                closeTargetedSpellWindow();

                if (CheckActors_proximity()) {
                    if (pParty->bTurnBasedModeOn) {
                        engine->_statusBar->setEvent(LSTR_YOU_CANT_REST_IN_TURN_BASED_MODE);
                        continue;
                    }

                    if (pParty->uFlags & (PARTY_FLAG_AIRBORNE | PARTY_FLAG_STANDING_ON_WATER)) // airbourne or on water
                        engine->_statusBar->setEvent(LSTR_YOU_CANT_REST_HERE);
                    else
                        engine->_statusBar->setEvent(LSTR_THERE_ARE_HOSTILE_ENEMIES_NEAR);

                    if (!pParty->hasActiveCharacter()) continue;
                    pParty->activeCharacter().playReaction(SPEECH_CANT_REST_HERE);
                    continue;
                }
                if (pParty->bTurnBasedModeOn) {
                    engine->_statusBar->setEvent(LSTR_YOU_CANT_REST_IN_TURN_BASED_MODE);
                    continue;
                }

                if (!(pParty->uFlags & (PARTY_FLAG_AIRBORNE | PARTY_FLAG_STANDING_ON_WATER))) {
                    pGUIWindow_CurrentMenu = new GUIWindow_Rest();
                    continue;
                } else {
                    if (pParty->uFlags & PARTY_FLAG_AIRBORNE)
                        logger->trace("Party is airborne");
                    if (pParty->uFlags & PARTY_FLAG_STANDING_ON_WATER)
                        logger->trace("Party on water");
                }

                if (pParty->bTurnBasedModeOn) {
                    engine->_statusBar->setEvent(LSTR_YOU_CANT_REST_IN_TURN_BASED_MODE);
                    continue;
                }

                if (pParty->uFlags & (PARTY_FLAG_AIRBORNE | PARTY_FLAG_STANDING_ON_WATER))
                    engine->_statusBar->setEvent(LSTR_YOU_CANT_REST_HERE);
                else
                    engine->_statusBar->setEvent(LSTR_THERE_ARE_HOSTILE_ENEMIES_NEAR);

                if (!pParty->hasActiveCharacter()) continue;
                pParty->activeCharacter().playReaction(SPEECH_CANT_REST_HERE);
                continue;
            case UIMSG_Rest8Hour:
                engine->_messageQueue->clear(); // TODO: sometimes it is called twice, prevent that for now and investigate why later
                if (currentRestType != REST_NONE) {
                    engine->_statusBar->setEvent(LSTR_YOU_ARE_ALREADY_RESTING);
                    pAudioPlayer->playUISound(SOUND_error);
                    continue;
                }
                if (pParty->GetFood() < foodRequiredToRest) {
                    engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_FOOD_TO_REST);
                    if (pParty->hasActiveCharacter() && pParty->activeCharacter().CanAct()) {
                        pParty->activeCharacter().playReaction(SPEECH_NOT_ENOUGH_FOOD);
                    }
                } else {
                    for (Character &character : pParty->pCharacters) {
                        character.conditions.set(CONDITION_SLEEP, pParty->GetPlayingTime());
                    }
                    MapId mapIdx = engine->_currentLoadedMapId;
                    assert(mapIdx != MAP_INVALID);
                    // Was this, which made exactly zero sense:
                    // if (mapIdx == MAP_INVALID)
                    //    mapIdx = static_cast<MAP_TYPE>(grng->random(pMapStats->uNumMaps + 1));
                    MapInfo *pMapInfo = &pMapStats->pInfos[mapIdx];

                    if (grng->random(100) + 1 <= pMapInfo->encounterChance && !engine->config->debug.NoActors.value()) {
                        v91 = grng->random(100);
                        v92 = pMapInfo->encounter1Chance;
                        v93 = v91 + 1;
                        // TODO(captainurist): this is some weird code here.
                        if (v93 > v92)
                            encounter_index = v93 > v92 + pMapInfo->encounter2Chance + 2;
                        else
                            encounter_index = 1;

                        if (!SpawnEncounterMonsters(pMapInfo, encounter_index))
                            encounter_index = 0;

                        if (encounter_index) {
                            pPlayerNum = grng->random(4);
                            pParty->pCharacters[pPlayerNum].conditions.reset(CONDITION_SLEEP);
                            Rest(Duration::fromHours(1) + Duration::fromMinutes(grng->random(6)));
                            remainingRestTime = Duration();
                            currentRestType = REST_NONE;

                            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
                            engine->_statusBar->setEvent(LSTR_ENCOUNTER);
                            pAudioPlayer->playUISound(SOUND_encounter);
                            continue;
                        }
                    }
                    pParty->TakeFood(foodRequiredToRest);
                    remainingRestTime = Duration::fromHours(8);
                    currentRestType = REST_HEAL;
                    pParty->restAndHeal();
                    pParty->days_played_without_rest = 0;
                    for (Character &character : pParty->pCharacters) {
                        character.conditions.set(CONDITION_SLEEP, Time::fromTicks(1));
                    }
                }
                continue;
            case UIMSG_WaitTillDawn:
                if (currentRestType == REST_HEAL) {
                    engine->_statusBar->setEvent(LSTR_YOU_ARE_ALREADY_RESTING);
                    pAudioPlayer->playUISound(SOUND_error);
                    continue;
                }
                new OnButtonClick2(pButton_RestUI_WaitUntilDawn->rect.topLeft(), {0, 0}, pButton_RestUI_WaitUntilDawn,
                                   localization->str(LSTR_WAIT_UNTIL_DAWN));
                currentRestType = REST_WAIT;
                remainingRestTime = timeUntilDawn();
                continue;

            case UIMSG_ClickInstallRemoveQuickSpellBtn: {
                new OnButtonClick2(pBtn_InstallRemoveSpell->rect.topLeft(), {0, 0}, pBtn_InstallRemoveSpell);
                if (!pParty->hasActiveCharacter())
                    continue;
                Character *character = &pParty->activeCharacter();
                if (spellbookSelectedSpell == SPELL_NONE || spellbookSelectedSpell == character->uQuickSpell) {
                    character->uQuickSpell = SPELL_NONE;
                    spellbookSelectedSpell = SPELL_NONE;
                    pAudioPlayer->playUISound(SOUND_fizzle);
                    continue;
                }
                pParty->activeCharacter().uQuickSpell = spellbookSelectedSpell;
                if (pParty->hasActiveCharacter()) {
                    character->playReaction(SPEECH_SET_QUICK_SPELL);
                }
                continue;
            }

            case UIMSG_SpellBook_PressTab:  //перелистывание страниц
                                            //клавишей Tab
            {
                if (!pParty->hasActiveCharacter()) continue;
                int skill_count = 0;
                int uAction = 0;
                for (MagicSchool page : allMagicSchools()) {
                    Skill skill = skillForMagicSchool(page);
                    if (pParty->activeCharacter().pActiveSkills[skill] || engine->config->debug.AllMagic.value()) {
                        if (pParty->activeCharacter().lastOpenedSpellbookPage == page)
                            uAction = skill_count;
                        spellbookPages[skill_count++] = page;
                    }
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
                    ((GUIWindow_Spellbook *)pGUIWindow_CurrentMenu)->openSpellbookPage(spellbookPages[uAction]);
                }
                continue;
            }
            case UIMSG_OpenSpellbookPage:
                if (pTurnEngine->turn_stage == TE_MOVEMENT ||
                    !pParty->hasActiveCharacter() ||
                    static_cast<MagicSchool>(uMessageParam) == pParty->activeCharacter().lastOpenedSpellbookPage) {
                    continue;
                }
                ((GUIWindow_Spellbook *)pGUIWindow_CurrentMenu)->openSpellbookPage(static_cast<MagicSchool>(uMessageParam));
                continue;
            case UIMSG_SelectSpell: {
                if (pTurnEngine->turn_stage == TE_MOVEMENT) {
                    continue;
                }
                if (!pParty->hasActiveCharacter()) {
                    continue;
                }

                Character *character = &pParty->activeCharacter();
                SpellId selectedSpell = static_cast<SpellId>(uMessageParam);
                if (character->bHaveSpell[selectedSpell] || engine->config->debug.AllMagic.value()) {
                    if (spellbookSelectedSpell == selectedSpell) {
                        pGUIWindow_CurrentMenu->Release();  // spellbook close
                        pEventTimer->setPaused(false);
                        current_screen_type = SCREEN_GAME;
                        // Processing must happen on next frame because need to close spell book and update
                        // drawing object list which is used to count actors for some spells
                        engine->_messageQueue->addMessageNextFrame(UIMSG_CastSpellFromBook, std::to_underlying(selectedSpell), pParty->activeCharacterIndex() - 1);
                    } else {
                        spellbookSelectedSpell = selectedSpell;
                    }
                }
                continue;
            }

            case UIMSG_CastSpellFromBook:
                if (pTurnEngine->turn_stage != TE_MOVEMENT) {
                    pushSpellOrRangedAttack(static_cast<SpellId>(uMessageParam), uMessageParam2, CombinedSkillValue::none(), 0, 0);
                }
                continue;

            case UIMSG_SpellScrollUse:
                if (pTurnEngine->turn_stage != TE_MOVEMENT) {
                    pushScrollSpell(static_cast<SpellId>(uMessageParam), uMessageParam2);
                }
                continue;

            case UIMSG_SpellBookWindow:
                if (pTurnEngine->turn_stage == TE_MOVEMENT) {
                    continue;
                }
                if (engine->IsUnderwater()) {
                    engine->_statusBar->setEvent(LSTR_YOU_CAN_NOT_DO_THAT_WHILE_YOU_ARE);
                    pAudioPlayer->playUISound(SOUND_error);
                } else {
                    engine->_messageQueue->clear();
                    if (pParty->hasActiveCharacter()) {
                        if (!pParty->activeCharacter().timeToRecovery) {
                            // toggle
                            if (current_screen_type == SCREEN_SPELL_BOOK) {
                                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
                                continue;
                            }
                            // cant open screen - talking or in shop or map transition
                            if (!IsWindowSwitchable()) {
                                continue;
                            } else {
                                // close out current window
                                back_to_game();
                                onEscape();
                                engine->_statusBar->clearAll();
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
                engine->_messageQueue->clear();
                // toggle
                if (current_screen_type == SCREEN_QUICK_REFERENCE) {
                    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
                    continue;
                }
                // cant open screen - talking or in shop or map transition
                if (!IsWindowSwitchable()) {
                    continue;
                } else {
                    // close out current window
                    back_to_game();
                    onEscape();
                    engine->_statusBar->clearAll();
                }
                // open window
                new OnButtonClick2({560, 450}, {0, 0}, pBtn_QuickReference);
                pGUIWindow_CurrentMenu = new GUIWindow_QuickReference();
                continue;
            case UIMSG_GameMenuButton:
                if (current_screen_type != SCREEN_GAME) {
                    pGUIWindow_CurrentMenu->Release();
                    pEventTimer->setPaused(false);
                    current_screen_type = SCREEN_GAME;
                }

                if (gamma_preview_image) {
                    gamma_preview_image->release();
                    gamma_preview_image = nullptr;
                }
                gamma_preview_image = GraphicsImage::Create(render->MakeViewportScreenshot(155, 117));

                new OnButtonClick({602, 450}, {0, 0}, pBtn_GameSettings);
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
                continue;
            case UIMSG_ClickAwardScrollBar:
                ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->clickAwardsScroll(mouse->position().y);
                pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                continue;
            case UIMSG_ClickAwardsUpBtn:
                new OnButtonClick3(WINDOW_CharacterWindow_Awards, pBtn_Up->rect.topLeft(), {0, 0}, pBtn_Up);
                ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->clickAwardsUp();
                continue;
            case UIMSG_ClickAwardsDownBtn:
                new OnButtonClick3(WINDOW_CharacterWindow_Awards, pBtn_Down->rect.topLeft(), {0, 0}, pBtn_Down);
                ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->clickAwardsDown();
                continue;
            case UIMSG_ChangeDetaliz:
                ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->ToggleRingsOverlay();
                continue;
            case UIMSG_ClickPaperdoll:
                OnPaperdollLeftClick();
                continue;
            case UIMSG_SkillUp:
            {
                Skill skill = static_cast<Skill>(uMessageParam);
                Character *character = &pParty->activeCharacter();
                CombinedSkillValue skillValue = character->getSkillValue(skill);
                int cost = skillValue.level() + 1;

                if (character->uSkillPoints < cost) {
                    engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_SKILL_POINTS);
                } else {
                    if (skillValue.level() < skills_max_level[skill]) {
                        character->setSkillValue(skill, CombinedSkillValue::increaseLevel(skillValue));
                        character->uSkillPoints -= cost;
                        character->playReaction(SPEECH_SKILL_INCREASE);
                        pAudioPlayer->playUISound(SOUND_quest);
                    } else {
                        engine->_statusBar->setEvent(LSTR_YOU_HAVE_ALREADY_MASTERED_THIS_SKILL);
                    }
                }
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
                new OnCancel2(pCharacterScreen_ExitBtn->rect.topLeft(), {0, 0}, pCharacterScreen_ExitBtn);
                continue;
            case UIMSG_ClickBooksBtn:
                switch (BookButtonAction(uMessageParam)) {
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
                ((GUIWindow_Book *)pGUIWindow_CurrentMenu)->bookButtonClicked(BookButtonAction(uMessageParam));
                continue;
            case UIMSG_SelectCharacter:
                engine->_messageQueue->clear();
                GameUI_OnPlayerPortraitLeftClick(uMessageParam);
                continue;
            case UIMSG_CHEST_ClickItem:
                if (current_screen_type == SCREEN_CHEST_INVENTORY) {
                    pParty->activeCharacter().OnInventoryLeftClick();
                    continue;
                }
                Chest::OnChestLeftClick();
                continue;
            case UIMSG_InventoryLeftClick:
                pParty->activeCharacter().OnInventoryLeftClick();
                continue;
            case UIMSG_MouseLeftClickInGame:
                engine->_messageQueue->clear();
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_MouseLeftClickInScreen, 0, 0);
                continue;
            case UIMSG_MouseLeftClickInScreen:  // срабатывает при нажатии на
                                                // правую кнопку мыши после
                                                // UIMSGmouseLeftClickInGame
                engine->_messageQueue->clear();
                engine->onGameViewportClick();
                continue;
            case UIMSG_F:  // what event?
                assert(false);
                //pButton2 = (GUIButton *)(uint16_t)vis->get_picked_object_zbuf_val().object_pid;
                assert(false);  // GUIWindow::Create(0, 0, 0, 0, WINDOW_F, (int)pButton2, 0);
                continue;
            case UIMSG_54:  // what event?
                assert(false);
                //pButton2 = (GUIButton *)uMessageParam;
                assert(false);  // GUIWindow::Create(0, 0, 0, 0, WINDOW_22, (int)pButton2, 0);
                continue;
            case UIMSG_Game_Action:
                engine->_messageQueue->clear();
                // if currently in a chest
                if (current_screen_type == SCREEN_CHEST) {
                    Chest::GrabItem(keyboardInputHandler->IsTakeAllToggled());
                } else {
                    onPressSpace();
                }
                continue;
            case UIMSG_ClickZoomInBtn:
                if (!(current_screen_type == SCREEN_GAME)) continue;
                new OnButtonClick2({519, 136}, {0, 0}, pBtn_ZoomIn);
                uNumSeconds = 131072;

                viewparams->uMinimapZoom *= 2;

                if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                    if (viewparams->uMinimapZoom > 4096) {
                        viewparams->uMinimapZoom = 4096;
                    }
                } else {
                    if (viewparams->uMinimapZoom > 2048) {
                        viewparams->uMinimapZoom = 2048;
                    }
                }

                break;
            case UIMSG_ClickZoomOutBtn:
                if (!(current_screen_type == SCREEN_GAME)) continue;
                new OnButtonClick2({574, 136}, {0, 0}, pBtn_ZoomOut);
                uNumSeconds = 32768;

                viewparams->uMinimapZoom /= 2;

                if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                    if (viewparams->uMinimapZoom < 512) {
                        viewparams->uMinimapZoom = 512;
                    }
                } else {
                    if (viewparams->uMinimapZoom < 256) {
                        viewparams->uMinimapZoom = 256;
                    }
                }

                break;

            case UIMSG_OpenInventory: {
                if (pParty->hasActiveCharacter()) {
                    pGUIWindow_CurrentMenu = new GUIWindow_CharacterRecord(pParty->activeCharacterIndex(), SCREEN_CHARACTERS);
                    ((GUIWindow_CharacterRecord *)pGUIWindow_CurrentMenu)->ShowInventoryTab();
                }
                break;
            }
            case UIMSG_QuickSave:
                if (engine->_currentLoadedMapId == MAP_ARENA) {
                    engine->_statusBar->setEvent(LSTR_NO_SAVING_IN_THE_ARENA);
                    pAudioPlayer->playUISound(SOUND_error);
                } else {
                    quickSaveGame();
                }
                continue;
            case UIMSG_QuickLoad:
                quickLoadGame();
                continue;
            default:
                logger->warning("Game::processQueuedMessages - Unhandled message type: {}", static_cast<int>(uMessage));
                continue;
        }
    }

    engine->_messageQueue->swapFrames();

    if (AfterEnchClickEventId != UIMSG_0) {
        AfterEnchClickEventTimeout = std::max(0_ticks, AfterEnchClickEventTimeout - pEventTimer->dt());
        if (!AfterEnchClickEventTimeout) {
            engine->_messageQueue->addMessageCurrentFrame(AfterEnchClickEventId, AfterEnchClickEventSecondParam, 0);
            AfterEnchClickEventId = UIMSG_0;
            AfterEnchClickEventSecondParam = 0;
        }
    }
    CastSpellInfoHelpers::castSpell();
}

//----- (0046A14B) --------------------------------------------------------
void Game::onPressSpace() {
    Pid pid = engine->PickKeyboard(engine->config->gameplay.KeyboardInteractionDepth.value(),
                                    &vis_decoration_noevent_filter, &vis_door_filter).pid;
    if (pid) {
        DoInteractionWithTopmostZObject(pid);
    }
}

void Game::gameLoop() {
    bool bLoading;
    MapId mapid;

    bLoading = sCurrentMenuID == MENU_LoadingProcInMainMenu;
    SetCurrentMenuID(MENU_NONE);
    if (bLoading) {
        uGameState = GAME_STATE_PLAYING;
        loadGame(pSavegameList->selectedSlot);
    }

    extern bool use_music_folder;
    GameUI_LoadPlayerPortraitsAndVoices();
    pIcons_LOD->reserveLoadedTextures();
    // pAudioPlayer->SetMusicVolume(engine->config->music_level);

    while (true) {
        engine->_messageQueue->clear();

        pPartyActionQueue->uNumActions = 0;

        pTurnEngine->End(false);
        pParty->bTurnBasedModeOn = false;  // Make sure turn engine and party turn based mode flag are in sync.

        DoPrepareWorld(bLoading, 1);
        pEventTimer->setPaused(false);
        dword_6BE364_game_settings_1 |= GAME_SETTINGS_0080_SKIP_USER_INPUT_THIS_FRAME;
        // uGame_if_0_else_ui_id__11_save__else_load__8_drawSpellInfoPopup__22_final_window__26_keymapOptions__2_options__28_videoOptions
        // = 0;
        current_screen_type = SCREEN_GAME;

        bool game_finished = false;
        do {
            MessageLoopWithWait();

            engine->particle_engine->UpdateParticles();
            engine->decal_builder->bloodsplat_container->uNumBloodsplats = 0;
            if (engine->uNumStationaryLights_in_pStationaryLightsStack != pStationaryLightsStack->uNumLightsActive) {
                engine->uNumStationaryLights_in_pStationaryLightsStack = pStationaryLightsStack->uNumLightsActive;
            }

            keyboardInputHandler->GenerateInputActions();
            processQueuedMessages();
            if (pArcomageGame->bGameInProgress) {
                ArcomageGame::Loop();
                render->Present();
                continue;
            }

            pMediaPlayer->HouseMovieLoop();

            pEventTimer->tick();
            pMiscTimer->tick();

            if (pMiscTimer->isPaused() && !pEventTimer->isPaused())
                pMiscTimer->setPaused(false);
            if (pEventTimer->isTurnBased() && !pParty->bTurnBasedModeOn)
                pEventTimer->setTurnBased(false);
            if (!pEventTimer->isPaused() && uGameState == GAME_STATE_PLAYING) {
                onTimer();

                if (!pEventTimer->isTurnBased()) {
                    _494035_timed_effects__water_walking_damage__etc(pEventTimer->dt());
                } else {
                    // Need to process party death in turn-based mode.
                    maybeWakeSoloSurvivor();
                    updatePartyDeathState();
                }

                if (dword_6BE364_game_settings_1 & GAME_SETTINGS_SKIP_WORLD_UPDATE) {
                    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_SKIP_WORLD_UPDATE;
                } else {
                    Actor::UpdateActorAI();
                    UpdateUserInput_and_MapSpecificStuff();
                }
            }

            pAudioPlayer->UpdateSounds();

            GameUI_WritePointedObjectStatusString();
            engine->_statusBar->update();
            turnBasedOverlay.update(pMiscTimer->dt(), pTurnEngine->turn_stage);

            if (uGameState == GAME_STATE_PLAYING) {
                engine->Draw();
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
                uGameState == GAME_STATE_GAME_QUITTING_TO_MAIN_MENU ||
                uGameState == GAME_FINISHED) {
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
                engine->Draw();  // when could this occur - can it be dropped?
                continue;
            }
            if (uGameState == GAME_STATE_PARTY_DIED) {
                pAudioPlayer->stopSounds();
                pParty->pHirelings[0] = NPCData();
                pParty->pHirelings[1] = NPCData();
                for (int i = 0; i < (signed int)pNPCStats->uNumNewNPCs; ++i) {
                    if (pNPCStats->pNPCData[i].field_24)
                        pNPCStats->pNPCData[i].flags &= ~NPC_HIRED;
                }
                pMediaPlayer->PlayFullscreenMovie("losegame");
                if (pMovie_Track)
                    pMediaPlayer->Unload();
                ++pParty->uNumDeaths;
                for (Character &character : pParty->pCharacters) {
                    character.SetVariable(VAR_Award, std::to_underlying(AWARD_DEATHS));
                }
                pParty->days_played_without_rest = 0;
                pParty->GetPlayingTime() += Duration::fromDays(7);  // += 2580480
                pParty->uFlags &= ~(PARTY_FLAG_WATER_DAMAGE | PARTY_FLAG_BURNING);
                pParty->SetGold(0);
                pActiveOverlayList->Reset();
                pParty->pPartyBuffs.fill(SpellBuff());

                if (pParty->bTurnBasedModeOn) {
                    pTurnEngine->End(true);
                    pParty->bTurnBasedModeOn = false;
                }
                for (Character &character : pParty->pCharacters) {
                    character.conditions.resetAll();
                    character.pCharacterBuffs.fill(
                        SpellBuff());  // ???
                                       // memset(pParty->pCharacters[i].conditions_times.data(),
                                       // 0, 0xA0u);//(pConditions, 0, 160)
                                       // memset(pParty->pCharacters[i].pCharacterBuffs.data(),
                                       // 0, 0x180u);//(pCharacterBuffs[0], 0, 384)
                    character.health = 1;
                }
                pParty->setActiveCharacterIndex(1);

                if (pParty->_questBits[QBIT_ESCAPED_EMERALD_ISLE]) {
                    pParty->pos = Vec3f(-17331, 12547, 465); // respawn in harmondale
                    pParty->_viewYaw = 0;
                    mapid = MAP_HARMONDALE;
                } else {
                    pParty->pos = Vec3f(12552, 1816, 193); // respawn on emerald isle
                    pParty->_viewYaw = 512;
                    mapid = pMapStats->GetMapInfo(_config->gameplay.StartingMap.value());
                    // TODO(Nik-RE-dev): should not be an assert but an exception or error message.
                    assert(mapid != MAP_INVALID);
                }
                pParty->uFallStartZ = pParty->pos.z;
                pParty->_viewPitch = 0;
                pParty->velocity = Vec3f();
                // change map
                if (engine->_currentLoadedMapId != mapid) {
                    engine->_transitionMapId = mapid;
                    engine->_teleportPoint.setTeleportTarget(pParty->pos, pParty->_viewYaw, pParty->_viewPitch, 0);
                    PrepareWorld(1);
                }
                pMiscTimer->setPaused(false);
                pEventTimer->setPaused(false);

                Actor::InitializeActors();

                int playerId = pParty->getRandomActiveCharacterId(vrng);

                if (playerId != -1) {
                    pParty->pCharacters[playerId].playReaction(SPEECH_CHEATED_DEATH);
                }

                engine->_statusBar->setEvent(LSTR_ONCE_AGAIN_YOUVE_CHEATED_DEATH);
                uGameState = GAME_STATE_PLAYING;

                // need to clear messages here??
            }
        } while (!game_finished);

        pEventTimer->setPaused(true);
        engine->ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
        if (uGameState == GAME_STATE_LOADING_GAME) {
            GameUI_LoadPlayerPortraitsAndVoices();
            uGameState = GAME_STATE_PLAYING;
            bLoading = true;
            continue;
        }
        break;
    }
    current_screen_type = SCREEN_VIDEO;
}
