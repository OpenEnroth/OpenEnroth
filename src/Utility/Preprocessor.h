#pragma once

/**
 * Concatenates two tokens together, performing macro substitution first.
 */
#define MM_PP_CAT(x, y) MM_PP_CAT_I(x, y)

#define MM_PP_CAT_I(x, y) x ## y

