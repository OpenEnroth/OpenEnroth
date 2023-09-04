#pragma once

#ifdef _WINDOWS
#   include <cstdint>
#else
#   include <sys/types.h>
#endif

#ifdef _WINDOWS
using ssize_t = intptr_t;
#endif
