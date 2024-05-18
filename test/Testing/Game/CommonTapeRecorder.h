#pragma once

#include <cstdint>
#include <utility>
#include <string>
#include <type_traits>

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/SpriteEnums.h"
#include "Engine/Time/Time.h"
#include "Engine/MapEnums.h"
#include "Media/Audio/SoundEnums.h"
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

    TestTape<int> totalHirelings();

    TestTape<bool> hasItem(ItemId itemId);

    TestTape<int> gold();

    TestTape<int> food();

    TestTape<int> deaths();

    TestTape<MapId> map();

    TestTape<ScreenType> screen();

    TestTape<std::string> statusBar();

    TestTape<DialogueId> dialogueType();

    TestTape<Time> time();

    TestTape<bool> turnBasedMode();

    template<class T>
    TestTape<T> config(const ConfigEntry<T> &entry) {
        return custom([&] { return entry.value(); });
    }

    TestTape<int> mapItemCount();

    TestTape<int> mapItemCount(ItemId itemId);

    TestTape<HouseId> house();

    TestMultiTape<SpriteId> sprites();

    TestTape<int> activeCharacterIndex(); // Remember that 0 means none!

    TestMultiTape<SoundId> sounds();

    TestMultiTape<std::string> hudTextures();

    /**
     * Return a tape object of strings listing all the hints whenever a pop up message box was called.
     *
     * @return                          Tape object.
     */
    TestMultiTape<std::string> messageBoxes();
    /**
     * Return a tape object of strings listing all the text drawn by GUIWindow::DrawText.
     * TODO(pskelton): Tape will be spammy as it is recording everything.
     * 
     * @return                          Tape object.
     */
    TestMultiTape<std::string> allGUIWindowsText();

 private:
    TestController *_controller = nullptr;
};
