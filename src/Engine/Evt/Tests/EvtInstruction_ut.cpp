#include <utility>
#include <string>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Evt/EvtInstruction.h"
#include "Engine/GameType.h"
#include "Utility/Streams/MemoryInputStream.h"

// Every real MM8 `EVENT_MoveToMap` instruction carries one extra byte between
// `exit_pic_id` and the null-terminated destination string that MM7's layout doesn't have -
// confirmed by scanning all 61 real MM8 maps' `.evt` data (values seen: 0, 1, 3). Left unconsumed,
// that byte becomes `str`'s leading character, so `MapTable::GetMapInfo`'s exact filename match
// can never succeed for a real door/decoration transition. This builds a raw record byte-for-byte
// (step, opcode, the 26-byte header, the extra byte, then the string) instead of going through
// `EvtProgram::load`/a real `.evt` file, so the parser itself is pinned independently of any real
// game data being present.
static EvtInstruction parseMoveToMap(GameType gameType, const std::vector<uint8_t> &tail, uint8_t houseLow = 0, uint8_t houseHigh = 0) {
    setGameType(gameType);

    std::vector<uint8_t> record = {
        0,                                   // step
        static_cast<uint8_t>(EVENT_MoveToMap), // opcode
        0, 0, 0, 0,                          // x
        0, 0, 0, 0,                          // y
        0, 0, 0, 0,                          // z
        0, 0, 0, 0,                          // yaw
        0, 0, 0, 0,                          // pitch
        0, 0, 0, 0,                          // zspeed
        houseLow,                            // house_id, in MM8 its low byte
        houseHigh,                           // exit_pic_id, in MM8 the high byte of house_id
    };
    record.insert(record.end(), tail.begin(), tail.end());

    // +3 for the size byte and eventId `EvtProgram::load` would otherwise have consumed before
    // calling `parse` - `requireSize` only checks a lower bound, so the exact value doesn't matter
    // as long as it's at least that much bigger than `record.size()`.
    MemoryInputStream stream(record.data(), record.size());
    EvtInstruction ir = EvtInstruction::parse(stream, record.size() + 3);

    setGameType(GAME_TYPE_MM7); // Restore the default so other tests aren't affected.
    return ir;
}

UNIT_TEST(EvtInstruction, MoveToMapMm8ConsumesLeadingByteBeforeDestinationString) {
    EvtInstruction ir = parseMoveToMap(GAME_TYPE_MM8, {0x01, 'd', '4', '0', '.', 'b', 'l', 'v', 0});

    EXPECT_EQ(ir.data.move_map_descr.mm8_exit_pic_id, 1);
    EXPECT_EQ(ir.str, "d40.blv");
}

UNIT_TEST(EvtInstruction, MoveToMapMm8HandlesNonUnityLeadingByteValue) {
    EvtInstruction ir = parseMoveToMap(GAME_TYPE_MM8, {0x03, 'o', 'u', 't', '1', '3', '.', 'o', 'd', 'm', 0});

    EXPECT_EQ(ir.data.move_map_descr.mm8_exit_pic_id, 3);
    EXPECT_EQ(ir.str, "out13.odm");
}

UNIT_TEST(EvtInstruction, MoveToMapMm7HasNoLeadingByte) {
    EvtInstruction ir = parseMoveToMap(GAME_TYPE_MM7, {'d', '4', '0', '.', 'b', 'l', 'v', 0});

    EXPECT_EQ(ir.data.move_map_descr.mm8_exit_pic_id, 0);
    EXPECT_EQ(ir.str, "d40.blv");
}

UNIT_TEST(EvtInstruction, MoveToMapMm8ReadsATwoByteHouseId) {
    // The Plane Between Planes, event 503: the door of house 455, "Prison of the Lord of Fire".
    EvtInstruction ir = parseMoveToMap(GAME_TYPE_MM8, {0x01, 'd', '3', '7', '.', 'b', 'l', 'v', 0}, 0xC7, 0x01);

    EXPECT_EQ(std::to_underlying(ir.data.move_map_descr.house_id), 455);
    EXPECT_EQ(ir.data.move_map_descr.exit_pic_id, 0);
    EXPECT_EQ(ir.data.move_map_descr.mm8_exit_pic_id, 1);
    EXPECT_EQ(ir.str, "d37.blv");
}
