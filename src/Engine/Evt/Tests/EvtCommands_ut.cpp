#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Testing/Game/GameTest.h"

#include "Engine/Engine.h"
#include "Engine/Evt/EvtCommands.h"
#include "Engine/MapEnumFunctions.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Tables/MapTable.h"

#include "Utility/Memory/Blob.h"

using namespace std::string_literals; // NOLINT: the record bytes below hold zeros.

static std::vector<std::string> evtFileNames() {
    std::vector<std::string> result = {"global"};
    for (MapId map : allMaps()) {
        const std::string &fileName = pMapTable->pInfos[map].fileName;
        result.push_back(fileName.substr(0, fileName.rfind('.')));
    }
    return result;
}

GAME_TEST(EvtCommands, RecordBytes) {
    // A record has to come out as the bytes an evt file holds for it, and come back from them.
    EvtRecord record;
    record.eventId = 376;
    record.step = 2;
    record.opcode = EVENT_SetSprite;
    record.values = std::vector<EvtFieldValue>{int64_t(20), int64_t(1), "dec05"s};

    std::string bytes = encodeEvtRecord(record);
    EXPECT_EQ(bytes, "\x0f\x78\x01\x02\x0d\x14\x00\x00\x00\x01"s "dec05\x00"s);

    std::vector<EvtRecord> decoded = decodeEvtRecords(Blob::view(bytes));
    ASSERT_EQ(decoded.size(), 1);
    EXPECT_EQ(decoded[0].eventId, 376);
    EXPECT_EQ(decoded[0].step, 2);
    EXPECT_EQ(decoded[0].opcode, EVENT_SetSprite);
    EXPECT_EQ(decoded[0].values, record.values);
}

GAME_TEST(EvtCommands, Instruction) {
    // A record has to turn into the instruction that the interpreter runs.
    EvtRecord record;
    record.opcode = EVENT_Compare;
    record.values = std::vector<EvtFieldValue>{int64_t(std::to_underlying(*evtVariableByName("QBits"))), int64_t(240), int64_t(7)};

    EvtInstruction ir = evtInstruction(record);
    EXPECT_EQ(ir.opcode, EVENT_Compare);
    EXPECT_EQ(ir.data.variable_descr.type, VAR_QBits_QuestsDone);
    EXPECT_EQ(ir.data.variable_descr.value, 240);
    EXPECT_EQ(ir.target_step, 7);

    record.values = std::vector<EvtFieldValue>{int64_t(0), int64_t(240)}; // The jump is missing.
    EXPECT_ANY_THROW((void) evtInstruction(record));
}

GAME_TEST(EvtCommands, Table) {
    // Scripts call commands by name, and a condition reports its result through its jump, so it needs exactly one.
    std::set<std::string_view> names;
    for (const EvtCommandInfo &command : evtCommands()) {
        EXPECT_TRUE(names.insert(command.name).second) << command.name;
        EXPECT_FALSE(command.fields.empty()) << command.name;
        if (command.kind == EVT_COMMAND_CONDITION)
            EXPECT_EQ(std::ranges::count(command.fields, EVT_FIELD_JUMP, &EvtFieldInfo::type), 1) << command.name;
    }
    EXPECT_TRUE(std::ranges::is_sorted(evtCommands(), std::ranges::less(), &EvtCommandInfo::opcode));
}

GAME_TEST(EvtCommands, EncodeErrors) {
    // A value that a field can't hold must not get into a record, a script typo would reach the interpreter as it is.
    auto encode = [](EvtOpcode opcode, std::vector<EvtFieldValue> values) {
        EvtRecord record;
        record.opcode = opcode;
        record.values = std::move(values);
        return encodeEvtRecord(record);
    };
    int64_t gold = std::to_underlying(VAR_FixedGold);

    EXPECT_NO_THROW((void) encode(EVENT_Add, {gold, int64_t(-1000)}));
    EXPECT_ANY_THROW((void) encode(EVENT_Add, {gold, int64_t(3000000000)}));
    EXPECT_NO_THROW((void) encode(EVENT_SetFacesBit, {int64_t(1), int64_t(0x80000000), int64_t(1)})); // A bit mask is unsigned.
    EXPECT_ANY_THROW((void) encode(EVENT_ForPartyMember, {int64_t(7)}));
    EXPECT_ANY_THROW((void) encode(EVENT_ChangeDoorState, {int64_t(256), int64_t(0)}));
    EXPECT_ANY_THROW((void) encode(EVENT_ChangeDoorState, {"door"s, int64_t(0)}));
    EXPECT_ANY_THROW((void) encode(EVENT_SetTexture, {int64_t(1), "a\0b"s}));
    EXPECT_ANY_THROW((void) encode(EVENT_SetTexture, {int64_t(1), std::string(300, 'a')}));

    std::vector<EvtFieldValue> spell = {int64_t(6), int64_t(4), int64_t(10), int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(0), int64_t(0)};
    EXPECT_NO_THROW((void) encode(EVENT_CastSpell, spell));
    spell[1] = int64_t(5); // There is no mastery above grandmaster.
    EXPECT_ANY_THROW((void) encode(EVENT_CastSpell, spell));

    EvtRecord record;
    record.opcode = EVENT_Exit;
    record.values = std::vector<EvtFieldValue>{int64_t(0)};
    record.eventId = 0x10000;
    EXPECT_ANY_THROW((void) encodeEvtRecord(record));
}

GAME_TEST(EvtCommands, UnmatchedRecords) {
    // A record that the table can't describe keeps its bytes, and can't be run.
    std::string bytes = "\x04\x01\x00\x00\x24"s // A jump without its target.
                        "\x06\x01\x00\x01\xff\xaa\xbb"s; // No such opcode.

    std::vector<EvtRecord> decoded = decodeEvtRecords(Blob::view(bytes));
    ASSERT_EQ(decoded.size(), 2);
    EXPECT_FALSE(decoded[0].values);
    EXPECT_FALSE(decoded[1].values);
    EXPECT_EQ(decoded[1].payload, "\xaa\xbb");
    EXPECT_EQ(encodeEvtRecord(decoded[0]) + encodeEvtRecord(decoded[1]), bytes);
    EXPECT_ANY_THROW((void) evtInstruction(decoded[0]));
    EXPECT_ANY_THROW((void) evtInstruction(decoded[1]));

    EvtRecord question;
    question.opcode = EVENT_InputString;
    question.values = std::vector<EvtFieldValue>{int64_t(1), int64_t(2), int64_t(3), int64_t(4)};
    EXPECT_NO_THROW((void) encodeEvtRecord(question));
    EXPECT_ANY_THROW((void) evtInstruction(question)); // The interpreter doesn't run it.

    EXPECT_ANY_THROW((void) decodeEvtRecords(Blob::view("\x03\x01\x00\x00"s))); // Too short for a record.
    EXPECT_ANY_THROW((void) decodeEvtRecords(Blob::view("\x05\x01\x00\x00\x01"s))); // Cut off.
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
    // The command table has to describe every record of every evt file of the game, down to the byte.
    for (const std::string &name : evtFileNames()) {
        Blob data = engine->resources()->eventsData(name + ".evt");

        std::string encoded;
        for (const EvtRecord &record : decodeEvtRecords(data)) {
            ASSERT_TRUE(record.values) << name << ".evt, event " << record.eventId;
            encoded += encodeEvtRecord(record);
        }
        EXPECT_EQ(encoded, data.str()) << name << ".evt";
    }
}
