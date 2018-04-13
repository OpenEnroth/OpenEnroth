#pragma once

#include "GUI/GUIWindow.h"

class GUIFont;

class GUICredits : public GUIWindow {
 public:
  GUICredits();
  virtual ~GUICredits();

  virtual void Update();

  static void ExecuteCredits();
  void EventLoop();

 protected:
  GUIFont *pFontQuick;
  GUIFont *pFontCChar;

  Image *mm6title;

  int width;
  int height;
  Image *cred_texture;
  int move_Y;
};
