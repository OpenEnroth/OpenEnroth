#include <string>
#include <utility>
#include <vector>

#include "Testing/Game/GameTest.h"

#include "Engine/Evt/EvtCommands.h"
#include "Engine/Evt/EvtDecompiler.h"

using namespace std::string_literals; // NOLINT: for the string values of the records.

static const int64_t MAP_VAR0 = std::to_underlying(VAR_MapPersistentVariable_0);
static const int64_t GOLD = std::to_underlying(VAR_FixedGold);

static EvtRecord record(int eventId, int step, EvtOpcode opcode, std::vector<EvtFieldValue> values) {
    EvtRecord result;
    result.eventId = eventId;
    result.step = step;
    result.opcode = opcode;
    result.values = std::move(values);
    return result;
}

GAME_TEST(EvtDecompiler, IfElse) {
    // A condition that jumps over a block ending in a forward jump is an if with an else.
    std::vector<EvtRecord> records = {
        record(1, 0, EVENT_MouseOver, {int64_t(1)}),
        record(1, 0, EVENT_Compare, {MAP_VAR0, int64_t(1), int64_t(3)}),
        record(1, 1, EVENT_StatusText, {int64_t(2)}),
        record(1, 2, EVENT_Jmp, {int64_t(4)}),
        record(1, 3, EVENT_Add, {GOLD, int64_t(100)}),
        record(1, 4, EVENT_ChangeDoorState, {int64_t(5), int64_t(2)}),
        record(1, 5, EVENT_Exit, {int64_t(0)}),
    };

    EXPECT_EQ(decompileEvt(records, {"", "Lever", "Locked"}, false),
              "local TXT = Localize{\n"
              "    [1] = \"Lever\",\n"
              "    [2] = \"Locked\",\n"
              "}\n"
              "table.copy(TXT, evt.str, true)\n"
              "\n"
              "Game.MapEvtLines.Count = 0  -- Deactivate all standard events\n"
              "\n"
              "evt.hint[1] = evt.str[1]  -- \"Lever\"\n"
              "evt.map[1] = function()\n"
              "    if evt.Cmp(\"MapVar0\", 1) then\n"
              "        evt.Add(\"Gold\", 100)\n"
              "    else\n"
              "        evt.StatusText(2)  -- \"Locked\"\n"
              "    end\n"
              "    evt.SetDoorState{Id = 5, State = 2}\n"
              "end\n"
              "\n");
}

GAME_TEST(EvtDecompiler, Goto) {
    // Random jumps and jumps back can't be blocks. They are gotos, and an Exit in the middle is a return in a block of
    // its own.
    std::vector<EvtRecord> records = {
        record(2, 0, EVENT_RandomGoTo, {int64_t(1), int64_t(3), int64_t(0), int64_t(0), int64_t(0), int64_t(0)}),
        record(2, 1, EVENT_Add, {GOLD, int64_t(1)}),
        record(2, 2, EVENT_Exit, {int64_t(0)}),
        record(2, 3, EVENT_Add, {GOLD, int64_t(2)}),
        record(2, 4, EVENT_Jmp, {int64_t(1)}),
    };

    EXPECT_EQ(decompileEvt(records, {}, false),
              "Game.MapEvtLines.Count = 0  -- Deactivate all standard events\n"
              "\n"
              "evt.map[2] = function()\n"
              "    local i\n"
              "    i = Game.Rand() % 2\n"
              "    if i == 0 then\n"
              "        goto _1\n"
              "    else\n"
              "        goto _3\n"
              "    end\n"
              "::_1::\n"
              "    evt.Add(\"Gold\", 1)\n"
              "    do return end\n"
              "::_3::\n"
              "    evt.Add(\"Gold\", 2)\n"
              "    goto _1\n"
              "end\n"
              "\n");
}

GAME_TEST(EvtDecompiler, Triggers) {
    // An event that starts with a trigger does nothing when it's run by id. What follows the trigger is a handler.
    std::vector<EvtRecord> records = {
        record(3, 0, EVENT_OnMapReload, {int64_t(0)}),
        record(3, 1, EVENT_SetSprite, {int64_t(20), int64_t(1), "dec05"s}),
        record(3, 2, EVENT_Exit, {int64_t(0)}),
        record(4, 0, EVENT_OnTimer, {int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(5), int64_t(0)}),
        record(4, 1, EVENT_ForPartyMember, {int64_t(std::to_underlying(CHOOSE_PARTY))}),
        record(4, 2, EVENT_Subtract, {GOLD, int64_t(1)}),
        record(5, 0, EVENT_OnLongTimer, {int64_t(0), int64_t(0), int64_t(0), int64_t(9), int64_t(30), int64_t(0), int64_t(0), int64_t(0)}),
        record(5, 1, EVENT_Set, {MAP_VAR0, int64_t(0)}),
    };

    EXPECT_EQ(decompileEvt(records, {}, false),
              "Game.MapEvtLines.Count = 0  -- Deactivate all standard events\n"
              "\n"
              "evt.map[3] = function()\n"
              "end\n"
              "function events.LoadMap()\n"
              "    evt.SetSprite{SpriteId = 20, Visible = 1, Name = \"dec05\"}\n"
              "end\n"
              "\n"
              "evt.map[4] = function()\n"
              "end\n"
              "Timer(function()\n"
              "    evt.ForPlayer(\"All\")\n"
              "    evt.Subtract(\"Gold\", 1)\n"
              "end, 2.5*const.Minute)\n"
              "\n"
              "evt.map[5] = function()\n"
              "end\n"
              "RefillTimer(function()\n"
              "    evt.Set(\"MapVar0\", 0)\n"
              "end, const.Day, 9*const.Hour + 30*const.Minute)\n"
              "\n");
}

GAME_TEST(EvtDecompiler, Topics) {
    // The commands that decide whether an NPC topic shows up run on their own, before the topic is picked.
    std::vector<EvtRecord> records = {
        record(10, 0, EVENT_OnCanShowDialogItemCmp, {GOLD, int64_t(500), int64_t(3)}),
        record(10, 1, EVENT_SetCanShowDialogItem, {int64_t(0)}),
        record(10, 2, EVENT_EndCanShowDialogItem, {int64_t(0)}),
        record(10, 3, EVENT_SetCanShowDialogItem, {int64_t(1)}),
        record(10, 4, EVENT_EndCanShowDialogItem, {int64_t(0)}),
        record(10, 5, EVENT_ShowMessage, {int64_t(15)}),
        record(10, 6, EVENT_Exit, {int64_t(0)}),
    };

    EXPECT_EQ(decompileEvt(records, {}, true),
              "Game.GlobalEvtLines.Count = 0  -- Deactivate all standard events\n"
              "\n"
              "evt.CanShowTopic[10] = function()\n"
              "    local visible = true\n"
              "    if not evt.All.Cmp(\"Gold\", 500) then\n"
              "        visible = false\n"
              "        return visible\n"
              "    end\n"
              "    visible = true\n"
              "    return visible\n"
              "end\n"
              "evt.global[10] = function()\n"
              "    evt.SetMessage(15)\n"
              "end\n"
              "\n");
}
