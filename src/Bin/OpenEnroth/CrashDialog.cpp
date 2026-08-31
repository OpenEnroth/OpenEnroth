#include "CrashDialog.h"

#ifdef __APPLE__

#include <atomic>
#include <string>

#include <unistd.h> // NOLINT: not a C++ system header.
#include <CoreFoundation/CoreFoundation.h> // NOLINT: not a C++ system header. Confined to this file, MacTypes.h defines Point, Size and Duration, and so does the engine.

static const bool stderrIsTerminal = isatty(STDERR_FILENO);
static std::atomic<CFStringRef> crashDialogText = CFSTR("OpenEnroth has crashed."); // Relaxed throughout, it's written once at startup.

void setCrashDialogText(std::string_view text) {
    std::string terminated(text);
    crashDialogText.store(CFStringCreateWithCString(nullptr, terminated.c_str(), kCFStringEncodingUTF8), std::memory_order_relaxed); // Never released, it's for the crash.
}

void showCrashDialog() {
    if (stderrIsTerminal)
        return;

    // The alert is rendered out of process by the system - no app object, no run loop, no main thread, all of
    // which a cocoa alert would want from a crashed worker thread. Everything it displays was created at
    // startup, so this is the one call. A zero timeout blocks until the dialog is dismissed, and the re-raise
    // that gets the system its own crash report happens after that.
    CFOptionFlags response;
    CFUserNotificationDisplayAlert(0, kCFUserNotificationStopAlertLevel, nullptr, nullptr, nullptr,
                                   CFSTR("OpenEnroth crashed"), crashDialogText.load(std::memory_order_relaxed),
                                   nullptr, nullptr, nullptr, &response);
}

#endif // __APPLE__
