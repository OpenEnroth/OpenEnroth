#pragma once

#include <queue>
#include <string>

#include "GUI/GUIEnums.h"

struct GUIMessage {
    enum UIMessageType eType;
    int param;
    int field_8;
};

struct GUIFrameMessageQueue {
    GUIFrameMessageQueue() {}

    void Flush();
    void Clear();
    bool Empty() { return qMessages.empty(); }
    void PopMessage(UIMessageType *pMsg, int *pParam, int *a4);
    void AddGUIMessage(UIMessageType msg, int param, int a4);

    std::queue<GUIMessage> qMessages;
};

class GUIMessageQueue {
 public:
    GUIMessageQueue() {}

    bool haveMessages() {
        return !_currentFrameQueue.Empty();
    }

    void flush() {
        _currentFrameQueue.Flush();
    }

    void clear() {
        _currentFrameQueue.Clear();
    }

    void clearAll() {
        _currentFrameQueue.Clear();
        _nextFrameQueue.Clear();
    }

    void swapFrames() {
        _nextFrameQueue.qMessages.swap(_currentFrameQueue.qMessages);
    }

    void addMessageCurrentFrame(UIMessageType msg, int param, int a4) {
        _currentFrameQueue.AddGUIMessage(msg, param, a4);
    }

    void addMessageNextFrame(UIMessageType msg, int param, int a4) {
        _nextFrameQueue.AddGUIMessage(msg, param, a4);
    }

    void popMessage(UIMessageType *msg, int *param, int *a4) {
        _currentFrameQueue.PopMessage(msg, param, a4);
    }

 private:
    GUIFrameMessageQueue _currentFrameQueue;
    GUIFrameMessageQueue _nextFrameQueue;
};
