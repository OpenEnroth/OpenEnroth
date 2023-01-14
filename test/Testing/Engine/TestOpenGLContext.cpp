#include "TestOpenGLContext.h"

#include <utility>

#include "TestStateHandle.h"

TestOpenGLContext::TestOpenGLContext(std::unique_ptr<PlatformOpenGLContext> base, TestStateHandle state) :
    ProxyOpenGLContext(base.get()),
    base_(std::move(base)),
    state_(std::move(state))
{}

void TestOpenGLContext::SwapBuffers() {
    base_->SwapBuffers();

    if (state_->terminating) {
        if (state_->terminationHandler) {
            state_->terminationHandler();
            state_->terminationHandler = nullptr;
        }
    } else {
        state_.YieldExecution();
    }

    // 16ms translates to 62.5fps.
    // It is possible to use double for state here (or store microseconds / nanoseconds), but this will result in
    // staggered frame times, with every 1st and 2nd frame taking 17ms, and every 3rd one taking 16ms.
    // This might result in some non-determinism down the line, e.g. changing the code in level loading will change
    // the number of frames it takes to load a level, and this will shift the timing sequence for the actual game
    // frames after the level is loaded. Unlikely to really affect anything, but we'd rather not find out.
    state_->time += 16;
}
