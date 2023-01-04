/**
 * SPDX-License-Identifier: (WTFPL OR CC0-1.0) AND Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/egl.h>

#ifndef GLAD_IMPL_UTIL_C_
#define GLAD_IMPL_UTIL_C_

#ifdef _MSC_VER
#define GLAD_IMPL_UTIL_SSCANF sscanf_s
#else
#define GLAD_IMPL_UTIL_SSCANF sscanf
#endif

#endif /* GLAD_IMPL_UTIL_C_ */

#ifdef __cplusplus
extern "C" {
#endif



int GLAD_EGL_VERSION_1_0 = 0;
int GLAD_EGL_VERSION_1_1 = 0;
int GLAD_EGL_VERSION_1_2 = 0;
int GLAD_EGL_VERSION_1_3 = 0;
int GLAD_EGL_VERSION_1_4 = 0;
int GLAD_EGL_VERSION_1_5 = 0;


static void _pre_call_egl_callback_default(const char *name, GLADapiproc apiproc, int len_args, ...) {
    GLAD_UNUSED(name);
    GLAD_UNUSED(apiproc);
    GLAD_UNUSED(len_args);
}
static void _post_call_egl_callback_default(void *ret, const char *name, GLADapiproc apiproc, int len_args, ...) {
    GLAD_UNUSED(ret);
    GLAD_UNUSED(name);
    GLAD_UNUSED(apiproc);
    GLAD_UNUSED(len_args);
}

static GLADprecallback _pre_call_egl_callback = _pre_call_egl_callback_default;
void gladSetEGLPreCallback(GLADprecallback cb) {
    _pre_call_egl_callback = cb;
}
static GLADpostcallback _post_call_egl_callback = _post_call_egl_callback_default;
void gladSetEGLPostCallback(GLADpostcallback cb) {
    _post_call_egl_callback = cb;
}

PFNEGLBINDAPIPROC glad_eglBindAPI = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglBindAPI(EGLenum api) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglBindAPI", (GLADapiproc) glad_eglBindAPI, 1, api);
    ret = glad_eglBindAPI(api);
    _post_call_egl_callback((void*) &ret, "eglBindAPI", (GLADapiproc) glad_eglBindAPI, 1, api);
    return ret;
}
PFNEGLBINDAPIPROC glad_debug_eglBindAPI = glad_debug_impl_eglBindAPI;
PFNEGLBINDTEXIMAGEPROC glad_eglBindTexImage = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglBindTexImage", (GLADapiproc) glad_eglBindTexImage, 3, dpy, surface, buffer);
    ret = glad_eglBindTexImage(dpy, surface, buffer);
    _post_call_egl_callback((void*) &ret, "eglBindTexImage", (GLADapiproc) glad_eglBindTexImage, 3, dpy, surface, buffer);
    return ret;
}
PFNEGLBINDTEXIMAGEPROC glad_debug_eglBindTexImage = glad_debug_impl_eglBindTexImage;
PFNEGLCHOOSECONFIGPROC glad_eglChooseConfig = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglChooseConfig(EGLDisplay dpy, const EGLint * attrib_list, EGLConfig * configs, EGLint config_size, EGLint * num_config) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglChooseConfig", (GLADapiproc) glad_eglChooseConfig, 5, dpy, attrib_list, configs, config_size, num_config);
    ret = glad_eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);
    _post_call_egl_callback((void*) &ret, "eglChooseConfig", (GLADapiproc) glad_eglChooseConfig, 5, dpy, attrib_list, configs, config_size, num_config);
    return ret;
}
PFNEGLCHOOSECONFIGPROC glad_debug_eglChooseConfig = glad_debug_impl_eglChooseConfig;
PFNEGLCLIENTWAITSYNCPROC glad_eglClientWaitSync = NULL;
static EGLint GLAD_API_PTR glad_debug_impl_eglClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout) {
    EGLint ret;
    _pre_call_egl_callback("eglClientWaitSync", (GLADapiproc) glad_eglClientWaitSync, 4, dpy, sync, flags, timeout);
    ret = glad_eglClientWaitSync(dpy, sync, flags, timeout);
    _post_call_egl_callback((void*) &ret, "eglClientWaitSync", (GLADapiproc) glad_eglClientWaitSync, 4, dpy, sync, flags, timeout);
    return ret;
}
PFNEGLCLIENTWAITSYNCPROC glad_debug_eglClientWaitSync = glad_debug_impl_eglClientWaitSync;
PFNEGLCOPYBUFFERSPROC glad_eglCopyBuffers = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglCopyBuffers", (GLADapiproc) glad_eglCopyBuffers, 3, dpy, surface, target);
    ret = glad_eglCopyBuffers(dpy, surface, target);
    _post_call_egl_callback((void*) &ret, "eglCopyBuffers", (GLADapiproc) glad_eglCopyBuffers, 3, dpy, surface, target);
    return ret;
}
PFNEGLCOPYBUFFERSPROC glad_debug_eglCopyBuffers = glad_debug_impl_eglCopyBuffers;
PFNEGLCREATECONTEXTPROC glad_eglCreateContext = NULL;
static EGLContext GLAD_API_PTR glad_debug_impl_eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint * attrib_list) {
    EGLContext ret;
    _pre_call_egl_callback("eglCreateContext", (GLADapiproc) glad_eglCreateContext, 4, dpy, config, share_context, attrib_list);
    ret = glad_eglCreateContext(dpy, config, share_context, attrib_list);
    _post_call_egl_callback((void*) &ret, "eglCreateContext", (GLADapiproc) glad_eglCreateContext, 4, dpy, config, share_context, attrib_list);
    return ret;
}
PFNEGLCREATECONTEXTPROC glad_debug_eglCreateContext = glad_debug_impl_eglCreateContext;
PFNEGLCREATEIMAGEPROC glad_eglCreateImage = NULL;
static EGLImage GLAD_API_PTR glad_debug_impl_eglCreateImage(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib * attrib_list) {
    EGLImage ret;
    _pre_call_egl_callback("eglCreateImage", (GLADapiproc) glad_eglCreateImage, 5, dpy, ctx, target, buffer, attrib_list);
    ret = glad_eglCreateImage(dpy, ctx, target, buffer, attrib_list);
    _post_call_egl_callback((void*) &ret, "eglCreateImage", (GLADapiproc) glad_eglCreateImage, 5, dpy, ctx, target, buffer, attrib_list);
    return ret;
}
PFNEGLCREATEIMAGEPROC glad_debug_eglCreateImage = glad_debug_impl_eglCreateImage;
PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC glad_eglCreatePbufferFromClientBuffer = NULL;
static EGLSurface GLAD_API_PTR glad_debug_impl_eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint * attrib_list) {
    EGLSurface ret;
    _pre_call_egl_callback("eglCreatePbufferFromClientBuffer", (GLADapiproc) glad_eglCreatePbufferFromClientBuffer, 5, dpy, buftype, buffer, config, attrib_list);
    ret = glad_eglCreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
    _post_call_egl_callback((void*) &ret, "eglCreatePbufferFromClientBuffer", (GLADapiproc) glad_eglCreatePbufferFromClientBuffer, 5, dpy, buftype, buffer, config, attrib_list);
    return ret;
}
PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC glad_debug_eglCreatePbufferFromClientBuffer = glad_debug_impl_eglCreatePbufferFromClientBuffer;
PFNEGLCREATEPBUFFERSURFACEPROC glad_eglCreatePbufferSurface = NULL;
static EGLSurface GLAD_API_PTR glad_debug_impl_eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint * attrib_list) {
    EGLSurface ret;
    _pre_call_egl_callback("eglCreatePbufferSurface", (GLADapiproc) glad_eglCreatePbufferSurface, 3, dpy, config, attrib_list);
    ret = glad_eglCreatePbufferSurface(dpy, config, attrib_list);
    _post_call_egl_callback((void*) &ret, "eglCreatePbufferSurface", (GLADapiproc) glad_eglCreatePbufferSurface, 3, dpy, config, attrib_list);
    return ret;
}
PFNEGLCREATEPBUFFERSURFACEPROC glad_debug_eglCreatePbufferSurface = glad_debug_impl_eglCreatePbufferSurface;
PFNEGLCREATEPIXMAPSURFACEPROC glad_eglCreatePixmapSurface = NULL;
static EGLSurface GLAD_API_PTR glad_debug_impl_eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint * attrib_list) {
    EGLSurface ret;
    _pre_call_egl_callback("eglCreatePixmapSurface", (GLADapiproc) glad_eglCreatePixmapSurface, 4, dpy, config, pixmap, attrib_list);
    ret = glad_eglCreatePixmapSurface(dpy, config, pixmap, attrib_list);
    _post_call_egl_callback((void*) &ret, "eglCreatePixmapSurface", (GLADapiproc) glad_eglCreatePixmapSurface, 4, dpy, config, pixmap, attrib_list);
    return ret;
}
PFNEGLCREATEPIXMAPSURFACEPROC glad_debug_eglCreatePixmapSurface = glad_debug_impl_eglCreatePixmapSurface;
PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC glad_eglCreatePlatformPixmapSurface = NULL;
static EGLSurface GLAD_API_PTR glad_debug_impl_eglCreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config, void * native_pixmap, const EGLAttrib * attrib_list) {
    EGLSurface ret;
    _pre_call_egl_callback("eglCreatePlatformPixmapSurface", (GLADapiproc) glad_eglCreatePlatformPixmapSurface, 4, dpy, config, native_pixmap, attrib_list);
    ret = glad_eglCreatePlatformPixmapSurface(dpy, config, native_pixmap, attrib_list);
    _post_call_egl_callback((void*) &ret, "eglCreatePlatformPixmapSurface", (GLADapiproc) glad_eglCreatePlatformPixmapSurface, 4, dpy, config, native_pixmap, attrib_list);
    return ret;
}
PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC glad_debug_eglCreatePlatformPixmapSurface = glad_debug_impl_eglCreatePlatformPixmapSurface;
PFNEGLCREATEPLATFORMWINDOWSURFACEPROC glad_eglCreatePlatformWindowSurface = NULL;
static EGLSurface GLAD_API_PTR glad_debug_impl_eglCreatePlatformWindowSurface(EGLDisplay dpy, EGLConfig config, void * native_window, const EGLAttrib * attrib_list) {
    EGLSurface ret;
    _pre_call_egl_callback("eglCreatePlatformWindowSurface", (GLADapiproc) glad_eglCreatePlatformWindowSurface, 4, dpy, config, native_window, attrib_list);
    ret = glad_eglCreatePlatformWindowSurface(dpy, config, native_window, attrib_list);
    _post_call_egl_callback((void*) &ret, "eglCreatePlatformWindowSurface", (GLADapiproc) glad_eglCreatePlatformWindowSurface, 4, dpy, config, native_window, attrib_list);
    return ret;
}
PFNEGLCREATEPLATFORMWINDOWSURFACEPROC glad_debug_eglCreatePlatformWindowSurface = glad_debug_impl_eglCreatePlatformWindowSurface;
PFNEGLCREATESYNCPROC glad_eglCreateSync = NULL;
static EGLSync GLAD_API_PTR glad_debug_impl_eglCreateSync(EGLDisplay dpy, EGLenum type, const EGLAttrib * attrib_list) {
    EGLSync ret;
    _pre_call_egl_callback("eglCreateSync", (GLADapiproc) glad_eglCreateSync, 3, dpy, type, attrib_list);
    ret = glad_eglCreateSync(dpy, type, attrib_list);
    _post_call_egl_callback((void*) &ret, "eglCreateSync", (GLADapiproc) glad_eglCreateSync, 3, dpy, type, attrib_list);
    return ret;
}
PFNEGLCREATESYNCPROC glad_debug_eglCreateSync = glad_debug_impl_eglCreateSync;
PFNEGLCREATEWINDOWSURFACEPROC glad_eglCreateWindowSurface = NULL;
static EGLSurface GLAD_API_PTR glad_debug_impl_eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint * attrib_list) {
    EGLSurface ret;
    _pre_call_egl_callback("eglCreateWindowSurface", (GLADapiproc) glad_eglCreateWindowSurface, 4, dpy, config, win, attrib_list);
    ret = glad_eglCreateWindowSurface(dpy, config, win, attrib_list);
    _post_call_egl_callback((void*) &ret, "eglCreateWindowSurface", (GLADapiproc) glad_eglCreateWindowSurface, 4, dpy, config, win, attrib_list);
    return ret;
}
PFNEGLCREATEWINDOWSURFACEPROC glad_debug_eglCreateWindowSurface = glad_debug_impl_eglCreateWindowSurface;
PFNEGLDESTROYCONTEXTPROC glad_eglDestroyContext = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglDestroyContext", (GLADapiproc) glad_eglDestroyContext, 2, dpy, ctx);
    ret = glad_eglDestroyContext(dpy, ctx);
    _post_call_egl_callback((void*) &ret, "eglDestroyContext", (GLADapiproc) glad_eglDestroyContext, 2, dpy, ctx);
    return ret;
}
PFNEGLDESTROYCONTEXTPROC glad_debug_eglDestroyContext = glad_debug_impl_eglDestroyContext;
PFNEGLDESTROYIMAGEPROC glad_eglDestroyImage = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglDestroyImage(EGLDisplay dpy, EGLImage image) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglDestroyImage", (GLADapiproc) glad_eglDestroyImage, 2, dpy, image);
    ret = glad_eglDestroyImage(dpy, image);
    _post_call_egl_callback((void*) &ret, "eglDestroyImage", (GLADapiproc) glad_eglDestroyImage, 2, dpy, image);
    return ret;
}
PFNEGLDESTROYIMAGEPROC glad_debug_eglDestroyImage = glad_debug_impl_eglDestroyImage;
PFNEGLDESTROYSURFACEPROC glad_eglDestroySurface = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglDestroySurface", (GLADapiproc) glad_eglDestroySurface, 2, dpy, surface);
    ret = glad_eglDestroySurface(dpy, surface);
    _post_call_egl_callback((void*) &ret, "eglDestroySurface", (GLADapiproc) glad_eglDestroySurface, 2, dpy, surface);
    return ret;
}
PFNEGLDESTROYSURFACEPROC glad_debug_eglDestroySurface = glad_debug_impl_eglDestroySurface;
PFNEGLDESTROYSYNCPROC glad_eglDestroySync = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglDestroySync(EGLDisplay dpy, EGLSync sync) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglDestroySync", (GLADapiproc) glad_eglDestroySync, 2, dpy, sync);
    ret = glad_eglDestroySync(dpy, sync);
    _post_call_egl_callback((void*) &ret, "eglDestroySync", (GLADapiproc) glad_eglDestroySync, 2, dpy, sync);
    return ret;
}
PFNEGLDESTROYSYNCPROC glad_debug_eglDestroySync = glad_debug_impl_eglDestroySync;
PFNEGLGETCONFIGATTRIBPROC glad_eglGetConfigAttrib = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint * value) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglGetConfigAttrib", (GLADapiproc) glad_eglGetConfigAttrib, 4, dpy, config, attribute, value);
    ret = glad_eglGetConfigAttrib(dpy, config, attribute, value);
    _post_call_egl_callback((void*) &ret, "eglGetConfigAttrib", (GLADapiproc) glad_eglGetConfigAttrib, 4, dpy, config, attribute, value);
    return ret;
}
PFNEGLGETCONFIGATTRIBPROC glad_debug_eglGetConfigAttrib = glad_debug_impl_eglGetConfigAttrib;
PFNEGLGETCONFIGSPROC glad_eglGetConfigs = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglGetConfigs(EGLDisplay dpy, EGLConfig * configs, EGLint config_size, EGLint * num_config) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglGetConfigs", (GLADapiproc) glad_eglGetConfigs, 4, dpy, configs, config_size, num_config);
    ret = glad_eglGetConfigs(dpy, configs, config_size, num_config);
    _post_call_egl_callback((void*) &ret, "eglGetConfigs", (GLADapiproc) glad_eglGetConfigs, 4, dpy, configs, config_size, num_config);
    return ret;
}
PFNEGLGETCONFIGSPROC glad_debug_eglGetConfigs = glad_debug_impl_eglGetConfigs;
PFNEGLGETCURRENTCONTEXTPROC glad_eglGetCurrentContext = NULL;
static EGLContext GLAD_API_PTR glad_debug_impl_eglGetCurrentContext(void) {
    EGLContext ret;
    _pre_call_egl_callback("eglGetCurrentContext", (GLADapiproc) glad_eglGetCurrentContext, 0);
    ret = glad_eglGetCurrentContext();
    _post_call_egl_callback((void*) &ret, "eglGetCurrentContext", (GLADapiproc) glad_eglGetCurrentContext, 0);
    return ret;
}
PFNEGLGETCURRENTCONTEXTPROC glad_debug_eglGetCurrentContext = glad_debug_impl_eglGetCurrentContext;
PFNEGLGETCURRENTDISPLAYPROC glad_eglGetCurrentDisplay = NULL;
static EGLDisplay GLAD_API_PTR glad_debug_impl_eglGetCurrentDisplay(void) {
    EGLDisplay ret;
    _pre_call_egl_callback("eglGetCurrentDisplay", (GLADapiproc) glad_eglGetCurrentDisplay, 0);
    ret = glad_eglGetCurrentDisplay();
    _post_call_egl_callback((void*) &ret, "eglGetCurrentDisplay", (GLADapiproc) glad_eglGetCurrentDisplay, 0);
    return ret;
}
PFNEGLGETCURRENTDISPLAYPROC glad_debug_eglGetCurrentDisplay = glad_debug_impl_eglGetCurrentDisplay;
PFNEGLGETCURRENTSURFACEPROC glad_eglGetCurrentSurface = NULL;
static EGLSurface GLAD_API_PTR glad_debug_impl_eglGetCurrentSurface(EGLint readdraw) {
    EGLSurface ret;
    _pre_call_egl_callback("eglGetCurrentSurface", (GLADapiproc) glad_eglGetCurrentSurface, 1, readdraw);
    ret = glad_eglGetCurrentSurface(readdraw);
    _post_call_egl_callback((void*) &ret, "eglGetCurrentSurface", (GLADapiproc) glad_eglGetCurrentSurface, 1, readdraw);
    return ret;
}
PFNEGLGETCURRENTSURFACEPROC glad_debug_eglGetCurrentSurface = glad_debug_impl_eglGetCurrentSurface;
PFNEGLGETDISPLAYPROC glad_eglGetDisplay = NULL;
static EGLDisplay GLAD_API_PTR glad_debug_impl_eglGetDisplay(EGLNativeDisplayType display_id) {
    EGLDisplay ret;
    _pre_call_egl_callback("eglGetDisplay", (GLADapiproc) glad_eglGetDisplay, 1, display_id);
    ret = glad_eglGetDisplay(display_id);
    _post_call_egl_callback((void*) &ret, "eglGetDisplay", (GLADapiproc) glad_eglGetDisplay, 1, display_id);
    return ret;
}
PFNEGLGETDISPLAYPROC glad_debug_eglGetDisplay = glad_debug_impl_eglGetDisplay;
PFNEGLGETERRORPROC glad_eglGetError = NULL;
static EGLint GLAD_API_PTR glad_debug_impl_eglGetError(void) {
    EGLint ret;
    _pre_call_egl_callback("eglGetError", (GLADapiproc) glad_eglGetError, 0);
    ret = glad_eglGetError();
    _post_call_egl_callback((void*) &ret, "eglGetError", (GLADapiproc) glad_eglGetError, 0);
    return ret;
}
PFNEGLGETERRORPROC glad_debug_eglGetError = glad_debug_impl_eglGetError;
PFNEGLGETPLATFORMDISPLAYPROC glad_eglGetPlatformDisplay = NULL;
static EGLDisplay GLAD_API_PTR glad_debug_impl_eglGetPlatformDisplay(EGLenum platform, void * native_display, const EGLAttrib * attrib_list) {
    EGLDisplay ret;
    _pre_call_egl_callback("eglGetPlatformDisplay", (GLADapiproc) glad_eglGetPlatformDisplay, 3, platform, native_display, attrib_list);
    ret = glad_eglGetPlatformDisplay(platform, native_display, attrib_list);
    _post_call_egl_callback((void*) &ret, "eglGetPlatformDisplay", (GLADapiproc) glad_eglGetPlatformDisplay, 3, platform, native_display, attrib_list);
    return ret;
}
PFNEGLGETPLATFORMDISPLAYPROC glad_debug_eglGetPlatformDisplay = glad_debug_impl_eglGetPlatformDisplay;
PFNEGLGETPROCADDRESSPROC glad_eglGetProcAddress = NULL;
static __eglMustCastToProperFunctionPointerType GLAD_API_PTR glad_debug_impl_eglGetProcAddress(const char * procname) {
    __eglMustCastToProperFunctionPointerType ret;
    _pre_call_egl_callback("eglGetProcAddress", (GLADapiproc) glad_eglGetProcAddress, 1, procname);
    ret = glad_eglGetProcAddress(procname);
    _post_call_egl_callback((void*) &ret, "eglGetProcAddress", (GLADapiproc) glad_eglGetProcAddress, 1, procname);
    return ret;
}
PFNEGLGETPROCADDRESSPROC glad_debug_eglGetProcAddress = glad_debug_impl_eglGetProcAddress;
PFNEGLGETSYNCATTRIBPROC glad_eglGetSyncAttrib = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglGetSyncAttrib(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib * value) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglGetSyncAttrib", (GLADapiproc) glad_eglGetSyncAttrib, 4, dpy, sync, attribute, value);
    ret = glad_eglGetSyncAttrib(dpy, sync, attribute, value);
    _post_call_egl_callback((void*) &ret, "eglGetSyncAttrib", (GLADapiproc) glad_eglGetSyncAttrib, 4, dpy, sync, attribute, value);
    return ret;
}
PFNEGLGETSYNCATTRIBPROC glad_debug_eglGetSyncAttrib = glad_debug_impl_eglGetSyncAttrib;
PFNEGLINITIALIZEPROC glad_eglInitialize = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglInitialize(EGLDisplay dpy, EGLint * major, EGLint * minor) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglInitialize", (GLADapiproc) glad_eglInitialize, 3, dpy, major, minor);
    ret = glad_eglInitialize(dpy, major, minor);
    _post_call_egl_callback((void*) &ret, "eglInitialize", (GLADapiproc) glad_eglInitialize, 3, dpy, major, minor);
    return ret;
}
PFNEGLINITIALIZEPROC glad_debug_eglInitialize = glad_debug_impl_eglInitialize;
PFNEGLMAKECURRENTPROC glad_eglMakeCurrent = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglMakeCurrent", (GLADapiproc) glad_eglMakeCurrent, 4, dpy, draw, read, ctx);
    ret = glad_eglMakeCurrent(dpy, draw, read, ctx);
    _post_call_egl_callback((void*) &ret, "eglMakeCurrent", (GLADapiproc) glad_eglMakeCurrent, 4, dpy, draw, read, ctx);
    return ret;
}
PFNEGLMAKECURRENTPROC glad_debug_eglMakeCurrent = glad_debug_impl_eglMakeCurrent;
PFNEGLQUERYAPIPROC glad_eglQueryAPI = NULL;
static EGLenum GLAD_API_PTR glad_debug_impl_eglQueryAPI(void) {
    EGLenum ret;
    _pre_call_egl_callback("eglQueryAPI", (GLADapiproc) glad_eglQueryAPI, 0);
    ret = glad_eglQueryAPI();
    _post_call_egl_callback((void*) &ret, "eglQueryAPI", (GLADapiproc) glad_eglQueryAPI, 0);
    return ret;
}
PFNEGLQUERYAPIPROC glad_debug_eglQueryAPI = glad_debug_impl_eglQueryAPI;
PFNEGLQUERYCONTEXTPROC glad_eglQueryContext = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint * value) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglQueryContext", (GLADapiproc) glad_eglQueryContext, 4, dpy, ctx, attribute, value);
    ret = glad_eglQueryContext(dpy, ctx, attribute, value);
    _post_call_egl_callback((void*) &ret, "eglQueryContext", (GLADapiproc) glad_eglQueryContext, 4, dpy, ctx, attribute, value);
    return ret;
}
PFNEGLQUERYCONTEXTPROC glad_debug_eglQueryContext = glad_debug_impl_eglQueryContext;
PFNEGLQUERYSTRINGPROC glad_eglQueryString = NULL;
static const char * GLAD_API_PTR glad_debug_impl_eglQueryString(EGLDisplay dpy, EGLint name) {
    const char * ret;
    _pre_call_egl_callback("eglQueryString", (GLADapiproc) glad_eglQueryString, 2, dpy, name);
    ret = glad_eglQueryString(dpy, name);
    _post_call_egl_callback((void*) &ret, "eglQueryString", (GLADapiproc) glad_eglQueryString, 2, dpy, name);
    return ret;
}
PFNEGLQUERYSTRINGPROC glad_debug_eglQueryString = glad_debug_impl_eglQueryString;
PFNEGLQUERYSURFACEPROC glad_eglQuerySurface = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint * value) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglQuerySurface", (GLADapiproc) glad_eglQuerySurface, 4, dpy, surface, attribute, value);
    ret = glad_eglQuerySurface(dpy, surface, attribute, value);
    _post_call_egl_callback((void*) &ret, "eglQuerySurface", (GLADapiproc) glad_eglQuerySurface, 4, dpy, surface, attribute, value);
    return ret;
}
PFNEGLQUERYSURFACEPROC glad_debug_eglQuerySurface = glad_debug_impl_eglQuerySurface;
PFNEGLRELEASETEXIMAGEPROC glad_eglReleaseTexImage = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglReleaseTexImage", (GLADapiproc) glad_eglReleaseTexImage, 3, dpy, surface, buffer);
    ret = glad_eglReleaseTexImage(dpy, surface, buffer);
    _post_call_egl_callback((void*) &ret, "eglReleaseTexImage", (GLADapiproc) glad_eglReleaseTexImage, 3, dpy, surface, buffer);
    return ret;
}
PFNEGLRELEASETEXIMAGEPROC glad_debug_eglReleaseTexImage = glad_debug_impl_eglReleaseTexImage;
PFNEGLRELEASETHREADPROC glad_eglReleaseThread = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglReleaseThread(void) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglReleaseThread", (GLADapiproc) glad_eglReleaseThread, 0);
    ret = glad_eglReleaseThread();
    _post_call_egl_callback((void*) &ret, "eglReleaseThread", (GLADapiproc) glad_eglReleaseThread, 0);
    return ret;
}
PFNEGLRELEASETHREADPROC glad_debug_eglReleaseThread = glad_debug_impl_eglReleaseThread;
PFNEGLSURFACEATTRIBPROC glad_eglSurfaceAttrib = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglSurfaceAttrib", (GLADapiproc) glad_eglSurfaceAttrib, 4, dpy, surface, attribute, value);
    ret = glad_eglSurfaceAttrib(dpy, surface, attribute, value);
    _post_call_egl_callback((void*) &ret, "eglSurfaceAttrib", (GLADapiproc) glad_eglSurfaceAttrib, 4, dpy, surface, attribute, value);
    return ret;
}
PFNEGLSURFACEATTRIBPROC glad_debug_eglSurfaceAttrib = glad_debug_impl_eglSurfaceAttrib;
PFNEGLSWAPBUFFERSPROC glad_eglSwapBuffers = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglSwapBuffers", (GLADapiproc) glad_eglSwapBuffers, 2, dpy, surface);
    ret = glad_eglSwapBuffers(dpy, surface);
    _post_call_egl_callback((void*) &ret, "eglSwapBuffers", (GLADapiproc) glad_eglSwapBuffers, 2, dpy, surface);
    return ret;
}
PFNEGLSWAPBUFFERSPROC glad_debug_eglSwapBuffers = glad_debug_impl_eglSwapBuffers;
PFNEGLSWAPINTERVALPROC glad_eglSwapInterval = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglSwapInterval(EGLDisplay dpy, EGLint interval) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglSwapInterval", (GLADapiproc) glad_eglSwapInterval, 2, dpy, interval);
    ret = glad_eglSwapInterval(dpy, interval);
    _post_call_egl_callback((void*) &ret, "eglSwapInterval", (GLADapiproc) glad_eglSwapInterval, 2, dpy, interval);
    return ret;
}
PFNEGLSWAPINTERVALPROC glad_debug_eglSwapInterval = glad_debug_impl_eglSwapInterval;
PFNEGLTERMINATEPROC glad_eglTerminate = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglTerminate(EGLDisplay dpy) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglTerminate", (GLADapiproc) glad_eglTerminate, 1, dpy);
    ret = glad_eglTerminate(dpy);
    _post_call_egl_callback((void*) &ret, "eglTerminate", (GLADapiproc) glad_eglTerminate, 1, dpy);
    return ret;
}
PFNEGLTERMINATEPROC glad_debug_eglTerminate = glad_debug_impl_eglTerminate;
PFNEGLWAITCLIENTPROC glad_eglWaitClient = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglWaitClient(void) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglWaitClient", (GLADapiproc) glad_eglWaitClient, 0);
    ret = glad_eglWaitClient();
    _post_call_egl_callback((void*) &ret, "eglWaitClient", (GLADapiproc) glad_eglWaitClient, 0);
    return ret;
}
PFNEGLWAITCLIENTPROC glad_debug_eglWaitClient = glad_debug_impl_eglWaitClient;
PFNEGLWAITGLPROC glad_eglWaitGL = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglWaitGL(void) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglWaitGL", (GLADapiproc) glad_eglWaitGL, 0);
    ret = glad_eglWaitGL();
    _post_call_egl_callback((void*) &ret, "eglWaitGL", (GLADapiproc) glad_eglWaitGL, 0);
    return ret;
}
PFNEGLWAITGLPROC glad_debug_eglWaitGL = glad_debug_impl_eglWaitGL;
PFNEGLWAITNATIVEPROC glad_eglWaitNative = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglWaitNative(EGLint engine) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglWaitNative", (GLADapiproc) glad_eglWaitNative, 1, engine);
    ret = glad_eglWaitNative(engine);
    _post_call_egl_callback((void*) &ret, "eglWaitNative", (GLADapiproc) glad_eglWaitNative, 1, engine);
    return ret;
}
PFNEGLWAITNATIVEPROC glad_debug_eglWaitNative = glad_debug_impl_eglWaitNative;
PFNEGLWAITSYNCPROC glad_eglWaitSync = NULL;
static EGLBoolean GLAD_API_PTR glad_debug_impl_eglWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags) {
    EGLBoolean ret;
    _pre_call_egl_callback("eglWaitSync", (GLADapiproc) glad_eglWaitSync, 3, dpy, sync, flags);
    ret = glad_eglWaitSync(dpy, sync, flags);
    _post_call_egl_callback((void*) &ret, "eglWaitSync", (GLADapiproc) glad_eglWaitSync, 3, dpy, sync, flags);
    return ret;
}
PFNEGLWAITSYNCPROC glad_debug_eglWaitSync = glad_debug_impl_eglWaitSync;


