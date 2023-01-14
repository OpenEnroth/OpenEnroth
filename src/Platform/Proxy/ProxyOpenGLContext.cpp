#include "ProxyOpenGLContext.h"

ProxyOpenGLContext::ProxyOpenGLContext(PlatformOpenGLContext *base) : ProxyBase<PlatformOpenGLContext>(base) {}

bool ProxyOpenGLContext::MakeCurrent() {
    return NonNullBase()->MakeCurrent();
}

void ProxyOpenGLContext::SwapBuffers() {
    NonNullBase()->SwapBuffers();
}

void *ProxyOpenGLContext::GetProcAddress(const char *name) {
    return NonNullBase()->GetProcAddress(name);
}
