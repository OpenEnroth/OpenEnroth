#include "CharacterTapeRecorder.h"

#include <cassert>
#include <functional>

#include "Engine/Party.h"

#include "Utility/Lambda.h"

CharacterTapeRecorder::CharacterTapeRecorder(TestController *controller) : _controller(controller) {
    assert(controller);
}

std::span<Character> CharacterTapeRecorder::characters() {
    return pParty->pCharacters;
}

TestTape<int64_t> CharacterTapeRecorder::experience(int characterIndex) {
    return custom(characterIndex, std::bind<int64_t>(&Character::experience, _1));
}

TestMultiTape<int64_t> CharacterTapeRecorder::experiences() {
    return custom(std::bind<int64_t>(&Character::experience, _1));
}

TestTape<PortraitId> CharacterTapeRecorder::portrait(int characterIndex) {
    return custom(characterIndex, std::bind(&Character::portrait, _1));
}

TestMultiTape<PortraitId> CharacterTapeRecorder::portraits() {
    return custom(std::bind(&Character::portrait, _1));
}

TestTape<int> CharacterTapeRecorder::hp(int characterIndex) {
    return custom(characterIndex, std::bind(&Character::health, _1));
}

TestMultiTape<int> CharacterTapeRecorder::hps() {
    return custom(std::bind(&Character::health, _1));
}

TestTape<int> CharacterTapeRecorder::mp(int characterIndex) {
    return custom(characterIndex, std::bind(&Character::mana, _1));
}

TestMultiTape<int> CharacterTapeRecorder::mps() {
    return custom(std::bind(&Character::mana, _1));
}

TestTape<int> CharacterTapeRecorder::ac(int characterIndex) {
    return custom(characterIndex, std::bind(&Character::GetActualAC, _1));
}

TestMultiTape<int> CharacterTapeRecorder::acs() {
    return custom(std::bind(&Character::GetActualAC, _1));
}

TestTape<int> CharacterTapeRecorder::level(int characterIndex) {
    return custom(characterIndex, std::bind(&Character::GetActualLevel, _1));
}

TestMultiTape<int> CharacterTapeRecorder::levels() {
    return custom(std::bind(&Character::GetActualLevel, _1));
}

TestTape<int> CharacterTapeRecorder::skillLevel(int characterIndex, Skill skill) {
    return custom(characterIndex, std::bind(&Character::actualSkillLevel, _1, skill));
}

TestMultiTape<int> CharacterTapeRecorder::skillLevels(Skill skill) {
    return custom(std::bind(&Character::actualSkillLevel, _1, skill));
}

TestTape<bool> CharacterTapeRecorder::hasSkill(int characterIndex, Skill skill) {
    return custom(characterIndex, std::bind(&Character::HasSkill, _1, skill));
}

TestMultiTape<bool> CharacterTapeRecorder::haveSkills(Skill skill) {
    return custom(std::bind(&Character::HasSkill, _1, skill));
}

TestTape<Condition> CharacterTapeRecorder::condition(int characterIndex) {
    return custom(characterIndex, std::bind(&Character::GetMajorConditionIdx, _1));
}

TestMultiTape<Condition> CharacterTapeRecorder::conditions() {
    return custom(std::bind(&Character::GetMajorConditionIdx, _1));
}

TestTape<int> CharacterTapeRecorder::resistance(int characterIndex, Attribute resistance) {
    return custom(characterIndex, std::bind(&Character::GetActualResistance, _1, resistance));
}

TestMultiTape<int> CharacterTapeRecorder::resistances(Attribute resistance) {
    return custom(std::bind(&Character::GetActualResistance, _1, resistance));
}

TestTape<int> CharacterTapeRecorder::stat(int characterIndex, Attribute stat) {
    return custom(characterIndex, std::bind(&Character::GetActualStat, _1, stat));
}

TestMultiTape<int> CharacterTapeRecorder::stats(Attribute stat) {
    return custom(std::bind(&Character::GetActualStat, _1, stat));
}

TestTape<bool> CharacterTapeRecorder::hasBuff(int characterIndex, CharacterBuff buff) {
    return custom(characterIndex, [=](const Character &character) { return character.pCharacterBuffs[buff].Active(); });
}

TestMultiTape<bool> CharacterTapeRecorder::haveBuffs(CharacterBuff buff) {
    return custom([=](const Character &character) { return character.pCharacterBuffs[buff].Active(); });
}

TestTape<SpellId> CharacterTapeRecorder::quickSpell(int characterIndex) {
    return custom(characterIndex, std::bind(&Character::uQuickSpell, _1));
}

TestMultiTape<SpellId> CharacterTapeRecorder::quickSpells() {
    return custom(std::bind(&Character::uQuickSpell, _1));
}

TestTape<bool> CharacterTapeRecorder::hasItem(int characterIndex, ItemId itemId) {
    return custom(characterIndex, [=](const Character &character) { return !!character.inventory.find(itemId); });
}

TestMultiTape<bool> CharacterTapeRecorder::haveItem(ItemId itemId) {
    return custom([=](const Character &character) { return !!character.inventory.find(itemId); });
}

TestTape<Class> CharacterTapeRecorder::clazz(int characterIndex) {
    return custom(characterIndex, std::bind(&Character::classType, _1));
}

TestMultiTape<Class> CharacterTapeRecorder::classes() {
    return custom(std::bind(&Character::classType, _1));
}

TestTape<bool> CharacterTapeRecorder::isRecovering(int characterIndex) {
    return custom(characterIndex, [] (const Character &character) { return character.timeToRecovery > 0_ticks; });
}

TestMultiTape<bool> CharacterTapeRecorder::areRecovering() {
    return custom([] (const Character &character) { return character.timeToRecovery > 0_ticks; });
}
