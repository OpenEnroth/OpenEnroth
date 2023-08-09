#include "CharacterTapeRecorder.h"

#include "Engine/Party.h"

template<class Member, class... Args>
static auto bind(Member member, Args... args) {
    return [member, ...args = std::move(args)] (const Character &character) {
        return std::invoke(member, character, args...);
    };
}

template<class T, class Member, class... Args>
static auto bindAs(Member member, Args... args) {
    return [base = bind(member, std::move(args)...)] (const Character &character) {
        return static_cast<T>(base(character));
    };
}

CharacterTapeRecorder::CharacterTapeRecorder(TestController *controller) : _controller(controller) {
    assert(controller);
}

std::span<Character> CharacterTapeRecorder::characters() {
    return pParty->pCharacters;
}

TestTape<int64_t> CharacterTapeRecorder::experience(int characterIndex) {
    return custom(characterIndex, bindAs<int64_t>(&Character::experience));
}

TestMultiTape<int64_t> CharacterTapeRecorder::experiences() {
    return custom(bindAs<int64_t>(&Character::experience));
}

TestTape<CharacterExpressionID> CharacterTapeRecorder::expression(int characterIndex) {
    return custom(characterIndex, bind(&Character::expression));
}

TestMultiTape<CharacterExpressionID> CharacterTapeRecorder::expressions() {
    return custom(bind(&Character::expression));
}

TestTape<int> CharacterTapeRecorder::hp(int characterIndex) {
    return custom(characterIndex, bind(&Character::health));
}

TestMultiTape<int> CharacterTapeRecorder::hps() {
    return custom(bind(&Character::health));
}

TestTape<int> CharacterTapeRecorder::mp(int characterIndex) {
    return custom(characterIndex, bind(&Character::mana));
}

TestMultiTape<int> CharacterTapeRecorder::mps() {
    return custom(bind(&Character::mana));
}

TestTape<int> CharacterTapeRecorder::level(int characterIndex) {
    return custom(characterIndex, bind(&Character::GetActualLevel));
}

TestMultiTape<int> CharacterTapeRecorder::levels() {
    return custom(bind(&Character::GetActualLevel));
}

TestMultiTape<int> CharacterTapeRecorder::skillLevels(CharacterSkillType skill) {
    return custom(bind(&Character::actualSkillLevel, skill));
}

TestMultiTape<Condition> CharacterTapeRecorder::conditions() {
    return custom(bind(&Character::GetMajorConditionIdx));
}

TestMultiTape<int> CharacterTapeRecorder::resistances(CharacterAttributeType resistance) {
    return custom(bind(&Character::GetActualResistance, resistance));
}
