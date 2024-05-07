#include "ImGuiWrapper.h"

#include <imgui/imgui.h>

ImGuiWrapper::ImGuiWrapper() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
}

ImGuiWrapper::~ImGuiWrapper() {
    ImGui::DestroyContext();
}
