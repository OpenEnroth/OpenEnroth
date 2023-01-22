#include "EventTrace.h"

#include <fstream>
#include <memory>
#include <string>

#include "Library/Serialization/EnumSerialization.h"
#include "Library/Json/Json.h"

#include "Io/Key.h" // TODO(captainurist): doesn't belong here

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PlatformEvent::Type, CASE_SENSITIVE, {
    {PlatformEvent::KeyPress,                   "keyPress"},
    {PlatformEvent::KeyRelease,                 "keyRelease"},
    {PlatformEvent::GamepadConnected,           "gamepadConnected"},
    {PlatformEvent::GamepadDisconnected,        "gamepadDisconnected"},
    {PlatformEvent::MouseButtonPress,           "mouseButtonPress"},
    {PlatformEvent::MouseButtonRelease,         "mouseButtonRelease"},
    {PlatformEvent::MouseMove,                  "mouseMove"},
    {PlatformEvent::MouseWheel,                 "mouseWheel"},
    {PlatformEvent::WindowMove,                 "windowMove"},
    {PlatformEvent::WindowResize,               "windowResize"},
    {PlatformEvent::WindowActivate,             "windowActivate"},
    {PlatformEvent::WindowDeactivate,           "windowDeactivate"},
    {PlatformEvent::WindowCloseRequest,         "windowCloseRequest"},
    {EventTrace::PaintEvent,                    "paint"}
})
MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(PlatformEvent::Type)

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PlatformKeyType, CASE_SENSITIVE, {
    {KEY_TYPE_KEYBOARD_BUTTON, "keyboard"},
    {KEY_TYPE_GAMEPAD_BUTTON, "gamepadButton"},
    {KEY_TYPE_GAMEPAD_AXIS, "gamepadAxis"},
    {KEY_TYPE_GAMEPAD_TRIGGER, "gamepadTrigger"}
})
MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(PlatformKeyType)

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PlatformMouseButton, CASE_SENSITIVE, {
    {PlatformMouseButton::None, "none"},
    {PlatformMouseButton::Left, "left"},
    {PlatformMouseButton::Middle, "middle"},
    {PlatformMouseButton::Right, "right"}
})
MM_DEFINE_FLAGS_SERIALIZATION_FUNCTIONS(PlatformMouseButtons)
MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(PlatformMouseButton)
MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(PlatformMouseButtons)

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PlatformModifier, CASE_SENSITIVE, {
    {PlatformModifier::Shift, "shift"},
    {PlatformModifier::Ctrl, "ctrl"},
    {PlatformModifier::Alt, "alt"},
    {PlatformModifier::Meta, "meta"},
    {PlatformModifier::NumPad, "num"},
})
MM_DEFINE_FLAGS_SERIALIZATION_FUNCTIONS(PlatformModifiers)
MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(PlatformModifiers)

MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(PlatformKey)

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(Pointi, (
    (x, "x"),
    (y, "y")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(Sizei, (
    (w, "w"),
    (h, "h")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PlatformEvent, (
    (type, "type")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PlatformKeyEvent, (
    (type, "type"),
    (id, "id"),
    (key, "key"),
    (keyType, "keyType"),
    (keyValue, "keyValue"),
    (mods, "mods"),
    (isAutoRepeat, "isAutoRepeat")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PlatformMouseEvent, (
    (type, "type"),
    (button, "button"),
    (buttons, "buttons"),
    (pos, "pos"),
    (isDoubleClick, "isDoubleClick")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PlatformWheelEvent, (
    (type, "type"),
    (angleDelta, "angleDelta"),
    (inverted, "inverted")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PlatformMoveEvent, (
    (type, "type"),
    (pos, "pos")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PlatformResizeEvent, (
    (type, "type"),
    (size, "size")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PlatformGamepadDeviceEvent, (
    (type, "type"),
    (id, "id")
))

template<class Callable>
inline void DispatchByEventType(PlatformEvent::Type type, Callable &&callable) {
    using PlatformEventType = PlatformEvent::Type; // TODO(captainurist): workaround for gcc-12 ice, drop once we have gcc-13.

    switch (type) {
    case PlatformEventType::KeyPress:
    case PlatformEventType::KeyRelease:
        callable(static_cast<PlatformKeyEvent *>(nullptr));
        break;
    case PlatformEventType::GamepadConnected:
    case PlatformEventType::GamepadDisconnected:
        callable(static_cast<PlatformGamepadDeviceEvent *>(nullptr));
        break;
    case PlatformEventType::MouseButtonPress:
    case PlatformEventType::MouseButtonRelease:
    case PlatformEventType::MouseMove:
        callable(static_cast<PlatformMouseEvent *>(nullptr));
        break;
    case PlatformEventType::MouseWheel:
        callable(static_cast<PlatformWheelEvent *>(nullptr));
        break;
    case PlatformEventType::WindowMove:
        callable(static_cast<PlatformMoveEvent *>(nullptr));
        break;
    case PlatformEventType::WindowResize:
        callable(static_cast<PlatformResizeEvent *>(nullptr));
        break;
    case PlatformEventType::WindowActivate:
    case PlatformEventType::WindowDeactivate:
    case PlatformEventType::WindowCloseRequest:
        callable(static_cast<PlatformWindowEvent *>(nullptr));
    case EventTrace::PaintEvent:
        callable(static_cast<PlatformEvent *>(nullptr)); // TODO(captainurist): UserEvent.
        break;
    default:
        return;
    }
}

static void to_json(Json &json, const std::unique_ptr<PlatformEvent> &value) {
    if (!value) {
        json = nullptr;
        return;
    }

    DispatchByEventType(value->type, [&]<class T>(T *) {
        to_json(json, *static_cast<T *>(value.get()));
    });
}

static void from_json(const Json &json, std::unique_ptr<PlatformEvent> &value) {
    if (json == nullptr) {
        value.reset();
        return;
    }

    PlatformEvent event;
    from_json(json, event);

    DispatchByEventType(event.type, [&]<class T>(T *) {
        value.reset(new T());
        from_json(json, *static_cast<T *>(value.get()));
    });
}

void EventTrace::saveToFile(std::string_view path) const {
    std::ofstream f;
    f.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    f.open(std::string(path));

    // TODO(captainurist): well, nlohmann json is retarded in that it chokes if we throw exceptions inside
    // to_json calls for individual elements. Fix upstream?
    // Note: there is an example in tests to reproduce.
    Json json;
    for (size_t i = 0; i < _events.size(); i++)
        to_json(json[i], _events[i]);

    f << std::setw(4) << json;
}

EventTrace EventTrace::loadFromFile(std::string_view path, PlatformWindow *window) {
    std::ifstream f;
    f.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    f.open(std::string(path));

    Json json;
    f >> json;

    EventTrace result;
    from_json(json, result._events);

    for (std::unique_ptr<PlatformEvent> &event : result._events) {
        DispatchByEventType(event->type, [&]<class T>(T *) {
            if constexpr (std::is_base_of_v<PlatformWindowEvent, T>) {
                static_cast<PlatformWindowEvent *>(event.get())->window = window;
            }
        });
    }

    return result;
}

void EventTrace::recordEvent(const PlatformEvent *event) {
    assert(event->type != PaintEvent);

    DispatchByEventType(event->type, [&]<class T>(T *) {
        _events.emplace_back(std::make_unique<T>(*static_cast<const T *>(event)));
    });
}

void EventTrace::recordRepaint() {
    PlatformEvent tmp;
    tmp.type = PaintEvent;

    _events.emplace_back(std::make_unique<PlatformEvent>(tmp));
}

void EventTrace::clear() {
    _events.clear();
}
