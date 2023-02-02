#include "EngineControlPlugin.h"

#include <utility>

#include "Platform/PlatformEventHandler.h"

#include "Engine/EngineGlobals.h"

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
        }
        // TODO(captainurist): we need to handle all other exceptions here. EngineController reports errors with
        // exceptions, and if it throws one we'll be running right into std::terminate.

        state->controlRoutineQueue.pop();
    }
}

EngineControlPlugin::EngineControlPlugin(): _unsafeState(std::make_unique<EngineControlState>()), _state(SIDE_GAME, _unsafeState.get()) {
    _emptyHandler = std::make_unique<PlatformEventHandler>();
    _controlThread = std::thread(&controlThread, _unsafeState.get());
}

EngineControlPlugin::~EngineControlPlugin() {
    _state->terminating = true;
    _state.yieldExecution();
    _controlThread.join();
}

void EngineControlPlugin::runControlRoutine(ControlRoutine routine) {
    assert(std::this_thread::get_id() != _controlThread.get_id());

    _state->controlRoutineQueue.push(std::move(routine));
}

bool EngineControlPlugin::hasControlRoutine() const {
    return !_state->controlRoutineQueue.empty();
}

void EngineControlPlugin::processSyntheticEvents(PlatformEventHandler *eventHandler, int count) {
    while (!_state->postedEvents.empty() && count != 0) {
        std::unique_ptr<PlatformEvent> event = std::move(_state->postedEvents.front());
        _state->postedEvents.pop();
        eventHandler->Event(event.get());
        count--; // Negative count will never get to zero, as intended.
    }
}

void EngineControlPlugin::Exec(PlatformEventHandler *eventHandler) {
    if (!hasControlRoutine()) {
        assert(_state->postedEvents.empty());
        ProxyEventLoop::Exec(eventHandler);
    } else {
        processSyntheticEvents(eventHandler);
        ProxyEventLoop::Exec(_emptyHandler.get());
    }
}

void EngineControlPlugin::ProcessMessages(PlatformEventHandler *eventHandler, int count) {
    if (!hasControlRoutine()) {
        assert(_state->postedEvents.empty());
        ProxyEventLoop::ProcessMessages(eventHandler, count);
    } else {
        processSyntheticEvents(eventHandler, count);
        ProxyEventLoop::ProcessMessages(_emptyHandler.get());
    }
}

void EngineControlPlugin::WaitForMessages() {
    if (!hasControlRoutine()) {
        assert(_state->postedEvents.empty());
        ProxyEventLoop::WaitForMessages();
    } else {
        return; // Don't hang up in this function when control routine is running.
    }
}

void EngineControlPlugin::SwapBuffers() {
    // Not tail calling here for a reason - the control routine should run AFTER all the proxies are done.
    ProxyOpenGLContext::SwapBuffers();

    if (hasControlRoutine()) {
        while (true) {
            _state.yieldExecution();
            if (!_state->gameRoutine)
                break;
            _state->gameRoutine();
            _state->gameRoutine = EngineControlState::GameRoutine();
        }
    }
}
