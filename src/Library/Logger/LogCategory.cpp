#include "LogCategory.h"

#include <cassert>
#include <unordered_map>
#include <vector>

#include "Utility/MapAccess.h"

static std::unordered_map<std::string_view, LogCategory *> &logCategoriesStorage() {
    static std::unordered_map<std::string_view, LogCategory *> result; // Wrapping in a function static to avoid static init order fiasco.
    return result;
}

LogCategory::LogCategory(std::string_view name, LogSource *source): _name(name), _source(source) {
    auto &storage = logCategoriesStorage();
    assert(!storage.contains(_name));
    storage.emplace(_name, this);
}

LogCategory::~LogCategory() {
    auto &storage = logCategoriesStorage();
    assert(storage.contains(_name));
    storage.erase(_name);
}

std::vector<LogCategory *> LogCategory::instances() {
    std::vector<LogCategory *> result;
    for (const auto &[_, category] : logCategoriesStorage())
        result.push_back(category);
    return result;
}

LogCategory *LogCategory::instance(std::string_view name) {
    return valueOr(logCategoriesStorage(), name, nullptr);
}
