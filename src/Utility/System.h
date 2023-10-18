#pragma once

#include <string>

/**
 * Switches to UTF-8 locale on Windows, does nothing on POSIX.
 *
 * Note that for this to work on older Windows versions, CRT should be statically linked. This is how OE releases
 * are built right now.
 *
 * @throws Exception                    On failure.
 * @see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?view=msvc-170#utf-8-support
 */
void winUseUtf8Crt();

/**
 * Windows-only function for querying the registry. Always returns an empty string on non-Windows systems.
 *
 * @param path                      Registry path to query.
 * @return                          Value at the given path, or an empty string in case of an error.
 */
std::string winQueryRegistry(const std::string &path);

/**
 * Same as `std::getenv`, but accepts & returns UTF-8 strings.
 *
 * This function is needed because on Windows even after switching to UTF-8 with a call to `winUseUtf8Crt`, CRT-supplied
 * `std::getenv` still can't handle unicode data.
 *
 * @param key                           Key to retrieve the env value for. Key is supposed to be utf8-encoded.
 * @return                              Utf8-encoded value for the provided key. Note that this function doesn't
 *                                      distinguish between non-existent and empty keys.
 */
std::string u8getenv(const std::string &key);
