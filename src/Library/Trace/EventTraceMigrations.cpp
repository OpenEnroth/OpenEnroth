#include "EventTraceMigrations.h"

#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <utility>

#include "Library/Platform/Interface/PlatformEvents.h"

#include "Utility/MapAccess.h"

#include "EventTrace.h"
#include "PaintEvent.h"

static std::vector<std::vector<std::unique_ptr<PlatformEvent>>> splitIntoFrames(std::vector<std::unique_ptr<PlatformEvent>> events) {
    std::vector<std::vector<std::unique_ptr<PlatformEvent>>> result;
    for (auto &event : events) {
        if (event->type == EVENT_PAINT) {
            result.emplace_back();
            result.back().push_back(std::move(event)); // Paint is always the first in a frame when present.
        } else {
            if (result.empty())
                result.emplace_back();
            result.back().push_back(std::move(event));
        }
    }
    return result;
}

static std::vector<std::unique_ptr<PlatformEvent>> mergeFromFrames(std::vector<std::vector<std::unique_ptr<PlatformEvent>>> frames) {
    std::vector<std::unique_ptr<PlatformEvent>> result;
    for (auto &frame : frames)
        for (auto &event : frame)
            if (event)
                result.push_back(std::move(event));
    return result;
}

void trace::migrateDropRedundantKeyEvents(EventTrace *trace) {
    std::unordered_set<PlatformKey> pressedKeys;
    for (std::unique_ptr<PlatformEvent> &event : trace->events) {
        if (event->type != EVENT_KEY_PRESS && event->type != EVENT_KEY_RELEASE)
            continue;

        PlatformKey key = static_cast<PlatformKeyEvent *>(event.get())->key;
        if (event->type == EVENT_KEY_PRESS) {
            if (pressedKeys.contains(key)) {
                event.reset(); // Drop redundant key press events.
            } else {
                pressedKeys.insert(key);
            }
        } else {
            if (!pressedKeys.contains(key)) {
                event.reset(); // Drop redundant key release events.
            } else {
                pressedKeys.erase(key);
            }
        }
    }

    std::erase_if(trace->events, [](const auto &event) { return !event; });
}

void trace::migrateDropKeyPressReleaseEvents(const std::unordered_set<PlatformKey> &keys, EventTrace *trace) {
    // Non-negative value => in-frame index, negative value => event happened in another frame.
    std::unordered_map<PlatformKey, int> pressIndexByKey;
    std::unordered_map<PlatformKey, int> releaseIndexByKey;

    auto frames = splitIntoFrames(std::move(trace->events));
    for (auto &frame : frames) {
        for (size_t i = 0; i < frame.size(); i++) {
            if (frame[i]->type != EVENT_KEY_PRESS && frame[i]->type != EVENT_KEY_RELEASE)
                continue;

            PlatformKey key = static_cast<PlatformKeyEvent *>(frame[i].get())->key;
            if (!keys.contains(key))
                continue;

            if (frame[i]->type == EVENT_KEY_PRESS) {
                assert(!pressIndexByKey.contains(key));
                pressIndexByKey[key] = i;

                int releaseIndex = valueOr(releaseIndexByKey, key, -1);
                if (releaseIndex >= 0) {
                    // Release & press inside a single frame, should drop.
                    frame[releaseIndex].reset();
                    frame[i].reset();
                }

                releaseIndexByKey.erase(key);
            } else {
                assert(!releaseIndexByKey.contains(key));
                assert(pressIndexByKey.contains(key)); // Every release must have a matching press.
                releaseIndexByKey[key] = i;

                int pressIndex = valueOr(pressIndexByKey, key, -1);
                if (pressIndex >= 0) {
                    // Press & release inside a single frame, should drop.
                    frame[pressIndex].reset();
                    frame[i].reset();
                }

                pressIndexByKey.erase(key);
            }
        }

        for (auto &[_, index] : pressIndexByKey)
            index = -1;
        for (auto &[_, index] : releaseIndexByKey)
            index = -1;
    }
    trace->events = mergeFromFrames(std::move(frames));
}

void trace::migrateDropPaintAfterActivate(EventTrace *trace) {
    std::queue<int64_t> paintTicks;
    bool dropNextPaintEvent = false;

    for (std::unique_ptr<PlatformEvent> &event : trace->events) {
        if (event->type == EVENT_PAINT) {
            PaintEvent *paintEvent = static_cast<PaintEvent *>(event.get());
            paintTicks.push(paintEvent->tickCount);

            if (dropNextPaintEvent) {
                dropNextPaintEvent = false;
                event.reset();
            } else {
                paintEvent->tickCount = paintTicks.front();
                paintTicks.pop();
            }
        } else if (event->type == EVENT_WINDOW_ACTIVATE) {
            dropNextPaintEvent = true;
        }
    }

    std::erase_if(trace->events, [](const auto &event) { return !event; });
}

void trace::migrateTightenKeyEvents(const std::unordered_set<PlatformKey> &keys, EventTrace *trace) {
    // Frame index for key presses.
    std::unordered_map<PlatformKey, size_t> pressFrameByKey;

    auto frames = splitIntoFrames(std::move(trace->events));
    for (size_t i = 0; i < frames.size(); i++) {
        auto &frame = frames[i];

        for (auto &event : frame) {
            if (event->type != EVENT_KEY_PRESS && event->type != EVENT_KEY_RELEASE)
                continue;

            PlatformKey key = static_cast<PlatformKeyEvent *>(event.get())->key;
            if (!keys.contains(key))
                continue;

            if (event->type == EVENT_KEY_PRESS) {
                pressFrameByKey[key] = i;
            } else {
                if (!pressFrameByKey.contains(key))
                    continue; // No matching press recorded, skip.

                size_t pressFrameIndex = pressFrameByKey[key];
                pressFrameByKey.erase(key);

                if (pressFrameIndex == i)
                    continue; // Already tight enough.

                frames[pressFrameIndex].push_back(std::move(event));
            }
        }
    }

    trace->events = mergeFromFrames(std::move(frames));
}
