#include "EventTrace.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Library/Serialization/EnumSerialization.h"
#include "Library/Json/Json.h"

#include "Io/Key.h" // TODO(captainurist): doesn't belong here

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"

#include "PaintEvent.h"

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(Pointi, (
    (x, "x"),
    (y, "y")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(Sizei, (
    (w, "w"),
    (h, "h")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(Vec3i, (
    (x, "x"),
    (y, "y"),
    (z, "z")
))

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PlatformEventType, CASE_SENSITIVE, {
    {EVENT_KEY_PRESS,               "keyPress"},
    {EVENT_KEY_RELEASE,             "keyRelease"},
    {EVENT_GAMEPAD_CONNECTED,       "gamepadConnected"},
    {EVENT_GAMEPAD_DISCONNECTED,    "gamepadDisconnected"},
    {EVENT_MOUSE_BUTTON_PRESS,      "mouseButtonPress"},
    {EVENT_MOUSE_BUTTON_RELEASE,    "mouseButtonRelease"},
    {EVENT_MOUSE_MOVE,              "mouseMove"},
    {EVENT_MOUSE_WHEEL,             "mouseWheel"},
    {EVENT_WINDOW_MOVE,             "windowMove"},
    {EVENT_WINDOW_RESIZE,           "windowResize"},
    {EVENT_WINDOW_ACTIVATE,         "windowActivate"},
    {EVENT_WINDOW_DEACTIVATE,       "windowDeactivate"},
    {EVENT_WINDOW_CLOSE_REQUEST,    "windowCloseRequest"},
    {EVENT_PAINT,                   "paint"}
})
MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(PlatformEventType)

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PlatformMouseButton, CASE_SENSITIVE, {
    {BUTTON_NONE, "none"},
    {BUTTON_LEFT, "left"},
    {BUTTON_MIDDLE, "middle"},
    {BUTTON_RIGHT, "right"}
})
MM_DEFINE_FLAGS_SERIALIZATION_FUNCTIONS(PlatformMouseButtons)
MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(PlatformMouseButton)
MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(PlatformMouseButtons)

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PlatformModifier, CASE_SENSITIVE, {
    {MOD_SHIFT, "shift"},
    {MOD_CTRL, "ctrl"},
    {MOD_ALT, "alt"},
    {MOD_META, "meta"},
    {MOD_NUM, "num"},
})
MM_DEFINE_FLAGS_SERIALIZATION_FUNCTIONS(PlatformModifiers)
MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(PlatformModifiers)

MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(PlatformKey)

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PlatformEvent, (
    (type, "type")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PlatformKeyEvent, (
    (type, "type"),
    (key, "key"),
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
    (angleDelta, "angleDelta")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PlatformMoveEvent, (
    (type, "type"),
    (pos, "pos")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PlatformResizeEvent, (
    (type, "type"),
    (size, "size")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(PaintEvent, (
    (type, "type"),
    (tickCount, "tickCount"),
    (randomState, "randomState")
))

template<class Callable>
inline void dispatchByEventType(PlatformEventType type, Callable &&callable) {
    switch (type) {
    case EVENT_KEY_PRESS:
    case EVENT_KEY_RELEASE:
        callable(static_cast<PlatformKeyEvent *>(nullptr));
        break;
    case EVENT_MOUSE_BUTTON_PRESS:
    case EVENT_MOUSE_BUTTON_RELEASE:
    case EVENT_MOUSE_MOVE:
        callable(static_cast<PlatformMouseEvent *>(nullptr));
        break;
    case EVENT_MOUSE_WHEEL:
        callable(static_cast<PlatformWheelEvent *>(nullptr));
        break;
    case EVENT_WINDOW_MOVE:
        callable(static_cast<PlatformMoveEvent *>(nullptr));
        break;
    case EVENT_WINDOW_RESIZE:
        callable(static_cast<PlatformResizeEvent *>(nullptr));
        break;
    case EVENT_WINDOW_ACTIVATE:
    case EVENT_WINDOW_DEACTIVATE:
    case EVENT_WINDOW_CLOSE_REQUEST:
        callable(static_cast<PlatformWindowEvent *>(nullptr));
        break;
    case EVENT_PAINT:
        callable(static_cast<PaintEvent *>(nullptr));
        break;
    default:
        return; // No gamepad events.
    }
}

static void to_json(Json &json, const std::unique_ptr<PlatformEvent> &value) {
    if (!value) {
        json = nullptr;
        return;
    }

    dispatchByEventType(value->type, [&]<class T>(T *) {
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

    dispatchByEventType(event.type, [&]<class T>(T *) {
        value.reset(new T());
        from_json(json, *static_cast<T *>(value.get()));
    });
}

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(ConfigPatchEntry, (
    (section, "section"),
    (key, "key"),
    (value, "value")
))

static void to_json(Json &json, const ConfigPatch &patch) {
    to_json(json, patch.entries());
}

static void from_json(const Json &json, ConfigPatch &patch) {
    std::vector<ConfigPatchEntry> entries;
    from_json(json, entries);
    patch = ConfigPatch::fromEntries(std::move(entries));
}

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(EventTraceCharacterState, (
    (hp, "hp"),
    (mp, "mp"),
    (might, "might"),
    (intelligence, "intelligence"),
    (personality, "personality"),
    (endurance, "endurance"),
    (accuracy, "accuracy"),
    (speed, "speed"),
    (luck, "luck")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(EventTraceGameState, (
    (locationName, "locationName"),
    (partyPosition, "partyPosition"),
    (characters, "characters")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(EventTraceHeader, (
    (saveFileSize, "saveFileSize"),
    (config, "config"),
    (startState, "startState"),
    (endState, "endState"),
    (afterLoadRandomState, "afterLoadRandomState")
))

MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(EventTrace, (
    (header, "header"),
    (events, "trace")
))

void EventTrace::saveToFile(std::string_view path, const EventTrace &trace) {
    FileOutputStream output(path);

    // TODO(captainurist): well, nlohmann json is retarded in that it chokes if we throw exceptions inside
    // to_json calls for individual elements. Fix upstream?
    // Note: there is an example in tests to reproduce.
    Json json;
    to_json(json, trace);
    output.write(json.dump(/*indent=*/4));
}

EventTrace EventTrace::loadFromFile(std::string_view path, PlatformWindow *window) {
    FileInputStream input(path);
    Json json = Json::parse(input.handle());

    EventTrace result;
    from_json(json, result);

    for (std::unique_ptr<PlatformEvent> &event : result.events) {
        dispatchByEventType(event->type, [&]<class T>(T *) {
            if constexpr (std::is_base_of_v<PlatformWindowEvent, T>) {
                static_cast<PlatformWindowEvent *>(event.get())->window = window;
            }
        });
    }

    return result;
}

bool EventTrace::isTraceable(const PlatformEvent *event) {
    bool result = false;
    dispatchByEventType(event->type, [&](auto) { result = true; }); // Callback not invoked => not supported.
    return result;
}

std::unique_ptr<PlatformEvent> EventTrace::cloneEvent(const PlatformEvent *event) {
    assert(isTraceable(event));

    std::unique_ptr<PlatformEvent> result;

    dispatchByEventType(event->type, [&]<class T>(T *) {
        result = std::make_unique<T>(*static_cast<const T *>(event));
    });

    return result;
}
