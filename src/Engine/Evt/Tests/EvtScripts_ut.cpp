#include <string>
#include <vector>

#include "Testing/Game/GameTest.h"

#include "Application/GameConfig.h"

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Evt/EvtProgram.h"
#include "Engine/Evt/EvtScripts.h"
#include "Engine/Evt/Processor.h"
#include "Engine/MapEnumFunctions.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Tables/MapTable.h"
#include "Engine/mm7_data.h"

#include "Utility/ScopeGuard.h"

static EvtProgram evtFile(std::string_view evtName) {
    return EvtProgram::load(engine->resources()->eventsData(fmt::format("{}.evt", evtName)));
}

GAME_TEST(EvtScripts, DecompiledEvents) {
    // With debug.decompiled_events on, the Lua script that the decompiler makes of an evt file has to load for every
    // file of the game, empty the file's events and bring every one of them back as a handler or a hint, with the
    // same hint as the evt event. A face whose event only shows a hint can't be clicked.
    bool wasDecompiling = engine->config->debug.DecompiledEvents.value();
    MM_AT_SCOPE_EXIT(engine->config->debug.DecompiledEvents.setValue(wasDecompiling));
    engine->config->debug.DecompiledEvents.setValue(true);

    game.startNewGame();
    EXPECT_EQ(engine->_globalEventMap.eventCount(), 0);
    for (int eventId : evtFile("global").eventIds())
        EXPECT_TRUE(evtScripts()->hasEvent(true, eventId)) << "global.evt, event " << eventId;

    for (MapId map : allMaps()) {
        std::string name = pMapTable->pInfos[map].fileName;
        name = name.substr(0, name.rfind('.'));

        engine->_pendingTransition = MapDestination(map, MAP_START_POINT_PARTY);
        dword_6BE364_game_settings_1 |= GAME_SETTINGS_SKIP_WORLD_UPDATE;
        uGameState = GAME_STATE_CHANGE_LOCATION;
        onMapLeave();
        game.tick();
        game.skipLoadingScreen();

        EvtProgram program = evtFile(name);
        EXPECT_EQ(engine->_localEventMap.eventCount(), 0) << name << ".evt";
        for (int eventId : program.eventIds()) {
            EXPECT_TRUE(evtScripts()->hasEvent(false, eventId) || evtScripts()->eventHint(eventId)) << name << ".evt, event " << eventId;
            EXPECT_EQ(hasEventHint(eventId), program.hasHint(eventId)) << name << ".evt, event " << eventId;
            EXPECT_EQ(getEventHintString(eventId), program.hint(eventId)) << name << ".evt, event " << eventId;
        }
    }
}
