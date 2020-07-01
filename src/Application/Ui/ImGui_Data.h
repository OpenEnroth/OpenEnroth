#pragma once
#include <memory>

#include <imgui/imgui.h>
#include <SDL.h>

#include "Platform/OSWindow.h"


// Data
extern std::shared_ptr<OSWindow> g_Window;
extern Uint64       g_Time;
extern bool         g_MousePressed[3];
extern SDL_Cursor*  g_MouseCursors[ImGuiMouseCursor_COUNT];
extern char*        g_ClipboardTextData;
extern bool         g_MouseCanUseGlobalState;
extern unsigned int g_FontTexture;
