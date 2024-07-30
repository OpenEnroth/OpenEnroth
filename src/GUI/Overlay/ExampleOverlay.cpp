#include "ExampleOverlay.h"

#include <imgui/imgui.h> // NOLINT: not a C system header.

void ExampleOverlay::update() {
    ImGui::ShowDemoWindow();
}
