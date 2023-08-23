#pragma once

#include <cstdint>
#include <utility>
#include <string>
#include <type_traits>

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Time.h"
#include "GUI/GUIEnums.h"
#include "GUI/GUIDialogues.h"

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
    template<class Callback, class T = std::invoke_result_t<Callback>>
    TestTape<T> custom(Callback callback) {
        return _controller->recordTape(std::move(callback));
    }

    TestTape<int64_t> totalExperience();

    TestTape<int> totalHp();

    TestTape<int> totalItemCount();

    TestTape<bool> hasItem(ItemId item);

    TestTape<int> gold();

    TestTape<int> food();

    TestTape<std::string> map();

    TestTape<ScreenType> screen();

    TestTape<std::string> statusBar();

    TestTape<DIALOGUE_TYPE> dialogueType();

    TestTape<GameTime> time();

    template<class T>
    TestTape<T> config(const ConfigEntry<T> &entry) {
        return custom([&] { return entry.value(); });
    }

 private:
    TestController *_controller = nullptr;
};
