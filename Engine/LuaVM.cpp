#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include "LuaVM.h"
#include "lib/lua/lua.h"
#include "Log.h"
#include "OSAPI.h"

extern "C" int luaopen_UIControl(lua_State *L); // declare the wrapped module
extern "C" int luaopen_Player(lua_State *L); // declare the wrapped module

LuaVM *lua = nullptr;

void LuaVM::Initialize()
{
  if (L)
    Log::Warning(L"Overwriting previous Lua state");

  L = luaL_newstate();
  if (!L)
    Log::Warning(L"Error creating Lua context.\n");

  // open default lua libs
  luaL_openlibs(L);

  // open each cxx module
  luaopen_UIControl(L);

  //if ( luaL_dofile(L,GetScriptFileLocation("script.lua")))
  //    Log::Warning(L"Error opening script.lua\n");
}

bool LuaVM::DoFile(const char *filename)
{
  if (luaL_dofile(L, GetScriptFileLocation(filename)))
  {
    Log::Warning(L"Error opening script %s", filename);
    return false;
  }
  return true;
}

const char *LuaVM::GetScriptFileLocation(const char *script_name)
{
  static DWORD tls_index = TlsAlloc();

  auto buf = (char *)TlsGetValue(tls_index);
  if (!buf)
  {
    buf = new char[1024];
    TlsSetValue(tls_index, buf);
  }

  strcpy(buf, "data/scripts/lua/core/");
  strcat(buf, script_name);
  return buf;
}