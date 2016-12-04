#pragma once

class LuaVM
{
  public:
    inline LuaVM(): L(nullptr) {}

    void Initialize();
    bool DoFile(const char *filename);

  protected:
    struct lua_State *L;
    
    const char *GetScriptFileLocation(const char *script_name);
};
extern LuaVM *lua;