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
    // startup, so this is the one call.
    //
    // The timeout is what keeps an unwatched run from parking here forever, a zero would mean no timeout at
    // all. On expiry the system cancels the notification and this returns, and the re-raise that earns the
    // process its own crash report happens after that either way.
    CFOptionFlags response;
    CFUserNotificationDisplayAlert(60, kCFUserNotificationStopAlertLevel, nullptr, nullptr, nullptr,
                                   CFSTR("OpenEnroth crashed"), crashDialogText,
                                   nullptr, nullptr, nullptr, &response);
}

#endif // __APPLE__
