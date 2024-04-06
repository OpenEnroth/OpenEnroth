#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "Library/Platform/Interface/PlatformEnums.h"

#include "GUI/GUIEnums.h"

#define NUKLEAR_MAX_VERTEX_MEMORY 512 * 1024
#define NUKLEAR_MAX_ELEMENT_MEMORY 128 * 1024

struct lua_State;

class Nuklear {
 public:
     enum NUKLEAR_MODE: int32_t {
         NUKLEAR_MODE_SHARED = 1,
         NUKLEAR_MODE_EXCLUSIVE
     };

     enum NUKLEAR_ACTION: int32_t {
         NUKLEAR_ACTION_CREATE = 1,
         NUKLEAR_ACTION_DRAW,
         NUKLEAR_ACTION_RELEASE
     };

     enum NUKLEAR_STAGE: int32_t {
         NUKLEAR_STAGE_PRE = 1,
         NUKLEAR_STAGE_POST
     };

     Nuklear();

     static std::unique_ptr<Nuklear> Initialize();
     bool Create(WindowType winType);
     bool Draw(NUKLEAR_STAGE stage, WindowType winType, int type);
     int KeyEvent(PlatformKey key);
     bool Reload();
     void Release(WindowType winType);
     void Destroy();
     void addInitLuaLibs(std::function<void(lua_State*)> callback);
     void addInitLuaFile(const char* lua_file);
     enum NUKLEAR_MODE Mode(WindowType winType);

     struct nk_context *ctx = nullptr;

 private:
     void Release(WindowType winType, bool reload);
     bool LuaInit();
     void LuaRelease();
     bool LuaLoadTemplate(WindowType winType);

     std::vector<std::string> _init_lua_files;
     std::vector<std::function<void(lua_State*)>> _init_lua_lib_callbacks;

 protected:
};

extern Nuklear *nuklear;
