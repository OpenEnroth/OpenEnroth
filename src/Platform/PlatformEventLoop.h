#pragma once

class PlatformEventHandler;

/**
 * Abstraction around a platform-specific event loop.
 */
class PlatformEventLoop {
 public:
    virtual ~PlatformEventLoop() = default;

    /**
     * Starts this event loop.
     *
     * This function blocks until either `Quit` is called from inside the event handler code, or an application
     * is closed by the user (e.g. after the last window is closed, or `Command+Q` is pressed on Mac).
     *
     * @param eventHandler              Callback for event processing.
     */
    virtual void Exec(PlatformEventHandler *eventHandler) = 0; // TODO(captainurist): add a flag to proxy all native events

    /**
     * Tells this event loop to exit. Does nothing if the event loop is not running.
     */
    virtual void Quit() = 0;

    // TODO(captainurist): count parameter should be dropped.
    /**
     * Processes the messages that are currently in the message queue, and returns. Returns immediately if there are
     * no messages in the message queue.
     *
     * @param eventHandler              Callback for event processing.
     * @param count                     Maximum number of messages to process, `-1` means unlimited.
     */
    virtual void ProcessMessages(PlatformEventHandler *eventHandler, int count = -1) = 0;

    // TODO(captainurist): this should be dropped.
    /**
     * Blocks until at least one message is delivered to the queue, and returns.
     */
    virtual void WaitForMessages() = 0;
};
