#pragma once

#include <cstdio>
#include <string>

/**
 * @return                              Stack trace of the calling function and everything below it, one frame per
 *                                      line. Returns a message saying so on platforms with no stack trace support.
 */
std::string stackTraceToString();

/**
 * Prints what `stackTraceToString` returns into the provided stream.
 *
 * @param stream                        Stream to print into.
 */
void printStackTrace(FILE *stream);
