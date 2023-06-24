#include <GUI/GUIMessageQueue.h>

#include <utility>

void GUIFrameMessageQueue::Flush() {
    if (messageQueue.size()) {
        GUIMessage message = messageQueue.front();
        Clear();
        if (message.field_8 != 0) { // TODO(Nik-RE-dev): what's the semantics here?
            messageQueue.push(message);
        }
    }
}

void GUIFrameMessageQueue::Clear() {
    std::queue<GUIMessage> empty;

    messageQueue.swap(empty);
}

void GUIFrameMessageQueue::PopMessage(UIMessageType *pType, int *pParam, int *a4) {
    *pType = UIMSG_Invalid;
    *pParam = 0;
    *a4 = 0;

    if (messageQueue.empty()) {
        return;
    }

    GUIMessage message = messageQueue.front();
    messageQueue.pop();

    *pType = message.eType;
    *pParam = message.param;
    *a4 = message.field_8;
}

void GUIFrameMessageQueue::AddGUIMessage(UIMessageType msg, int param, int a4) {
    GUIMessage message;
    message.eType = msg;
    message.param = param;
    message.field_8 = a4;
    messageQueue.push(message);
}
