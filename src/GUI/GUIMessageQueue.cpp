#include <GUI/GUIMessageQueue.h>

#include <utility>

void GUIFrameMessageQueue::Flush() {
    if (qMessages.size()) {
        GUIMessage message = qMessages.front();
        Clear();
        if (message.field_8 != 0) { // TODO(Nik-RE-dev): what's the semantics here?
            qMessages.push(message);
        }
    }
}

void GUIFrameMessageQueue::Clear() {
    std::queue<GUIMessage> empty;
    std::swap(qMessages, empty);
}

void GUIFrameMessageQueue::PopMessage(UIMessageType *pType, int *pParam, int *a4) {
    *pType = UIMSG_Invalid;
    *pParam = 0;
    *a4 = 0;

    if (qMessages.empty()) {
        return;
    }

    GUIMessage message = qMessages.front();
    qMessages.pop();

    *pType = message.eType;
    *pParam = message.param;
    *a4 = message.field_8;
}

void GUIFrameMessageQueue::AddGUIMessage(UIMessageType msg, int param, int a4) {
    GUIMessage message;
    message.eType = msg;
    message.param = param;
    message.field_8 = a4;
    qMessages.push(message);
}
