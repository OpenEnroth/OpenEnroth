#include <memory>
#include <string>

#include "src/Application/Game.h"
#include "src/Application/GameConfig.h"
#include "src/Application/GameFactory.h"

#include "Engine/AssetLocator.h"
#include "Engine/CommandLine.h"

#include "Platform/Api.h"


using Application::Game;
using Application::GameConfig;
using Application::GameFactory;

std::shared_ptr<AssetLocator> asset_locator;

std::string FindMm6Directory();
std::string FindMm7Directory();
std::string FindMm8Directory();


int MM_Main(const std::string &args) {
    void IntegrityTest();
    IntegrityTest();

    auto command_line = std::make_shared<CommandLine>(args);

    asset_locator = std::make_shared<AssetLocator>();
    if (command_line->TryFindKey("-mm6")) {
        asset_locator->SetBaseGameRoot(
            Mm6Assets,
            FindMm6Directory()
        );
    } else if (command_line->TryFindKey("-mm8")) {
        asset_locator->SetBaseGameRoot(
            Mm8Assets,
            FindMm8Directory()
        );
    } else {
        // defaults to mm7
        asset_locator->SetBaseGameRoot(
            Mm7Assets,
            FindMm7Directory()
        );
    }

    std::shared_ptr<const GameConfig> gameConfig = std::make_shared<GameConfig>(command_line);
    auto game = GameFactory().CreateGame(gameConfig, asset_locator);
    game->Run();

    return 0;
}


static std::string FindMm7Directory() {
    bool mm7_installation_found = false;

    // env variable override to a custom folder
    if (!mm7_installation_found) {
        if (const char* path = std::getenv("WOMM_PATH_OVERRIDE")) {
            mm7_installation_found = true;
            logger->Info("MM7 Custom Folder (ENV path override): %s", path);
            return path;
        }
    }

    // standard 1.0 installation
    char path_buffer[2048];
    if (!mm7_installation_found) {
        mm7_installation_found = OS_GetAppString(
            "HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic VII/1.0/AppPath",
            path_buffer,
            sizeof(path_buffer)
        );

        if (mm7_installation_found) {
            logger->Info("Standard MM7 installation found: %s", path_buffer);
            return path_buffer;
        }
    }

    // GoG old version
    if (!mm7_installation_found) {
        mm7_installation_found = OS_GetAppString(
            "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM7/PATH",
            path_buffer,
            sizeof(path_buffer)
        );

        if (mm7_installation_found) {
            logger->Info("GoG MM7 installation found: %s", path_buffer);
            return path_buffer;
        }
    }

    // GoG new version ( 2018 builds )
    if (!mm7_installation_found) {
        mm7_installation_found = OS_GetAppString(
            "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/Games/1207658916/Path",
            path_buffer,
            sizeof(path_buffer)
        );

        if (mm7_installation_found) {
            logger->Info("GoG MM7 2018 build installation found: %s", path_buffer);
            return path_buffer;
        }
    }

    // Hack path fix - pskelton
    if (!mm7_installation_found) {
        mm7_installation_found = 1;
        strcpy(path_buffer, "E:/Programs/GOG Galaxy/Games/Might and Magic 7");
        logger->Info("Hack Path MM7 installation found: %s", path_buffer);
        return path_buffer;
    }

    return "";
}


static std::string FindMm8Directory() {
    bool mm8_installation_found = false;
    char path_buffer[2048];

    // standard 1.0 installation
    if (!mm8_installation_found) {
        mm8_installation_found = OS_GetAppString(
            "HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic Day of the Destroyer/1.0/AppPath",
            path_buffer,
            sizeof(path_buffer)
        );

        if (mm8_installation_found) {
            logger->Info("Standard MM8 installation found: %s", path_buffer);
            return path_buffer;
        }
    }

    // GoG old version
    if (!mm8_installation_found) {
        mm8_installation_found = OS_GetAppString(
            "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM8/PATH",
            path_buffer,
            sizeof(path_buffer)
        );

        if (mm8_installation_found) {
            logger->Info("GoG MM8 installation found: %s", path_buffer);
            return path_buffer;
        }
    }

    return "";
}


static std::string FindMm6Directory() {
    bool mm6_installation_found = false;
    char path_buffer[2048];

    // standard 1.0 verison does not specify path in registry...

    // GoG old version
    // ???

    // GoG new version ( 2018 builds )
    if (!mm6_installation_found) {
        mm6_installation_found = OS_GetAppString(
            "HKEY_LOCAL_MACHINE/SOFTWARE/WOW6432Node/GOG.com/Games/1207661253/PATH",
            path_buffer,
            sizeof(path_buffer)
        );

        if (mm6_installation_found) {
            logger->Info("GoG MM6 2018 build installation found: %s", path_buffer);
            return path_buffer;
        }
    }

    return "";
}
