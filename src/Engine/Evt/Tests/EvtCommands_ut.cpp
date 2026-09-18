#include <string>
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
    record.values = {int64_t(20), int64_t(1), "dec05"s};

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
    record.values = {int64_t(std::to_underlying(*evtVariableByName("QBits"))), int64_t(240), int64_t(7)};

    EvtInstruction ir = evtInstruction(record);
    EXPECT_EQ(ir.opcode, EVENT_Compare);
    EXPECT_EQ(ir.data.variable_descr.type, VAR_QBits_QuestsDone);
    EXPECT_EQ(ir.data.variable_descr.value, 240);
    EXPECT_EQ(ir.target_step, 7);

    record.values = {int64_t(0), int64_t(240)}; // The jump is missing.
    EXPECT_ANY_THROW((void) evtInstruction(record));
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
}

GAME_TEST(EvtCommands, GameFilesRoundTrip) {
    // The command table has to describe every record of every evt file of the game, down to the byte.
    for (const std::string &name : evtFileNames()) {
        Blob data = engine->resources()->eventsData(name + ".evt");

        std::string encoded;
        for (const EvtRecord &record : decodeEvtRecords(data)) {
            ASSERT_NE(evtCommand(record.opcode), nullptr) << name << ".evt, event " << record.eventId;
            ASSERT_EQ(record.values.size(), evtCommand(record.opcode)->fields.size()) << name << ".evt, event " << record.eventId;
            encoded += encodeEvtRecord(record);
        }
        EXPECT_EQ(encoded, data.str()) << name << ".evt";
    }
}
