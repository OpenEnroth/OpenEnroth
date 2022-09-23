#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <cstdio>

#include "Engine/Point.h"
#include "Utility/String.h"

bool OS_GetAppString(const char *path, char *out_string, int out_string_size);

void OS_MsgBox(const char *msg, const char *title);

void OS_Sleep(int ms);
void OS_ShowCursor(bool show);

bool OS_OpenConsole();

/**
 * @return                              Current value of a monotonic clock in milliseconds (1/1000s of a second).
 */
unsigned int OS_GetTime();
uint64_t OS_GetPrecisionTime();

std::vector<std::string> OS_FindFiles(const std::string &folder, const std::string &mask);

/**
 * On linux and on mac this function handles home-relative paths, so this is the preferred method of constructing
 * absolute paths instead of calling `std::filesystem::path` constructor.
 *
 * @param path                          Path as a string.
 * @return                              Path as `std::filesystem::path`.
 */
std::filesystem::path OS_makepath(std::string path);

/**
 * This function emulates windows behavior on posix. You pass in a path, this function traverses it as if
 * the underlying filesystem was case-insensitive, and returns a case-corrected path that actually exists.
 *
 * On windows this function just returns the path as is.
 *
 * @param path                          Requested path.
 * @return                              Case-corrected path.
 */
std::filesystem::path OS_casepath(std::filesystem::path path);

/**
 * A counterpart to the standard `fopen` that works with `std::filesystem::path`.
 *
 * @param path                          Path to the file to open.
 * @param mode                          Open mode.
 * @return                              Opened file pointer, or `nullptr` in case of an error.
 */
FILE *OS_fopen(std::filesystem::path path, const char *mode);

bool OS_FileExists(const std::string &path);
