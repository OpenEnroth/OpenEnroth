#include "WatchControl.h"

#include <fmt/format.h>

#include <algorithm>
#include <chrono>
#include <csignal>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "Application/GameConfig.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Party.h"
#include "Library/Logger/Logger.h"

WatchState g_watch;

namespace {
constexpr float kSpeeds[] = {1, 2, 4, 8, 16, 0};

float rank(float speed) {
    return speed <= 0 ? 1e9f : speed;
}

struct Event {
    uint64_t frame;
    std::string action;
};

std::vector<Event> parseSchedule() {
    std::vector<Event> result;
    std::stringstream stream(g_watch.schedule);
    std::string item;
    while (std::getline(stream, item, ',')) {
        size_t colon = item.find(':');
        if (colon == std::string::npos)
            continue;
        result.push_back({std::strtoull(item.substr(0, colon).c_str(), nullptr, 10), item.substr(colon + 1)});
    }
    return result;
}
} // namespace

bool watchInit() {
    g_watch.frame = 0;  // The frame index restarts with every run, watched or headless.
    const char *on = std::getenv("MM8_SCENARIO_WATCH");
    g_watch.enabled = on && *on && std::string(on) != "0";
    if (!g_watch.enabled)
        return false;
    if (const char *speed = std::getenv("MM8_SCENARIO_WATCH_SPEED"))
        g_watch.speed = std::strtof(speed, nullptr);
    if (const char *schedule = std::getenv("MM8_SCENARIO_WATCH_SCHEDULE"))
        g_watch.schedule = schedule;
    if (const char *until = std::getenv("MM8_SCENARIO_WATCH_FF_UNTIL"))
        g_watch.fastForwardUntilStep = static_cast<int>(std::strtol(until, nullptr, 10));
#ifndef _WIN32
    // A watched run is windowed, so SIGINT and SIGTERM end the process at once. _exit is async-signal-safe.
    std::signal(SIGTERM, [](int) { _exit(143); });
    std::signal(SIGINT, [](int) { _exit(130); });
#endif
    if (const char *card = std::getenv("MM8_SCENARIO_WATCH_CARD"))
        g_watch.cardName = card;
    if (const char *segment = std::getenv("MM8_SCENARIO_WATCH_SEGMENT"))
        g_watch.segment = segment;
    if (const char *marks = std::getenv("MM8_SCENARIO_WATCH_MARKS"))
        g_watch.marksPath = marks;
    g_watch.paused = false;
    g_watch.appliedFpsLimit = -1;
    return true;
}

void watchSetStep(const std::string &card, int step, int stepCount, const std::string &verb, const std::string &target) {
    if (!g_watch.enabled)
        return;
    g_watch.overlayCard = g_watch.cardName.empty() ? card : g_watch.cardName;
    // A plain `F` ends at this step boundary; Shift+F stays until pressed again. FF_UNTIL covers the steps before K.
    if (!g_watch.fastForwardSticky)
        g_watch.fastForward = step < g_watch.fastForwardUntilStep;
    std::string what = fmt::format("{}{}", verb, target.empty() ? "" : " '" + target + "'");
    g_watch.overlayLine = fmt::format("step {}/{}  {}", step, stepCount, what);
    g_watch.markStep = fmt::format("{}/{} {}", step, stepCount, what);
}

void watchCountFrame() {
    g_watch.frame++;
    // When set, one `FRAME n` line per frame brackets every log line to its frame.
    static const bool frameLog = std::getenv("MM8_SCENARIO_FRAME_LOG") != nullptr;
    if (frameLog)
        MM_INFO("[Watch] FRAME {}", g_watch.frame);
    if (g_watch.enabled && pParty) {
        g_watch.lastPos[0] = pParty->pos.x;
        g_watch.lastPos[1] = pParty->pos.y;
        g_watch.lastPos[2] = pParty->pos.z;
    }
}

void watchMark() {
    // One self-contained, machine-parseable line.
    std::string line = fmt::format("MARK card={} segment={} frame={} step={} pos=({:.0f},{:.0f},{:.0f})",
                                   g_watch.overlayCard.empty() ? "-" : g_watch.overlayCard,
                                   g_watch.segment.empty() ? "-" : g_watch.segment, g_watch.frame,
                                   g_watch.markStep.empty() ? "-" : g_watch.markStep, g_watch.lastPos[0], g_watch.lastPos[1],
                                   g_watch.lastPos[2]);
    MM_INFO("[Watch] {}", line);
    if (!g_watch.marksPath.empty()) {
        std::ofstream out(g_watch.marksPath, std::ios::app);
        out << line << "\n";
    }
}

