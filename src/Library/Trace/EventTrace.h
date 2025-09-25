#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include <string>
#include <unordered_set>

#include "Library/Platform/Interface/PlatformEvents.h"
#include "Library/Config/ConfigPatch.h"
#include "Library/Geometry/Vec.h"

#include "Utility/Memory/Blob.h"

// TODO(captainurist): this should go to Core/, not Library/,

struct EventTraceCharacterState {
    int hp = 0;
    int mp = 0;
    int might = 0;
    int intelligence = 0;
    int personality = 0;
    int endurance = 0;
    int accuracy = 0;
    int speed = 0;
    int luck = 0;
    std::vector<std::string> equipment;
    std::vector<std::string> backpack;
};

struct EventTraceGameState {
    std::string locationName;
    Vec3i partyPosition;
    std::vector<EventTraceCharacterState> characters;
};

struct EventTraceHeader {
    /** Size of the save file associated with the trace. */
    int saveFileSize = -1;

    /** Config diff relative to the default config. Applied before loading the save associated with the trace. */
    ConfigPatch config;

    /** Game state at the start of the trace, right after the save is loaded. */
    EventTraceGameState startState;

    /** Game state at the end of the trace. */
    EventTraceGameState endState;

    /** Random state after loading the save file associated with the trace, as returned by `grng->peek(1024)`. */
    int afterLoadRandomState = -1;

    // TODO(captainurist): std::string saveFileChecksum;
};

struct EventTrace {
    static Blob toJsonBlob(const EventTrace &trace);
    static EventTrace fromJsonBlob(const Blob &blob, PlatformWindow *window);

    static bool isTraceable(const PlatformEvent *event);
    static std::unique_ptr<PlatformEvent> cloneEvent(const PlatformEvent *event);

    /**
     * Removes all keyboard events that have no effect from a trace. This includes autorepeat events (OE handles
     * autorepeat internally), and key releases w/o a corresponding key press.
     *
     * @param[in, out] trace            Trace to update.
     */
    static void migrateDropRedundantKeyEvents(EventTrace *trace);

    /**
     * This migration has to do with how OE handles key presses for input actions that trigger continuously.
     * Originally, if a key was pressed and released inside a single frame, the continuous action would not trigger,
     * so this key press was effectively ignored. We are changing that, but we already have a bunch of traces with
     * keypresses that were ignored, so we need to drop them first.
     *
     * Note that we pass in `keys` and don't resolve them dynamically. This is not 100% correct but work.
     *
     * @param keys                      Set of keys for input actions that trigger continuously.
     * @param[in, out] trace            Trace to update.
     */
    static void migrateCollapseKeyEvents(const std::unordered_set<PlatformKey> &keys, EventTrace *trace);

    EventTraceHeader header;
    std::vector<std::unique_ptr<PlatformEvent>> events;
};
