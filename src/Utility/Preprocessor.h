#pragma once

/**
 * Concatenates two tokens together, performing macro substitution first.
 */
#define MM_PP_CAT(x, y) MM_PP_CAT_I(x, y)

#define MM_PP_CAT_I(x, y) x ## y

/**
 * Removes parentheses from the provided argument.
 */
#define MM_PP_REMOVE_PARENS(x) MM_PP_REMOVE_PARENS_I x

#define MM_PP_REMOVE_PARENS_I(...) __VA_ARGS__

/**
 * Prevents the compiler from inlining a function. Note that a function that's not inlined is also a function
 * that shows up as its own frame in a stack trace.
 */
#ifdef _MSC_VER
#define MM_NOINLINE __declspec(noinline)
#else
#define MM_NOINLINE [[gnu::noinline]]
#endif
