#include <functional>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "Testing/Game/GameTest.h"

#include "Engine/Evt/EvtCommands.h"
#include "Engine/Evt/EvtDecompiler.h"
#include "Engine/Evt/EvtProgram.h"

using namespace std::string_literals; // NOLINT: for the string values of the commands.

static const int64_t MAP_VAR0 = std::to_underlying(VAR_MapPersistentVariable_0);
static const int64_t GOLD = std::to_underlying(VAR_FixedGold);

static std::pair<int, EvtInstruction> op(int eventId, int step, EvtOpcode opcode, std::function<void(EvtInstruction &)> fill = {}) {
    EvtInstruction result = {};
    result.step = step;
    result.opcode = opcode;
    if (fill)
        fill(result);
    return {eventId, result};
}

static std::pair<int, EvtInstruction> call(int eventId, int step, std::string_view name, std::vector<EvtFieldValue> values, int target = 0) {
    const EvtCommandInfo *command = evtCommand(name);
    auto [_, result] = op(eventId, step, command->opcode);
    result.target_step = target;
    for (size_t i = 0; i < values.size(); i++)
        if (command->fields[i].set)
            command->fields[i].set(&result, values[i]);
    return {eventId, result};
}

static EvtProgram program(std::initializer_list<std::pair<int, EvtInstruction>> instructions) {
    EvtProgram result;
    for (const auto &[eventId, ir] : instructions)
        result.add(eventId, ir);
    return result;
}

