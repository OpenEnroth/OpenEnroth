#pragma once

#include <span>
#include <string_view>

#include "Utility/Memory/Blob.h"

class FileSystem;

namespace pp {

/**
 * Preprocesses source code, handling `#include` and `#define` directives.
 *
 * @param source                        The source code to preprocess.
 * @param pwd                           `FileSystem` for resolving includes (relative to its root).
 * @param preamble                      Optional preamble prepended before preprocessing (e.g., `#define GL_ES\n`).
 * @param passthroughDirectives         Directives that should pass through unchanged (e.g., `{"version", "extension"}`).
 * @return                              Preprocessed source code as a `Blob`.
 * @throws Exception                    On preprocessing errors.
 */
Blob preprocess(const Blob &source, const FileSystem *pwd, std::string_view preamble = {},
                std::span<const std::string_view> passthroughDirectives = {});

} // namespace pp
