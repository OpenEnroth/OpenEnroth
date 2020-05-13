#pragma once
#include <memory>
#include <string>
#include <vector>

namespace Engine_ {
    class CommandLine {
     public:
        explicit CommandLine(const std::string& command_line);

        bool TryFindKey(const std::string& key) const;
        bool TryGetValue(const std::string& key, std::shared_ptr<std::string>* out_value) const;

     private:
        std::vector<std::string> command_line_tokens;
    };
}  // namespace Engine_
