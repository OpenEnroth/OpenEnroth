#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Snapshots/LegacyEncodingDetector.h"

// Names below are raw bytes, exactly as they're stored in a savegame.

UNIT_TEST(LegacyEncodingDetector, Russian) {
    // "Иван", "Ольга", "Сергей", "Зоя" in windows-1251.
    EXPECT_EQ(detectLegacySaveEncoding("\xC8\xE2\xE0\xED"), ENCODING_WINDOWS_1251);
    EXPECT_EQ(detectLegacySaveEncoding("\xCE\xEB\xFC\xE3\xE0"), ENCODING_WINDOWS_1251);
    EXPECT_EQ(detectLegacySaveEncoding("\xD1\xE5\xF0\xE3\xE5\xE9"), ENCODING_WINDOWS_1251);
    EXPECT_EQ(detectLegacySaveEncoding("\xC7\xEE\xFF"), ENCODING_WINDOWS_1251);

    // A full party, the way the detector actually sees it - all names concatenated.
    EXPECT_EQ(detectLegacySaveEncoding("\xC8\xE2\xE0\xED\xCE\xEB\xFC\xE3\xE0\xD1\xE5\xF0\xE3\xE5\xE9"),
              ENCODING_WINDOWS_1251);
}

UNIT_TEST(LegacyEncodingDetector, Western) {
    EXPECT_EQ(detectLegacySaveEncoding("Gareth"), ENCODING_WINDOWS_1252);
    EXPECT_EQ(detectLegacySaveEncoding("Zoltan"), ENCODING_WINDOWS_1252);

    // "Renée", "Frédéric", "Jürgen", "Schäfer" - accented, but still mostly ASCII.
    EXPECT_EQ(detectLegacySaveEncoding("Ren\xE9\x65"), ENCODING_WINDOWS_1252);
    EXPECT_EQ(detectLegacySaveEncoding("Fr\xE9\x64\xE9ric"), ENCODING_WINDOWS_1252);
    EXPECT_EQ(detectLegacySaveEncoding("J\xFCrgen"), ENCODING_WINDOWS_1252);
    EXPECT_EQ(detectLegacySaveEncoding("Sch\xE4""fer"), ENCODING_WINDOWS_1252);

    // Worst case for us - a short name that's half accented letters.
    EXPECT_EQ(detectLegacySaveEncoding("\xC9tienne"), ENCODING_WINDOWS_1252);
}

UNIT_TEST(LegacyEncodingDetector, NoLetters) {
    // Nothing to go on. Both encodings agree on ASCII, so windows-1252 is as good an answer as any.
    EXPECT_EQ(detectLegacySaveEncoding(""), ENCODING_WINDOWS_1252);
    EXPECT_EQ(detectLegacySaveEncoding("123"), ENCODING_WINDOWS_1252);
    EXPECT_EQ(detectLegacySaveEncoding("   "), ENCODING_WINDOWS_1252);
}
