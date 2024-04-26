#pragma once

class Nuklear;

class DebugView {
 public:
    virtual ~DebugView() = default;
    virtual void update(Nuklear &nuklear) = 0;
};
