#pragma once

/**
 * Prints an empty line, then Google Test's own command line help. For test binaries that have just printed their
 * CLI11 help.
 *
 * @param app                           Name of the executable, `argv[0]`.
 */
void printGoogleTestHelp(char *app);
