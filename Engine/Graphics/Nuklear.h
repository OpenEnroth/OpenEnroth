#pragma once

#include <cstdint>
#include <string>

#include "Io/GameKey.h"

#include <lua.hpp>
#include "nuklear/nuklear_config.h"

class Image;
class GUIWindow;

class Nuklear {
 public:
     enum NUKLEAR_MODE: __int32 {
         NUKLEAR_MODE_SHARED = 1,
         NUKLEAR_MODE_EXCLUSIVE
     };

     enum NUKLEAR_ACTION : __int32 {
         NUKLEAR_ACTION_CREATE = 1,
         NUKLEAR_ACTION_DRAW,
         NUKLEAR_ACTION_RELEASE
     };

     enum NUKLEAR_STAGE : __int32 {
         NUKLEAR_STAGE_PRE = 1,
         NUKLEAR_STAGE_POST
     };

     Nuklear();

     std::shared_ptr<Nuklear> Initialize();
     bool Create(enum WindowType winType);
     bool Draw(enum NUKLEAR_STAGE stage, enum WindowType winType, int type);
     int KeyEvent(Io::GameKey key);
     bool Reload();
     void Release(enum WindowType winType);
     void Destroy();
     enum NUKLEAR_MODE Mode(enum WindowType winType);

     struct nk_context *ctx = nullptr;

 private:
     void Release(WindowType winType, bool reload);
     bool LuaInit();
     void LuaRelease();
     bool LuaLoadTemplate(enum WindowType winType);

 protected:
};

extern std::shared_ptr<Nuklear> nuklear;