GAME_TEST(EvtDecompiler, IfElse) {
    // A condition that jumps over a block ending in a forward jump is an if with an else.
    EvtProgram events = program({
        op(1, 0, EVENT_MouseOver, [](EvtInstruction &ir) { ir.data.text_id = 1; }),
        call(1, 0, "Cmp", {MAP_VAR0, int64_t(1)}, 3),
        call(1, 1, "StatusText", {int64_t(2)}),
        op(1, 2, EVENT_Jmp, [](EvtInstruction &ir) { ir.target_step = 4; }),
        call(1, 3, "Add", {GOLD, int64_t(100)}),
        call(1, 4, "SetDoorState", {int64_t(5), int64_t(2)}),
        op(1, 5, EVENT_Exit),
    });

    EXPECT_EQ(decompileEvt(events, {"", "Lever", "Locked"}, false).text(),
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
    EvtProgram events = program({
        op(2, 0, EVENT_RandomGoTo, [](EvtInstruction &ir) { ir.data.random_goto_descr = {{1, 3, 0, 0, 0, 0}, 2}; }),
        call(2, 1, "Add", {GOLD, int64_t(1)}),
        op(2, 2, EVENT_Exit),
        call(2, 3, "Add", {GOLD, int64_t(2)}),
        op(2, 4, EVENT_Jmp, [](EvtInstruction &ir) { ir.target_step = 1; }),
    });

    EXPECT_EQ(decompileEvt(events, {}, false).text(),
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
    EvtProgram events = program({
        op(3, 0, EVENT_OnMapReload),
        call(3, 1, "SetSprite", {int64_t(20), int64_t(1), "dec05"s}),
        op(3, 2, EVENT_Exit),
        op(4, 0, EVENT_OnTimer, [](EvtInstruction &ir) { ir.data.timer_descr = {0, 0, 0, 0, 0, 0, 5}; }),
        op(4, 1, EVENT_ForPartyMember, [](EvtInstruction &ir) { ir.who = CHOOSE_PARTY; }),
        call(4, 2, "Subtract", {GOLD, int64_t(1)}),
        op(5, 0, EVENT_OnLongTimer, [](EvtInstruction &ir) { ir.data.timer_descr = {0, 0, 0, 9, 30, 0, 0}; }),
        call(5, 1, "Set", {MAP_VAR0, int64_t(0)}),
    });

    EXPECT_EQ(decompileEvt(events, {}, false).text(),
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
    EvtProgram events = program({
        op(10, 0, EVENT_OnCanShowDialogItemCmp, [](EvtInstruction &ir) { ir.data.variable_descr = {VAR_FixedGold, 500}; ir.target_step = 3; }),
        op(10, 1, EVENT_SetCanShowDialogItem, [](EvtInstruction &ir) { ir.data.can_show_npc_dialogue = 0; }),
        op(10, 2, EVENT_EndCanShowDialogItem),
        op(10, 3, EVENT_SetCanShowDialogItem, [](EvtInstruction &ir) { ir.data.can_show_npc_dialogue = 1; }),
        op(10, 4, EVENT_EndCanShowDialogItem),
        call(10, 5, "SetMessage", {int64_t(15)}),
        op(10, 6, EVENT_Exit),
    });

    EXPECT_EQ(decompileEvt(events, {}, true).text(),
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
    EXPECT_EQ(decompileEvt(events, {}, false).text(),
              "Game.MapEvtLines.Count = 0\n"
              "\n"
              "evt.map[10] = function()\n"
              "    evt.SetMessage(15)\n"
              "end\n"
              "\n");
}

GAME_TEST(EvtDecompiler, NestedIf) {
    // A condition nests inside another one's block. A block that is jumped into from outside can't be a Lua block.
    EvtProgram nested = program({
        call(20, 0, "Cmp", {MAP_VAR0, int64_t(1)}, 4),
        call(20, 1, "Cmp", {GOLD, int64_t(100)}, 3),
        call(20, 2, "Add", {GOLD, int64_t(5)}),
        call(20, 3, "StatusText", {int64_t(2)}),
        op(20, 4, EVENT_Exit),
    });

    EXPECT_EQ(decompileEvt(nested, {}, false).text(),
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

    EvtProgram entered = program({
        call(21, 0, "Cmp", {MAP_VAR0, int64_t(1)}, 2),
        call(21, 1, "Add", {GOLD, int64_t(1)}),
        call(21, 2, "Add", {GOLD, int64_t(2)}),
        call(21, 3, "Cmp", {GOLD, int64_t(100)}, 1),
        op(21, 4, EVENT_Exit),
    });

    EXPECT_EQ(decompileEvt(entered, {}, false).text(),
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
    EvtProgram events = program({
        call(22, 1, "Add", {GOLD, int64_t(1)}),
        call(22, 0, "Add", {GOLD, int64_t(2)}),
        op(22, 2, EVENT_Exit),
        op(23, 0, EVENT_MouseOver, [](EvtInstruction &ir) { ir.data.text_id = 1; }), // The hint is all there is at step 0, so the event does nothing.
        call(23, 1, "Add", {GOLD, int64_t(3)}),
        op(23, 2, EVENT_Exit),
    });

    EXPECT_EQ(decompileEvt(events, {"", "Door"}, false).text(),
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
    EvtProgram events = program({
        op(30, 0, EVENT_MouseOver, [](EvtInstruction &ir) { ir.data.text_id = 1; }),
        op(30, 0, EVENT_Exit),
        op(31, 0, EVENT_MouseOver, [](EvtInstruction &ir) { ir.data.text_id = 1; }),
        op(31, 0, EVENT_OnMapReload),
        call(31, 1, "Add", {GOLD, int64_t(1)}),
        op(31, 2, EVENT_Exit),
        op(32, 0, EVENT_MouseOver, [](EvtInstruction &ir) { ir.data.text_id = 9; }), // There is no such string, and the engine shows an empty hint then.
        op(32, 0, EVENT_Exit),
        op(33, 0, EVENT_MouseOver, [](EvtInstruction &ir) { ir.data.text_id = 1; }),
        call(33, 0, "EnterHouse", {int64_t(2)}),
        op(33, 1, EVENT_Exit),
        op(34, 0, EVENT_MouseOver, [](EvtInstruction &ir) { ir.data.text_id = 1; }),
        call(34, 0, "EnterHouse", {int64_t(9999)}), // There is no such house, and the engine keeps the hint then.
        op(34, 1, EVENT_Exit),
    });

    EXPECT_EQ(decompileEvt(events, {"", "Door"}, false).text(),
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
    EvtProgram events = program({
        op(40, 0, EVENT_OnTimer, [](EvtInstruction &ir) { ir.data.timer_descr = {0, 0, 0, 1, 0, 0, 0}; }),
        call(40, 1, "Add", {GOLD, int64_t(1)}),
        op(41, 0, EVENT_OnLongTimer, [](EvtInstruction &ir) { ir.data.timer_descr = {0, 0, 0, 0, 0, 1, 0}; }),
        call(41, 1, "Add", {GOLD, int64_t(2)}),
        op(42, 0, EVENT_OnLongTimer, [](EvtInstruction &ir) { ir.data.timer_descr = {0, 0, 1, 0, 0, 0, 0}; }),
        call(42, 1, "Add", {GOLD, int64_t(3)}),
        op(43, 0, EVENT_OnLongTimer, [](EvtInstruction &ir) { ir.data.timer_descr = {0, 0, 0, 0, 0, 0, 2}; }),
        call(43, 1, "Add", {GOLD, int64_t(4)}),
    });

    EXPECT_EQ(decompileEvt(events, {}, false).text(),
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
    EvtProgram events = program({
        call(50, 0, "ShowMovie", {int64_t(1), int64_t(1), "\"family reunion\" "s}),
        call(50, 1, "StatusText", {int64_t(1)}),
        op(50, 2, EVENT_Exit),
    });

    EXPECT_EQ(decompileEvt(events, {"", "a\nb\\c"}, false).text(),
              "local TXT = Localize{\n"
              "    [1] = \"a\\nb\\\\c\",\n"
              "}\n"
              "table.copy(TXT, evt.str, true)\n"
              "\n"
              "Game.MapEvtLines.Count = 0\n"
              "\n"
              "evt.map[50] = function()\n"
              "    evt.ShowMovie{ExitCurrentScreen = true, Name = \"\\\"family reunion\\\" \"}\n"
              "    evt.StatusText(1)  -- \"a\\nb\\\\c\"\n"
              "end\n"
              "\n");
}

GAME_TEST(EvtDecompiler, Unsupported) {
    // The interpreter ends an event on a question, and doesn't run a command it doesn't support.
    EvtProgram events = program({
        op(60, 0, EVENT_InputString, [](EvtInstruction &ir) { ir.data.text_id = 1; }),
        call(60, 1, "Add", {GOLD, int64_t(1)}),
        op(60, 2, EVENT_Exit),
        op(61, 0, EVENT_SetActorGroup),
        op(61, 1, EVENT_Exit),
    });

    EXPECT_EQ(decompileEvt(events, {}, false).text(),
              "Game.MapEvtLines.Count = 0\n"
              "\n"
              "evt.map[60] = function()\n"
              "    -- EVENT_InputString isn't supported by OpenEnroth.\n"
              "end\n"
              "\n"
              "evt.map[61] = function()\n"
              "    -- EVENT_SetActorGroup isn't supported by OpenEnroth.\n"
              "end\n"
              "\n");
}
