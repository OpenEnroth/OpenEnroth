#include <optional>
#include <string>
#include <string_view>

#include "Testing/Game/GameTest.h"

#include "Core/Time/Time.h"

#include "Engine/Engine.h"
#include "Engine/Evt/EvtScripts.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Party.h"

#include "Utility/ScopeGuard.h"

static const int TEST_TIMER = 1000000; // Past the handles of the scripts that run in the decompiled events mode.

/**
 * Counts how many times the test's timer fires, and starts a map transition from it if told to.
 */
class TimerScripts : public EvtScripts {
 public:
    TimerScripts() : _oldScripts(engine->_evtScripts) {
        engine->_evtScripts = this;
    }

    virtual ~TimerScripts() {
        engine->_evtScripts = _oldScripts;
    }

    virtual void loadGlobalScripts() override {}
    virtual void loadMapScripts(std::string_view) override {}
    virtual bool hasEvent(EvtSource, int) const override { return false; }
    virtual EvtTransition runEvent(const EvtContext &) override { return EVT_TRANSITION_NONE; }
    virtual EvtTransition resumeEvent(int) override { return EVT_TRANSITION_NONE; }
    virtual std::optional<std::string> eventHint(int) const override { return std::nullopt; }
    virtual std::optional<bool> canShowTopic(int) override { return std::nullopt; }
    virtual EvtTransition onMapLoad() override { return EVT_TRANSITION_NONE; }
    virtual void onMapLeave() override {}

    virtual EvtTransition fireTimer(int handle) override {
        if (handle != TEST_TIMER)
            return EVT_TRANSITION_NONE;
        fires++;
        return transition;
    }

    int fires = 0;
    EvtTransition transition = EVT_TRANSITION_NONE;

 private:
    EvtScripts *_oldScripts = nullptr;
};

static void tickFor(EngineController &game, Duration duration) {
    Time end = pParty->GetPlayingTime() + duration;
    while (pParty->GetPlayingTime() < end)
        game.tick();
}

GAME_TEST(Processor, GameTimerThatLeavesTheMap) {
    // A game timer whose callback left the map fired again on the next pass, the leave skipped its next alarm.
    game.startNewGame();
    TimerScripts scripts;
    scripts.transition = EVT_TRANSITION_STARTED;
    addTimer(TEST_TIMER, {.type = EVT_TIMER_INTERVAL, .duration = Duration::fromMinutes(5)}, EVT_TIMER_START_NOW, EVT_TIMER_KIND_REGULAR,
             EVT_TIMER_LIFETIME_GAME);
    MM_AT_SCOPE_EXIT(removeTimer(TEST_TIMER));
    tickFor(game, Duration::fromMinutes(7));
    EXPECT_EQ(scripts.fires, 1);
}

GAME_TEST(Processor, TimerCountsFromNow) {
    // A daily timer added while the map ran fired at once on a first visit, as if the map had just loaded.
    game.startNewGame();
    TimerScripts scripts;
    Time now = pParty->GetPlayingTime();
    Duration timeOfDay = now - Time::fromDays(now.toDays()) - Duration::fromMinutes(1);
    addTimer(TEST_TIMER, {.type = EVT_TIMER_DAILY, .duration = timeOfDay}, EVT_TIMER_START_NOW, EVT_TIMER_KIND_REGULAR, EVT_TIMER_LIFETIME_MAP);
    MM_AT_SCOPE_EXIT(removeTimer(TEST_TIMER));
    tickFor(game, Duration::fromMinutes(2));
    EXPECT_EQ(scripts.fires, 0);
}
