#pragma once

#include <span>
#include <utility>
#include <type_traits>

#include "Engine/Objects/Character.h"

#include "TestController.h"

class CharacterTapeRecorder {
 public:
    explicit CharacterTapeRecorder(TestController *controller);

    template<class Callback, class T = std::invoke_result_t<Callback, const Character &>>
    TestTape<TestVector<T>> custom(Callback callback) {
        return _controller->recordTape([callback = std::move(callback)] {
            TestVector<T> result;
            for (const Character &character : characters())
                result.push_back(callback(character));
            return result;
        });
    }

    template<class Callback, class T = std::invoke_result_t<Callback, const Character &>>
    TestTape<T> custom(int characterIndex, Callback callback) {
        return _controller->recordTape([characterIndex, callback = std::move(callback)] {
            return callback(characters()[characterIndex]);
        });
    }

    TestTape<int64_t> experience(int characterIndex);
    TestMultiTape<int64_t> experiences();

    TestTape<CharacterExpressionID> expression(int characterIndex);
    TestMultiTape<CharacterExpressionID> expressions();

    TestTape<int> hp(int characterIndex);
    TestMultiTape<int> hps();

    TestTape<int> mp(int characterIndex);
    TestMultiTape<int> mps();

    TestTape<int> level(int characterIndex);
    TestMultiTape<int> levels();

    TestMultiTape<int> skillLevels(CharacterSkillType skill);

    TestMultiTape<Condition> conditions();

    TestMultiTape<int> resistances(CharacterAttributeType resistance);

 private:
    static std::span<Character> characters();

 private:
    TestController *_controller = nullptr;
};
