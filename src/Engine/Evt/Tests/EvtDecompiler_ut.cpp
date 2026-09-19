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
              "Game.MapEvtLines.Count = 0\n"
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
              "Game.MapEvtLines.Count = 0\n"
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
              "Game.MapEvtLines.Count = 0\n"
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
              "Game.GlobalEvtLines.Count = 0\n"
              "\n"
              "evt.CanShowTopic[10] = function()\n"
              "    local visible = true\n"
              "    if not evt.Cmp(\"Gold\", 500) then\n"
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

    // Topics are global events, so in a map's file the same commands do nothing.
    EXPECT_EQ(decompileEvt(records, {}, false),
              "Game.MapEvtLines.Count = 0\n"
              "\n"
              "evt.map[10] = function()\n"
              "    evt.SetMessage(15)\n"
              "end\n"
              "\n");
}

GAME_TEST(EvtDecompiler, NestedIf) {
    // A condition nests inside another one's block. A block that is jumped into from outside can't be a Lua block.
    std::vector<EvtRecord> nested = {
        record(20, 0, EVENT_Compare, {MAP_VAR0, int64_t(1), int64_t(4)}),
        record(20, 1, EVENT_Compare, {GOLD, int64_t(100), int64_t(3)}),
        record(20, 2, EVENT_Add, {GOLD, int64_t(5)}),
        record(20, 3, EVENT_StatusText, {int64_t(2)}),
        record(20, 4, EVENT_Exit, {int64_t(0)}),
    };

    EXPECT_EQ(decompileEvt(nested, {}, false),
              "Game.MapEvtLines.Count = 0\n"
              "\n"
              "evt.map[20] = function()\n"
              "    if not evt.Cmp(\"MapVar0\", 1) then\n"
              "        if not evt.Cmp(\"Gold\", 100) then\n"
              "            evt.Add(\"Gold\", 5)\n"
              "        end\n"
              "        evt.StatusText(2)\n"
              "    end\n"
              "end\n"
              "\n");

    std::vector<EvtRecord> entered = {
        record(21, 0, EVENT_Compare, {MAP_VAR0, int64_t(1), int64_t(2)}),
        record(21, 1, EVENT_Add, {GOLD, int64_t(1)}),
        record(21, 2, EVENT_Add, {GOLD, int64_t(2)}),
        record(21, 3, EVENT_Compare, {GOLD, int64_t(100), int64_t(1)}),
        record(21, 4, EVENT_Exit, {int64_t(0)}),
    };

    EXPECT_EQ(decompileEvt(entered, {}, false),
              "Game.MapEvtLines.Count = 0\n"
              "\n"
              "evt.map[21] = function()\n"
              "    if evt.Cmp(\"MapVar0\", 1) then\n"
              "        goto _2\n"
              "    end\n"
              "::_1::\n"
              "    evt.Add(\"Gold\", 1)\n"
              "::_2::\n"
              "    evt.Add(\"Gold\", 2)\n"
              "    if evt.Cmp(\"Gold\", 100) then\n"
              "        goto _1\n"
              "    end\n"
              "end\n"
              "\n");
}

GAME_TEST(EvtDecompiler, StepOrder) {
    // The interpreter starts from step 0 and looks every step up, so the order of the records in a file doesn't count.
    std::vector<EvtRecord> records = {
        record(22, 1, EVENT_Add, {GOLD, int64_t(1)}),
        record(22, 0, EVENT_Add, {GOLD, int64_t(2)}),
        record(22, 2, EVENT_Exit, {int64_t(0)}),
        record(23, 0, EVENT_MouseOver, {int64_t(1)}), // The hint is all there is at step 0, so the event does nothing.
        record(23, 1, EVENT_Add, {GOLD, int64_t(3)}),
        record(23, 2, EVENT_Exit, {int64_t(0)}),
    };

    EXPECT_EQ(decompileEvt(records, {"", "Door"}, false),
              "local TXT = Localize{\n"
              "    [1] = \"Door\",\n"
              "}\n"
              "table.copy(TXT, evt.str, true)\n"
              "\n"
              "Game.MapEvtLines.Count = 0\n"
              "\n"
              "evt.map[22] = function()\n"
              "    goto _0\n"
              "::_1::\n"
              "    evt.Add(\"Gold\", 1)\n"
              "    goto _2\n"
              "::_0::\n"
              "    evt.Add(\"Gold\", 2)\n"
              "    goto _1\n"
              "::_2::\n"
              "end\n"
              "\n"
              "evt.hint[23] = evt.str[1]  -- \"Door\"\n"
              "evt.map[23] = function()\n"
              "end\n"
              "\n");
}

