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

/**
 * @def MM_LIFETIMEBOUND
 *
 * Marks a member function whose result points into `*this`, or a parameter that the result points into, so that the
 * compiler can warn when the result outlives a temporary it was taken from. Clang warns by default, MSVC only with
 * its lifetime checker on, and GCC has no such attribute.
 */
#if defined(__clang__)
#   define MM_LIFETIMEBOUND [[clang::lifetimebound]]
#elif defined(_MSC_VER)
#   define MM_LIFETIMEBOUND [[msvc::lifetimebound]]
#else
#   define MM_LIFETIMEBOUND
#endif
