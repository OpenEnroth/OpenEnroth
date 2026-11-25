#pragma once

#include <cstdint>
#include <string>

/**
 * Watch mode for scripted test runs (`MM8_SCENARIO_WATCH=1`). Speed keys, pause and an on-screen
 * overlay. It only ever touches `graphics.FPSLimit` and whether the control routine yields back to the game -
 * never the frame time - so a watched run plays the exact frames a headless one does.
 */
struct WatchState {
    bool enabled = false;
    bool paused = false;
    int schedulePauseMs = 0;    // Scripted pause (`pause=` in the schedule): auto-resumes after this much real time.
    std::string pendingShot;    // Schedule `shot=<path>`: the NEXT frame is read back (overlay included) to this PNG.
    int stepOnce = 0;           // `N` while paused: let this many frames through.
    float speed = 4.0f;         // Multiples of real time (1x = 1000 / TraceFrameTimeMs fps). <= 0 means uncapped.
    bool fastForward = false;       // `F`: uncapped until the next step boundary. Shift+F: until `F` is pressed again.
    bool fastForwardSticky = false;
    int fastForwardUntilStep = 0;   // MM8_SCENARIO_WATCH_FF_UNTIL=K: uncapped until step K starts.
    int appliedFpsLimit = -1;
    uint64_t frame = 0;         // Frames presented since the run started, counted in headless runs too.
    std::string cardName;       // MM8_SCENARIO_WATCH_CARD: label shown in the overlay and MARK lines.
    std::string segment;       // MM8_SCENARIO_WATCH_SEGMENT: segment label, e.g. "2/7".
    std::string marksPath;      // MM8_SCENARIO_WATCH_MARKS: `M` appends its MARK line here.
    std::string markStep;       // "4/7 go-to 'event 61'" - the current step, for MARK lines.
    float lastPos[3] = {0, 0, 0};   // Party position at the last presented frame (what movement-log row `frame` holds).
    std::string schedule;       // MM8_SCENARIO_WATCH_SCHEDULE, e.g. "100:speed=1,200:speed=16,300:pause=500,400:speed=4".
    std::string overlayLine;    // "step 3/20 go-to 'Alvar'" - current step text, set by watchSetStep().
    std::string overlayCard;
};

extern WatchState g_watch;

/** Reads the `MM8_SCENARIO_WATCH*` environment; true if watch mode is on. */
bool watchInit();

/** Called at the start of every scripted step, updates the overlay and MARK step text. */
void watchSetStep(const std::string &card, int step, int stepCount, const std::string &verb, const std::string &target);

/** `F` / `Shift+F` handler. */
void watchToggleFastForward(bool sticky);

/** Next/previous speed preset (1x 2x 4x 8x 16x max). */
void watchBumpSpeed(int direction);

/** Overlay text for the ImGui hook in OverlaySystem. */
std::string watchOverlayText();

/** Reads the default (present-size) framebuffer - where the overlay is drawn - into a PNG. Defined in OpenGLRenderer.cpp. */
void watchWritePresentPng(const std::string &path, int width, int height);

/** Counts one presented frame. Called from the swap for every scripted run, watched or not. */
void watchCountFrame();

/** `M` / schedule `mark`: prints one `MARK ...` line to the log, and appends it to the MM8_SCENARIO_WATCH_MARKS file when set. */
void watchMark();

/** Called once per presented frame by EngineControlComponent: runs the schedule and applies the FPS limit. */
void watchOnFrame();
