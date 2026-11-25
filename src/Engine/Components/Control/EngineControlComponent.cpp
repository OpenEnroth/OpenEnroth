#include "EngineControlComponent.h"

#include <utility>
#include <memory>
#include <exception>
#include <chrono>
#include <thread>

#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Library/Platform/Interface/PlatformEventHandler.h"
#include "Library/Platform/Interface/PlatformEvents.h"

#include "EngineController.h"
#include "WatchControl.h"

namespace {
// In watch mode real key events drive speed and pause.
class WatchEventHandler : public PlatformEventHandler {
 public:
    virtual void keyPressEvent(const PlatformKeyEvent *event) override {
        if (!g_watch.enabled)
            return;
        switch (event->key) {
        case PlatformKey::KEY_ADD: watchBumpSpeed(+1); break;
        case PlatformKey::KEY_SUBTRACT: watchBumpSpeed(-1); break;
        case PlatformKey::KEY_DIGIT_1: g_watch.speed = 1; break;
        case PlatformKey::KEY_DIGIT_2: g_watch.speed = 2; break;
        case PlatformKey::KEY_DIGIT_3: g_watch.speed = 4; break;
        case PlatformKey::KEY_DIGIT_4: g_watch.speed = 8; break;
        case PlatformKey::KEY_DIGIT_5: g_watch.speed = 16; break;
        case PlatformKey::KEY_DIGIT_6: g_watch.speed = 0; break;
        case PlatformKey::KEY_SPACE:
            if (!event->isAutoRepeat) {
                g_watch.paused = !g_watch.paused;
                g_watch.schedulePauseMs = 0;
            }
            break;
        case PlatformKey::KEY_F:
            if (!event->isAutoRepeat)
                watchToggleFastForward(bool(event->mods & MOD_SHIFT));
            break;
        case PlatformKey::KEY_M:
            if (!event->isAutoRepeat)
                watchMark();
            break;
        case PlatformKey::KEY_N:
            if (g_watch.paused)
                g_watch.stepOnce++;
            break;
        default: break;
        }
    }
};
} // namespace

static void controlThread(EngineControlState *unsafeState) {
    EngineControlStateHandle state(SIDE_CONTROL, unsafeState);
    EngineController controller(state);

    while (true) {
        while (state->controlRoutineQueue.empty() && !state->terminating)
            state.yieldExecution();

        if (state->terminating)
            return;

        try {
            // std::queue uses std::deque which doesn't move elements around on reallocation, so we're safe even if
            // another control routine is queued from inside this call.
            state->controlRoutineQueue.front()(&controller);

            // Let the game consume all posted events before dropping the current control routine. Note that it's
            // important to do this inside a try block as tick() throws.
            if (!state->postedEvents.empty())
                controller.tick();
            assert(state->postedEvents.empty()); // We assume that the main thread processes all events each tick.
        } catch (EngineControlState::TerminationException) {
            return;
        } catch (...) {
            state->gameRoutine = [exception = std::current_exception()] {
                std::rethrow_exception(exception);
            };
            state.yieldExecution();
        }

        state->controlRoutineQueue.pop();
    }
}

EngineControlComponent::EngineControlComponent(): _unsafeState(std::make_unique<EngineControlState>()), _state(SIDE_GAME, _unsafeState.get()) {
    _emptyHandler = std::make_unique<WatchEventHandler>();
    _controlThread = std::thread(&controlThread, _unsafeState.get());
}

EngineControlComponent::~EngineControlComponent() {
    assert(!application()); // Should be uninstalled.
    assert(!_controlThread.joinable()); // Thread should be joined at this point.
}

void EngineControlComponent::runControlRoutine(ControlRoutine routine) {
    assert(std::this_thread::get_id() != _controlThread.get_id());

    _state->controlRoutineQueue.push(std::move(routine));
}

bool EngineControlComponent::hasControlRoutine() const {
    return !_state->controlRoutineQueue.empty();
}

void EngineControlComponent::processSyntheticEvents(PlatformEventHandler *eventHandler) {
    while (!_state->postedEvents.empty()) {
        std::unique_ptr<PlatformEvent> event = std::move(_state->postedEvents.front());
        _state->postedEvents.pop();
        eventHandler->event(event.get());
    }
}

void EngineControlComponent::exec(PlatformEventHandler *eventHandler) {
    if (!hasControlRoutine()) {
        assert(_state->postedEvents.empty());
        ProxyEventLoop::exec(eventHandler);
    } else {
        processSyntheticEvents(eventHandler);
        ProxyEventLoop::exec(_emptyHandler.get());
    }
}

void EngineControlComponent::processMessages(PlatformEventHandler *eventHandler) {
    if (!hasControlRoutine()) {
        assert(_state->postedEvents.empty());
        ProxyEventLoop::processMessages(eventHandler);
    } else {
        processSyntheticEvents(eventHandler);
        ProxyEventLoop::processMessages(_emptyHandler.get());
    }
}

void EngineControlComponent::waitForMessages() {
    if (!hasControlRoutine()) {
        assert(_state->postedEvents.empty());
        ProxyEventLoop::waitForMessages();
    } else {
        return; // Don't hang up in this function when control routine is running.
    }
}

void EngineControlComponent::swapBuffers() {
    if (g_watch.enabled && !g_watch.pendingShot.empty()) {
        // Schedule `shot=`: the finished frame, overlay included, read back before the swap.
        Sizei size = render->GetPresentDimensions();
        watchWritePresentPng(g_watch.pendingShot, size.w, size.h);
        g_watch.pendingShot.clear();
    }

    // Not tail calling here for a reason - the control routine should run AFTER all the proxies are done.
    ProxyOpenGLContext::swapBuffers();

    if (hasControlRoutine()) {
        if (g_watch.enabled) {
            // Paused: (checked first so a pause set by a key/schedule on frame N takes effect after frame N+1 was
            // drawn WITH its PAUSED overlay) the game does not advance (no yield to the control routine), the window keeps pumping events.
            auto pausedAt = std::chrono::steady_clock::now();
            while (g_watch.paused && g_watch.stepOnce == 0) {
                ProxyEventLoop::processMessages(_emptyHandler.get());
                if (g_watch.schedulePauseMs > 0 && std::chrono::steady_clock::now() - pausedAt >=
                                                       std::chrono::milliseconds(g_watch.schedulePauseMs)) {
                    g_watch.paused = false;
                    g_watch.schedulePauseMs = 0;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if (g_watch.stepOnce > 0)
                g_watch.stepOnce--;
            watchCountFrame();
            watchOnFrame();
        } else {
            watchCountFrame();  // A headless run indexes frames exactly like a watched one.
        }
        while (true) {
            _state.yieldExecution();

            if (_state->gameRoutine) {
                auto gameRoutine = std::move(_state->gameRoutine);
                _state->gameRoutine = {};
                gameRoutine(); // This can throw.
            } else {
                break;
            }
        }
    }
}

void EngineControlComponent::removeNotify() {
    _state->terminating = true;
    _state.yieldExecution();
    _controlThread.join();
}
