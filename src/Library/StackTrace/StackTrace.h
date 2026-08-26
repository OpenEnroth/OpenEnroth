#pragma once

#include <cstddef>
#include <cstdio>
#include <string>

namespace detail {
/**
 * Deep enough for any call stack worth reading, shallow enough that a runaway recursion prints a trace instead
 * of megabytes of one.
 */
constexpr std::size_t MAX_TRACE_DEPTH = 128;
} // namespace detail

/**
 * @return                              Stack trace starting at this function, one frame per line. On
 *                                      platforms with no stack trace support, returns a message saying so.
 */
std::string stackTraceToString();

/**
 * Prints what `stackTraceToString` returns into the provided stream.
 *
 * @param stream                        Stream to print into.
 */
void printStackTrace(FILE *stream);
