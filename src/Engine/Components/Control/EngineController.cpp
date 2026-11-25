#include "EngineController.h"

#include <algorithm>
#include <cassert>
#include <utility>
#include <thread>
#include <string>
#include <memory>
#include <vector>


#include "GUI/GUIMessageQueue.h"
#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UISpell.h"
#include "GUI/UI/UIStatusBar.h"

#include "Engine/Engine.h"
#include "Engine/GameType.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/PartyPlacement.h"
#include "Engine/SaveLoad.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/EngineGlobals.h"
#include "Engine/mm7_data.h"
#include "Engine/Party.h"
#include "Engine/Random/Random.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Item.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Tables/DecorationTable.h"
#include "Engine/Tables/ObjectTable.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/Snapshots/EntitySnapshots.h"
#include "Engine/Timer.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Vis.h"

#include "Io/Mouse.h"
#include "Engine/Spells/SpellEnumFunctions.h"
#include "Engine/Spells/Spells.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Platform/Interface/PlatformEnumFunctions.h"
#include "Library/Platform/Interface/PlatformEvents.h"
#include "Library/Snapshots/CommonSnapshots.h"

#include "Utility/Exception.h"
#include "Utility/ScopedRollback.h"

namespace {
class ThrowingTicker {
 public:
    explicit ThrowingTicker(EngineController *controller, std::string_view exceptionMessage, int maxTicks = 128) : _controller(controller), _exceptionMessage(exceptionMessage), _maxTicks(maxTicks) {}

    void tick(int count = 1) {
        for (int i = 0; i < count; i++) {
            if (++_ticks >= _maxTicks)
                throw Exception("{}", _exceptionMessage);
            _controller->tick();
        }
    }

 private:
    EngineController *_controller = nullptr;
    std::string _exceptionMessage;
    int _ticks = 0;
    int _maxTicks = 128;
};
} // anonymous namespace

EngineController::EngineController(EngineControlStateHandle state): _state(std::move(state)) {}

EngineController::~EngineController() = default;

void (*g_engineTickObserver)() = nullptr;

void EngineController::tick(int count) {
    for (int i = 0; i < count; i++) {
        _state.yieldExecution();

        // We should check `terminating` after a call to `yieldExecution` because it cannot be set before the call -
        // the only place it's set is the main thread, and main thread wasn't running before the call.
        if (_state->terminating)
            throw EngineControlState::TerminationException();

        if (g_engineTickObserver)
            g_engineTickObserver();
    }
}

void EngineController::postEvent(std::unique_ptr<PlatformEvent> event) {
    _state->postedEvents.push(std::move(event));
}

void EngineController::pressKey(PlatformKey key) {
    std::unique_ptr<PlatformKeyEvent> event = std::make_unique<PlatformKeyEvent>();
    event->type = EVENT_KEY_PRESS;
    event->window = ::application->window();
    event->key = key;
    event->mods = 0;
    event->isAutoRepeat = false;
    postEvent(std::move(event));
}

void EngineController::pressAutoRepeatedKey(PlatformKey key) {
    std::unique_ptr<PlatformKeyEvent> event = std::make_unique<PlatformKeyEvent>();
    event->type = EVENT_KEY_PRESS;
    event->window = ::application->window();
    event->key = key;
    event->mods = 0;
    event->isAutoRepeat = true;
    postEvent(std::move(event));
}

void EngineController::releaseKey(PlatformKey key) {
    std::unique_ptr<PlatformKeyEvent> event = std::make_unique<PlatformKeyEvent>();
    event->type = EVENT_KEY_RELEASE;
    event->window = ::application->window();
    event->key = key;
    event->mods = 0;
    event->isAutoRepeat = false;
    postEvent(std::move(event));
}

void EngineController::pressButton(PlatformMouseButton button, int x, int y, bool isDoubleClick) {
    pressOrReleaseButton(EVENT_MOUSE_BUTTON_PRESS, button, x, y, isDoubleClick);
}

void EngineController::pressButton(PlatformMouseButton button, Pointi point, bool isDoubleClick) {
    pressButton(button, point.x, point.y, isDoubleClick);
}

void EngineController::pressButton(PlatformMouseButton button, bool isDoubleClick) {
    pressButton(button, mouse->position(), isDoubleClick);
}

