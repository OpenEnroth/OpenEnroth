#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Objects/CombinedSkillValue.h"

#include "Library/Serialization/Serialization.h"

#include "Utility/Exception.h"

UNIT_TEST(CombinedSkillValue, Serialization) {
    // A skill value serializes as the mastery letter followed by the level.
    EXPECT_EQ(toString(CombinedSkillValue(8, MASTERY_MASTER)), "M8");
    EXPECT_EQ(toString(CombinedSkillValue(63, MASTERY_GRANDMASTER)), "G63");
    EXPECT_EQ(fromString<CombinedSkillValue>("N1"), CombinedSkillValue(1, MASTERY_NOVICE));
    EXPECT_EQ(fromString<CombinedSkillValue>("e12"), CombinedSkillValue(12, MASTERY_EXPERT));
    EXPECT_THROW(toString(CombinedSkillValue::none()), Exception);

    for (Mastery mastery : allSkillMasteries())
        for (int level = 1; level <= 63; level++)
            EXPECT_EQ(fromString<CombinedSkillValue>(toString(CombinedSkillValue(level, mastery))), CombinedSkillValue(level, mastery));
}

UNIT_TEST(CombinedSkillValue, DeserializationFailures) {
    // Strings that don't make a valid skill value are rejected, the constructor would assert on them.
    CombinedSkillValue skill;
    for (std::string_view text : {"", "M", "8", "M0", "M64", "M-5", "M5x", "X5", " M5"})
        EXPECT_FALSE(tryDeserialize(text, &skill)) << text;
}
