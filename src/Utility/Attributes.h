#pragma once

/**
 * @def MM_NOINLINE
 *
 * Prevents the compiler from inlining a function.
 */
#ifdef _MSC_VER
#   define MM_NOINLINE __declspec(noinline)
#else
#   define MM_NOINLINE [[gnu::noinline]]
#endif