void EngineController::releaseButton(PlatformMouseButton button, int x, int y) {
    pressOrReleaseButton(EVENT_MOUSE_BUTTON_RELEASE, button, x, y, false);
}

void EngineController::releaseButton(PlatformMouseButton button, Pointi point) {
    releaseButton(button, point.x, point.y);
}

void EngineController::releaseButton(PlatformMouseButton button) {
    releaseButton(button, mouse->position());
}

void EngineController::moveMouse(int x, int y) {
    std::unique_ptr<PlatformMouseEvent> event = std::make_unique<PlatformMouseEvent>();
    event->type = EVENT_MOUSE_MOVE;
    event->window = ::application->window();
    event->button = BUTTON_NONE;
    event->buttons = BUTTON_NONE;
    event->pos = render->MapToPresent({ x, y });
    event->isDoubleClick = false;
    postEvent(std::move(event));
}

void EngineController::moveMouse(Pointi point) {
    moveMouse(point.x, point.y);
}

void EngineController::pressAndReleaseKey(PlatformKey key) {
    pressKey(key);
    releaseKey(key);
}

void EngineController::pressAndReleaseButton(PlatformMouseButton button, int x, int y) {
    pressButton(button, x, y);
    releaseButton(button, x, y);
}

void EngineController::pressAndReleaseButton(PlatformMouseButton button, Pointi point) {
    pressAndReleaseButton(button, point.x, point.y);
}

void EngineController::pressAndReleaseButton(PlatformMouseButton button) {
    pressAndReleaseButton(button, mouse->position());
}

void EngineController::pressGuiButton(std::string_view buttonId, PlatformMouseButton button) {
    pressAndReleaseButton(button, clickPointOf(existingButton(buttonId)));
}

void EngineController::doubleClickGuiButton(std::string_view buttonId) {
    Pointi center = clickPointOf(existingButton(buttonId));
    pressAndReleaseButton(BUTTON_LEFT, center.x, center.y);
    tick(1);
    pressButton(BUTTON_LEFT, center.x, center.y, true);
    releaseButton(BUTTON_LEFT, center.x, center.y);
}

void EngineController::hoverGuiButton(std::string_view buttonId) {
    moveMouse(clickPointOf(existingButton(buttonId)));
}

void EngineController::goToGame() {
    goToGameOrMainMenu();
    if (GetCurrentMenuID() == MENU_MAIN)
        throw Exception("Can't go to game from the main menu");
}

void EngineController::goToInventory(int characterIndex) {
    activateCharacter(characterIndex);
    pressAndReleaseKey(PlatformKey::KEY_I);
    tick(2); // Need two ticks for inventory to be shown.

    if (current_screen_type != SCREEN_CHARACTERS || current_character_screen_window != WINDOW_CHARACTER_INVENTORY)
        throw Exception("Couldn't to go to inventory");
}

void EngineController::goToMainMenu() {
    goToGameOrMainMenu();
    if (GetCurrentMenuID() == MENU_MAIN)
        return;
    assert(GetCurrentMenuID() == MENU_NONE);

    ThrowingTicker ticker(this, "Couldn't return to main menu");

    // Go to in-game menu.
    while (current_screen_type != SCREEN_MENU) {
        pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
        ticker.tick();
    }

    // Leave to main menu from there.
    pressGuiButton("GameMenu_Quit");
    tick(1);
    pressGuiButton("GameMenu_Quit");
    while (GetCurrentMenuID() != MENU_MAIN)
        ticker.tick();
}

void EngineController::exitGame() {
    goToMainMenu();
    pressGuiButton("MainMenu_ExitGame");
}

void EngineController::startNewGame() {
    goToMainMenu();
    pressGuiButton("MainMenu_NewGame");
    tick(2);
    // The map event variables are an engine-global that nothing clears on a new game (a real new game is a
    // fresh process), so one test's event side effects would leak into the next test.
    engine->_persistentVariables = {};
    // ...and the same for the two timers (their totals drive actor/sprite animation phase) and the visual RNG (actor
    // AI draws from it): a test must start from the same clock/RNG regardless of what ran before it in the process.
    for (Timer *timer : {animTimer, gameTimer}) {
        Timer_MM7 snap;
        snapshot(*timer, &snap);
        snap.totalGameTimeElapsed = 0;
        reconstruct(snap, timer);
    }
    pressGuiButton("PartyCreation_OK");
    skipLoadingScreen();
    tick(2);
    vrng->seed(20260926);
}

