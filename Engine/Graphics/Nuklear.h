#pragma once

#include <cstdint>
#include <string>

#include <lua.hpp>

#include "GUI/GUIWindow.h"

class Image;

class Nuklear {
 public:
     enum NUKLEAR_MODE: __int32 {
         NUKLEAR_SHARED = 1,
         NUKLEAR_EXCLUSIVE
     };

     enum NUKLEAR_INSTANCE : __int32 {
         NUKLEAR_CREATE = 1,
         NUKLEAR_DRAW,
         NUKLEAR_RELEASE
     };

     enum NUKLEAR_STAGE : __int32 {
         NUKLEAR_PRE = 1,
         NUKLEAR_POST
     };

     Nuklear();

     bool Initialize();
     bool Create(enum WindowType winType);
     bool Draw(enum NUKLEAR_STAGE stage, enum WindowType winType, int type);
     bool Reload(enum WindowType winType);
     void Release(enum WindowType winType);
     void Destroy();
     enum NUKLEAR_MODE Mode(enum WindowType winType);

 private:
     bool LuaInit();
     void LuaRelease();
     bool LuaLoadTemplate(enum WindowType winType);

 protected:
};

extern Nuklear *pNuklear;
