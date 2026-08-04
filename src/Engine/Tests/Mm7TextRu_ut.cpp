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

    // Negative numbers are numbers too, not a "no ^I yet" marker.
    EXPECT_EQ(sprintfex(ru("^I[-1] д^L[ень;ня;ней]")), ru("-1 день"));
    EXPECT_EQ(sprintfex(ru("^I[-2147483648] д^L[ень;ня;ней]")), ru("-2147483648 дней"));

    // Unparsable ^I contents are printed but don't occupy a number slot, like in the original DLL.
    EXPECT_EQ(sprintfex(ru("^I[?]^I[2] штук^L[а;и;]")), ru("?2 штуки"));
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
    // String 126 of the Buka global.txt. The DLL used the genitive here - "покинуть Врат в Бездну".
    EXPECT_EQ(sprintfex(ru("Вы хотите покинуть ^Pv[Врата в Бездну]?")), ru("Вы хотите покинуть Врата в Бездну?"));
    EXPECT_EQ(sprintfex(ru("Вы не нашли ^Pr[Врата в Бездну]")), ru("Вы не нашли Врат в Бездну"));
    EXPECT_EQ(sprintfex(ru("к ^Pd[Врата в Бездну]")), ru("к Вратам в Бездну"));
    EXPECT_EQ(sprintfex(ru("о ^Pp[Стены тумана]")), ru("о Стенах тумана"));
    EXPECT_EQ(sprintfex(ru("^Pi[Мэри Джо] ушл^R[;а;о]")), ru("Мэри Джо ушла"));

    // A name that merely starts with a special name isn't truncated to it.
    EXPECT_EQ(sprintfex(ru("^Pi[Мэри Джонсон]")), ru("Мэри Джонсон"));
}

UNIT_TEST(Mm7TextRu, PluralBoundaries) {
    EXPECT_EQ(sprintfex(ru("^I[0] д^L[ень;ня;ней]")), ru("0 дней"));
    EXPECT_EQ(sprintfex(ru("^I[22] д^L[ень;ня;ней]")), ru("22 дня"));
    EXPECT_EQ(sprintfex(ru("^I[25] д^L[ень;ня;ней]")), ru("25 дней"));
    EXPECT_EQ(sprintfex(ru("^I[101] д^L[ень;ня;ней]")), ru("101 день"));
    EXPECT_EQ(sprintfex(ru("^I[111] д^L[ень;ня;ней]")), ru("111 дней"));
}

UNIT_TEST(Mm7TextRu, NumberIndexing) {
    // Explicit ^L<n> indices address the n-th recorded number.
    EXPECT_EQ(sprintfex(ru("^I[1] ^I[2] ^I[5] очк^L3[о;а;ов]")), ru("1 2 5 очков"));

    // An index past the recorded numbers means zero, not the last recorded number.
    EXPECT_EQ(sprintfex(ru("^I[1] д^L2[ень;ня;ней]")), ru("1 дней"));

    // An unparsable ^I doesn't occupy a slot, so later explicit indices shift.
    EXPECT_EQ(sprintfex("^I[x]^I[3] ^L1[a;b;c] ^L2[a;b;c]"), "x3 b c");
}

UNIT_TEST(Mm7TextRu, GenderState) {
    // One ^P gender drives all following ^R tokens...
    EXPECT_EQ(sprintfex(ru("^Pi[Анна] пришл^R[;а;о] и ушл^R[;а;о]")), ru("Анна пришла и ушла"));
    // ...until the next ^P overwrites it.
    EXPECT_EQ(sprintfex(ru("^Pi[Адам] и ^Pi[Анна] - последн^R[ий;яя;ее] ушл^R[;а;о]")),
              ru("Адам и Анна - последняя ушла"));

    // Neuter from the capitalized table, not just the common-noun one.
    EXPECT_EQ(sprintfex(ru("^Pi[Солнце] взошл^R[;а;о]")), ru("Солнце взошло"));

    // Unknown Cyrillic names default to masculine, same as Latin ones.
    EXPECT_EQ(sprintfex(ru("^Pi[Зорг] был^R[;а;о] здесь")), ru("Зорг был здесь"));

    // A table entry matches if it starts with the looked-up name, like in the original DLL.
    EXPECT_EQ(sprintfex(ru("^Pi[Валери] - портн^R[ой;иха;]")), ru("Валери - портниха"));

    // The case letter is ignored for regular names - they're printed undeclined.
    // String 164 of the Buka global.txt.
    EXPECT_EQ(sprintfex(ru("Скелет наносит удар ^Pd[Гоблин], отбирая 5 ед. здоровья")),
              ru("Скелет наносит удар Гоблин, отбирая 5 ед. здоровья"));

    // Empty forms and empty names are not an error.
    EXPECT_EQ(sprintfex("^R[;;]"), "");
    EXPECT_EQ(sprintfex("^Pi[]"), "");
    EXPECT_EQ(sprintfex(ru("^Pi[]: ^R[он;она;оно]")), ru(": он"));
}

