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
