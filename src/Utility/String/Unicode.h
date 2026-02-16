#pragma once

namespace unicode {

/**
 * Check if a Unicode code point is a whitespace character.
 *
 * @param c                             Unicode code point.
 * @return                              Whether the character is a whitespace.
 */
inline bool isSpace(char32_t c) {
    switch (c) {
    case U'\u0020':  // SPACE
    case U'\u00A0':  // NO-BREAK SPACE
    case U'\u1680':  // OGHAM SPACE MARK
    case U'\u2000':  // EN QUAD
    case U'\u2001':  // EM QUAD
    case U'\u2002':  // EN SPACE
    case U'\u2003':  // EM SPACE
    case U'\u2004':  // THREE-PER-EM SPACE
    case U'\u2005':  // FOUR-PER-EM SPACE
    case U'\u2006':  // SIX-PER-EM SPACE
    case U'\u2007':  // FIGURE SPACE
    case U'\u2008':  // PUNCTUATION SPACE
    case U'\u2009':  // THIN SPACE
    case U'\u200A':  // HAIR SPACE
    case U'\u202F':  // NARROW NO-BREAK SPACE
    case U'\u205F':  // MEDIUM MATHEMATICAL SPACE
    case U'\u3000':  // IDEOGRAPHIC SPACE
        return true;
    default:
        return false;
    }
}

} // namespace unicode