static void glad_egl_load_EGL_VERSION_1_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_VERSION_1_0) return;
    glad_eglChooseConfig = (PFNEGLCHOOSECONFIGPROC) load(userptr, "eglChooseConfig");
    glad_eglCopyBuffers = (PFNEGLCOPYBUFFERSPROC) load(userptr, "eglCopyBuffers");
    glad_eglCreateContext = (PFNEGLCREATECONTEXTPROC) load(userptr, "eglCreateContext");
    glad_eglCreatePbufferSurface = (PFNEGLCREATEPBUFFERSURFACEPROC) load(userptr, "eglCreatePbufferSurface");
    glad_eglCreatePixmapSurface = (PFNEGLCREATEPIXMAPSURFACEPROC) load(userptr, "eglCreatePixmapSurface");
    glad_eglCreateWindowSurface = (PFNEGLCREATEWINDOWSURFACEPROC) load(userptr, "eglCreateWindowSurface");
    glad_eglDestroyContext = (PFNEGLDESTROYCONTEXTPROC) load(userptr, "eglDestroyContext");
    glad_eglDestroySurface = (PFNEGLDESTROYSURFACEPROC) load(userptr, "eglDestroySurface");
    glad_eglGetConfigAttrib = (PFNEGLGETCONFIGATTRIBPROC) load(userptr, "eglGetConfigAttrib");
    glad_eglGetConfigs = (PFNEGLGETCONFIGSPROC) load(userptr, "eglGetConfigs");
    glad_eglGetCurrentDisplay = (PFNEGLGETCURRENTDISPLAYPROC) load(userptr, "eglGetCurrentDisplay");
    glad_eglGetCurrentSurface = (PFNEGLGETCURRENTSURFACEPROC) load(userptr, "eglGetCurrentSurface");
    glad_eglGetDisplay = (PFNEGLGETDISPLAYPROC) load(userptr, "eglGetDisplay");
    glad_eglGetError = (PFNEGLGETERRORPROC) load(userptr, "eglGetError");
    glad_eglGetProcAddress = (PFNEGLGETPROCADDRESSPROC) load(userptr, "eglGetProcAddress");
    glad_eglInitialize = (PFNEGLINITIALIZEPROC) load(userptr, "eglInitialize");
    glad_eglMakeCurrent = (PFNEGLMAKECURRENTPROC) load(userptr, "eglMakeCurrent");
    glad_eglQueryContext = (PFNEGLQUERYCONTEXTPROC) load(userptr, "eglQueryContext");
    glad_eglQueryString = (PFNEGLQUERYSTRINGPROC) load(userptr, "eglQueryString");
    glad_eglQuerySurface = (PFNEGLQUERYSURFACEPROC) load(userptr, "eglQuerySurface");
    glad_eglSwapBuffers = (PFNEGLSWAPBUFFERSPROC) load(userptr, "eglSwapBuffers");
    glad_eglTerminate = (PFNEGLTERMINATEPROC) load(userptr, "eglTerminate");
    glad_eglWaitGL = (PFNEGLWAITGLPROC) load(userptr, "eglWaitGL");
    glad_eglWaitNative = (PFNEGLWAITNATIVEPROC) load(userptr, "eglWaitNative");
}
static void glad_egl_load_EGL_VERSION_1_1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_VERSION_1_1) return;
    glad_eglBindTexImage = (PFNEGLBINDTEXIMAGEPROC) load(userptr, "eglBindTexImage");
    glad_eglReleaseTexImage = (PFNEGLRELEASETEXIMAGEPROC) load(userptr, "eglReleaseTexImage");
    glad_eglSurfaceAttrib = (PFNEGLSURFACEATTRIBPROC) load(userptr, "eglSurfaceAttrib");
    glad_eglSwapInterval = (PFNEGLSWAPINTERVALPROC) load(userptr, "eglSwapInterval");
}
static void glad_egl_load_EGL_VERSION_1_2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_VERSION_1_2) return;
    glad_eglBindAPI = (PFNEGLBINDAPIPROC) load(userptr, "eglBindAPI");
    glad_eglCreatePbufferFromClientBuffer = (PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC) load(userptr, "eglCreatePbufferFromClientBuffer");
    glad_eglQueryAPI = (PFNEGLQUERYAPIPROC) load(userptr, "eglQueryAPI");
    glad_eglReleaseThread = (PFNEGLRELEASETHREADPROC) load(userptr, "eglReleaseThread");
    glad_eglWaitClient = (PFNEGLWAITCLIENTPROC) load(userptr, "eglWaitClient");
}
static void glad_egl_load_EGL_VERSION_1_4( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_VERSION_1_4) return;
    glad_eglGetCurrentContext = (PFNEGLGETCURRENTCONTEXTPROC) load(userptr, "eglGetCurrentContext");
}
static void glad_egl_load_EGL_VERSION_1_5( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_VERSION_1_5) return;
    glad_eglClientWaitSync = (PFNEGLCLIENTWAITSYNCPROC) load(userptr, "eglClientWaitSync");
    glad_eglCreateImage = (PFNEGLCREATEIMAGEPROC) load(userptr, "eglCreateImage");
    glad_eglCreatePlatformPixmapSurface = (PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC) load(userptr, "eglCreatePlatformPixmapSurface");
    glad_eglCreatePlatformWindowSurface = (PFNEGLCREATEPLATFORMWINDOWSURFACEPROC) load(userptr, "eglCreatePlatformWindowSurface");
    glad_eglCreateSync = (PFNEGLCREATESYNCPROC) load(userptr, "eglCreateSync");
    glad_eglDestroyImage = (PFNEGLDESTROYIMAGEPROC) load(userptr, "eglDestroyImage");
    glad_eglDestroySync = (PFNEGLDESTROYSYNCPROC) load(userptr, "eglDestroySync");
    glad_eglGetPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYPROC) load(userptr, "eglGetPlatformDisplay");
    glad_eglGetSyncAttrib = (PFNEGLGETSYNCATTRIBPROC) load(userptr, "eglGetSyncAttrib");
    glad_eglWaitSync = (PFNEGLWAITSYNCPROC) load(userptr, "eglWaitSync");
}



