#include "src/Application/Ui/ImGui_Data.h"

// Data
std::shared_ptr<OSWindow> g_Window = nullptr;
Uint64          g_Time = 0;
bool            g_MousePressed[3] = { false, false, false };
SDL_Cursor*     g_MouseCursors[ImGuiMouseCursor_COUNT] = {};
char*           g_ClipboardTextData = nullptr;
bool            g_MouseCanUseGlobalState = true;
unsigned int    g_FontTexture = (unsigned int )-1;
