#pragma once

#include <string>
#include <string_view>

/**
 * Reimplementation of `_sprintfex` from `mm7text.dll` that shipped with the Russian (Buka) localization of MM7.
 * Expands the `^`-tokens that Buka format strings pick Russian word forms with:
 * - `^I[<number>]` prints the number and remembers it for `^L`.
 * - `^L[a;b;c]` / `^L<n>[a;b;c]` picks a plural form by the n-th remembered number: `5 д^L[ень;ня;ней]` => `5 дней`.
 * - `^P<c>[<name>]` prints a name and remembers its gender for `^R`. `<c>` is a grammatical case letter
 *   (`IRDVTP`), only honored for a few special names.
 * - `^R[m;f;n]` picks an ending by the last `^P` gender: `^Pi[Анна] учитель^R[;ница;]` => `Анна учительница`.
 * Malformed tokens are copied through verbatim.
 *
 * @param str                           Formatted string, Windows-1251 encoded like all Buka in-game strings.
 * @return                              String with all valid `^`-tokens expanded, Windows-1251 encoded.
 */
[[nodiscard]] std::string sprintfex(std::string_view str);
