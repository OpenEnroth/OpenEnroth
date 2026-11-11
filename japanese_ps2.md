# Japanese Might and Magic 8 (PS2) - Font Analysis

## Source

- **Game**: Might and Magic VIII: Day of the Destroyer (マイト&マジック8 デイ オブ ザ デストロイヤー)
- **Platform**: PlayStation 2
- **Game ID**: SLPS-25031
- **Publisher**: Imagineer (イマジニア)
- **Release Date**: September 6, 2001
- **Source**: CDRomance PS2 ISO

## Text Encoding

The game uses **Shift-JIS** encoding for all Japanese text. Internally, the code converts
Shift-JIS to JIS using `ysSjisToJis()` for font lookup. Key functions found in the ELF:

- `ysSjisToJis__FUl` — Shift-JIS to JIS conversion
- `isKanji__FUs` — kanji detection
- `GetKanji__FPUc` — retrieve kanji glyph
- `SetKanjiFont` / `SetKanjiFontB` — font selection
- `Ascii2Sjis__FUc` / `Ascii2SjisStr` — ASCII to Shift-JIS conversion
- `kinsokuHeadSjis`, `kinsokuTailSjis` — Japanese line-breaking (kinsoku) rules
- Error string: `"Kanji not found!! code = %02x%02x"`
- Dev reference: `"host:../font_gf/sce24i26.gf"` (Sony SCE 24-dot JIS font)
- Font-related: `ascfont.cpp`, `fontDisp()`, `_FONT` struct, `FONTPAL` palette,
  `isPrintablePS2__FcP5_FONT`, `ascFontSprDisp`, `ascFontWinDisp`

## Archive Format: FCAT

