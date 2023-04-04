#include "TestConfig.h"

#include "Application/GameConfig.h"

void ResetTestConfig(GameConfig *config) {
    config->reset();

    config->debug.NoSound.setValue(true);
    config->debug.NoVideo.setValue(true);
    config->window.MouseGrab.setValue(false);
    config->graphics.FPSLimit.setValue(0); // Unlimited
}
