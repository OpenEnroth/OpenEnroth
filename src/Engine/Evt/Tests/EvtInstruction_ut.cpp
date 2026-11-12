#include <string>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Evt/EvtEnums.h"
#include "Engine/Evt/EvtInstruction.h"

#include "Library/Serialization/Serialization.h"

#include "Utility/Streams/MemoryInputStream.h"

static EvtInstruction parse(EvtOpcode opcode, std::vector<uint8_t> payload, uint8_t step = 0) {
    // An evt record without its size and event id, which `EvtProgram::load` reads.
    payload.insert(payload.begin(), {step, std::to_underlying(opcode)});
    MemoryInputStream stream(payload.data(), payload.size());
    return EvtInstruction::parse(stream);
}

UNIT_TEST(EvtInstruction, ParseFields) {
    // Numbers are little endian, a string runs up to its zero, and a byte that OpenEnroth doesn't keep is skipped.
    EvtInstruction sprite = parse(EVENT_SetSprite, {20, 1, 0, 0, 1, 'd', 'e', 'c', '0', '5', 0}, 3);
    EXPECT_EQ(sprite.opcode, EVENT_SetSprite);
    EXPECT_EQ(sprite.step, 3);
    EXPECT_EQ(sprite.data.sprite_texture_descr.cog, 276);
    EXPECT_EQ(sprite.data.sprite_texture_descr.hide, 1);
    EXPECT_EQ(sprite.str, "dec05");

    EvtInstruction movie = parse(EVENT_ShowMovie, {1, 1, 'a', 0});
    EXPECT_EQ(movie.data.movie_unknown_field, 1);
    EXPECT_EQ(movie.str, "a");

    EvtInstruction compare = parse(EVENT_Compare, {0x10, 0, 240, 0, 0, 0, 7});
    EXPECT_EQ(compare.data.variable_descr.type, VAR_QBits_QuestsDone);
    EXPECT_EQ(compare.data.variable_descr.value, 240);
    EXPECT_EQ(compare.target_step, 7);

    EvtInstruction add = parse(EVENT_Add, {0x15, 0, 0xFF, 0xFF, 0xFF, 0xFF});
    EXPECT_EQ(add.data.variable_descr.value, -1);

    EvtInstruction damage = parse(EVENT_ReceiveDamage, {5, 2, 50, 0, 0, 0});
    EXPECT_EQ(damage.who, CHOOSE_PARTY);
    EXPECT_EQ(damage.data.damage_descr.damage, 50);
}

UNIT_TEST(EvtInstruction, ParseMasteries) {
    // A spell's mastery is stored from 0 for novice. A skill check's is stored as 0 for any mastery, then from 1 for
    // expert.
    auto spell = [](uint8_t mastery) {
        std::vector<uint8_t> payload(27);
        payload[1] = mastery;
        return parse(EVENT_CastSpell, payload).data.spell_descr.spell_mastery;
    };
    EXPECT_EQ(spell(0), MASTERY_NOVICE);
    EXPECT_EQ(spell(3), MASTERY_GRANDMASTER);

    auto check = [](uint8_t mastery) {
        return parse(EVENT_CheckSkill, {0, mastery, 4, 0, 0, 0, 9}).data.check_skill_descr.skill_mastery;
    };
    EXPECT_EQ(check(0), MASTERY_NONE);
    EXPECT_EQ(check(1), MASTERY_EXPERT);
    EXPECT_EQ(check(3), MASTERY_GRANDMASTER);
}

UNIT_TEST(EvtInstruction, ParseKillCheck) {
    // Monster ids are stored from 0 and kept from 1. Groups and counts are kept as stored.
    EvtInstruction byMonster = parse(EVENT_IsActorKilled, {std::to_underlying(KILL_CHECK_MONSTERID), 10, 0, 0, 0, 2, 5});
    EXPECT_EQ(byMonster.data.actor_descr.param, 11);
    EXPECT_EQ(byMonster.data.actor_descr.num, 2);
    EXPECT_EQ(byMonster.target_step, 5);

    EvtInstruction byGroup = parse(EVENT_IsActorKilled, {std::to_underlying(KILL_CHECK_GROUPID), 10, 0, 0, 0, 2, 5});
    EXPECT_EQ(byGroup.data.actor_descr.param, 10);
}

