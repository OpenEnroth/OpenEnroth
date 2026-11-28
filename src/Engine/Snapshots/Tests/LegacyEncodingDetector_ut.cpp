#include <string>
#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Snapshots/LegacyEncodingDetector.h"

// Names below are raw bytes, exactly as they're stored in a savegame.
static TextEncoding detect(std::string_view names) {
    return detectLegacySaveEncoding(names, ENCODING_WINDOWS_1252);
}

UNIT_TEST(LegacyEncodingDetector, Russian) {
    // "Иван", "Ольга", "Сергей", "Зоя" in windows-1251.
    EXPECT_EQ(detect("\xC8\xE2\xE0\xED"), ENCODING_WINDOWS_1251);
    EXPECT_EQ(detect("\xCE\xEB\xFC\xE3\xE0"), ENCODING_WINDOWS_1251);
    EXPECT_EQ(detect("\xD1\xE5\xF0\xE3\xE5\xE9"), ENCODING_WINDOWS_1251);
    EXPECT_EQ(detect("\xC7\xEE\xFF"), ENCODING_WINDOWS_1251);

    // A full party, the way the detector actually sees it - all names concatenated.
    EXPECT_EQ(detect("\xC8\xE2\xE0\xED\xCE\xEB\xFC\xE3\xE0\xD1\xE5\xF0\xE3\xE5\xE9"),
              ENCODING_WINDOWS_1251);
}

UNIT_TEST(LegacyEncodingDetector, Western) {
    EXPECT_EQ(detect("Gareth"), ENCODING_WINDOWS_1252);
    EXPECT_EQ(detect("Zoltan"), ENCODING_WINDOWS_1252);

    // "Renée", "Frédéric", "Jürgen", "Schäfer" - accented, but still mostly ASCII.
    EXPECT_EQ(detect("Ren\xE9\x65"), ENCODING_WINDOWS_1252);
    EXPECT_EQ(detect("Fr\xE9\x64\xE9ric"), ENCODING_WINDOWS_1252);
    EXPECT_EQ(detect("J\xFCrgen"), ENCODING_WINDOWS_1252);
    EXPECT_EQ(detect("Sch\xE4""fer"), ENCODING_WINDOWS_1252);

    // Worst case for us - a short name that starts with an accented letter.
    EXPECT_EQ(detect("\xC9tienne"), ENCODING_WINDOWS_1252);
}

UNIT_TEST(LegacyEncodingDetector, Fallback) {
    // Names with nothing outside ASCII give nothing to go on, and get the fallback.
    for (TextEncoding fallback : {ENCODING_WINDOWS_1251, ENCODING_WINDOWS_1252}) {
        EXPECT_EQ(detectLegacySaveEncoding("", fallback), fallback);
        EXPECT_EQ(detectLegacySaveEncoding("123", fallback), fallback);
        EXPECT_EQ(detectLegacySaveEncoding("Gareth", fallback), fallback);
    }
}
