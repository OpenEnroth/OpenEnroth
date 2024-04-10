#pragma once

#include <RmlUi/Core/SystemInterface.h>

class RmlSystemInterface : public Rml::SystemInterface {
 public:
    RmlSystemInterface();

    // Inherited via SystemInterface
    double GetElapsedTime() override;
    bool LogMessage(Rml::Log::Type type, const Rml::String &message) override;
};