void EngineController::skipLoadingScreen() {
    ThrowingTicker ticker1(this, "Can't skip a non-existent loading screen");
    while (!pGameLoadingUI_ProgressBar->IsActive())
        ticker1.tick();

    ThrowingTicker ticker2(this, "Couldn't skip a loading screen");
    while (pGameLoadingUI_ProgressBar->IsActive())
        ticker2.tick();
    // The flag only clears while the event timer runs. A map-reload greet that opens a real dialogue (an NPC
    // in MM8's Abandoned Temple) pauses it for the player to answer, so don't wait on it then.
    while ((engineFlags & ENGINE_SKIP_NEXT_USER_INPUT) && !gameTimer->isPaused())
        ticker2.tick();
}

Blob EngineController::saveGame() {
    // AutoSave makes a screenshot and needs the opengl context that's bound in game thread, so we cannot call it from
    // the control thread. One option is to unbind every time we switch to control thread, but this is slow, and not
    // needed 99% of the time. So we just call back into the game thread.
    Blob result;
    runGameRoutine([&] { result = createSaveData(false, "").second; });
    return result;
}

void EngineController::loadGame(const Blob &savedGame, bool paused) {
    MemoryFileSystem ramFs("ramfs");
    ramFs.write("saves/!!!save.mm7", savedGame);

    ScopedRollback<FileSystem *> rollback(&ufs, &ramFs);

    goToMainMenu();
    pressGuiButton("MainMenu_LoadGame");
    tick(4);
    pressGuiButton("LoadMenu_Slot0");
    tick(2);
    engine->_pauseAfterNextWorldLoad = paused;
    pressGuiButton("LoadMenu_Load");
    skipLoadingScreen();

    // Restores the actor list createSaveData stores for MM8 saves, see kMm8LiveActorsKey in SaveLoad.h. Absent in older saves.
    if (gameType() == GAME_TYPE_MM8) {
        auto it = pMapDeltas.find(std::string(kMm8LiveActorsKey));
        if (it != pMapDeltas.end()) {
            std::vector<Actor_MM8> actorsMm8;
            deserialize(it->second, &actorsMm8);
            reconstruct(actorsMm8, &pActors);
            nextActorReuseScanStart = 0;
            for (size_t i = 0; i < pActors.size(); i++)
                pActors[i].id = i;
        }
        if (auto sprites = pMapDeltas.find(std::string(kMm8LiveSpritesKey)); sprites != pMapDeltas.end()) {
            std::vector<SpriteObject_MM7> spritesMm8;
            deserialize(sprites->second, &spritesMm8);
            reconstruct(spritesMm8, &pSpriteObjects);
        }
    }
}

void EngineController::runGameRoutine(GameRoutine routine) {
    _state->gameRoutine = std::move(routine);
    _state.yieldExecution();
    assert(!_state->gameRoutine); // Must have finished.
    if (_state->terminating)
        throw EngineControlState::TerminationException();
}

void EngineController::resizeWindow(int w, int h) {
    runGameRoutine([=] { ::application->window()->resize({w, h});});

    // Spontaneous events are ignored, gotta post one.
    std::unique_ptr<PlatformResizeEvent> event = std::make_unique<PlatformResizeEvent>();
    event->type = EVENT_WINDOW_RESIZE;
    event->window = ::application->window();
    event->size = {w, h};
    postEvent(std::move(event));
}

void EngineController::restAndHeal() {
    ThrowingTicker ticker(this, "Couldn't finish rest & heal");

    goToGame();
    pressGuiButton("Game_Rest");
    tick();
    pressGuiButton("Rest_RestAndHeal");
    while (current_screen_type != SCREEN_GAME)
        ticker.tick();
    tick(); // This is when the characters actually wake up.
}