static int glad_egl_get_extensions(EGLDisplay display, const char **extensions) {
    *extensions = eglQueryString(display, EGL_EXTENSIONS);

    return extensions != NULL;
}

static int glad_egl_has_extension(const char *extensions, const char *ext) {
    const char *loc;
    const char *terminator;
    if(extensions == NULL) {
        return 0;
    }
    while(1) {
        loc = strstr(extensions, ext);
        if(loc == NULL) {
            return 0;
        }
        terminator = loc + strlen(ext);
        if((loc == extensions || *(loc - 1) == ' ') &&
            (*terminator == ' ' || *terminator == '\0')) {
            return 1;
        }
        extensions = terminator;
    }
}

static GLADapiproc glad_egl_get_proc_from_userptr(void *userptr, const char *name) {
    return (GLAD_GNUC_EXTENSION (GLADapiproc (*)(const char *name)) userptr)(name);
}

static int glad_egl_find_extensions_egl(EGLDisplay display) {
    const char *extensions;
    if (!glad_egl_get_extensions(display, &extensions)) return 0;

    GLAD_UNUSED(glad_egl_has_extension);

    return 1;
}

static int glad_egl_find_core_egl(EGLDisplay display) {
    int major, minor;
    const char *version;

    if (display == NULL) {
        display = EGL_NO_DISPLAY; /* this is usually NULL, better safe than sorry */
    }
    if (display == EGL_NO_DISPLAY) {
        display = eglGetCurrentDisplay();
    }
#ifdef EGL_VERSION_1_4
    if (display == EGL_NO_DISPLAY) {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    }
#endif
#ifndef EGL_VERSION_1_5
    if (display == EGL_NO_DISPLAY) {
        return 0;
    }
#endif

    version = eglQueryString(display, EGL_VERSION);
    (void) eglGetError();

    if (version == NULL) {
        major = 1;
        minor = 0;
    } else {
        GLAD_IMPL_UTIL_SSCANF(version, "%d.%d", &major, &minor);
    }

    GLAD_EGL_VERSION_1_0 = (major == 1 && minor >= 0) || major > 1;
    GLAD_EGL_VERSION_1_1 = (major == 1 && minor >= 1) || major > 1;
    GLAD_EGL_VERSION_1_2 = (major == 1 && minor >= 2) || major > 1;
    GLAD_EGL_VERSION_1_3 = (major == 1 && minor >= 3) || major > 1;
    GLAD_EGL_VERSION_1_4 = (major == 1 && minor >= 4) || major > 1;
    GLAD_EGL_VERSION_1_5 = (major == 1 && minor >= 5) || major > 1;

    return GLAD_MAKE_VERSION(major, minor);
}

