#pragma once

#include <time.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION

#define NK_ASSERT(E) { if (!(E)) { \
    time_t t = time(NULL); \
	struct tm tm = *localtime(&t); \
	fprintf(stderr, "[%04d/%02d/%02d %02d:%02d:%02d] Nuklear Engine: error on %s:%d, condition: '%s'\n", \
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, __FILE__, __LINE__, #E); \
}}

#ifdef _DEBUG
#define STBTT_malloc(ptr, userdata) malloc(ptr)
#define STBTT_free(ptr, userdata) free(ptr)
#endif

#define STBTT_RASTERIZER_VERSION 2

#include "nuklear.h"
#include "nuklear_internal.h"
