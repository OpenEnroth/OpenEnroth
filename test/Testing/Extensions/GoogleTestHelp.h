#pragma once

/**
 * Prints Google Test's own command line help, for test binaries that parse their options with CLI11 first and have
 * already printed that help.
 *
 * @param app                           Name of the executable, `argv[0]`.
 */
void printGoogleTestHelp(char *app);
