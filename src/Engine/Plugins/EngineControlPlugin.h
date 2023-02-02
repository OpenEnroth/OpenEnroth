#pragma once

#include <thread>
#include <queue>
#include <functional>
#include <memory>

#include "Platform/Proxy/ProxyOpenGLContext.h"
#include "Platform/Proxy/ProxyEventLoop.h"

#include "EngineControlStateHandle.h"

class EngineController;
class EngineControlState;
class PlatformEvent;

/**
 * This plugin exposes a coroutine-like API that makes it possible to control the game by passing in synthetic
 * platform events.
 *
 * The implementation just uses a separate thread that runs a control routine, and the execution can switch between
 * threads from inside the `SwapBuffers` call. This effectively means that the control routine runs in between game
 * frames.
 *
 * If the control plugin is destroyed while the control routine is still running, the control routine will be terminated
 * by throwing an exception from inside `EngineController`. If you use `catch(...)` inside the control routine then
 * this won't work (e.g. google test does this).
 *
 * @see EngineController
 */
class EngineControlPlugin : private ProxyOpenGLContext, private ProxyEventLoop {
 public:
    using ControlRoutine = std::function<void(EngineController *)>;

    EngineControlPlugin();
    virtual ~EngineControlPlugin();

    /**
     * Schedules a control routine for execution. It will be started in a control thread from inside the next
     * `SwapBuffers` call. All spontaneous (OS-generated) events will be blocked while the control routine is running.
     *
     * If another control routine is already running, passed routine will be added to the queue.
     *
     * Don't call this function from a control thread, just call the control routine directly.
     *
     * Note that it's up to the user to set up a notification for when the control routine has finished.
     *
     * @param routine                   Control routine to schedule.
     */
    void runControlRoutine(ControlRoutine routine);

    /**
     * @return                          Whether there is a control routine running or queued.
     */
    bool hasControlRoutine() const;

 private:
    friend class PlatformIntrospection; // Give access to private bases.

    void processSyntheticEvents(PlatformEventHandler *eventHandler, int count = -1);

    virtual void Exec(PlatformEventHandler *eventHandler) override;
    virtual void ProcessMessages(PlatformEventHandler *eventHandler, int count) override;
    virtual void WaitForMessages() override;
    virtual void SwapBuffers() override;

 private:
    std::thread _controlThread;
    std::unique_ptr<EngineControlState> _unsafeState;
    EngineControlStateHandle _state;
    std::unique_ptr<PlatformEventHandler> _emptyHandler;
};
