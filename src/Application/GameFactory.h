#pragma once

#include <memory>
#include <string>

#include "src/Application/Game.h"

namespace Application {

class GameFactory {
 public:
     std::shared_ptr<Game> CreateGame(const std::string &command_line);

 private:
     std::shared_ptr<const Configuration> CreateConfiguration(const std::string &command_line);
};

}  // namespace Application
