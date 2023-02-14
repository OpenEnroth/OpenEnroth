#include "TestConfig.h"

#include "Application/GameConfig.h"

void ResetTestConfig(Application::GameConfig *config) {
    config->Reset();

    config->debug.NoVideo.Set(true);
    config->window.MouseGrab.Set(false);
    config->graphics.FPSLimit.Set(0); // Unlimited
}
