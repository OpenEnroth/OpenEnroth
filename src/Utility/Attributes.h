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
 * @def MM_NO_SANITIZE_ADDRESS
 *
 * Excludes a function from AddressSanitizer instrumentation, and is inert in builds that don't enable it.
 */
#ifdef _MSC_VER
#   define MM_NO_SANITIZE_ADDRESS __declspec(no_sanitize_address)
#else
#   define MM_NO_SANITIZE_ADDRESS [[gnu::no_sanitize_address]]
#endif
