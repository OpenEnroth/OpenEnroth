#pragma once

#include <functional>
#include <string>
#include <vector>

#include "ConfigFwd.h"
#include "ConfigSection.h"
#include "ConfigSerialization.h"
#include "ConfigValue.h"

class Config {
 public:
    Config() = default;
    Config(const Config &other) = delete; // non-copyable
    Config(Config&& other) = delete; // non-movable

    void Load(const std::string &path);
    void Save(const std::string &path);
    void Reset();

    template<class T>
    void Register(ConfigValue<T> *value) {
        resetCallbacks.push_back([value] (mINI::INIStructure *) { value->Reset(); });
        saveCallbacks.push_back([value, this] (mINI::INIStructure *ini) { SaveOption(value, ini); });
        loadCallbacks.push_back([value, this] (mINI::INIStructure *ini) { LoadOption(value, ini); });
    }

 private:
    using Callback = std::function<void(mINI::INIStructure *)>;

    static void RunAll(std::vector<Callback> &callbacks, mINI::INIStructure *ini);
    static void SaveOption(const std::string& value, mINI::INIStructure *ini, const std::string &sectionName, const std::string &valueName);
    static void LoadOption(std::string* value, mINI::INIStructure *ini, const std::string &sectionName, const std::string &valueName);

    template<class T>
    static void SaveOption(ConfigValue<T> *value, mINI::INIStructure *ini) {
        std::string stringValue;
        SerializeConfigValue(value->Get(), &stringValue);
        SaveOption(stringValue, ini, value->Parent()->Name(), value->Name());
    }

    template<class T>
    static void LoadOption(ConfigValue<T> *value, mINI::INIStructure *ini) {
        std::string stringValue;
        LoadOption(&stringValue, ini, value->Parent()->Name(), value->Name());
        T tmp;
        DeserializeConfigValue(stringValue, &tmp);
        value->Set(tmp);
    }

 private:
    std::vector<Callback> resetCallbacks;
    std::vector<Callback> loadCallbacks;
    std::vector<Callback> saveCallbacks;
};

template<class T>
void RegisterConfigValue(ConfigValue<T> *value) {
    value->Parent()->Parent()->Register(value);
}
