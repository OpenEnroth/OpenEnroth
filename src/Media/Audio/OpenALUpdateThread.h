#pragma once

#include <functional>
#include <thread>

class OpenALUpdateThread {
 public:
    OpenALUpdateThread() : bRunning(false) {}

    virtual ~OpenALUpdateThread() {
        if (bRunning.load(std::memory_order_acquire)) {
            Stop();
        }
    }

    void Start(int interval, std::function<void(void)> func) {
        if (bRunning.load(std::memory_order_acquire)) {
            Stop();
        }
        bRunning.store(true, std::memory_order_release);
        theThread = std::thread([this, interval, func]() {
            while (bRunning.load(std::memory_order_acquire)) {
                func();
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(interval));
            }
        });
    }

    void Stop() {
        bRunning.store(false, std::memory_order_release);
        if (theThread.joinable()) {
            theThread.join();
        }
    }

    bool IsRunning() const noexcept {
        return (bRunning.load(std::memory_order_acquire) &&
                theThread.joinable());
    }

 private:
    std::atomic<bool> bRunning;
    std::thread theThread;
};