Actor *EngineController::spawnMonster(Vec3f position, MonsterId id, SpawnFlags flags) {
    Actor *actor = AllocateActor();
    if (!actor)
        throw Exception("Failed to spawn monster {}", static_cast<int>(id));

    actor->attributes |= ACTOR_AGGRESSOR; // Make the monster unconditionally hostile.
    actor->hp = pMonsterStats->infos[id].hp;
    actor->monsterInfo = pMonsterStats->infos[id];
    actor->monsterId = id;
    actor->radius = pMonsterList->monsters[id].monsterRadius;
    actor->height = pMonsterList->monsters[id].monsterHeight;
    actor->moveSpeed = pMonsterList->monsters[id].movementSpeed;
    actor->initialPosition = position;
    actor->pos = actor->initialPosition;
    actor->sectorId = uCurrentlyLoadedLevelType == LEVEL_INDOOR ? pIndoor->GetSector(position) : 0;
    actor->PrepareSprites(0);
    actor->monsterInfo.hostilityType = HOSTILITY_LONG;
    actor->hostilityGroup = hostilityMonsterTypeForMonsterId(actor->monsterInfo.id);
    actor->group = 0;
    actor->currentActionTime = 0_ticks;
    actor->aiState = Standing;
    actor->currentActionLength = 0_ticks;
    actor->UpdateAnimation();

    if (flags & SPAWN_STATIONARY)
        actor->moveSpeed = 1;
    if (flags & SPAWN_FRIENDLY) {
        actor->attributes &= ~ACTOR_AGGRESSOR;
        actor->monsterInfo.hostilityType = HOSTILITY_FRIENDLY;
        actor->hostilityGroup = MONSTER_TYPE_INVALID; // The party's own faction, what summons and resurrects get.
    }
    if (flags & SPAWN_NO_RESISTANCES) {
        actor->monsterInfo.resFire = 0;
        actor->monsterInfo.resAir = 0;
        actor->monsterInfo.resWater = 0;
        actor->monsterInfo.resEarth = 0;
        actor->monsterInfo.resMind = 0;
        actor->monsterInfo.resSpirit = 0;
        actor->monsterInfo.resBody = 0;
        actor->monsterInfo.resLight = 0;
        actor->monsterInfo.resDark = 0;
        actor->monsterInfo.resPhysical = 0;
    }
    if (flags & SPAWN_LEVEL_1)
        actor->monsterInfo.level = 1;

    return actor;
}

void EngineController::teleportTo(MapId map, Vec3f position, int viewYaw, int viewPitch) {
    if (engine->_currentLoadedMapId != map) {
        if (position.lengthSqr() < 1.0f)
            engine->_pendingTransition = MapDestination(map, MAP_START_POINT_PARTY);
        else
            engine->_pendingTransition = MapDestination(map, PartyPlacement(position, viewYaw * 512 / 90, viewPitch * 512 / 90, 0));
        engineFlags |= ENGINE_SKIP_NEXT_WORLD_UPDATE;
        uGameState = GAME_STATE_CHANGE_LOCATION;
        onMapLeave();
        tick();
        skipLoadingScreen();
    } else {
        pParty->pos = position;
        pParty->uFallStartZ = position.z;
        pParty->_viewPitch = viewPitch * 512 / 90;
        pParty->_viewYaw = viewYaw * 512 / 90;
        tick();
    }
}

void EngineController::resolveTargetedSpell(Pid pid, int charIndex) {
    spellTargetPicked(pid, charIndex);
    pGUIWindow_CastTargetedSpell = nullptr;
    mouse->SetCursorImage("MICON1");
    engine->_statusBar->clearEvent();
    IsEnchantingInProgress = false;
    back_to_game();
    tick(2); // let CastSpellInfoHelpers::castSpell() process the now-unblocked queued spell.
}

