#pragma once
#include "GUI/GUIWindow.h"



bool PlayerCreation_Choose4Skills();
void LoadPlayerPortraintsAndVoices();
void ReloadPlayerPortraits(int player_id, int face_id);
void DeleteCCharFont();
bool PlayerCreationUI_Loop();


struct GUIWindow_PartyCreation : public GUIWindow
{
             GUIWindow_PartyCreation();
    virtual ~GUIWindow_PartyCreation() {}

    virtual void Update();
};



extern class Image *ui_partycreation_left;
extern class Image *ui_partycreation_right;
extern class Image *ui_partycreation_minus;
extern class Image *ui_partycreation_plus;
extern class Image *ui_partycreation_buttmake2;
extern class Image *ui_partycreation_buttmake;