#include <GUI/GUIMessageQueue.h>

#include <utility>

struct GUIMessageQueue *pCurrentFrameMessageQueue = new GUIMessageQueue;
struct GUIMessageQueue *pNextFrameMessageQueue = new GUIMessageQueue;

void GUIMessageQueue::Flush() {
    if (qMessages.size()) {
        GUIMessage message = qMessages.front();
        Clear();
        if (message.field_8 != 0) {
            qMessages.push(message);
        }
    }
}

void GUIMessageQueue::Clear() {
    std::queue<GUIMessage> empty;
    std::swap(qMessages, empty);
}

void GUIMessageQueue::PopMessage(UIMessageType *pType, int *pParam, int *a4) {
    *pType = (UIMessageType)-1;
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

void GUIMessageQueue::AddMessageImpl(UIMessageType msg, int param, unsigned int a4, const char *file, int line) {
    // logger->Warning("{} @ ({} {})", UIMessage2String(msg), file, line);
    GUIMessage message;
    message.eType = msg;
    message.param = param;
    message.field_8 = a4;
    message.file = file;
    message.line = line;
    qMessages.push(message);
}
