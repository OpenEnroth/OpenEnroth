#pragma once

#include <memory>

struct nk_context;

class Nuklear {
 public:
    Nuklear();
    ~Nuklear();

    void update();

    struct nk_context *getContext() const { return _context.get(); }

 private:
    std::unique_ptr<nk_context> _context;
};
