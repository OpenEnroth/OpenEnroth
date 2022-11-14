#pragma once

class PlatformEventLoop {
 public:
    virtual ~PlatformEventLoop() = default;

    virtual void Exec() = 0;
    virtual void Quit() = 0;

    virtual void ProcessMessages(int count = -1) = 0;
    virtual void WaitForMessages() = 0;
};
