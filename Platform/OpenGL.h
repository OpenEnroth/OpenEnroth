#pragma once

#ifdef __APPLE__
    #include <OpenGL/glu.h>
    #include <OpenGL/gl.h>
#else
    #ifdef _WINDOWS
        #define NOMINMAX
        #include <Windows.h>
        #undef DrawText

        #pragma comment(lib, "opengl32.lib")
        #pragma comment(lib, "glu32.lib")

        //  on windows, this is required in gl/glu.h
        #if !defined(APIENTRY)
            #define APIENTRY __stdcall
        #endif

        #if !defined(WINGDIAPI)
            #define WINGDIAPI
        #endif

        #if !defined(CALLBACK)
            #define CALLBACK __stdcall
        #endif
    #endif

    #include <GL/glu.h>
    #include <GL/gl.h>
#endif

#include <SDL.h>
#include <SDL_opengl_glext.h>