UNIT_TEST(EvtInstruction, ParseRandomGoTo) {
    // A random jump has six slots, and the ones that hold a zero aren't targets.
    EvtInstruction jump = parse(EVENT_RandomGoTo, {4, 8, 12, 0, 0, 0});
    EXPECT_EQ(jump.data.random_goto_descr.random_goto_len, 3);
    EXPECT_EQ(jump.data.random_goto_descr.random_goto[2], 12);

    EXPECT_ANY_THROW((void) parse(EVENT_RandomGoTo, {0, 0, 0, 0, 0, 0}));
}

UNIT_TEST(EvtInstruction, ParseHints) {
    // A hint is stored with the step of the command that follows it, and gets a step that no command has.
    EvtInstruction hint = parse(EVENT_MouseOver, {7}, 0);
    EXPECT_EQ(hint.step, -1);
    EXPECT_EQ(hint.data.text_id, 7);
}

UNIT_TEST(EvtInstruction, ParseErrors) {
    // A record that is shorter or longer than its fields is malformed, and so is one that nobody knows the fields of.
    EXPECT_NO_THROW((void) parse(EVENT_OpenChest, {3}));
    EXPECT_ANY_THROW((void) parse(EVENT_OpenChest, {}));
    EXPECT_ANY_THROW((void) parse(EVENT_OpenChest, {3, 0}));
    EXPECT_ANY_THROW((void) parse(EVENT_SetTexture, {1, 0, 0}));
    EXPECT_ANY_THROW((void) parse(EVENT_CheckItemsCount, {0}));
    EXPECT_ANY_THROW((void) parse(static_cast<EvtOpcode>(200), {0}));
}

UNIT_TEST(EvtInstruction, ToString) {
    // The log shows an instruction as its step, its opcode and its fields by name, with variables by name too.
    EXPECT_EQ(parse(EVENT_SetSprite, {20, 0, 0, 0, 1, 'd', 'e', 'c', '0', '5', 0}, 3).toString(EVT_SOURCE_MAP),
              "3: SetSprite{SpriteId = 20, Visible = 1, Name = \"dec05\"}");
    EXPECT_EQ(parse(EVENT_Compare, {0x10, 0, 240, 0, 0, 0, 7}).toString(EVT_SOURCE_MAP), "0: Compare{VarNum = \"QBits\", Value = 240, Jump = 7}");
    EXPECT_EQ(parse(EVENT_Exit, {0}, 9).toString(EVT_SOURCE_MAP), "9: Exit{}");
    EXPECT_EQ(parse(EVENT_SetFacesBit, {1, 0, 0, 0, 0, 0, 0, 0x20, 1}).toString(EVT_SOURCE_MAP), "0: SetFacesBit{Id = 1, Bit = 0x20000000, On = 1}");
    EXPECT_EQ(parse(EVENT_RandomGoTo, {4, 8, 0, 0, 0, 0}).toString(EVT_SOURCE_MAP), "0: RandomGoTo{Jumps = {4, 8, 0, 0, 0, 0}}");
}

UNIT_TEST(EvtEnums, VariableNames) {
    // Variables go by MMExtension's names, and the numbered ones are counted the way MMExtension counts them.
    EXPECT_EQ(toString(VAR_QBits_QuestsDone), "QBits");
    EXPECT_EQ(toString(VAR_MapPersistentVariable_0), "MapVar0");
    EXPECT_EQ(toString(VAR_MapPersistentDecorVariable_24), "MapVar99");
    EXPECT_EQ(toString(VAR_Counter10), "Counter10");
    EXPECT_EQ(toString(VAR_History_0), "History1");
    EXPECT_EQ(toString(VAR_UnknownTimeEvent19), "SpecialDate20");

    EXPECT_EQ(fromString<EvtVariable>("Gold"), VAR_FixedGold);
    EXPECT_EQ(fromString<EvtVariable>("MapVar75"), VAR_MapPersistentDecorVariable_0);
    EXPECT_EQ(fromString<EvtVariable>("History29"), VAR_History_28);

    EvtVariable variable = VAR_Sex;
    std::string name;
    EXPECT_FALSE(trySerialize(VAR_Race, &name));
    EXPECT_FALSE(tryDeserialize("MapVar100", &variable));
    EXPECT_FALSE(tryDeserialize("MapVar01", &variable));
    EXPECT_FALSE(tryDeserialize("History0", &variable));
    EXPECT_FALSE(tryDeserialize("qbits", &variable));
}