int gladLoadEGLUserPtr(EGLDisplay display, GLADuserptrloadfunc load, void* userptr) {
    int version;
    eglGetDisplay = (PFNEGLGETDISPLAYPROC) load(userptr, "eglGetDisplay");
    eglGetCurrentDisplay = (PFNEGLGETCURRENTDISPLAYPROC) load(userptr, "eglGetCurrentDisplay");
    eglQueryString = (PFNEGLQUERYSTRINGPROC) load(userptr, "eglQueryString");
    eglGetError = (PFNEGLGETERRORPROC) load(userptr, "eglGetError");
    if (eglGetDisplay == NULL || eglGetCurrentDisplay == NULL || eglQueryString == NULL || eglGetError == NULL) return 0;

    version = glad_egl_find_core_egl(display);
    if (!version) return 0;
    glad_egl_load_EGL_VERSION_1_0(load, userptr);
    glad_egl_load_EGL_VERSION_1_1(load, userptr);
    glad_egl_load_EGL_VERSION_1_2(load, userptr);
    glad_egl_load_EGL_VERSION_1_4(load, userptr);
    glad_egl_load_EGL_VERSION_1_5(load, userptr);

    if (!glad_egl_find_extensions_egl(display)) return 0;


    return version;
}

int gladLoadEGL(EGLDisplay display, GLADloadfunc load) {
    return gladLoadEGLUserPtr(display, glad_egl_get_proc_from_userptr, GLAD_GNUC_EXTENSION (void*) load);
}

 
void gladInstallEGLDebug() {
    glad_debug_eglBindAPI = glad_debug_impl_eglBindAPI;
    glad_debug_eglBindTexImage = glad_debug_impl_eglBindTexImage;
    glad_debug_eglChooseConfig = glad_debug_impl_eglChooseConfig;
    glad_debug_eglClientWaitSync = glad_debug_impl_eglClientWaitSync;
    glad_debug_eglCopyBuffers = glad_debug_impl_eglCopyBuffers;
    glad_debug_eglCreateContext = glad_debug_impl_eglCreateContext;
    glad_debug_eglCreateImage = glad_debug_impl_eglCreateImage;
    glad_debug_eglCreatePbufferFromClientBuffer = glad_debug_impl_eglCreatePbufferFromClientBuffer;
    glad_debug_eglCreatePbufferSurface = glad_debug_impl_eglCreatePbufferSurface;
    glad_debug_eglCreatePixmapSurface = glad_debug_impl_eglCreatePixmapSurface;
    glad_debug_eglCreatePlatformPixmapSurface = glad_debug_impl_eglCreatePlatformPixmapSurface;
    glad_debug_eglCreatePlatformWindowSurface = glad_debug_impl_eglCreatePlatformWindowSurface;
    glad_debug_eglCreateSync = glad_debug_impl_eglCreateSync;
    glad_debug_eglCreateWindowSurface = glad_debug_impl_eglCreateWindowSurface;
    glad_debug_eglDestroyContext = glad_debug_impl_eglDestroyContext;
    glad_debug_eglDestroyImage = glad_debug_impl_eglDestroyImage;
    glad_debug_eglDestroySurface = glad_debug_impl_eglDestroySurface;
    glad_debug_eglDestroySync = glad_debug_impl_eglDestroySync;
    glad_debug_eglGetConfigAttrib = glad_debug_impl_eglGetConfigAttrib;
    glad_debug_eglGetConfigs = glad_debug_impl_eglGetConfigs;
    glad_debug_eglGetCurrentContext = glad_debug_impl_eglGetCurrentContext;
    glad_debug_eglGetCurrentDisplay = glad_debug_impl_eglGetCurrentDisplay;
    glad_debug_eglGetCurrentSurface = glad_debug_impl_eglGetCurrentSurface;
    glad_debug_eglGetDisplay = glad_debug_impl_eglGetDisplay;
    glad_debug_eglGetError = glad_debug_impl_eglGetError;
    glad_debug_eglGetPlatformDisplay = glad_debug_impl_eglGetPlatformDisplay;
    glad_debug_eglGetProcAddress = glad_debug_impl_eglGetProcAddress;
    glad_debug_eglGetSyncAttrib = glad_debug_impl_eglGetSyncAttrib;
    glad_debug_eglInitialize = glad_debug_impl_eglInitialize;
    glad_debug_eglMakeCurrent = glad_debug_impl_eglMakeCurrent;
    glad_debug_eglQueryAPI = glad_debug_impl_eglQueryAPI;
    glad_debug_eglQueryContext = glad_debug_impl_eglQueryContext;
    glad_debug_eglQueryString = glad_debug_impl_eglQueryString;
    glad_debug_eglQuerySurface = glad_debug_impl_eglQuerySurface;
    glad_debug_eglReleaseTexImage = glad_debug_impl_eglReleaseTexImage;
    glad_debug_eglReleaseThread = glad_debug_impl_eglReleaseThread;
    glad_debug_eglSurfaceAttrib = glad_debug_impl_eglSurfaceAttrib;
    glad_debug_eglSwapBuffers = glad_debug_impl_eglSwapBuffers;
    glad_debug_eglSwapInterval = glad_debug_impl_eglSwapInterval;
    glad_debug_eglTerminate = glad_debug_impl_eglTerminate;
    glad_debug_eglWaitClient = glad_debug_impl_eglWaitClient;
    glad_debug_eglWaitGL = glad_debug_impl_eglWaitGL;
    glad_debug_eglWaitNative = glad_debug_impl_eglWaitNative;
    glad_debug_eglWaitSync = glad_debug_impl_eglWaitSync;
}

