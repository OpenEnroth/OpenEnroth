#pragma once

#include <queue>
#include <string>

#include "GUI/GUIEnums.h"

struct GUIMessage {
    enum UIMessageType eType;
    int param;
    int field_8;
    std::string file;
    unsigned int line;
};

#define AddGUIMessage(msg, param, a4) \
    AddMessageImpl((msg), (param), (a4), __FILE__, __LINE__)

struct GUIMessageQueue {
    GUIMessageQueue() {}

    void Flush();
    void Clear();
    bool Empty() { return qMessages.empty(); }
    void PopMessage(UIMessageType *pMsg, int *pParam, int *a4);
    void AddMessageImpl(UIMessageType msg, int param, unsigned int a4, const char *file = nullptr, int line = 0);

    std::queue<GUIMessage> qMessages;
};

/**
 * Message queue for current frame.
 *
 * @offset 0x50CBD0
 */
extern struct GUIMessageQueue *pCurrentFrameMessageQueue;

/**
 * Message queue that will be processed on next frame.
 * @offset 0x50C9E8
 */
extern struct GUIMessageQueue *pNextFrameMessageQueue;
