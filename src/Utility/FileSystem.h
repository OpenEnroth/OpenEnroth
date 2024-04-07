#pragma once

#include <filesystem>
#include <string>

/**
 * This function handles home-relative paths, so this is the preferred method of constructing
 * absolute paths instead of calling `std::filesystem::path` constructor.
 *
 * @param path                          Path as a string.
 * @param home                          Home path as a string.
 * @return                              Path as `std::filesystem::path`.
 */
std::filesystem::path expandUserPath(std::string_view path, std::string_view home);

/**
 * This function emulates the behavior of a case-insensitive filesystem. You pass in a path, this function traverses
 * it as if the underlying filesystem was case-insensitive, and returns a case-corrected path that actually exists.
 *
 * Case sensitivity is properly handled only for ascii characters.
 *
 * Note that this is not about Windows vs Linux. On Windows case sensitivity can be set per directory. On Linux it's
 * possible to mount case-insensitive partitions.
 *
 * @param path                          Requested path.
 * @return                              Case-corrected path if it exists, original path otherwise.
 */
std::filesystem::path makeCaseInsensitivePath(std::filesystem::path path);
