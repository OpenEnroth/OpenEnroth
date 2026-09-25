#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Testing/Game/GameTest.h"

#include "Engine/Engine.h"
#include "Engine/Evt/EvtCommands.h"
#include "Engine/Evt/EvtProgram.h"
#include "Engine/MapEnumFunctions.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Tables/MapTable.h"

using namespace std::string_literals; // NOLINT: for the string values of the commands.

static std::vector<std::string> evtFileNames() {
    std::vector<std::string> result = {"global"};
    for (MapId map : allMaps()) {
        const std::string &fileName = pMapTable->pInfos[map].fileName;
        result.push_back(fileName.substr(0, fileName.rfind('.')));
    }
    return result;
}

static EvtInstruction instruction(std::string_view name, std::vector<EvtFieldValue> values) {
    const EvtCommandInfo *command = evtCommand(name);
    EvtInstruction result = {};
    result.opcode = command->opcode;
    for (size_t i = 0; i < values.size(); i++)
        if (command->fields[i].set)
            command->fields[i].set(&result, values[i]);
    return result;
}

GAME_TEST(EvtCommands, Table) {
    // Scripts call commands by name, and each field has to be both readable and writable or neither.
    std::set<std::string_view> names;
    for (const EvtCommandInfo &command : evtCommands()) {
        EXPECT_TRUE(names.insert(command.name).second) << command.name;
        EXPECT_FALSE(command.fields.empty()) << command.name;
        for (const EvtFieldInfo &field : command.fields)
            EXPECT_EQ(static_cast<bool>(field.get), static_cast<bool>(field.set)) << command.name << "." << field.name;
    }
    EXPECT_TRUE(std::ranges::is_sorted(evtCommands(), std::ranges::less(), &EvtCommandInfo::opcode));
}

GAME_TEST(EvtCommands, Instruction) {
    // The fields of a command have to land in the members of the instruction that the interpreter reads.
    EvtInstruction cmp = instruction("Cmp", {int64_t(std::to_underlying(*evtVariableByName("QBits"))), int64_t(240)});
    EXPECT_EQ(cmp.opcode, EVENT_Compare);
    EXPECT_EQ(cmp.data.variable_descr.type, VAR_QBits_QuestsDone);
    EXPECT_EQ(cmp.data.variable_descr.value, 240);

    EvtInstruction sprite = instruction("SetSprite", {int64_t(20), int64_t(1), "dec05"s});
    EXPECT_EQ(sprite.data.sprite_texture_descr.cog, 20);
    EXPECT_EQ(sprite.data.sprite_texture_descr.hide, 1);
    EXPECT_EQ(sprite.str, "dec05");

    EvtInstruction spell = instruction("CastSpell", {int64_t(6), int64_t(4), int64_t(10)});
    EXPECT_EQ(spell.data.spell_descr.spell_mastery, MASTERY_GRANDMASTER);

    EvtInstruction damage = instruction("DamagePlayer", {int64_t(std::to_underlying(CHOOSE_PARTY)), int64_t(0), int64_t(50)});
    EXPECT_EQ(damage.who, CHOOSE_PARTY);
    EXPECT_EQ(damage.data.damage_descr.damage, 50);
}

GAME_TEST(EvtCommands, SetErrors) {
    // A value that a field can't hold must not get into an instruction. A script's typo would reach the interpreter.
    int64_t gold = std::to_underlying(VAR_FixedGold);
    EXPECT_NO_THROW((void) instruction("Add", {gold, int64_t(-1000)}));
    EXPECT_ANY_THROW((void) instruction("Add", {gold, int64_t(3000000000)}));
    EXPECT_NO_THROW((void) instruction("SetFacetBit", {int64_t(1), int64_t(0x80000000), int64_t(1)})); // A bit mask is unsigned.
    EXPECT_ANY_THROW((void) instruction("SetFacetBit", {int64_t(1), int64_t(-1), int64_t(1)}));
    EXPECT_ANY_THROW((void) instruction("SetFacetBit", {int64_t(1), int64_t(1), int64_t(2)})); // On is a bool.
    EXPECT_ANY_THROW((void) instruction("DamagePlayer", {int64_t(7)}));
    EXPECT_ANY_THROW((void) instruction("CastSpell", {int64_t(6), int64_t(5)})); // There is no mastery above grandmaster.
    EXPECT_ANY_THROW((void) instruction("CastSpell", {int64_t(6), int64_t(0)}));
    EXPECT_ANY_THROW((void) instruction("SetDoorState", {"door"s}));
    EXPECT_ANY_THROW((void) instruction("SetSprite", {int64_t(20), int64_t(1), int64_t(5)}));
    EXPECT_NO_THROW((void) instruction("SpeakNPC", {int64_t(500)}));
    EXPECT_ANY_THROW((void) instruction("SpeakNPC", {int64_t(501)}));
    EXPECT_ANY_THROW((void) instruction("SetNPCGroupNews", {int64_t(51), int64_t(1)}));
}

GAME_TEST(EvtCommands, Names) {
    // Scripts name variables and players the way MMExtension does.
    EXPECT_EQ(evtVariableByName("Inventory"), VAR_PlayerItemInHands);
    EXPECT_EQ(evtVariableByName("Items"), VAR_PlayerItemInHands);
    EXPECT_EQ(evtVariableName(VAR_PlayerItemInHands), "Inventory");
    EXPECT_EQ(evtVariableName(VAR_MapPersistentVariable_0), "MapVar0");
    EXPECT_EQ(evtVariableName(VAR_MapPersistentDecorVariable_24), "MapVar99");
    EXPECT_EQ(evtVariableName(VAR_History_0), "History1");
    EXPECT_EQ(evtVariableName(VAR_History_28), "History29");
    EXPECT_EQ(evtVariableName(VAR_ItemEquipped), "IsWearingItem");
    EXPECT_EQ(evtVariableByName("NoSuchVariable"), std::nullopt);
    EXPECT_EQ(evtPlayerByName("All"), CHOOSE_PARTY);
    EXPECT_EQ(evtCommand("Sub"), evtCommand(EVENT_Subtract));

    for (int variable = 0; variable < 0x200; variable++)
        if (std::string name = evtVariableName(static_cast<EvtVariable>(variable)); !name.empty())
            EXPECT_EQ(evtVariableByName(name), static_cast<EvtVariable>(variable)) << name;
}

GAME_TEST(EvtCommands, GameFilesRoundTrip) {
    // Every field that the interpreter reads from a command of the game's evt files has to be one scripts can set.
    for (const std::string &name : evtFileNames()) {
        EvtProgram program = EvtProgram::load(engine->resources()->eventsData(name + ".evt"));
        for (int eventId : program.eventIds()) {
            for (const EvtInstruction &ir : program.function(eventId)) {
                const EvtCommandInfo *command = evtCommand(ir.opcode);
                if (!command)
                    continue;

                EvtInstruction copy = {};
                copy.opcode = ir.opcode;
                copy.step = ir.step;
                copy.target_step = ir.target_step;
                for (const EvtFieldInfo &field : command->fields)
                    if (field.get)
                        field.set(&copy, field.get(ir));
                EXPECT_EQ(copy.toString(), ir.toString()) << name << ".evt, event " << eventId;
                EXPECT_EQ(copy.str, ir.str) << name << ".evt, event " << eventId;
                EXPECT_EQ(copy.who, ir.who) << name << ".evt, event " << eventId;
            }
        }
    }
}
