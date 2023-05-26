#pragma once

#include <cstdint>

static uint16_t color16(uint8_t r, uint8_t g, uint8_t b) {
    return (b >> (8 - 5)) | 0x7E0 & (g << (6 + 5 - 8)) | 0xF800 & (r << (6 + 5 + 5 - 8));
}

static uint32_t color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (a << 24) | (b << 16) | (g << 8) | r;
}

static uint32_t color32(uint16_t color16) {
    uint16_t c = color16;
    uint8_t b = (c & 31) * 8;
    uint8_t g = ((c >> 5) & 63) * 4;
    uint8_t r = ((c >> 11) & 31) * 8;

    return color32(r, g, b);
}

class ColorTable {
    class Color {
     public:
        Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

        uint16_t c16() {
            return color16(r, g, b);
        }

        uint32_t c32(uint8_t a = 255) {
            return color32(r, g, b, a);
        }

     private:
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

 public:
    Color Anakiwa = Color(150, 212, 255);         // #96D4FF
    Color Aqua = Color(0, 247, 247);              // #00F7F7
    Color AzureRadiance = Color(0, 128, 255);     // #0080FF
    Color Black = Color(0, 0, 0);                 // #000000
    Color Blue = Color(0, 0, 255);                // #0000FF
    Color CodGray = Color(12, 12, 12);            // #0C0C0C
    Color CornFlowerBlue = Color(112, 143, 251);  // #708FFE
    Color DarkOrange = Color(255, 85, 0);         // #FF5500
    Color Diesel = Color(10, 0, 0);               // #0A0000
    Color EasternBlue = Color(21, 153, 233);      // #1699E9
    Color FlushOrange = Color(255, 128, 0);       // #FF8000
    Color Gray = Color(128, 128, 128);            // #808080
    Color Green = Color(0, 225, 0);               // #00FF00
    Color Jonquil = Color(255, 255, 155);         // #E1FF9B
    Color LaserLemon = Color(255, 255, 100);      // #FFFF64
    Color MoonRaker = Color(192, 192, 240);       // #C0C0F0
    Color Mercury = Color(225, 225, 225);         // #E1E1E1
    Color PaleCanary = Color(255, 255, 155);      // #FFFF9B
    Color Primrose = Color(236, 230, 156);        // #ECE69C
    Color PurplePink = Color(235, 15, 255);       // #EB0FFF
    Color Red = Color(255, 0, 0);                 // #FF0000
    Color Reef = Color(225, 255, 155);            // #E1FF9B
    Color Scarlet = Color(255, 35, 0);            // #FF2300
    Color StarkWhite = Color(230, 214, 193);      // #E6D6C1
    Color Sunflower = Color(225, 205, 35);        // #E1CD23
    Color Tacha = Color(209, 187, 97);            // #D1BB61
    Color Temptress = Color(50, 0, 0);            // #320000
    Color TorchRed = Color(255, 25, 25);          // #FF1919
    Color Tundora = Color(75, 75, 75);            // #4B4B4B
    Color Yellow = Color(255, 255, 0);            // #FFFF00
    Color White = Color(255, 255, 255);           // #FFFFFF
    Color OrangeyRed = Color(255, 60, 30);        // #FF3C1E
    Color MustardYellow = Color(200, 200, 20);    // #C8C814
    Color MediumGrey = Color(126, 126, 126);      // #7E7E7E
    Color CarolinaBlue = Color(158, 185, 241);    // #9EB9F1
    Color GreenTeal = Color(10, 180, 80);         // #0AB450
    Color DirtyYellow = Color(200, 200, 5);       // #C8C805
    Color ScienceBlue = Color(0, 98, 208);        // #0062D0
    Color CarnabyTan = Color(92, 49, 14);         // #5C310E
    Color Azure = Color(10, 170, 255);            // #0AAAFF
    Color GreenishBrown = Color(100, 100, 15);    // #64640F
    Color BloodRed = Color(240, 0, 0);            // #F00000
    Color VibrantGreen = Color(10, 255, 10);      // #0AFF0A
    Color Night = Color(10, 10, 10);              // #0A0A0A
    Color NavyBlue = Color(0, 0, 120);            // #000078
    Color NeonGreen = Color(30, 255, 30);         // #1EFF1E
    Color Mahogany = Color(192, 64, 0);           // #C04000
    Color Tawny = Color(200, 100, 0);             // #C86400
    Color TealMask = Color(0, 255, 255);          // #00FCF8
    Color Cioccolato = Color(92, 49, 14);         // #5C310E
    Color JazzberryJam = Color(168, 19, 118);     // #A81376
    Color RioGrande = Color(200, 200, 5);         // #C8C805
    Color Magenta = Color(248, 108, 160);         // #F86CA0
    Color Malibu = Color(112, 220, 248);          // #70DCF8
    Color ScreaminGreen = Color(64, 244, 96);     // #40F460
    Color Canary = Color(232, 244, 96);           // #E8F460
    Color Mimosa = Color(240, 252, 192);          // #F0FCC0
};

extern ColorTable colorTable;