void EngineController::castSpell(int characterIndex, SpellId spell, Actor *target, ItemId itemTarget, int targetCharacter) {
    assert(characterIndex >= 1 && characterIndex <= std::ssize(pParty->pCharacters));
    activateCharacter(characterIndex - 1);

    if (spell == SPELL_VAMPIRE_LEVITATE) {
        // A racial ability has no spellbook page, it is cast straight from the character.
        pushSpellOrRangedAttack(spell, characterIndex - 1, CombinedSkillValue::none(), 0);
        tick(1);
        return;
    }

    MagicSchool school = magicSchoolForSpell(spell);
    int index = spellIndexInMagicSchool(spell);

    // MM8 has no Game_CastSpell top-bar button - its spellbook opens from the skull
    // bookend (pBtn_Skull), created with the id "Game_Spellbook". Same UIMSG_SpellBookWindow
    // destination either way.
    pressGuiButton(gameType() == GAME_TYPE_MM8 ? "Game_Spellbook" : "Game_CastSpell");
    tick(1);
    // The book opens on the caster's last page, where that school's own tab sits under a spell icon in MM8 and is a no-op anyway.
    if (pParty->activeCharacter().lastOpenedSpellbookPage != school) {
        pressGuiButton(fmt::format("SpellBook_School{}", std::to_underlying(school)));
        tick(1);
    }
    pressGuiButton(fmt::format("SpellBook_Spell{}", index));
    tick(1);
    pressGuiButton(fmt::format("SpellBook_Spell{}", index)); // Confirm.
    tick(1);
    // The confirm click closes the spellbook and defers the actual cast to the NEXT frame
    // (`UIMSG_SelectSpell`'s handler posts `UIMSG_CastSpellFromBook` via `addMessageNextFrame`,
    // Game.cpp) - one more tick so a targeted spell's `pGUIWindow_CastTargetedSpell` is open by
    // the time the check below runs.
    tick(1);

    // A spell that needs a target (e.g. `ON_CAST_TargetedActor` for a damage spell) leaves
    // `pGUIWindow_CastTargetedSpell` open at this point, waiting for a click on an actor in the
    // 3D viewport - `UIMSG_CastSpell_TargetActor`'s handler resolves that via a real mouse
    // raycast (`Engine::PickMouseTarget`), which a headless run can't reproduce
    // deterministically. Resolve it directly against `target` instead - the same
    // `spellTargetPicked`/cleanup sequence that handler runs once its own raycast lands on an
    // actor (`Game::closeTargetedSpellWindow`).
    //
    // `ON_CAST_TargetedCharacter`/
    // `ON_CAST_TargetedActorOrCharacter` spells (e.g. heal, cure) instead wait for a click on a
    // character portrait - `UIMSG_CastSpell_TargetCharacter`'s handler resolves that via
    // `spellTargetPicked(Pid(), uMessageParam)` (`Game.cpp`), an EMPTY pid plus the 0-based
    // character index. Mirror that exact call plus the same cleanup sequence, resolved against
    // `targetCharacter` instead of a raycast pick.
    if (pGUIWindow_CastTargetedSpell && target) {
        resolveTargetedSpell(Pid::actor(target->id), -1);
    } else if (pGUIWindow_CastTargetedSpell && targetCharacter != -1) {
        resolveTargetedSpell(Pid(), targetCharacter);
    }

    // `ON_CAST_TargetedEnchantment` spells (e.g. Enchant Item) open an inventory window and wait for a
    // click on an item (`UICharacter.cpp`'s `IsEnchantingInProgress` branch), which a headless
    // run can't reproduce via a real pixel-precise click either. Resolve it directly against
    // `itemTarget` instead - the exact same field-assignment sequence that click handler runs
    // once a real click lands on an item, minus the purely cosmetic `AfterEnchClick*` auto-close
    // timer (real elapsed seconds, irrelevant to the spell's own effect - clearing `ON_CAST_
    // TargetedEnchantment` from the queued `CastSpellInfo`'s flags is what unblocks `CastSpellInfoHelpers::
    // castSpell()`'s own `ON_CAST_CastingInProgress` guard, the cumulative flag `ON_CAST_TargetedEnchantment`
    // is part of).
    if (IsEnchantingInProgress && itemTarget != ITEM_NULL) {
        InventoryEntry entry = pParty->pCharacters[characterIndex - 1].inventory.find(itemTarget);
        if (!entry)
            throw Exception("Couldn't find item {} to enchant", std::to_underlying(itemTarget));
        CastSpellInfo *pSpellInfo = pGUIWindow_CastTargetedSpell->spellInfo();
        pSpellInfo->flags &= ~ON_CAST_TargetedEnchantment;
        pSpellInfo->targetCharacterIndex = characterIndex - 1;
        pSpellInfo->targetInventoryIndex = entry.index();
        ptr_50C9A4_ItemToEnchant = entry.get();
        IsEnchantingInProgress = false;
        engine->_messageQueue->clear();
        mouse->SetCursorImage("MICON1");
        tick(2); // let CastSpellInfoHelpers::castSpell() process the now-unblocked queued spell.
    }
}

