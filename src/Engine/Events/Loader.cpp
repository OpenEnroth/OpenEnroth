#include "Engine/Events/Loader.h"

#include <memory>

#include "Engine/Engine.h"
#include "Engine/GameResourceManager.h"
#include "Engine/Events/EventIR.h"
#include "Engine/Events/EventMap.h"
#include "Utility/Memory/Blob.h"

void initGlobalEvents() {
    Blob blob = engine->_gameResourceManager->getEventsFile("global.evt");
    const char *pointer = (const char*)blob.data();
    const char *end = pointer + blob.size();

    engine->_globalEventMap.clear();
    while (pointer < end) {
        const _evt_raw *evtPtr = (const _evt_raw*)pointer;
        int eventId = evtPtr->v1 + (evtPtr->v2 << 8);
        engine->_globalEventMap.add(eventId, EventIR::parse(pointer, sizeof(_evt_raw)));
        pointer += evtPtr->_e_size + 1;
    }
}

void initLocalEvents(const std::string &mapName) {
    Blob blob = engine->_gameResourceManager->getEventsFile(mapName + ".evt");
    const char *pointer = (const char*)blob.data();
    const char *end = pointer + blob.size();

    engine->_localEventMap.clear();
    while (pointer < end) {
        const _evt_raw *evtPtr = (const _evt_raw*)pointer;
        int eventId = evtPtr->v1 + (evtPtr->v2 << 8);
        engine->_localEventMap.add(eventId, EventIR::parse(pointer, sizeof(_evt_raw)));
        pointer += evtPtr->_e_size + 1;
    }
}
