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
     * This function blocks until either `quit` is called from inside the event handler code, or an application
     * is closed by the user (e.g. after the last window is closed, or `Command+Q` is pressed on Mac).
     *
     * @param eventHandler              Callback for event processing.
     */
    virtual void exec(PlatformEventHandler *eventHandler) = 0;

    /**
     * Tells this event loop to exit. Does nothing if the event loop is not running.
     */
    virtual void quit() = 0;

    // TODO(captainurist): count parameter should be dropped.
    /**
     * Processes the messages that are currently in the message queue, and returns. Returns immediately if there are
     * no messages in the message queue.
     *
     * @param eventHandler              Callback for event processing.
     */
    virtual void processMessages(PlatformEventHandler *eventHandler) = 0;

    // TODO(captainurist): this should be dropped.
    /**
     * Blocks until at least one message is delivered to the queue, and returns.
     */
    virtual void waitForMessages() = 0;
};
