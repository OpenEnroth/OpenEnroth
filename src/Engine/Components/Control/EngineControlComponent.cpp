#include "EngineControlComponent.h"

#include <utility>
#include <memory>
#include <exception>

#include "Library/Platform/Interface/PlatformEventHandler.h"

#include "EngineController.h"

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
    _emptyHandler = std::make_unique<PlatformEventHandler>();
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

void EngineControlComponent::processSyntheticEvents(PlatformEventHandler *eventHandler, int count) {
    while (!_state->postedEvents.empty() && count != 0) {
        std::unique_ptr<PlatformEvent> event = std::move(_state->postedEvents.front());
        _state->postedEvents.pop();
        eventHandler->event(event.get());
        count--; // Negative count will never get to zero, as intended.
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

void EngineControlComponent::processMessages(PlatformEventHandler *eventHandler, int count) {
    if (!hasControlRoutine()) {
        assert(_state->postedEvents.empty());
        ProxyEventLoop::processMessages(eventHandler, count);
    } else {
        processSyntheticEvents(eventHandler, count);
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
    // Not tail calling here for a reason - the control routine should run AFTER all the proxies are done.
    ProxyOpenGLContext::swapBuffers();

    if (hasControlRoutine()) {
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
