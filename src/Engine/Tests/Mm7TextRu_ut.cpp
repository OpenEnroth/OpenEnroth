#include <string>
#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Engine/mm7text_ru.h"

UNIT_TEST(Mm7TextRu, PassThrough) {
    EXPECT_EQ(sprintfex(""), "");
    EXPECT_EQ(sprintfex("no tokens here"), "no tokens here");
    EXPECT_EQ(sprintfex("Привет, мир!"), "Привет, мир!");
}

UNIT_TEST(Mm7TextRu, Plurals) {
    EXPECT_EQ(sprintfex("^I[1] д^L[ень;ня;ней]"), "1 день");
    EXPECT_EQ(sprintfex("^I[2] д^L[ень;ня;ней]"), "2 дня");
    EXPECT_EQ(sprintfex("^I[5] д^L[ень;ня;ней]"), "5 дней");
    EXPECT_EQ(sprintfex("^I[21] д^L[ень;ня;ней]"), "21 день");
    EXPECT_EQ(sprintfex("^I[100] д^L[ень;ня;ней]"), "100 дней");

    // The original mm7text.dll only looked at the last digit and would produce "11 день" here.
    EXPECT_EQ(sprintfex("^I[11] д^L[ень;ня;ней]"), "11 дней");
    EXPECT_EQ(sprintfex("^I[14] д^L[ень;ня;ней]"), "14 дней");

    // Negative numbers are numbers too, not a "no ^I yet" marker.
    EXPECT_EQ(sprintfex("^I[-1] д^L[ень;ня;ней]"), "-1 день");
    EXPECT_EQ(sprintfex("^I[-2147483648] д^L[ень;ня;ней]"), "-2147483648 дней");

    // Unparsable ^I contents are printed but don't occupy a number slot, like in the original DLL.
    EXPECT_EQ(sprintfex("^I[?]^I[2] штук^L[а;и;]"), "?2 штуки");
}

UNIT_TEST(Mm7TextRu, NumberedPlurals) {
    // An actual string from the Buka global.txt, "Buy food for %d days for %d gold".
    EXPECT_EQ(sprintfex("Купить еды на ^I[3] д^L1[ень;ня;ней] за ^I[22] золот^L2[ой;ых;ых]"),
              "Купить еды на 3 дня за 22 золотых");
    EXPECT_EQ(sprintfex("Купить еды на ^I[1] д^L1[ень;ня;ней] за ^I[5] золот^L2[ой;ых;ых]"),
              "Купить еды на 1 день за 5 золотых");
}

UNIT_TEST(Mm7TextRu, Gender) {
    // Capitalized names come from one table...
    EXPECT_EQ(sprintfex("^Pi[Анна] - учитель^R[;ница;]"), "Анна - учительница");
    EXPECT_EQ(sprintfex("^Pi[Адам] - учитель^R[;ница;]"), "Адам - учитель");

    // ...common nouns from another.
    EXPECT_EQ(sprintfex("^Pi[гидра] был^R[;а;о] здесь"), "гидра была здесь");
    EXPECT_EQ(sprintfex("^Pi[голем] был^R[;а;о] здесь"), "голем был здесь");
    EXPECT_EQ(sprintfex("^Pi[училище] закрыт^R[;а;о]"), "училище закрыто");

    // Same expansion as NameAndTitle() does for a Buka LSTR_S_THE_S of "^Pi[%s] %s".
    EXPECT_EQ(sprintfex("^Pi[Валерия] портн^R[ой;иха;]"), "Валерия портниха");

    // Unknown names default to masculine.
    EXPECT_EQ(sprintfex("^Pi[Xyzzy] боец^R[; WAT;]"), "Xyzzy боец");
}

