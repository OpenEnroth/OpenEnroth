#pragma once

#include <memory>

class StackTraceOnCrash {
 public:
    StackTraceOnCrash();

 private:
    std::shared_ptr<void> _private;
};
