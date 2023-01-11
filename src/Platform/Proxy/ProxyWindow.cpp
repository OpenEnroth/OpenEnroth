#include "ProxyWindow.h"

#include "Platform/PlatformOpenGLContext.h"

ProxyWindow::ProxyWindow(PlatformWindow *base) : ProxyBase<PlatformWindow>(base) {}

void ProxyWindow::SetTitle(const std::string &title) {
    NonNullBase()->SetTitle(title);
}

std::string ProxyWindow::Title() const {
    return NonNullBase()->Title();
}

void ProxyWindow::Resize(const Sizei &size) {
    NonNullBase()->Resize(size);
}

Sizei ProxyWindow::Size() const {
    return NonNullBase()->Size();
}

void ProxyWindow::SetPosition(const Pointi &pos) {
    NonNullBase()->SetPosition(pos);
}

Pointi ProxyWindow::Position() const {
    return NonNullBase()->Position();
}

void ProxyWindow::SetVisible(bool visible) {
    NonNullBase()->SetVisible(visible);
}

bool ProxyWindow::IsVisible() const {
    return NonNullBase()->IsVisible();
}

void ProxyWindow::SetResizable(bool resizable) {
    NonNullBase()->SetResizable(resizable);
}

bool ProxyWindow::Resizable() const {
    return NonNullBase()->Resizable();
}

void ProxyWindow::SetWindowMode(PlatformWindowMode mode) {
    NonNullBase()->SetWindowMode(mode);
}

PlatformWindowMode ProxyWindow::WindowMode() {
    return NonNullBase()->WindowMode();
}

void ProxyWindow::SetOrientations(PlatformWindowOrientations orientations) {
    NonNullBase()->SetOrientations(orientations);
}

PlatformWindowOrientations ProxyWindow::Orientations() {
    return NonNullBase()->Orientations();
}

void ProxyWindow::SetGrabsMouse(bool grabsMouse) {
    NonNullBase()->SetGrabsMouse(grabsMouse);
}

bool ProxyWindow::GrabsMouse() const {
    return NonNullBase()->GrabsMouse();
}

Marginsi ProxyWindow::FrameMargins() const {
    return NonNullBase()->FrameMargins();
}

uintptr_t ProxyWindow::SystemHandle() const {
    return NonNullBase()->SystemHandle();
}

void ProxyWindow::Activate() {
    NonNullBase()->Activate();
}

std::unique_ptr<PlatformOpenGLContext> ProxyWindow::CreateOpenGLContext(const PlatformOpenGLOptions &options) {
    return NonNullBase()->CreateOpenGLContext(options);
}
