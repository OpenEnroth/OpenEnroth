#include "CrashDialog.h"

#ifdef __APPLE__

#include <string>

#include <CoreFoundation/CoreFoundation.h> // NOLINT: not a C++ system header. Confined to this file, MacTypes.h defines Point, Size and Duration, and so does the engine.

#include "Utility/String/Format.h"

static CFStringRef crashDialogText = CFSTR("OpenEnroth has crashed."); // Written once at startup, before any other thread is spawned.

void setCrashDialogLogPath(const NativePath &path) {
    std::string text = fmt::format("OpenEnroth has crashed.\nA crash log was written to:\n{}", path.displayString());
    crashDialogText = CFStringCreateWithCString(nullptr, text.c_str(), kCFStringEncodingUTF8); // Never released, it's for the crash.
}

void showCrashDialog() {
    // The alert is rendered out of process by the system - no app object, no run loop, no main thread, all of
    // which a cocoa alert would want from a crashed worker thread. Everything it displays was created at
    // startup, so this is the one call. A zero timeout blocks until the dialog is dismissed, and the re-raise
    // that gets the system its own crash report happens after that.
    CFOptionFlags response;
    CFUserNotificationDisplayAlert(0, kCFUserNotificationStopAlertLevel, nullptr, nullptr, nullptr,
                                   CFSTR("OpenEnroth crashed"), crashDialogText,
                                   nullptr, nullptr, nullptr, &response);
}

#endif // __APPLE__
