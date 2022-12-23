#include <vector>
#include <memory>
#include <string>

#include "Platform/Platform.h"

class SdlWindow;
class SdlPlatformSharedState;

class SdlPlatform: public Platform {
 public:
    explicit SdlPlatform(PlatformLogLevel platformLogLevel);
    virtual ~SdlPlatform();

    virtual PlatformLogger *Logger() const override;
    virtual std::unique_ptr<PlatformWindow> CreateWindow() override;
    virtual std::unique_ptr<PlatformEventLoop> CreateEventLoop() override;

    virtual void SetCursorShown(bool cursorShown) override;
    virtual bool IsCursorShown() const override;

    virtual std::vector<Recti> DisplayGeometries() const override;

    virtual void ShowMessageBox(const std::string &message, const std::string& title) const override;

    virtual int64_t TickCount() const override;

    virtual std::string WinQueryRegistry(const std::string &path) const override;

 private:
    friend class SdlWindow;
    friend class SdlEventLoop;

 private:
    bool initialized_ = false;
    std::unique_ptr<SdlPlatformSharedState> state_;
};