void gladUninstallEGLDebug() {
    glad_debug_eglBindAPI = glad_eglBindAPI;
    glad_debug_eglBindTexImage = glad_eglBindTexImage;
    glad_debug_eglChooseConfig = glad_eglChooseConfig;
    glad_debug_eglClientWaitSync = glad_eglClientWaitSync;
    glad_debug_eglCopyBuffers = glad_eglCopyBuffers;
    glad_debug_eglCreateContext = glad_eglCreateContext;
    glad_debug_eglCreateImage = glad_eglCreateImage;
    glad_debug_eglCreatePbufferFromClientBuffer = glad_eglCreatePbufferFromClientBuffer;
    glad_debug_eglCreatePbufferSurface = glad_eglCreatePbufferSurface;
    glad_debug_eglCreatePixmapSurface = glad_eglCreatePixmapSurface;
    glad_debug_eglCreatePlatformPixmapSurface = glad_eglCreatePlatformPixmapSurface;
    glad_debug_eglCreatePlatformWindowSurface = glad_eglCreatePlatformWindowSurface;
    glad_debug_eglCreateSync = glad_eglCreateSync;
    glad_debug_eglCreateWindowSurface = glad_eglCreateWindowSurface;
    glad_debug_eglDestroyContext = glad_eglDestroyContext;
    glad_debug_eglDestroyImage = glad_eglDestroyImage;
    glad_debug_eglDestroySurface = glad_eglDestroySurface;
    glad_debug_eglDestroySync = glad_eglDestroySync;
    glad_debug_eglGetConfigAttrib = glad_eglGetConfigAttrib;
    glad_debug_eglGetConfigs = glad_eglGetConfigs;
    glad_debug_eglGetCurrentContext = glad_eglGetCurrentContext;
    glad_debug_eglGetCurrentDisplay = glad_eglGetCurrentDisplay;
    glad_debug_eglGetCurrentSurface = glad_eglGetCurrentSurface;
    glad_debug_eglGetDisplay = glad_eglGetDisplay;
    glad_debug_eglGetError = glad_eglGetError;
    glad_debug_eglGetPlatformDisplay = glad_eglGetPlatformDisplay;
    glad_debug_eglGetProcAddress = glad_eglGetProcAddress;
    glad_debug_eglGetSyncAttrib = glad_eglGetSyncAttrib;
    glad_debug_eglInitialize = glad_eglInitialize;
    glad_debug_eglMakeCurrent = glad_eglMakeCurrent;
    glad_debug_eglQueryAPI = glad_eglQueryAPI;
    glad_debug_eglQueryContext = glad_eglQueryContext;
    glad_debug_eglQueryString = glad_eglQueryString;
    glad_debug_eglQuerySurface = glad_eglQuerySurface;
    glad_debug_eglReleaseTexImage = glad_eglReleaseTexImage;
    glad_debug_eglReleaseThread = glad_eglReleaseThread;
    glad_debug_eglSurfaceAttrib = glad_eglSurfaceAttrib;
    glad_debug_eglSwapBuffers = glad_eglSwapBuffers;
    glad_debug_eglSwapInterval = glad_eglSwapInterval;
    glad_debug_eglTerminate = glad_eglTerminate;
    glad_debug_eglWaitClient = glad_eglWaitClient;
    glad_debug_eglWaitGL = glad_eglWaitGL;
    glad_debug_eglWaitNative = glad_eglWaitNative;
    glad_debug_eglWaitSync = glad_eglWaitSync;
}

