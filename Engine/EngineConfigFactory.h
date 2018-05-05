#pragma once

#include <memory>

#include "Engine/EngineConfig.h"
#include "Engine/IocContainer.h"
#include "Engine/Log.h"

using EngineIoc = Engine_::IocContainer;

namespace Engine_ {

class EngineConfigFactory {
 public:
    inline EngineConfigFactory() {
        this->log = EngineIoc::ResolveLogger();
    }

    std::shared_ptr<EngineConfig> Create();
    std::shared_ptr<EngineConfig> CreateFromCommandLine(const std::string &cmd);

 private:
    std::shared_ptr<EngineConfig> CreateDefaultConfiguration();

    Log *log = nullptr;
};

}  // namespace Engine_