void EngineController::castQuickSpell(int characterIndex, SpellId spell) {
    activateCharacter(characterIndex);
    Character &character = pParty->activeCharacter();
    SpellId oldQuickSpell = character.uQuickSpell;
    character.uQuickSpell = spell;
    pressAndReleaseKey(PlatformKey::KEY_S);
    // UIMSG_CastQuickSpell is processed in the next frame, and we need to wait for it to be processed before we can
    // roll back the quick spell. Thus two ticks.
    tick(2);
    character.uQuickSpell = oldQuickSpell;
}

void EngineController::castQuickSpellAtActor(int characterIndex, SpellId spell, int actorId) {
    if (!IsSpellQuickCastableOnShiftClick(spell))
        throw Exception("Spell #{} can't be cast by shift-click", std::to_underlying(spell));

    activateCharacter(characterIndex);
    Character &character = pParty->activeCharacter();
    SpellId oldQuickSpell = character.uQuickSpell;
    character.uQuickSpell = spell;
    pointMouseAtActor(actorId);
    pressKey(PlatformKey::KEY_SHIFT);
    pressAndReleaseButton(BUTTON_LEFT);
    releaseKey(PlatformKey::KEY_SHIFT);
    tick(2); // The click is a queued event, so the quick spell must stay set until the tick that processes it.
    character.uQuickSpell = oldQuickSpell;
}

void EngineController::pointMouseAtActor(int actorId) {
    pointMouseAt(Pid::actor(actorId));
}

void EngineController::pointMouseAtDecoration(int decorationId) {
    pointMouseAt(Pid::decoration(decorationId));
}

void EngineController::pointMouseAtSprite(int spriteId) {
    pointMouseAt(Pid::sprite(spriteId));
}

void EngineController::pointMouseAtFace(int faceId) {
    pointMouseAt(Pid::face(faceId));
}

void EngineController::activateCharacter(int characterIndex) {
    assert(characterIndex >= 0 && characterIndex < std::ssize(pParty->pCharacters));

    goToGame();
    if (GetCurrentMenuID() != MENU_NONE)
        throw Exception("Can't activate a character from the main menu");

    if (!pParty->hasActiveCharacter() || pParty->activeCharacterIndex() != characterIndex) {
        pressAndReleaseKey(platformKeyForDigit(characterIndex + 1));
        tick(1);
        if (!pParty->hasActiveCharacter() || pParty->activeCharacterIndex() != characterIndex)
            throw Exception("Couldn't activate character #{}", characterIndex);
    }
}

void EngineController::goToGameOrMainMenu() {
    ThrowingTicker ticker(this, "Couldn't return to game");

    // Skip movies.
    while (current_screen_type == SCREEN_VIDEO) {
        pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
        ticker.tick();
    }

    // Can't always leave key settings menu by pressing ESC, so need custom handling.
    if (current_screen_type == SCREEN_KEYBOARD_OPTIONS) {
        pressGuiButton("KeyBinding_Default");
        ticker.tick();
    }

    // Leave to game screen if we're in the game, or to main menu if we're in menus.
    while (current_screen_type != SCREEN_GAME || (GetCurrentMenuID() != MENU_MAIN && GetCurrentMenuID() != MENU_NONE)) {
        pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
        ticker.tick(2); // Somehow tick(1) is not enough when we're trying to leave the game loading menu.
    }

    // If game is starting up - wait for main menu to appear.
    while (GetCurrentMenuID() == MENU_MAIN && lWindowList.empty())
        ticker.tick();
}

void EngineController::pressOrReleaseButton(PlatformEventType type, PlatformMouseButton button, int x, int y,
                                            bool isDoubleClick) {
    assert(type == EVENT_MOUSE_BUTTON_PRESS || type == EVENT_MOUSE_BUTTON_RELEASE);

    std::unique_ptr<PlatformMouseEvent> event = std::make_unique<PlatformMouseEvent>();
    event->type = type;
    event->window = ::application->window();
    event->button = button;
    if (type == EVENT_MOUSE_BUTTON_RELEASE)
        event->buttons = button;
    event->pos = render->MapToPresent({ x, y });
    event->isDoubleClick = isDoubleClick;
    postEvent(std::move(event));
}

