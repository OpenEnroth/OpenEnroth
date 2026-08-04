#include <string>
#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Engine/mm7text_ru.h"

#include "Utility/String/Encoding.h"

// sprintfex operates on Windows-1251 strings, and this file is UTF-8.
static std::string ru(std::string_view utf8) {
    return txt::utf8ToEncoded(utf8, ENCODING_WINDOWS_1251);
}

UNIT_TEST(Mm7TextRu, PassThrough) {
    EXPECT_EQ(sprintfex(""), "");
    EXPECT_EQ(sprintfex("no tokens here"), "no tokens here");
    EXPECT_EQ(sprintfex(ru("Привет, мир!")), ru("Привет, мир!"));
}

UNIT_TEST(Mm7TextRu, Plurals) {
    EXPECT_EQ(sprintfex(ru("^I[1] д^L[ень;ня;ней]")), ru("1 день"));
    EXPECT_EQ(sprintfex(ru("^I[2] д^L[ень;ня;ней]")), ru("2 дня"));
    EXPECT_EQ(sprintfex(ru("^I[5] д^L[ень;ня;ней]")), ru("5 дней"));
    EXPECT_EQ(sprintfex(ru("^I[21] д^L[ень;ня;ней]")), ru("21 день"));
    EXPECT_EQ(sprintfex(ru("^I[100] д^L[ень;ня;ней]")), ru("100 дней"));

    // The original mm7text.dll only looked at the last digit and would produce "11 день" here.
    EXPECT_EQ(sprintfex(ru("^I[11] д^L[ень;ня;ней]")), ru("11 дней"));
    EXPECT_EQ(sprintfex(ru("^I[14] д^L[ень;ня;ней]")), ru("14 дней"));
}

UNIT_TEST(Mm7TextRu, NumberedPlurals) {
    // An actual string from the Buka global.txt, "Buy food for %d days for %d gold".
    EXPECT_EQ(sprintfex(ru("Купить еды на ^I[3] д^L1[ень;ня;ней] за ^I[22] золот^L2[ой;ых;ых]")),
              ru("Купить еды на 3 дня за 22 золотых"));
    EXPECT_EQ(sprintfex(ru("Купить еды на ^I[1] д^L1[ень;ня;ней] за ^I[5] золот^L2[ой;ых;ых]")),
              ru("Купить еды на 1 день за 5 золотых"));
}

UNIT_TEST(Mm7TextRu, Gender) {
    // Capitalized names come from one table...
    EXPECT_EQ(sprintfex(ru("^Pi[Анна] - учитель^R[;ница;]")), ru("Анна - учительница"));
    EXPECT_EQ(sprintfex(ru("^Pi[Адам] - учитель^R[;ница;]")), ru("Адам - учитель"));

    // ...common nouns from another.
    EXPECT_EQ(sprintfex(ru("^Pi[гидра] был^R[;а;о] здесь")), ru("гидра была здесь"));
    EXPECT_EQ(sprintfex(ru("^Pi[голем] был^R[;а;о] здесь")), ru("голем был здесь"));
    EXPECT_EQ(sprintfex(ru("^Pi[училище] закрыт^R[;а;о]")), ru("училище закрыто"));

    // Same expansion as NameAndTitle() does for a Buka LSTR_S_THE_S of "^Pi[%s] %s".
    EXPECT_EQ(sprintfex(ru("^Pi[Валерия] портн^R[ой;иха;]")), ru("Валерия портниха"));

    // Unknown names default to masculine.
    EXPECT_EQ(sprintfex(ru("^Pi[Xyzzy] боец^R[; WAT;]")), ru("Xyzzy боец"));
}

UNIT_TEST(Mm7TextRu, SpecialNames) {
    EXPECT_EQ(sprintfex(ru("Вы вошли в ^Pv[Врата в Бездну]")), ru("Вы вошли в Врат в Бездну"));
    EXPECT_EQ(sprintfex(ru("к ^Pd[Врата в Бездну]")), ru("к Вратам в Бездну"));
    EXPECT_EQ(sprintfex(ru("о ^Pp[Стены тумана]")), ru("о Стенах тумана"));
    EXPECT_EQ(sprintfex(ru("^Pi[Мэри Джо] ушл^R[;а;о]")), ru("Мэри Джо ушла"));
}

UNIT_TEST(Mm7TextRu, Malformed) {
    // Malformed tokens are copied through verbatim, not a reason to crash.
    EXPECT_EQ(sprintfex("^"), "^");
    EXPECT_EQ(sprintfex("100^500"), "100^500");
    EXPECT_EQ(sprintfex("^I[unclosed"), "^I[unclosed");
    EXPECT_EQ(sprintfex("^Z[what]"), "^Z[what]");
    EXPECT_EQ(sprintfex(ru("^L[только;две]")), ru("^L[только;две]"));
    EXPECT_EQ(sprintfex("^L9[a;b;c]"), "^L9[a;b;c]"); // No ^I tokens before it.
}
