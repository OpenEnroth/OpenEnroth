#pragma once

namespace Application {
    enum class GameState : int {
        Intro,
        Startup,
        MainMenu
    };
}  // namespace Application

void ChangeState(Application::GameState new_state);
