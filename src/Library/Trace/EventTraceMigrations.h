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
 * Drops key press & release events if they are inside a single frame.
 *
 * @param keys                      Set of keys to migrate events for.
 * @param[in, out] trace            Trace to update.
 */
void migrateCollapseKeyPressReleaseEvents(const std::unordered_set<PlatformKey> &keys, EventTrace *trace);

/**
 * Drops one paint event after each `EVENT_WINDOW_ACTIVATE` and fixes frame timings.
 */
void migrateDropPaintAfterActivate(EventTrace *trace);

} // namespace trace
