#pragma once

#include <unordered_set>

#include "Library/Platform/Interface/PlatformEvents.h"

struct EventTrace;

namespace trace {

/**
 * Removes all keyboard events that have no effect from a trace. This includes autorepeat events, and key releases
 * w/o a corresponding key press.
 *
 * @param[in, out] trace            Trace to update.
 */
void migrateDropRedundantKeyEvents(EventTrace *trace);

/**
 * Drops key event pairs (press & release OR release & press) if they are inside a single frame.
 *
 * @param keys                      Set of keys to migrate events for.
 * @param[in, out] trace            Trace to update.
 */
void migrateDropKeyPressReleaseEvents(const std::unordered_set<PlatformKey> &keys, EventTrace *trace);

/**
 * Drops one paint event after each `EVENT_WINDOW_ACTIVATE` and fixes frame timings.
 *
 * @param[in, out] trace            Trace to update.
 */
void migrateDropPaintAfterActivate(EventTrace *trace);

/**
 * Moves all key release events into the frame where the corresponding key press event is.
 *
 * @param keys                      Set of keys to migrate events for.
 * @param[in, out] trace            Trace to update.
 */
void migrateTightenKeyEvents(const std::unordered_set<PlatformKey> &keys, EventTrace *trace);

} // namespace trace