#ifdef GLAD_EGL

#ifndef GLAD_LOADER_LIBRARY_C_
#define GLAD_LOADER_LIBRARY_C_

#include <stddef.h>
#include <stdlib.h>

#if GLAD_PLATFORM_WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif


static void* glad_get_dlopen_handle(const char *lib_names[], int length) {
    void *handle = NULL;
    int i;

    for (i = 0; i < length; ++i) {
#if GLAD_PLATFORM_WIN32
  #if GLAD_PLATFORM_UWP
        size_t buffer_size = (strlen(lib_names[i]) + 1) * sizeof(WCHAR);
        LPWSTR buffer = (LPWSTR) malloc(buffer_size);
        if (buffer != NULL) {
            int ret = MultiByteToWideChar(CP_ACP, 0, lib_names[i], -1, buffer, buffer_size);
            if (ret != 0) {
                handle = (void*) LoadPackagedLibrary(buffer, 0);
            }
            free((void*) buffer);
        }
  #else
        handle = (void*) LoadLibraryA(lib_names[i]);
  #endif
#else
        handle = dlopen(lib_names[i], RTLD_LAZY | RTLD_LOCAL);
#endif
        if (handle != NULL) {
            return handle;
        }
    }

    return NULL;
}

static void glad_close_dlopen_handle(void* handle) {
    if (handle != NULL) {
#if GLAD_PLATFORM_WIN32
        FreeLibrary((HMODULE) handle);
#else
        dlclose(handle);
#endif
    }
}

