#pragma once

#include <string>

/**
 * Like `std::getenv`, but takes and returns UTF8-encoded strings, even on Windows.
 *
 * @param key                           Name of the environmental variable to look for.
 * @return                              Value of the environmental variable or an empty string if it's not found.
 *                                      Note that this function doesn't differentiate between "no value" and
 *                                      "empty value", and you shouldn't either.
 */
std::string env(const std::string& key);

