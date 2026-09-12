#pragma once

#ifdef __APPLE__ // Declared only where it's defined, so that a call from portable code fails to compile.

#include "Utility/System/NativePath.h"

/**
 * Tells the dialog where the crash log went, so that it can point the user at it. Until this is called the
 * dialog says only that the game crashed. Call it on the main thread, before any other thread is spawned. The
 * crash path reads what it writes, with nothing in between to synchronize them.
 *
 * @param path                          Native path of the crash log.
 */
void setCrashDialogLogPath(const NativePath &path);

/**
 * Shows a modal alert about the crash and blocks until it's dismissed. This is the counterpart of the windows
 * key wait for a bundle launched from finder, where stderr goes nowhere.
 */
void showCrashDialog();

#endif // __APPLE__
