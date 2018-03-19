#pragma once
#include "ApplicationConfig.h"

class Application
{
    public:
        Application *Configure(ApplicationConfig &config);
        bool ValidateConfig(std::string &out_errors);

        const std::string &GetMm7InstallPath() const;
        void SetMm7InstallPath(const std::string &path);

        void Run();

    private:
        ApplicationConfig config;
};