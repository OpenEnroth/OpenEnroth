#pragma once

#include <string>
#include <memory>

#include "Engine/Spells/SpellEnums.h"
#include "Engine/Objects/MonsterEnums.h"
#include "Engine/MapEnums.h"

#include "Library/Platform/Interface/PlatformEnums.h"
#include "Library/Platform/Interface/PlatformEvents.h"

#include "Utility/Flags.h"
#include "Utility/Memory/Blob.h"

#include "EngineControlStateHandle.h"

class GUIButton;
class PlatformEvent;
class Actor;

enum class SpawnFlag {
    SPAWN_STATIONARY = 0x1, // Set moveSpeed to 1 so that the monster stays in place.
    SPAWN_NO_RESISTANCES = 0x2, // Zero out all resistances. Note that you might also want to set `SPAWN_LEVEL_1`.
    SPAWN_LEVEL_1 = 0x4, // Set level to 1. Level is used in to-hit, resistance and special attack rolls.
    SPAWN_FRIENDLY = 0x8, // Make the monster friendly to the party instead of unconditionally hostile.

    // A predictable stationary target for damage-related tests.
    SPAWN_DUMMY = SPAWN_STATIONARY | SPAWN_NO_RESISTANCES | SPAWN_LEVEL_1,
};
using enum SpawnFlag;
MM_DECLARE_FLAGS(SpawnFlags, SpawnFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(SpawnFlags)

/**
 * This is the interface to be used from a control routine to control the game thread.
 *
 * Most errors are reported as exceptions (derived from `std::runtime_error`) because the main way to use this class
 * is from unit tests.
 */
class EngineController {
 public:
    using GameRoutine = std::function<void()>;

    explicit EngineController(EngineControlStateHandle state);
    ~EngineController();

    /**
     * Passes execution to the game thread for the provided number of frames. This function provides a coroutine-like
     * interface for control routines as calling `tick` basically suspends the control routine until the next frame.
     *
     * @param count                     Number of frames to suspend the control routine for.
     */
    void tick(int count = 1);

    void postEvent(std::unique_ptr<PlatformEvent> event);
    void pressKey(PlatformKey key);
    void pressAutoRepeatedKey(PlatformKey key);
    void releaseKey(PlatformKey key);
    void pressButton(PlatformMouseButton button, int x, int y, bool isDoubleClick = false);
    void releaseButton(PlatformMouseButton button, int x, int y);
    void moveMouse(int x, int y);

    void pressAndReleaseKey(PlatformKey key);
    void pressAndReleaseButton(PlatformMouseButton button, int x, int y);

    /**
     * Presses a GUI button identified by the provided id by sending a mouse press and release event.
     *
     * @param buttonId                  Button id.
     * @throws Exception                If the button with the provided id doesn't exist.
     */
    void pressGuiButton(std::string_view buttonId);

    /**
     * Clicks a GUI button twice, the second click carrying the double click flag the platform would set.
     *
     * @param buttonId                  Id of the button to click.
     * @throws Exception                If the button with the provided id doesn't exist.
     */
    void doubleClickGuiButton(std::string_view buttonId);

    /**
     * Closes all menus and goes to the game screen. Will fail if main menu is currently open.
     */
    void goToGame();

    void goToInventory(int characterIndex);

    /**
     * Opens main menu no matter the current game state.
     */
    void goToMainMenu();

    /**
     * Start new game no matter the current game state.
     */
    void startNewGame();

    /**
     * Waits for the loading screen to complete.
     *
     * @throws Exception                If there is no loading screen.
     */
    void skipLoadingScreen();

    /**
     * Saves the game.
     *
     * @returns                         `Blob` containing saved game data.
     */
    Blob saveGame();

    /**
     * Loads the game by opening up the load game menu and actually clicking all the buttons.
     *
     * @param savedGame                 `Blob` containing saved game data.
     */
    void loadGame(const Blob &savedGame);

    /**
     * Runs the provided routine in game thread and returns once it's finished. This is mainly for running OpenGL code
     * as the corresponding context is bound in the main thread.
     *
     * @param routine                   Routine to run.
     */
    void runGameRoutine(GameRoutine routine);

    void resizeWindow(int w, int h);

    /**
     * Opens up the rest menu, presses the "rest and heal for 8 hours" button, and waits out the rest period.
     */
    void restAndHeal();

    /**
     * @param position                  Position to spawn a monster at.
     * @param id                        Id of the monster to spawn.
     * @param flags                     Post-spawn tweaks to apply.
     * @return                          Spawned monster, so you can tweak it after spawning.
     */
    Actor *spawnMonster(Vec3f position, MonsterId id, SpawnFlags flags = 0);

    void teleportTo(MapId map, Vec3f position, int viewYaw);

    void castSpell(int characterIndex, SpellId spell);

    /**
     * Casts a spell through the quick spell mechanism. Unlike `castSpell`, quick spells don't open the targeting
     * interface, and just auto-target the closest actor.
     *
     * @param characterIndex            1-based index of the casting character.
     * @param spell                     Spell to cast.
     */
    void castQuickSpell(int characterIndex, SpellId spell);

    /**
     * Finds a screen position at which the mouse points at the provided actor & moves the mouse there.
     *
     * @param actorId                   Id of the actor to point at.
     * @throws Exception                If pointing at the actor is not possible, e.g. it's not on the screen.
     */
    void pointMouseAtActor(int actorId);

    /**
     * Finds a screen position at which the mouse points at the provided decoration & moves the mouse there.
     *
     * @param decorationId              Id of the decoration to point at.
     * @throws Exception                If pointing at the decoration is not possible, e.g. it's not on the screen.
     */
    void pointMouseAtDecoration(int decorationId);

 private:
    void goToGameOrMainMenu();

    void pressOrReleaseButton(PlatformEventType type, PlatformMouseButton button, int x, int y, bool isDoubleClick);

    GUIButton *existingButton(std::string_view buttonId);

 private:
    EngineControlStateHandle _state;
};
