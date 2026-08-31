#pragma once

#include <string_view>

/**
 * Replaces the text of the crash dialog. Until this is called the dialog says only that the game crashed, so
 * call it as soon as the crash log path is known.
 *
 * @param text                          Dialog text, UTF-8.
 */
void setCrashDialogText(std::string_view text);

/**
 * Shows a modal alert about the crash and blocks until it's dismissed. Does nothing when stderr is a terminal -
 * a terminal user already sees the trace, and must not be blocked by a modal dialog. Mac only, this is the
 * counterpart of the windows key wait for a bundle launched from finder, where stderr goes nowhere.
 */
void showCrashDialog();
