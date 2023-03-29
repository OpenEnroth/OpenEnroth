#include "TestConfig.h"

#include "Application/GameConfig.h"

void ResetTestConfig(GameConfig *config) {
    config->Reset();

    config->debug.NoVideo.Set(true);
    config->window.MouseGrab.Set(false);
    config->graphics.FPSLimit.Set(0); // Unlimited
}
