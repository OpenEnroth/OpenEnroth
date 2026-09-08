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

/**
 * @def MM_NOASAN
 *
 * Excludes a function from AddressSanitizer instrumentation, and is inert in builds that don't enable it.
 */
#ifdef _MSC_VER
#   define MM_NOASAN __declspec(no_sanitize_address)
#else
#   define MM_NOASAN [[gnu::no_sanitize_address]]
#endif