GAME_TEST(EvtDecompiler, Hints) {
    // The engine takes an event for a hint alone only if it opens with a hint and an exit. Any other event has to get a
    // handler, even an empty one, or its faces stop taking clicks.
    std::vector<EvtRecord> records = {
        record(30, 0, EVENT_MouseOver, {int64_t(1)}),
        record(30, 0, EVENT_Exit, {int64_t(0)}),
        record(31, 0, EVENT_MouseOver, {int64_t(1)}),
        record(31, 0, EVENT_OnMapReload, {int64_t(0)}),
        record(31, 1, EVENT_Add, {GOLD, int64_t(1)}),
        record(31, 2, EVENT_Exit, {int64_t(0)}),
        record(32, 0, EVENT_MouseOver, {int64_t(9)}), // There is no such string, and the engine shows an empty hint then.
        record(32, 0, EVENT_Exit, {int64_t(0)}),
        record(33, 0, EVENT_MouseOver, {int64_t(1)}),
        record(33, 0, EVENT_SpeakInHouse, {int64_t(2)}),
        record(33, 1, EVENT_Exit, {int64_t(0)}),
        record(34, 0, EVENT_MouseOver, {int64_t(1)}),
        record(34, 0, EVENT_SpeakInHouse, {int64_t(9999)}), // There is no such house, and the engine keeps the hint then.
        record(34, 1, EVENT_Exit, {int64_t(0)}),
    };

    EXPECT_EQ(decompileEvt(records, {"", "Door"}, false),
              "local TXT = Localize{\n"
              "    [1] = \"Door\",\n"
              "}\n"
              "table.copy(TXT, evt.str, true)\n"
              "\n"
              "Game.MapEvtLines.Count = 0\n"
              "\n"
              "evt.hint[30] = evt.str[1]  -- \"Door\"\n"
              "\n"
              "evt.hint[31] = evt.str[1]  -- \"Door\"\n"
              "evt.map[31] = function()\n"
              "end\n"
              "function events.LoadMap()\n"
              "    evt.Add(\"Gold\", 1)\n"
              "end\n"
              "\n"
              "evt.hint[32] = \"\"\n"
              "\n"
              "evt.house[33] = 2  -- \"Tempered Steel\"\n"
              "evt.map[33] = function()\n"
              "    evt.EnterHouse(2)  -- \"Tempered Steel\"\n"
              "end\n"
              "\n"
              "evt.hint[34] = evt.str[1]  -- \"Door\"\n"
              "evt.map[34] = function()\n"
              "    evt.EnterHouse(9999)\n"
              "end\n"
              "\n");
}

