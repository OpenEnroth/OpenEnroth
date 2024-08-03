#include "ProxyWindow.h"

#include <string>
#include <memory>

#include "Library/Platform/Interface/PlatformOpenGLContext.h"

ProxyWindow::ProxyWindow(PlatformWindow *base) : ProxyBase<PlatformWindow>(base) {}

void ProxyWindow::setTitle(const std::string &title) {
    nonNullBase()->setTitle(title);
}

std::string ProxyWindow::title() const {
    return nonNullBase()->title();
}

void ProxyWindow::resize(const Sizei &size) {
    nonNullBase()->resize(size);
}

Sizei ProxyWindow::size() const {
    return nonNullBase()->size();
}

void ProxyWindow::setPosition(const Pointi &pos) {
    nonNullBase()->setPosition(pos);
}

Pointi ProxyWindow::position() const {
    return nonNullBase()->position();
}

void ProxyWindow::setVisible(bool visible) {
    nonNullBase()->setVisible(visible);
}

bool ProxyWindow::isVisible() const {
    return nonNullBase()->isVisible();
}

void ProxyWindow::setResizable(bool resizable) {
    nonNullBase()->setResizable(resizable);
}

bool ProxyWindow::isResizable() const {
    return nonNullBase()->isResizable();
}

void ProxyWindow::setWindowMode(PlatformWindowMode mode) {
    nonNullBase()->setWindowMode(mode);
}

PlatformWindowMode ProxyWindow::windowMode() {
    return nonNullBase()->windowMode();
}

void ProxyWindow::setOrientations(PlatformWindowOrientations orientations) {
    nonNullBase()->setOrientations(orientations);
}

PlatformWindowOrientations ProxyWindow::orientations() {
    return nonNullBase()->orientations();
}

void ProxyWindow::setGrabsMouse(bool grabsMouse) {
    nonNullBase()->setGrabsMouse(grabsMouse);
}

bool ProxyWindow::grabsMouse() const {
    return nonNullBase()->grabsMouse();
}

Marginsi ProxyWindow::frameMargins() const {
    return nonNullBase()->frameMargins();
}

void *ProxyWindow::nativeHandle() const {
    return nonNullBase()->nativeHandle();
}

void ProxyWindow::activate() {
    nonNullBase()->activate();
}

std::unique_ptr<PlatformOpenGLContext> ProxyWindow::createOpenGLContext(const PlatformOpenGLOptions &options) {
    return nonNullBase()->createOpenGLContext(options);
}