The PS2 version uses a custom archive format with magic `FCAT` (not the PC's `LOD` format).
LodTool cannot read these files. No documentation exists online for this format — it is a
proprietary Imagineer format.

### FCAT Header (16 bytes)
```
Offset  Size  Description
0x00    4     Magic: "FCAT"
0x04    4     Version: uint32 LE (always 1)
0x08    4     Total file size: uint32 LE
0x0C    4     Entry count: uint32 LE
```

### FCAT Entry (32 bytes each)
```
Offset  Size  Description
0x00    4     Flags: uint32 LE (bitfield, see below)
0x04    4     Data offset: uint32 LE (from start of file)
0x08    4     Compressed size: uint32 LE
0x0C    4     Decompressed size: uint32 LE
0x10    16    Name: null-terminated ASCII
```

Data entries are **zlib-compressed**. Use `zlib.decompress()` on the raw data at the given offset.

### FCAT Flags Field

The 4-byte flags field is a bitfield encoding compression status and file type:

- **Bit 12** (byte[1] & 0x10): **compression flag** — 1 = zlib compressed, 0 = uncompressed
- **Remaining bits**: file type identifier, correlating with content type:

| Flag bytes (hex) | Compressed | File types |
|---|---|---|
| `00 00 14 4b` | No | Raw bitmaps, sprites, textures (.tx2) |
| `00 10 14 4b` | Yes | Compressed bitmaps, sprites |
| `00 10 14 d3` | Yes | Text files (.txt) |
| `00 10 22 b9` | Yes | Binary data (.bin) |
| `00 10 84 b4` | Yes | Map data (.ddm) |
| `00 10 84 d9` | Yes | Level data (.dlv) |
| `00 10 8c 48` | Yes | UI assets |

The flags have no correspondence to the PC LOD format. LOD entries use a completely different
layout (name first, no type field) and encode compression separately via a `LodCompressionHeader`
with magic `"mvii"` and version `91969`.

### FCAT Archives Found

| File | Location | Entries | Description |
|------|----------|---------|-------------|
| ICONSJ.LDZ | MM8DAT/ | 3,782 | Japanese icons/UI (replaces icons.lod) |
| MM8VB.LD2 | MM8DAT/ | - | Game data |
| MOVIE.LD2 | MM8DAT/ | - | Video data |
| TEXT.LDZ | DATA/ | 32 | Japanese text files |
| BMP.LDZ | DATA/ | 1,339 | Bitmap data |
| NEW.LDZ | DATA/ | 73 | Dungeon/map data |
| SPRLZP.LD2 | DATA/ | 10,696 | Sprites |

## Font Files

### FONTHZ.FN2 — The Only Font File

FONTHZ.FN2 is the **only font file** in the PS2 ROM. It contains all characters needed by the
game: ASCII, Latin, fullwidth symbols, hiragana, katakana, and kanji.

- **Location**: `DATA/FONTHZ.FN2` (ELF reference: `"data/fonthz.fn2"`)
- **Size**: 133,120 bytes
- **Format**: Paired 2bpp bitmap font, no header
- **Glyph dimensions**: 16x16 pixels per character
- **Bits per pixel**: 2 (4 grayscale levels per character)
- **Characters per pair**: 2 (interleaved within each nibble)
- **Bytes per pair**: 128 (8 bytes/row × 16 rows)
- **Total pairs**: 1,040 (133,120 / 128)
- **Total characters**: 2,080 (1,040 × 2)

Character composition (2,014 mapped + 66 unmapped slots):
- ASCII symbols, digits, and Latin letters (upper and lowercase)
- Fullwidth punctuation and symbols
- Hiragana (85 characters)
- Katakana (88 characters)
- Kanji (~1,300+ CJK ideographs, game-specific subset)

### Latin Font References (Not Present)

The ELF references PC-style `.fnt` filenames (`arrus.fnt`, `book.fnt`, `comic.fnt`,
`lucida.fnt`, `create.fnt`, `smallnum.fnt`, `autonote.fnt`, `book2.fnt`, `cchar.fnt`,
`quick.fnt`) as well as `FONTPAL` (font palette). However, **none of these files exist**
on the PS2 disc — not as standalone files nor as entries in any FCAT archive. The PS2 port
routes all text rendering through FONTHZ.FN2.

### bu_kana (in ICONSJ.LDZ)

- **Size**: 3,200 bytes (compressed: 793 bytes)
- **Purpose**: Likely a UI button/label bitmap for kana display, not a general-purpose font

## How to Parse FONTHZ.FN2

### Pixel Format

Each 128-byte block stores **two** 16x16 characters at **2 bits per pixel**, interleaved within
each 4-bit nibble. Nibbles are packed low-nibble-first within each byte.

For each nibble (4 bits):
- **Bits 3-2**: pixel value for the **odd-indexed** character (index 2N+1)
- **Bits 1-0**: pixel value for the **even-indexed** character (index 2N)

Each pixel value is 2 bits: 0 = background, 1-3 = increasing intensity (4 grayscale levels).

If you read the data as 4bpp, the odd character appears at full brightness while the even
character appears as a "pale gray ghost" underneath — this is how the interleaving was
discovered.

```python
def parse_character(data, char_index):
    """Parse a single 16x16 2bpp character from FONTHZ.FN2.

    Each 128-byte block contains two characters interleaved per nibble.
    Even-indexed characters use bits 1-0, odd-indexed use bits 3-2.

    Returns a 16x16 array of intensity values (0-3).
    0 = transparent/background, 3 = fully opaque foreground.
    """
    pair_index = char_index // 2
    is_odd = char_index % 2
    offset = pair_index * 128

    pixels = []
    for row in range(16):
        row_pixels = []
        for col_byte in range(8):  # 8 bytes per row
            byte = data[offset + row * 8 + col_byte]
            lo_nibble = byte & 0x0F         # First pixel pair (low nibble)
            hi_nibble = (byte >> 4) & 0x0F  # Second pixel pair (high nibble)
            for nibble in [lo_nibble, hi_nibble]:
                if is_odd:
                    val = (nibble >> 2) & 3  # Bits 3-2
                else:
                    val = nibble & 3          # Bits 1-0
                row_pixels.append(val)
        pixels.append(row_pixels)
    return pixels
```

### Character Mapping

The character-to-index mapping table is embedded in the PS2 executable (`SLPS_250.31`)
at **offset 0x190c60**. It contains **2,080 Shift-JIS codes** (uint16 LE), one per character
(two per font pair). Of these, 2,014 are valid SJIS codes and 66 are zero/unused.

```python
import struct

def load_mapping(elf_path):
    """Load the SJIS-to-character mapping from the PS2 ELF.

    Returns a list of 2080 SJIS codes.
    mapping[char_index] = SJIS code for that character.
    """
    with open(elf_path, 'rb') as f:
        f.seek(0x190c60)
        codes = []
        for i in range(2080):
            code = struct.unpack('<H', f.read(2))[0]
            codes.append(code)
    return codes

def sjis_to_char_index(mapping, sjis_code):
    """Look up character index for a Shift-JIS code."""
    try:
        return mapping.index(sjis_code)
    except ValueError:
        return -1  # Character not in font

# Usage:
# mapping = load_mapping('SLPS_250.31')
# idx = sjis_to_char_index(mapping, 0x82A0)  # あ
# glyph = parse_character(font_data, idx)
```

### Rendering to Image

```python
from PIL import Image

def render_font(data, num_chars=2080, cols=40):
    """Render the full font to a PIL Image."""
    rows = (num_chars + cols - 1) // cols
    pad = 1
    img = Image.new("L", (cols * 17 + pad, rows * 17 + pad), 0)

    for char_idx in range(num_chars):
        pixels = parse_character(data, char_idx)
        x0 = pad + (char_idx % cols) * 17
        y0 = pad + (char_idx // cols) * 17
        for row in range(16):
            for col in range(16):
                img.putpixel((x0 + col, y0 + row), pixels[row][col] * 85)
    return img
```

## Mapping Table Reference

The mapping stores consecutive character pairs. Even indices use bits 1-0 of each nibble,
odd indices use bits 3-2.

| Char Index | Pair | Position | SJIS | Character |
|---|---|---|---|---|
| 0 | 0 | even (bits 1-0) | 0x8140 | 　 (ideographic space) |
| 1 | 0 | odd (bits 3-2) | 0x8141 | 、 |
| 2 | 1 | even | 0x8142 | 。 |
| 3 | 1 | odd | 0x8143 | ， |
| ... | | | | |
| 94 | 47 | even | 0x824F | ０ |
| 95 | 47 | odd | 0x8250 | １ |
| 96 | 48 | even | 0x8251 | ２ |
| 97 | 48 | odd | 0x8252 | ３ |
| ... | | | | |
| 2013 | 1006 | odd | (last valid) | |
| 2014-2079 | | | 0x0000 | (unused slots) |

The full mapping is saved in `extracted_fonts/sjis_mapping_2080.json`.

## File Locations

```
~/Downloads/mm8_japanese_ps2/
├── Might and Magic - Day of the Destroyer (Japan).iso
├── extracted/                              # ISO contents
│   ├── SLPS_250.31                         # PS2 ELF executable (MIPS)
│   ├── SYSTEM.CNF
│   ├── BOOT/                               # Boot screens (.TX2 textures)
│   ├── DATA/
│   │   ├── FONTHZ.FN2                      # Japanese font (133 KB, only font file)
│   │   ├── TEXT.LDZ                         # Japanese text (FCAT archive, 32 entries)
│   │   ├── BMP.LDZ                          # Bitmaps (FCAT, 1339 entries)
│   │   ├── NEW.LDZ                          # Map/dungeon data (FCAT, 73 entries)
│   │   ├── SPRLZP.LD2                       # Sprites (FCAT, 10696 entries)
│   │   └── ...
│   ├── MM8DAT/
│   │   ├── ICONSJ.LDZ                      # Japanese icons (FCAT, 3782 entries)
│   │   ├── MM8VB.LD2                        # Game data
│   │   ├── MOVIE.LD2                        # Videos
│   │   ├── ENGLISHT/                        # English text data (.BIN files)
│   │   └── SONG*.VBS                        # Music
│   ├── INDOOR1/, INDOOR2/, OUTDOOR/         # Level data (each with BMP/SPR archives)
│   ├── IOP/, IRX/                           # PS2 I/O processor modules
│   └── PSS/                                 # Video streams
└── extracted_fonts/
    ├── FONTHZ.FN2                           # Copy of font
    ├── bu_kana.bin                           # Kana bitmap from ICONSJ.LDZ
    ├── sjis_mapping_2080.json               # Full 2080-entry character mapping
    ├── fonthz_2bpp_interleaved.png          # Correct render (2bpp, 2080 chars)
    └── fonthz_rendered.png                  # Old 4bpp render (shows ghosting)
```

## Comparison with Chinese PC Version

| Feature | Chinese MM6 (PC) | Japanese MM8 (PS2) |
|---------|-------------------|---------------------|
| Text encoding | GB2312/GBK | Shift-JIS |
| Font storage | PE resource section (.rsrc/FNT/) | Standalone file (FONTHZ.FN2) |
| Font format | 1bpp monochrome bitmaps | 2bpp grayscale, paired per nibble |
| Glyph sizes | 14x14, 16x16, 24x24, 32x30 | 16x16 only |
| Character count | 8,334 - 8,836 (full charset) | 2,080 (game-specific subset) |
| Characters per byte | 8 pixels (1bpp) | 2 pixels × 2 characters (2×2bpp packed) |
| Character mapping | Linear GB2312 grid (row*94+col) | Lookup table in ELF at 0x190c60 |
| Latin fonts | .fnt files in icons.lod | None (all text uses FONTHZ.FN2) |
| Archive format | LOD (magic "LOD\0") | FCAT (magic "FCAT") |
| Compression | zlib (per-entry, magic "mvii") | zlib (per-entry) |

## Notes on Japanese PC Version

The Japanese PC versions of MM6/7/8 (published by Imagineer, 完全日本語版) are **not available
online**. They are rare physical CDs that occasionally appear on Japanese auction sites.
They likely use a similar approach to the Chinese PC version: CJK bitmap fonts embedded in
the PE resource section of the executable, with Shift-JIS encoding instead of GB2312.