static GLADapiproc glad_dlsym_handle(void* handle, const char *name) {
    if (handle == NULL) {
        return NULL;
    }

#if GLAD_PLATFORM_WIN32
    return (GLADapiproc) GetProcAddress((HMODULE) handle, name);
#else
    return GLAD_GNUC_EXTENSION (GLADapiproc) dlsym(handle, name);
#endif
}

#endif /* GLAD_LOADER_LIBRARY_C_ */

struct _glad_egl_userptr {
    void *handle;
    PFNEGLGETPROCADDRESSPROC get_proc_address_ptr;
};

static GLADapiproc glad_egl_get_proc(void *vuserptr, const char* name) {
    struct _glad_egl_userptr userptr = *(struct _glad_egl_userptr*) vuserptr;
    GLADapiproc result = NULL;

    result = glad_dlsym_handle(userptr.handle, name);
    if (result == NULL) {
        result = GLAD_GNUC_EXTENSION (GLADapiproc) userptr.get_proc_address_ptr(name);
    }

    return result;
}

static void* _egl_handle = NULL;

static void* glad_egl_dlopen_handle(void) {
#if GLAD_PLATFORM_APPLE
    static const char *NAMES[] = {"libEGL.dylib"};
#elif GLAD_PLATFORM_WIN32
    static const char *NAMES[] = {"libEGL.dll", "EGL.dll"};
#else
    static const char *NAMES[] = {"libEGL.so.1", "libEGL.so"};
#endif

    if (_egl_handle == NULL) {
        _egl_handle = glad_get_dlopen_handle(NAMES, sizeof(NAMES) / sizeof(NAMES[0]));
    }

    return _egl_handle;
}

static struct _glad_egl_userptr glad_egl_build_userptr(void *handle) {
    struct _glad_egl_userptr userptr;
    userptr.handle = handle;
    userptr.get_proc_address_ptr = (PFNEGLGETPROCADDRESSPROC) glad_dlsym_handle(handle, "eglGetProcAddress");
    return userptr;
}

int gladLoaderLoadEGL(EGLDisplay display) {
    int version = 0;
    void *handle = NULL;
    int did_load = 0;
    struct _glad_egl_userptr userptr;

    did_load = _egl_handle == NULL;
    handle = glad_egl_dlopen_handle();
    if (handle != NULL) {
        userptr = glad_egl_build_userptr(handle);

        if (userptr.get_proc_address_ptr != NULL) {
            version = gladLoadEGLUserPtr(display, glad_egl_get_proc, &userptr);
        }

        if (!version && did_load) {
            gladLoaderUnloadEGL();
        }
    }

    return version;
}


void gladLoaderUnloadEGL() {
    if (_egl_handle != NULL) {
        glad_close_dlopen_handle(_egl_handle);
        _egl_handle = NULL;
    }
}

#endif /* GLAD_EGL */

#ifdef __cplusplus
}
#endif
