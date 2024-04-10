#pragma once

class UiRenderer {
 public:
    virtual void render() = 0;
    virtual void setViewport(int width, int height) = 0;
    virtual void setOffset(int x, int y) = 0;
    virtual void getViewport(int &width, int &height) = 0;
};