void watchToggleFastForward(bool sticky) {
    if (g_watch.fastForward && g_watch.fastForwardSticky == sticky) {
        g_watch.fastForward = g_watch.fastForwardSticky = false;
        return;
    }
    g_watch.fastForward = true;
    g_watch.fastForwardSticky = sticky;
}

void watchBumpSpeed(int direction) {
    float current = rank(g_watch.speed);
    if (direction > 0) {
        for (float s : kSpeeds) {
            if (rank(s) > current) {
                g_watch.speed = s;
                return;
            }
        }
    } else {
        for (int i = 5; i >= 0; i--) {
            if (rank(kSpeeds[i]) < current) {
                g_watch.speed = kSpeeds[i];
                return;
            }
        }
    }
}

std::string watchOverlayText() {
    std::string speed = g_watch.speed <= 0 ? "max" : fmt::format("{:g}x", g_watch.speed);
    if (g_watch.fastForward)
        speed = ">> FF";
    // "frame N" = frames presented since this run started.
    std::string where = g_watch.segment.empty() ? "" : fmt::format("segment {} - ", g_watch.segment);
    std::string text = fmt::format("{}  {}\n{}frame {}  (M = mark)", speed, g_watch.overlayLine, where, g_watch.frame);
    if (!g_watch.overlayCard.empty())
        text += fmt::format("\ncard {}", g_watch.overlayCard);
    // The PAUSED line is always reserved (blank when running): the ImGui window auto-sizes one frame late and a pause
    // freezes the last drawn frame, so a line that only appears on pause would stay clipped for the whole pause.
    text += g_watch.paused ? "\nPAUSED (Space/N)" : "\n ";
    return text;
}

void watchOnFrame() {
    static std::vector<Event> events;
    static size_t next = 0;
    if (g_watch.frame == 1) {  // first presented frame (the swap already counted it)
        events = parseSchedule();
        next = 0;
    }
    static auto lastReport = std::chrono::steady_clock::now();
    static uint64_t lastReportFrame = 0;
    if (g_watch.frame - lastReportFrame >= 100) {
        auto now = std::chrono::steady_clock::now();
        double seconds = std::chrono::duration<double>(now - lastReport).count();
        MM_INFO("[Watch] WATCHFPS frame={} speed={} fps={:.1f}", g_watch.frame,
                     g_watch.speed <= 0 ? std::string("max") : fmt::format("{:g}", g_watch.speed),
                     (g_watch.frame - lastReportFrame) / std::max(seconds, 1e-6));
        lastReport = now;
        lastReportFrame = g_watch.frame;
    }
    while (next < events.size() && events[next].frame <= g_watch.frame) {
        const std::string &action = events[next++].action;
        MM_INFO("[Watch] WATCHEVENT frame={} action={}", g_watch.frame, action);
        if (action.rfind("speed=", 0) == 0) {
            g_watch.speed = std::strtof(action.c_str() + 6, nullptr);
        } else if (action.rfind("shot=", 0) == 0) {
            g_watch.pendingShot = action.substr(5);
        } else if (action == "mark") {
            watchMark();
        } else if (action.rfind("pause=", 0) == 0) {
            // Stands in for a human tapping Space and, `ms` of real time later, Space again.
            g_watch.paused = true;
            g_watch.stepOnce = 0;
            g_watch.schedulePauseMs = static_cast<int>(std::strtol(action.c_str() + 6, nullptr, 10));
        }
    }

    if (engine && engine->config) {
        int limit = 0;
        if (g_watch.speed > 0 && !g_watch.fastForward)
            limit = std::max(1, static_cast<int>(g_watch.speed * 1000 / engine->config->debug.TraceFrameTimeMs.value()));
        if (limit != g_watch.appliedFpsLimit) {
            engine->config->graphics.FPSLimit.setValue(limit);
            g_watch.appliedFpsLimit = limit;
        }
    }
}
