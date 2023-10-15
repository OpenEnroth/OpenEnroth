#pragma once

#include <cstdint>
#include <utility>
#include <string>
#include <type_traits>

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/ActorEnums.h"
#include "Engine/Time.h"
#include "GUI/GUIEnums.h"
#include "GUI/GUIDialogues.h"
#include "GUI/UI/UIHouseEnums.h"

#include "Library/Config/ConfigEntry.h"

#include "TestController.h"

class CommonTapeRecorder {
 public:
    explicit CommonTapeRecorder(TestController *controller);

    /**
     * Creates a tape object that will record changes in a value computed by the provided callback throughout the
     * execution of a trace.
     *
     * The callback is called on every frame, and also before and after running the trace, and unique values are stored
     * on a tape. This effectively means that if, for example, the provided callback always returns the same value,
     * then you'll get a tape of size 1.
     *
     * A typical use case is to create a tape, call `TestController::playTraceFromTestData` to play a trace, then check
     * the data stored on the tape.
     *
     * @param callback                  Callback that will calculate the values to store on a tape.
     * @return                          Tape object.
     */
    template<class Callback, class T = std::decay_t<std::invoke_result_t<Callback>>>
    TestTape<T> custom(Callback callback) {
        return _controller->recordTape(std::move(callback));
    }

    TestTape<int64_t> totalExperience();

    TestTape<int> totalHp();

    TestTape<int> totalItemCount();

    TestTape<bool> hasItem(ItemId itemId);

    TestTape<int> gold();

    TestTape<int> food();

    TestTape<int> deaths();

    TestTape<std::string> map();

    TestTape<ScreenType> screen();

    TestTape<std::string> statusBar();

    TestTape<DIALOGUE_TYPE> dialogueType();

    TestTape<GameTime> time();

    TestTape<bool> turnBasedMode();

    template<class T>
    TestTape<T> config(const ConfigEntry<T> &entry) {
        return custom([&] { return entry.value(); });
    }

    TestTape<int> mapItemCount();

    TestTape<int> mapItemCount(ItemId itemId);

    TestTape<HouseId> house();

 private:
    TestController *_controller = nullptr;
};
