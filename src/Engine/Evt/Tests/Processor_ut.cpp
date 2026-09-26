#include "Testing/Game/GameTest.h"

#include "Core/Time/Time.h"

#include "Engine/Evt/Processor.h"
#include "Engine/Party.h"

#include "GUI/GUIWindow.h"

#include "Utility/ScopeGuard.h"

static void tickFor(EngineController &game, Duration duration) {
    Time end = pParty->GetPlayingTime() + duration;
    while (pParty->GetPlayingTime() < end)
        game.tick();
}

GAME_TEST(Processor, GameTimerThatLeavesTheMap) {
    // A game timer whose callback left the map fired again on the next pass, the leave skipped its next alarm.
    game.startNewGame();
    int fires = 0;
    int handle = addTimer({.interval = Duration::fromMinutes(5)}, EVT_TIMER_KIND_REGULAR, EVT_TIMER_LIFETIME_GAME, [&] {
        fires++;
        onMapLeave();
    });
    MM_AT_SCOPE_EXIT(removeTimer(handle));
    tickFor(game, Duration::fromMinutes(7));
    EXPECT_EQ(fires, 1);
}

GAME_TEST(Processor, TimerCountsFromNow) {
    // A daily timer added while the map ran fired at once on a first visit, as if the map had just loaded.
    game.startNewGame();
    Time now = pParty->GetPlayingTime();
    Duration timeOfDay = now - Time::fromDays(now.toDays()) - Duration::fromMinutes(1);
    int fromNowFires = 0;
    int fromLastVisitFires = 0;
    int fromNow = addTimer({.period = Duration::fromDays(1), .timeOfDay = timeOfDay, .start = EVT_TIMER_START_NOW}, EVT_TIMER_KIND_REGULAR,
                           EVT_TIMER_LIFETIME_MAP, [&] { fromNowFires++; });
    int fromLastVisit = addTimer({.period = Duration::fromDays(1), .timeOfDay = timeOfDay}, EVT_TIMER_KIND_REGULAR, EVT_TIMER_LIFETIME_MAP,
                                 [&] { fromLastVisitFires++; });
    MM_AT_SCOPE_EXIT(removeTimer(fromNow));
    MM_AT_SCOPE_EXIT(removeTimer(fromLastVisit));
    tickFor(game, Duration::fromMinutes(2));
    EXPECT_EQ(fromNowFires, 0);
    EXPECT_EQ(fromLastVisitFires, 1);
}

GAME_TEST(Processor, EscapeDropsTheContinuation) {
    // Escape on a dungeon entrance's dialogue kept the continuation of the event that opened it.
    game.startNewGame();
    eventProcessor(101, Pid(), true); // A dungeon entrance on Emerald Island.
    game.tick();
    EXPECT_TRUE(hasEventContinuation());
    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick();
    EXPECT_EQ(current_screen_type, SCREEN_GAME);
    EXPECT_FALSE(hasEventContinuation());
}
