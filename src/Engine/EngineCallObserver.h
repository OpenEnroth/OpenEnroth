#pragma once

#include <typeinfo>

enum class EngineCall {
    CALL_PLAY_SOUND,
    CALL_DRAW_2D_TEXTURE,
    CALL_DRAW_MESSAGE_BOX,
    CALL_GUIWINDOW_DRAWTEXT,

    CALL_FIRST = CALL_PLAY_SOUND,
    CALL_LAST = CALL_GUIWINDOW_DRAWTEXT
};
using enum EngineCall;

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
