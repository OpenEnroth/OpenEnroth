#include "Preprocessor.h"

#include <memory>
#include <span>
#include <string>
#include <string_view>

#define TCPP_IMPLEMENTATION
#include <tcppLibrary.hpp>

#include "Library/FileSystem/Interface/FileSystem.h"

#include "Utility/Exception.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Format.h"

namespace pp {

Blob preprocess(const Blob &source, const FileSystem *pwd, std::string_view preamble,
                std::span<const std::string_view> passthroughDirectives) {
    std::string sourceStr;
    sourceStr.reserve(preamble.size() + source.size());
    sourceStr.append(preamble);
    sourceStr.append(static_cast<const char *>(source.data()), source.size());

    tcpp::Lexer lexer(std::make_unique<tcpp::StringInputStream>(sourceStr));

    std::string errorMessage;

    tcpp::Preprocessor::TPreprocessorConfigInfo config;
    config.mOnErrorCallback = [&](const tcpp::TErrorInfo &errorInfo) {
        errorMessage = fmt::format("Line {}: {}", source.displayPath(), errorInfo.mLine,
                                   tcpp::ErrorTypeToString(errorInfo.mType));
    };

    config.mOnIncludeCallback = [&](const std::string &path, bool /*isSystemInclude*/) -> tcpp::TInputStreamUniquePtr {
        if (!pwd->exists(path)) {
            errorMessage = fmt::format("Include file '{}' not found", source.displayPath(), path);
            return std::make_unique<tcpp::StringInputStream>(""); // Return empty stream, tcpp asserts on nullptr.
        }
        return std::make_unique<tcpp::StringInputStream>(std::string(pwd->read(path).str()));
    };

    tcpp::Preprocessor preprocessor(lexer, config);

    // Register handlers for directives that should pass through unchanged.
    for (std::string_view directive : passthroughDirectives) {
        std::string directiveStr(directive);
        preprocessor.AddCustomDirectiveHandler(directiveStr, [directiveStr](tcpp::Preprocessor &/*pp*/, tcpp::Lexer &lex) -> tcpp::TToken {
            std::string result = "#";
            result += directiveStr;
            tcpp::TToken token;
            while ((token = lex.GetNextToken()).mType != tcpp::E_TOKEN_TYPE::NEWLINE) {
                result += token.mRawView;
            }
            result += "\n";
            return {tcpp::E_TOKEN_TYPE::BLOB, result, lex.GetCurrLineIndex(), 0};
        });
    }

    auto tokens = preprocessor.Process();
    if (!errorMessage.empty())
        throw Exception("{}", errorMessage);

    return Blob::fromString(tcpp::Preprocessor::ToString(tokens)).withDisplayPath(source.displayPath());
}

} // namespace pp
