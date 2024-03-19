#pragma once

#include <queue>
#include <string>
#include <cassert>

#include "GUI/GUIEnums.h"

struct GUIMessage {
    UIMessageType type;
    int param;
    int param2;
};

struct GUIFrameMessageQueue {
    GUIFrameMessageQueue() {}

    void clear();
    bool empty() { return messageQueue.empty(); }
    void popMessage(UIMessageType *msg, int *param, int *param2);
    void peekMessage(UIMessageType *msg, int *param, int *param2);
    void addGUIMessage(UIMessageType msg, int param, int param2);

    std::queue<GUIMessage> messageQueue;
};

class GUIMessageQueue {
 public:
    GUIMessageQueue() {}

    bool haveMessages() {
        return !_currentFrameQueue.empty();
    }

    void clear() {
        _currentFrameQueue.clear();
    }

    void clearAll() {
        _currentFrameQueue.clear();
        _nextFrameQueue.clear();
    }

    void swapFrames() {
        _nextFrameQueue.messageQueue.swap(_currentFrameQueue.messageQueue);
        assert(_nextFrameQueue.empty());
    }

    void addMessageCurrentFrame(UIMessageType msg, int param = 0, int param2 = 0) {
        _currentFrameQueue.addGUIMessage(msg, param, param2);
    }

    void addMessageNextFrame(UIMessageType msg, int param = 0, int param2 = 0) {
        _nextFrameQueue.addGUIMessage(msg, param, param2);
    }

    void popMessage(UIMessageType *msg, int *param, int *param2) {
        _currentFrameQueue.popMessage(msg, param, param2);
    }

    void peekMessage(UIMessageType *msg, int *param, int *param2) {
        _currentFrameQueue.peekMessage(msg, param, param2);
    }

 private:
    GUIFrameMessageQueue _currentFrameQueue;
    GUIFrameMessageQueue _nextFrameQueue;
};
