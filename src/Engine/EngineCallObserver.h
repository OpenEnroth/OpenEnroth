#pragma once

#include <typeinfo>

enum class EngineCall {
    CALL_PLAY_SOUND,
    CALL_DRAW_2D_TEXTURE,
    CALL_DRAW_MESSAGE_BOX,
    CALL_GUIWINDOW_DRAWTEXT,
    CALL_SPECIAL_ATTACK,

    CALL_FIRST = CALL_PLAY_SOUND,
    CALL_LAST = CALL_SPECIAL_ATTACK
};
using enum EngineCall;

/**
 * An interface that's used to implement poor man's gmock for recording calls into different engine functions.
 * Currently it's used by the testing framework to check that the calls that the testing framework expects to be made
 * are actually made.
 *
 * When adding new calls to `EngineCall`, please make sure you don't add any complex logic around them. Otherwise we'll
 * end up with test code spilling over into the engine, and we generally want to avoid that. See how existing calls
 * are handled for examples.
 *
 * @see TestCallObserver
 * @see CommonTapeRecorder::sounds
 */
class EngineCallObserver {
 public:
    virtual ~EngineCallObserver() = default;

    template<class T>
    void notify(EngineCall call, const T &data) {
        notify(call, typeid(T), &data);
    }

 protected:
    virtual void notify(EngineCall call, const std::type_info &type, const void *data) = 0;
};
