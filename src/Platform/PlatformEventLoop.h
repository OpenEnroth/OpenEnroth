#pragma once

/**
 * Abstraction around a platform-specific event loop.
 */
class PlatformEventLoop {
 public:
    virtual ~PlatformEventLoop() = default;

    /**
     * Starts this event loop.
     *
     * This function blocks until either `Quit` is called from inside one of the event handlers, or an application
     * is closed by the user (e.g. after the last window is closed, or `Command+Q` is pressed on Mac).
     */
    virtual void Exec() = 0;

    /**
     * Tells this event loop to exit. Does nothing if the event loop is not running.
     */
    virtual void Quit() = 0;

    // TODO(captainurist): count parameter should be dropped.
    /**
     * Processes the messages that are currently in the message queue, and returns.
     *
     * @param count                     Maximum number of messages to process, `-1` means unlimited.
     */
    virtual void ProcessMessages(int count = -1) = 0;

    // TODO(captainurist): this should be dropped.
    /**
     * Blocks until at least one message is delivered to the queue, and returns.
     */
    virtual void WaitForMessages() = 0;
};
