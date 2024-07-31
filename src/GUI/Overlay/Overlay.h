#pragma once

struct nk_context;

class Overlay {
 public:
    virtual ~Overlay() = default;
    virtual void update() = 0;
};