GAME_TEST(EvtDecompiler, CalendarTimers) {
    // `Timer` stands for `OnTimer` and `RefillTimer` for `OnLongTimer` whatever the fields are, and the engine checks
    // all of the former before any of the latter.
    std::vector<EvtRecord> records = {
        record(40, 0, EVENT_OnTimer, {int64_t(0), int64_t(0), int64_t(0), int64_t(1), int64_t(0), int64_t(0), int64_t(0), int64_t(0)}),
        record(40, 1, EVENT_Add, {GOLD, int64_t(1)}),
        record(41, 0, EVENT_OnLongTimer, {int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(1), int64_t(0), int64_t(0)}),
        record(41, 1, EVENT_Add, {GOLD, int64_t(2)}),
        record(42, 0, EVENT_OnLongTimer, {int64_t(0), int64_t(0), int64_t(7), int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(0)}),
        record(42, 1, EVENT_Add, {GOLD, int64_t(3)}),
        record(43, 0, EVENT_OnLongTimer, {int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(2), int64_t(0)}),
        record(43, 1, EVENT_Add, {GOLD, int64_t(4)}),
    };

    EXPECT_EQ(decompileEvt(records, {}, false),
              "Game.MapEvtLines.Count = 0\n"
              "\n"
              "evt.map[40] = function()\n"
              "end\n"
              "Timer(function()\n"
              "    evt.Add(\"Gold\", 1)\n"
              "end, const.Day, 1*const.Hour, false)\n"
              "\n"
              "evt.map[41] = function()\n"
              "end\n"
              "RefillTimer(function()\n"
              "    evt.Add(\"Gold\", 2)\n"
              "end, const.Day, 1*const.Minute/60)\n"
              "\n"
              "evt.map[42] = function()\n"
              "end\n"
              "RefillTimer(function()\n"
              "    evt.Add(\"Gold\", 3)\n"
              "end, const.Week)\n"
              "\n"
              "evt.map[43] = function()\n"
              "end\n"
              "RefillTimer(function()\n"
              "    evt.Add(\"Gold\", 4)\n"
              "end, const.Minute)\n"
              "\n");
}

GAME_TEST(EvtDecompiler, Strings) {
    // Quotes, backslashes and line breaks in a string must not end the Lua string or the comment they sit in.
    std::vector<EvtRecord> records = {
        record(50, 0, EVENT_ShowMovie, {int64_t(1), int64_t(1), "\"family reunion\" "s}),
        record(50, 1, EVENT_StatusText, {int64_t(1)}),
        record(50, 2, EVENT_Exit, {int64_t(0)}),
    };

    EXPECT_EQ(decompileEvt(records, {"", "a\nb\\c"}, false),
              "local TXT = Localize{\n"
              "    [1] = \"a\\nb\\\\c\",\n"
              "}\n"
              "table.copy(TXT, evt.str, true)\n"
              "\n"
              "Game.MapEvtLines.Count = 0\n"
              "\n"
              "evt.map[50] = function()\n"
              "    evt.ShowMovie{DoubleSize = 1, ExitCurrentScreen = true, Name = \"\\\"family reunion\\\" \"}\n"
              "    evt.StatusText(1)  -- \"a\\nb\\\\c\"\n"
              "end\n"
              "\n");
}

GAME_TEST(EvtDecompiler, Unsupported) {
    // The interpreter ends an event on a question, and a record that the table can't describe has no fields to print.
    EvtRecord unknown;
    unknown.eventId = 61;
    unknown.opcode = static_cast<EvtOpcode>(0xFF);
    unknown.payload = "ab";

    std::vector<EvtRecord> records = {
        record(60, 0, EVENT_InputString, {int64_t(1), int64_t(2), int64_t(3), int64_t(2)}),
        record(60, 1, EVENT_Add, {GOLD, int64_t(1)}),
        record(60, 2, EVENT_Exit, {int64_t(0)}),
        unknown,
        record(61, 1, EVENT_Exit, {int64_t(0)}),
    };

    EXPECT_EQ(decompileEvt(records, {}, false),
              "Game.MapEvtLines.Count = 0\n"
              "\n"
              "evt.map[60] = function()\n"
              "    -- evt.Question{Question = 1, Answer1 = 2, Answer2 = 3} isn't supported by OpenEnroth.\n"
              "end\n"
              "\n"
              "evt.map[61] = function()\n"
              "    -- Command 255 with 2 bytes of data isn't known to OpenEnroth.\n"
              "end\n"
              "\n");
}