GUIButton *EngineController::existingButton(std::string_view buttonId) {
    for (GUIWindow *window : lWindowList)
        for (GUIButton *button : window->vButtons)
            if (button->id == buttonId)
                return button;
    throw Exception("GUI button '{}' not found", buttonId);
}

// The first button containing `point`, in the order the windows and their buttons are listed.
static GUIButton *buttonReachedAt(Pointi point) {
    for (GUIWindow *window : lWindowList)
        for (GUIButton *button : window->vButtons)
            if (button->Contains(point))
                return button;
    return nullptr;
}

Pointi EngineController::clickPointOf(GUIButton *button) {
    Pointi center = button->rect.center();
    if (buttonReachedAt(center) == button)
        return center;

    for (int y = button->rect.y; y < button->rect.y + button->rect.h; y++)
        for (int x = button->rect.x; x < button->rect.x + button->rect.w; x++)
            if (buttonReachedAt({x, y}) == button)
                return {x, y};

    throw Exception("Couldn't press GUI button '{}': every point of it is covered by another GUI button", button->id);
}

void EngineController::pointMouseAt(Pid target) {
    // Camera matrices are updated when a frame is rendered, so if the party was teleported without ticking, the
    // camera is still at the old position. Tick once to let it catch up.
    tick(1);

    Vec3f center;
    std::string targetName;
    if (target.type() == OBJECT_Actor) {
        center = pActors[target.id()].pos + Vec3f(0, 0, pActors[target.id()].height / 2);
        targetName = fmt::format("actor #{}", target.id());
    } else if (target.type() == OBJECT_Decoration) {
        const LevelDecoration &decoration = pLevelDecorations[target.id()];
        center = decoration.vPosition + Vec3f(0, 0, pDecorationTable->decoration(decoration.uDecorationDescID)->uDecorationHeight / 2);
        targetName = fmt::format("decoration #{}", target.id());
    } else if (target.type() == OBJECT_Sprite) {
        const SpriteObject &sprite = pSpriteObjects[target.id()];
        center = sprite.vPosition + Vec3f(0, 0, pObjectTable->pObjects[sprite.uObjectDescID].uHeight / 2);
        targetName = fmt::format("sprite #{}", target.id());
    } else if (target.type() == OBJECT_Face) {
        center = (uCurrentlyLoadedLevelType == LEVEL_INDOOR ? pIndoor->faces[target.id()] : pOutdoor->face(target)).boundingBox.center();
        targetName = fmt::format("face #{}", target.id());
    } else {
        throw Exception("Can't point mouse at an object of type {}", std::to_underlying(target.type()));
    }

    Vec3f viewPos = pCamera3D->ViewTransform(&center);
    if (viewPos.x <= 0)
        throw Exception("Can't point mouse at {}, it's behind the camera", targetName);
    Pointi screenPos = pCamera3D->Project(viewPos).toInt();

    // Sprites can be transparent in places, and a pick there goes through to whatever is behind. Points outside the
    // viewport button are skipped, because a pick still works there but a click doesn't reach the game.
    auto viewport = std::ranges::find(pPrimaryWindow->vButtons, "Game_Viewport", &GUIButton::id);
    if (viewport == pPrimaryWindow->vButtons.end())
        throw Exception("GUI button 'Game_Viewport' not found");
    auto pickAt = [](Pointi point) {
        return engine->PickMouse(engine->config->gameplay.RangedAttackDepth.value(), point.x, point.y, &vis_anything_filter, &vis_face_filter).pid;
    };
    std::vector<Pointi> points = {screenPos};
    for (int distance = 5; distance <= 50; distance += 5)
        points.insert(points.end(), {screenPos - Pointi(0, distance), screenPos + Pointi(0, distance), screenPos - Pointi(distance, 0), screenPos + Pointi(distance, 0)});
    auto point = std::ranges::find_if(points, [&](Pointi point) { return (*viewport)->Contains(point) && pickAt(point) == target; });
    if (point == points.end())
        throw Exception("Failed to point mouse at {}", targetName);

    moveMouse(*point);
    tick(1); // The mouse move is a queued event, the pick sees the new position only once it's processed.
    if (pickAt(mouse->position()) != target)
        throw Exception("Failed to point mouse at {}", targetName);
}
