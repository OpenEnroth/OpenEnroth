#pragma once
#include <string>

#include "GUI/GUIWindow.h"
#include "Utility/Geometry/Point.h"
#include "Utility/Geometry/Size.h"

class TargetedSpellUI : public GUIWindow {
 public:
    TargetedSpellUI(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string());
    virtual ~TargetedSpellUI() {}
};

class TargetedSpellUI_Hirelings : public TargetedSpellUI {
 public:
    TargetedSpellUI_Hirelings(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string());
    virtual ~TargetedSpellUI_Hirelings() {}
};

class TargetedSpellUI_Character : public TargetedSpellUI {
 public:
    TargetedSpellUI_Character(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string());
    virtual ~TargetedSpellUI_Character() {}
};

class TargetedSpellUI_Actor : public TargetedSpellUI {
 public:
    TargetedSpellUI_Actor(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string());
    virtual ~TargetedSpellUI_Actor() {}
};

class TargetedSpellUI_ActorOrCharacter : public TargetedSpellUI {
 public:
    TargetedSpellUI_ActorOrCharacter(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string());
    virtual ~TargetedSpellUI_ActorOrCharacter() {}
};

class TargetedSpellUI_Telekinesis : public TargetedSpellUI {
 public:
    TargetedSpellUI_Telekinesis(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string());
    virtual ~TargetedSpellUI_Telekinesis() {}
};
