#include "Engine/Events/Loader.h"
#include "Engine/LOD.h"
#include "Engine/Engine.h"

void initGlobalEvents() {
    Blob blob = pEvents_LOD->LoadCompressedTexture("global.evt");
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
    Blob blob = pEvents_LOD->LoadCompressedTexture(mapName + ".evt");
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
