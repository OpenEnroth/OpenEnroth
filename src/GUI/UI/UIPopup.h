#pragma once
#include <string>

unsigned int GetSpellColor(signed int a1);
uint64_t GetExperienceRequiredForLevel(int level);
void CharacterUI_DrawTooltip(const char *title, std::string &content);

class Image;
class Texture;
extern Texture *parchment;
extern Image *messagebox_corner_x;       // 5076AC
extern Image *messagebox_corner_y;       // 5076B4
extern Image *messagebox_corner_z;       // 5076A8
extern Image *messagebox_corner_w;       // 5076B0
extern Image *messagebox_border_top;     // 507698
extern Image *messagebox_border_bottom;  // 5076A4
extern Image *messagebox_border_left;    // 50769C
extern Image *messagebox_border_right;   // 5076A0

extern bool holdingMouseRightButton;
extern bool rightClickItemActionPerformed;
extern bool identifyOrRepairReactionPlayed;