UNIT_TEST(Mm7TextRu, SpecialNames) {
    // String 126 of the Buka global.txt. The DLL used the genitive here - "покинуть Врат в Бездну".
    EXPECT_EQ(sprintfex("Вы хотите покинуть ^Pv[Врата в Бездну]?"), "Вы хотите покинуть Врата в Бездну?");
    EXPECT_EQ(sprintfex("Вы не нашли ^Pr[Врата в Бездну]"), "Вы не нашли Врат в Бездну");
    EXPECT_EQ(sprintfex("к ^Pd[Врата в Бездну]"), "к Вратам в Бездну");
    EXPECT_EQ(sprintfex("о ^Pp[Стены тумана]"), "о Стенах тумана");
    EXPECT_EQ(sprintfex("^Pi[Мэри Джо] ушл^R[;а;о]"), "Мэри Джо ушла");

    // A name that merely starts with a special name isn't truncated to it.
    EXPECT_EQ(sprintfex("^Pi[Мэри Джонсон]"), "Мэри Джонсон");
}

UNIT_TEST(Mm7TextRu, PluralBoundaries) {
    EXPECT_EQ(sprintfex("^I[0] д^L[ень;ня;ней]"), "0 дней");
    EXPECT_EQ(sprintfex("^I[22] д^L[ень;ня;ней]"), "22 дня");
    EXPECT_EQ(sprintfex("^I[25] д^L[ень;ня;ней]"), "25 дней");
    EXPECT_EQ(sprintfex("^I[101] д^L[ень;ня;ней]"), "101 день");
    EXPECT_EQ(sprintfex("^I[111] д^L[ень;ня;ней]"), "111 дней");
}

UNIT_TEST(Mm7TextRu, NumberIndexing) {
    // Explicit ^L<n> indices address the n-th recorded number.
    EXPECT_EQ(sprintfex("^I[1] ^I[2] ^I[5] очк^L3[о;а;ов]"), "1 2 5 очков");

    // An index past the recorded numbers means zero, not the last recorded number.
    EXPECT_EQ(sprintfex("^I[1] д^L2[ень;ня;ней]"), "1 дней");

    // An unparsable ^I doesn't occupy a slot, so later explicit indices shift.
    EXPECT_EQ(sprintfex("^I[x]^I[3] ^L1[a;b;c] ^L2[a;b;c]"), "x3 b c");
}

UNIT_TEST(Mm7TextRu, GenderState) {
    // One ^P gender drives all following ^R tokens...
    EXPECT_EQ(sprintfex("^Pi[Анна] пришл^R[;а;о] и ушл^R[;а;о]"), "Анна пришла и ушла");
    // ...until the next ^P overwrites it.
    EXPECT_EQ(sprintfex("^Pi[Адам] и ^Pi[Анна] - последн^R[ий;яя;ее] ушл^R[;а;о]"),
              "Адам и Анна - последняя ушла");

    // Neuter from the capitalized table, not just the common-noun one.
    EXPECT_EQ(sprintfex("^Pi[Солнце] взошл^R[;а;о]"), "Солнце взошло");

    // Unknown Cyrillic names default to masculine, same as Latin ones.
    EXPECT_EQ(sprintfex("^Pi[Зорг] был^R[;а;о] здесь"), "Зорг был здесь");

    // A table entry matches if it starts with the looked-up name, like in the original DLL.
    EXPECT_EQ(sprintfex("^Pi[Валери] - портн^R[ой;иха;]"), "Валери - портниха");

    // The case letter is ignored for regular names - they're printed undeclined.
    // String 164 of the Buka global.txt.
    EXPECT_EQ(sprintfex("Скелет наносит удар ^Pd[Гоблин], отбирая 5 ед. здоровья"),
              "Скелет наносит удар Гоблин, отбирая 5 ед. здоровья");

    // Empty forms and empty names are not an error.
    EXPECT_EQ(sprintfex("^R[;;]"), "");
    EXPECT_EQ(sprintfex("^Pi[]"), "");
    EXPECT_EQ(sprintfex("^Pi[]: ^R[он;она;оно]"), ": он");
}

