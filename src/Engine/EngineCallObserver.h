#pragma once

#include <typeinfo>

enum class EngineCall {
    CALL_PLAY_SOUND,
    CALL_DRAW_2D_TEXTURE,

    CALL_FIRST = CALL_PLAY_SOUND,
    CALL_LAST = CALL_DRAW_2D_TEXTURE
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
