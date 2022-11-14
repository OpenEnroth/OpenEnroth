#include <vector>
#include <memory>

#include "Platform/Platform.h"
#include "Utility/Log.h"

class SdlWindow;
class SdlPlatformSharedState;
class Log;

class SdlPlatform: public Platform {
 public:
    SdlPlatform(Log *log);
    virtual ~SdlPlatform();

    virtual std::unique_ptr<PlatformWindow> CreateWindow(std::unique_ptr<PlatformEventHandler> eventHandler) override;
    virtual std::unique_ptr<PlatformEventLoop> CreateEventLoop() override;
    virtual bool SendEvent(PlatformWindow *window, PlatformEvent *event) override;

    virtual void SetCursorShown(bool cursorShown) override;
    virtual bool IsCursorShown() const override;

    virtual std::vector<Recti> DisplayGeometries() const override;

 private:
    friend class SdlWindow;
    friend class SdlEventLoop;

 private:
    bool initialized_ = false;
    std::unique_ptr<SdlPlatformSharedState> state_;
};
