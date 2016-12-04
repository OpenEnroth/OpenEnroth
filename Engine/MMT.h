#pragma once

void ShowLogoVideo();
void MMT_MainMenu_Loop();
void MMT_MenuMessageProc();
void DrawMMTCopyrightWindow();
extern bool FileExists(const char *fname);

extern bool use_MMT;
extern bool use_music_folder;
extern bool for_refactoring;