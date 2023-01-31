#include "ProxyOpenGLContext.h"

ProxyOpenGLContext::ProxyOpenGLContext(PlatformOpenGLContext *base) : ProxyBase<PlatformOpenGLContext>(base) {}

bool ProxyOpenGLContext::Bind() {
    return NonNullBase()->Bind();
}

bool ProxyOpenGLContext::Unbind() {
    return NonNullBase()->Unbind();
}

void ProxyOpenGLContext::SwapBuffers() {
    NonNullBase()->SwapBuffers();
}

void *ProxyOpenGLContext::GetProcAddress(const char *name) {
    return NonNullBase()->GetProcAddress(name);
}
