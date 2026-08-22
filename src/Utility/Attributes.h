#pragma once

/**
 * @def MM_NOINLINE
 *
 * Prevents the compiler from inlining a function. Note that a function that's not inlined is also a function
 * that shows up as its own frame in a stack trace.
 */
#ifdef _MSC_VER
#   define MM_NOINLINE __declspec(noinline)
#else
#   define MM_NOINLINE [[gnu::noinline]]
#endif