UNIT_TEST(Mm7TextRu, SpecialNamesDeclension) {
    // The remaining cases of a declined special name - v/r/d are covered in SpecialNames.
    EXPECT_EQ(sprintfex("^Pi[Врата в Бездну] закрыт^R[;а;о]"), "Врата в Бездну закрыта");
    EXPECT_EQ(sprintfex("перед ^Pt[Врата в Бездну]"), "перед Вратами в Бездну");
    EXPECT_EQ(sprintfex("о ^Pp[Врата в Бездну]"), "о Вратах в Бездну");

    // The case letter is ASCII case-insensitive.
    EXPECT_EQ(sprintfex("^PR[Врата в Бездну]"), "Врат в Бездну");

    // Indeclinable special names print the same in every case, and still carry a gender.
    EXPECT_EQ(sprintfex("к ^Pd[Ли Энн]"), "к Ли Энн");
    EXPECT_EQ(sprintfex("^Pi[Ли Энн] ушл^R[;а;о]"), "Ли Энн ушла");

    // A prefix match keeps the tail verbatim and still records the gender...
    EXPECT_EQ(sprintfex("^Pi[Мэри Джонсон] ушл^R[;а;о]"), "Мэри Джонсон ушла");
    // ...and a declined prefix declines while the tail stays as is.
    EXPECT_EQ(sprintfex("^Pd[Врата в Бездну снов]"), "Вратам в Бездну снов");
}

UNIT_TEST(Mm7TextRu, RealData) {
    // String 302 of the Buka global.txt, all four token kinds in one string.
    EXPECT_EQ(sprintfex("^Pi[Анна] украл^R[;а;] ^I[41] золот^L[ой;ых;ых]!"),
              "Анна украла 41 золотой!");

    // String 311, a token in the middle of a word.
    EXPECT_EQ(sprintfex("охотни^R[к;ца;]-следопыт"), "охотник-следопыт");
    EXPECT_EQ(sprintfex("^Pi[Анна] - охотни^R[к;ца;]-следопыт"), "Анна - охотница-следопыт");
}

UNIT_TEST(Mm7TextRu, Idempotency) {
    // Expansion removes every well-formed token, so a second pass is a no-op.
    std::string expanded = sprintfex("^Pi[Анна] украл^R[;а;] ^I[41] золот^L[ой;ых;ых]!");
    EXPECT_EQ(sprintfex(expanded), expanded);

    // Malformed tokens survive verbatim and keep surviving.
    std::string malformed = sprintfex("^L[только;две]");
    EXPECT_EQ(sprintfex(malformed), malformed);
}

UNIT_TEST(Mm7TextRu, Malformed) {
    // Malformed tokens are copied through verbatim, not a reason to crash.
    EXPECT_EQ(sprintfex("^"), "^");
    EXPECT_EQ(sprintfex("100^500"), "100^500");
    EXPECT_EQ(sprintfex("^I[unclosed"), "^I[unclosed");
    EXPECT_EQ(sprintfex("^Z[what]"), "^Z[what]");
    EXPECT_EQ(sprintfex("^L[только;две]"), "^L[только;две]");
    EXPECT_EQ(sprintfex("^I[5]^L[a;b;c;d]"), "5c;d"); // Extra semicolons fold into the third form.
    EXPECT_EQ(sprintfex("^R[к;ца]"), "^R[к;ца]"); // ^R needs three forms too.
    EXPECT_EQ(sprintfex("^L0[a;b;c]"), "^L0[a;b;c]"); // ^L indices start at 1.
    EXPECT_EQ(sprintfex("^Px[Анна]"), "^Px[Анна]"); // 'x' is not a case letter.
    EXPECT_EQ(sprintfex("^P[Анна]"), "^P[Анна]"); // ^P requires a case letter.

    // A '^' inside token contents is consumed with the token, not parsed as a new one.
    EXPECT_EQ(sprintfex("^I[5^] x"), "5^ x");
}

UNIT_TEST(Mm7TextRu, StandaloneFallbacks) {
    // ^R with no ^P name and ^L with no ^I number fall back to masculine/zero, like in the original DLL.
    // This is how Buka profession names render when displayed outside of a sentence.
    EXPECT_EQ(sprintfex("охотни^R[к;ца;]"), "охотник");
    EXPECT_EQ(sprintfex("д^L[ень;ня;ней]"), "дней");
    EXPECT_EQ(sprintfex("^L9[a;b;c]"), "c");
}
