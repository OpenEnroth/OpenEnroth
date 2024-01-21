#include "Environment.h"

#include <filesystem>

//#include "Library/Logger/Logger.h"
/*
Environment::GamePaths Environment::resolveGamePath(const PathResolutionConfig& config) const {
    auto envKey = config.overrideEnvKey;
    auto envPath = getenv(envKey);
    if (!envPath.empty()) {
        //logger->info("Path override provided, '{}={}'.", envKey, envPath);
        return { envPath };
    }

    Environment::GamePaths result{ std::filesystem::current_path().string() };
    auto specificPaths = getGamePaths(config);

    result.insert(result.end(), std::make_move_iterator(specificPaths.begin()), std::make_move_iterator(specificPaths.end()));

    return result;
}
*/