UNIT_TEST(Mm7TextRu, SpecialNamesDeclension) {
    // The remaining cases of a declined special name - v/r/d are covered in SpecialNames.
    EXPECT_EQ(sprintfex(ru("^Pi[Врата в Бездну] закрыт^R[;а;о]")), ru("Врата в Бездну закрыта"));
    EXPECT_EQ(sprintfex(ru("перед ^Pt[Врата в Бездну]")), ru("перед Вратами в Бездну"));
    EXPECT_EQ(sprintfex(ru("о ^Pp[Врата в Бездну]")), ru("о Вратах в Бездну"));

    // The case letter is ASCII case-insensitive.
    EXPECT_EQ(sprintfex(ru("^PR[Врата в Бездну]")), ru("Врат в Бездну"));

    // Indeclinable special names print the same in every case, and still carry a gender.
    EXPECT_EQ(sprintfex(ru("к ^Pd[Ли Энн]")), ru("к Ли Энн"));
    EXPECT_EQ(sprintfex(ru("^Pi[Ли Энн] ушл^R[;а;о]")), ru("Ли Энн ушла"));

    // A prefix match keeps the tail verbatim and still records the gender...
    EXPECT_EQ(sprintfex(ru("^Pi[Мэри Джонсон] ушл^R[;а;о]")), ru("Мэри Джонсон ушла"));
    // ...and a declined prefix declines while the tail stays as is.
    EXPECT_EQ(sprintfex(ru("^Pd[Врата в Бездну снов]")), ru("Вратам в Бездну снов"));
}

UNIT_TEST(Mm7TextRu, RealData) {
    // String 302 of the Buka global.txt, all four token kinds in one string.
    EXPECT_EQ(sprintfex(ru("^Pi[Анна] украл^R[;а;] ^I[41] золот^L[ой;ых;ых]!")),
              ru("Анна украла 41 золотой!"));

    // String 311, a token in the middle of a word.
    EXPECT_EQ(sprintfex(ru("охотни^R[к;ца;]-следопыт")), ru("охотник-следопыт"));
    EXPECT_EQ(sprintfex(ru("^Pi[Анна] - охотни^R[к;ца;]-следопыт")), ru("Анна - охотница-следопыт"));
}

UNIT_TEST(Mm7TextRu, Idempotency) {
    // Expansion removes every well-formed token, so a second pass is a no-op.
    std::string expanded = sprintfex(ru("^Pi[Анна] украл^R[;а;] ^I[41] золот^L[ой;ых;ых]!"));
    EXPECT_EQ(sprintfex(expanded), expanded);

    // Malformed tokens survive verbatim and keep surviving.
    std::string malformed = sprintfex(ru("^L[только;две]"));
    EXPECT_EQ(sprintfex(malformed), malformed);
}

UNIT_TEST(Mm7TextRu, Malformed) {
    // Malformed tokens are copied through verbatim, not a reason to crash.
    EXPECT_EQ(sprintfex("^"), "^");
    EXPECT_EQ(sprintfex("100^500"), "100^500");
    EXPECT_EQ(sprintfex("^I[unclosed"), "^I[unclosed");
    EXPECT_EQ(sprintfex("^Z[what]"), "^Z[what]");
    EXPECT_EQ(sprintfex(ru("^L[только;две]")), ru("^L[только;две]"));
    EXPECT_EQ(sprintfex("^I[5]^L[a;b;c;d]"), "5c;d"); // Extra semicolons fold into the third form.
    EXPECT_EQ(sprintfex(ru("^R[к;ца]")), ru("^R[к;ца]")); // ^R needs three forms too.
    EXPECT_EQ(sprintfex("^L0[a;b;c]"), "^L0[a;b;c]"); // ^L indices start at 1.
    EXPECT_EQ(sprintfex(ru("^Px[Анна]")), ru("^Px[Анна]")); // 'x' is not a case letter.
    EXPECT_EQ(sprintfex(ru("^P[Анна]")), ru("^P[Анна]")); // ^P requires a case letter.

    // A '^' inside token contents is consumed with the token, not parsed as a new one.
    EXPECT_EQ(sprintfex("^I[5^] x"), "5^ x");
}

UNIT_TEST(Mm7TextRu, StandaloneFallbacks) {
    // ^R with no ^P name and ^L with no ^I number fall back to masculine/zero, like in the original DLL.
    // This is how Buka profession names render when displayed outside of a sentence.
    EXPECT_EQ(sprintfex(ru("охотни^R[к;ца;]")), ru("охотник"));
    EXPECT_EQ(sprintfex(ru("д^L[ень;ня;ней]")), ru("дней"));
    EXPECT_EQ(sprintfex("^L9[a;b;c]"), "c");
}
