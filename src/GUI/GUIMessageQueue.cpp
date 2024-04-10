#include "GUIMessageQueue.h"

#include <queue>

void GUIFrameMessageQueue::clear() {
    std::queue<GUIMessage> empty;

    messageQueue.swap(empty);
}

void GUIFrameMessageQueue::popMessage(UIMessageType *msg, int *param, int *param2) {
    peekMessage(msg, param, param2);
    if (!messageQueue.empty())
        messageQueue.pop();
}

void GUIFrameMessageQueue::peekMessage(UIMessageType *msg, int *param, int *param2) {
    assert(msg != nullptr);

    *msg = UIMSG_Invalid;
    if (param) {
        *param = 0;
    }
    if (param2) {
        *param2 = 0;
    }

    if (messageQueue.empty()) {
        return;
    }

    const GUIMessage &message = messageQueue.front();

    *msg = message.type;
    if (param) {
        *param = message.param;
    }
    if (param2) {
        *param2 = message.param2;
    }
}

void GUIFrameMessageQueue::addGUIMessage(UIMessageType msg, int param, int param2) {
    GUIMessage message;
    message.type = msg;
    message.param = param;
    message.param2 = param2;
    messageQueue.push(message);
}
