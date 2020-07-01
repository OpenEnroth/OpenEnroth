#pragma once
#include <memory>

#include <imgui.h>

#include "Platform/OSWindow.h"

void InitializeUi(std::shared_ptr<OSWindow> window);
void ShutdownUi();

void UiNewFrame();
void UiEndFrame();
