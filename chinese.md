# Chinese Might and Magic 6 - Font Analysis

## Source

- **Game**: Might and Magic VI: The Mandate of Heaven (完全中文版, Simplified Chinese)
- **Archive.org**: https://archive.org/details/Might_and_Magic_VI_The_Mandate_of_Heaven_China
- **Format**: 2 RAR archives containing MDF/MDS disc images
- **Publisher**: New World Computing / 3DO (Chinese localization)

## How Text Rendering Works

The Chinese version does **not** use Win32 GDI for font rendering. Analysis of the PE import
table confirms only 4 GDI32.dll imports:

- `GetDeviceCaps`
- `GetStockObject`
- `SetDIBitsToDevice`
- `DeleteDC`

There are **no** text rendering imports (`CreateFont`, `TextOut`, `DrawText`, `ExtTextOut`,
`GetGlyphOutline`, etc.). The game uses its own bitmap font engine (`D:\mm6\code\FONT.CPP`
referenced in the binary).

## Font Storage

Fonts are stored in **two locations**:

### 1. Standard Latin Fonts (in `icons.lod`)

14 standard `.fnt` files in the LOD archive, identical to the English version:

| File | Size |
|------|------|
| arrus.fnt | 44 KB |
| autonote.fnt | 33 KB |
| book.fnt | 67 KB |
| book2.fnt | 106 KB |
| calig.fnt | 82 KB |
| cchar.fnt | 78 KB |
| comic.fnt | 33 KB |
| create.fnt | 33 KB |
| endgame.fnt | 43 KB |
| legal.fnt | 28 KB |
| lucida.fnt | 34 KB |
| quick.fnt | 50 KB |
| smallnum.fnt | 23 KB |
| spell.fnt | 25 KB |

These use the standard MM6 `.fnt` format (3-byte magic header, 768 bytes glyph metrics,
256 glyph offset table, bitmap data). They cover ASCII/Latin characters only.

### 2. Chinese Bitmap Fonts (embedded in PE resources)

4 bitmap font resources are embedded in the `.rsrc` section of `mm6.exe` under the
custom resource type `FNT`. The `.rsrc` section is 2.6 MB, with ~2.1 MB being font data.

These were extracted using `7z x mm6.exe` which produces `.rsrc/FNT/120`, `.rsrc/FNT/121`,
`.rsrc/FNT/123`, `.rsrc/FNT/124`.

| Resource | Size (bytes) | Glyph Size | Bytes/Glyph | Glyph Count | Encoding |
|----------|-------------|------------|-------------|-------------|----------|
| FNT/120 | 600,048 | 24x24 px | 72 | 8,334 | GB2312 subset |
| FNT/121 | 261,697 | 16x16 px | 32 | 8,178 | GB2312 subset (1-byte header) |
| FNT/123 | 247,408 | 14x14 px | 28 | 8,836 | Full GB2312 grid |
| FNT/124 | 1,060,320 | ~27x30 px | 120 | 8,836 | Full GB2312 grid |

## How to Parse the Font Resources

### FNT/123 — 14x14 Bitmap Font (simplest, exact GB2312)

**Format**: Raw bitmap array, no header. Each glyph is 28 bytes.

```
Glyph layout:
  - Width: 14 pixels (stored in 2 bytes per row, 16 bits, 2 MSBs unused)
  - Height: 14 rows
  - Bytes per glyph: 2 bytes/row × 14 rows = 28 bytes
  - Total: 8,836 glyphs × 28 bytes = 247,408 bytes
  - Byte order: Big-endian
```

**Parsing each glyph**:
```python
def parse_glyph_14x14(data, glyph_index):
    offset = glyph_index * 28
    rows = []
    for row in range(14):
        # 2 bytes per row, big-endian, top 14 bits are the pixel data
        hi = data[offset + row * 2]
        lo = data[offset + row * 2 + 1]
        word = (hi << 8) | lo
        # Bits 15..2 contain pixel data (14 pixels), bits 1..0 unused
        pixels = [(word >> (15 - col)) & 1 for col in range(14)]
        rows.append(pixels)
    return rows
```

### FNT/124 — 32x30 Bitmap Font (largest, exact GB2312)

**Format**: Raw bitmap array, no header. Each glyph is 120 bytes.

```
Glyph layout:
  - Width: ~27 pixels (stored in 4 bytes per row, 32 bits, ~5 LSBs unused)
  - Height: 30 rows
  - Bytes per glyph: 4 bytes/row × 30 rows = 120 bytes
  - Total: 8,836 glyphs × 120 bytes = 1,060,320 bytes
  - Byte order: Big-endian
```

**Parsing each glyph**:
```python
def parse_glyph_30x4(data, glyph_index):
    offset = glyph_index * 120
    rows = []
    for row in range(30):
        b = data[offset + row * 4 : offset + row * 4 + 4]
        word = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]
        # Top ~27 bits are pixel data
        pixels = [(word >> (31 - col)) & 1 for col in range(27)]
        rows.append(pixels)
    return rows
```

Note: The exact pixel width might be 27 or up to 32. The first glyph (A1A1 = full block `十`-like
shape) uses pattern `ffff ffe0` = `1111 1111 1111 1111 1111 1111 1110 0000`, confirming
27 active pixels and 5 trailing zero bits.

### FNT/120 — 24x24 Bitmap Font (GB2312 subset)

**Format**: Raw bitmap array, no header. Each glyph is 72 bytes.

```
Glyph layout:
  - Width: 24 pixels (stored in 3 bytes per row, all 24 bits used)
  - Height: 24 rows
  - Bytes per glyph: 3 bytes/row × 24 rows = 72 bytes
  - Total: 8,334 glyphs × 72 bytes = 600,048 bytes
  - Byte order: Big-endian
```

8,334 is less than the full 8,836 GB2312 grid. This font covers a subset — likely
the most commonly used characters. The exact subset mapping needs further investigation
(it may omit some rows from the GB2312 table, or it may use a different indexing).

### FNT/121 — 16x16 Bitmap Font (GB2312 subset, 1-byte header)

**Format**: 1-byte header followed by raw bitmap array. Each glyph is 32 bytes.

```
Glyph layout:
  - Header: 1 byte (purpose unknown, possibly flags or version)
  - Width: 16 pixels (stored in 2 bytes per row, all 16 bits used)
  - Height: 16 rows
  - Bytes per glyph: 2 bytes/row × 16 rows = 32 bytes
  - Total: 1 + (8,178 glyphs × 32 bytes) = 261,697 bytes
  - Byte order: Big-endian
```

8,178 glyphs is also less than the full 8,836. Similar subset as FNT/120.

## Mapping Glyph Index to GB2312 Characters

### GB2312 Encoding Structure

GB2312 organizes characters in a 94×94 grid:
- **Row** (区, qū): 0x01–0x5E (1–94), encoded as byte value 0xA1–0xFE
- **Column** (位, wèi): 0x01–0x5E (1–94), encoded as byte value 0xA1–0xFE

Total grid positions: 94 × 94 = **8,836**

### For FNT/123 and FNT/124 (exact 8,836 glyphs)

These contain the complete GB2312 grid. The mapping is linear:

```python
def glyph_index_to_gb2312(index):
    """Convert linear glyph index to GB2312 double-byte code."""
    row = index // 94      # 0-based row (区)
    col = index % 94       # 0-based column (位)
    byte1 = row + 0xA1     # High byte
    byte2 = col + 0xA1     # Low byte
    return bytes([byte1, byte2])

def gb2312_to_glyph_index(byte1, byte2):
    """Convert GB2312 double-byte code to linear glyph index."""
    row = byte1 - 0xA1
    col = byte2 - 0xA1
    return row * 94 + col
```

### GB2312 Row Layout

| Rows | Content |
|------|---------|
| 01 (0xA1A1–0xA1FE) | General punctuation and symbols |
| 02 (0xA2A1–0xA2FE) | Numbering symbols (①②③ etc.) |
| 03 (0xA3A1–0xA3FE) | Full-width ASCII |
| 04 (0xA4A1–0xA4FE) | Hiragana |
| 05 (0xA5A1–0xA5FE) | Katakana |
| 06 (0xA6A1–0xA6FE) | Greek letters |
| 07 (0xA7A1–0xA7FE) | Cyrillic letters |
| 08 (0xA8A1–0xA8FE) | Pinyin and symbols |
| 09 (0xA9A1–0xA9FE) | Box drawing characters |
| 10–15 | Unused/reserved |
| 16–55 (0xB0A1–0xD7FE) | Level 1 Chinese characters (3,755 chars, sorted by pinyin) |
| 56–87 (0xD8A1–0xF7FE) | Level 2 Chinese characters (3,008 chars, sorted by radical/stroke) |
| 88–94 | Unused |

### For FNT/120 and FNT/121 (subsets)

These contain fewer glyphs (8,334 and 8,178 respectively). The exact mapping needs
investigation. Possibilities:
- They may skip the unused/reserved rows (10–15, 88–94), reducing the count by
  94 × 13 = 1,222, giving 8,836 - 1,222 = 7,614 (less than 8,334, so this isn't it)
- They may include only rows with defined characters (rows 1–9, 16–87 = 81 rows = 7,614)
  plus some extras
- They may use a sequential index of only the defined codepoints in GB2312 (6,763 CJK +
  punctuation/symbols ≈ 7,445 defined positions), plus perhaps extended GBK characters
- Further binary analysis of `mm6.exe` is needed to determine the exact lookup method

### Converting to Unicode

```python
def gb2312_to_unicode(byte1, byte2):
    """Convert GB2312 bytes to Unicode character."""
    gb_bytes = bytes([byte1, byte2])
    return gb_bytes.decode('gb2312')

# Example: glyph index 0 in FNT/123
byte1, byte2 = 0xA1, 0xA1  # Row 1, Column 1
char = bytes([byte1, byte2]).decode('gb2312')
# Result: '　' (ideographic space) — but note that the actual first glyph
# in FNT/123 appears to be a cross/plus shape, which suggests row 1 col 1
# might be mapped to a different character (十) or the first glyph might
# correspond to a different starting position.
```

### Verification of Character Mapping

The glyph patterns at the start of FNT/123 match GB2312 punctuation:
- **Glyph 0** (offset 0): Cross/plus shape — maps to position A1A1
- **Glyph 1** (offset 28): Comma-like dot in lower-right — maps to A1A2 = `、`
- **Glyph 2** (offset 56): Small circle — maps to A1A3 = `。`

Note: A1A1 is officially "ideographic space" in GB2312, but this font renders it as
a visible cross shape (possibly a placeholder or the font uses a slightly different
mapping starting point).

## Text Encoding in Game Data

The game text files in `icons.lod` (e.g., `2devents.txt`, `npcnames.txt`) use GB2312/GBK
encoding. When read as raw bytes they appear as mojibake; decode with `gb2312` or `gbk`
codec to get the correct Chinese text.

## File Locations

```
~/Downloads/mm6_chinese/
├── Disc 1.rar                          # Original download
├── Disc 2 v1.rar                       # Original download (audio tracks)
├── disc1/
│   ├── MM6_Disk1.mdf                   # Original disc image
│   ├── MM6_Disk1.iso                   # Converted from MDF
│   └── MM6_Disk1.mds
├── disc1_extracted/                    # ISO contents
│   ├── SETUP.EXE
│   └── _setup/
│       ├── data1.cab                   # InstallShield cab (game data)
│       ├── _sys1.cab                   # System files
│       └── _user1.cab                  # User files
├── disc1_game/                         # Extracted from data1.cab via unshield
│   ├── Minimum/
│   │   ├── mm6.exe                     # Game binary (PE32 i386, 3.8 MB)
│   │   ├── data/
│   │   │   ├── icons.lod               # 31 MB - icons, fonts, text data
│   │   │   ├── BITMAPS.LOD             # 45 MB
│   │   │   └── games.lod              # 10 MB
│   │   ├── Anims/Anims1.vid
│   │   └── Sounds/Audio.snd
│   ├── Anims/Anims/Anims2.vid
│   ├── SpritesHI/data/SPRITES.LOD      # 50 MB
│   └── SpritesLO/data/spriteLO.lod     # 17 MB
├── exe_resources/                      # Extracted from mm6.exe via 7z
│   └── .rsrc/
│       └── FNT/
│           ├── 120                     # 24x24 Chinese bitmap font (586 KB)
│           ├── 121                     # 16x16 Chinese bitmap font (256 KB)
│           ├── 123                     # 14x14 Chinese bitmap font (242 KB)
│           └── 124                     # 32x30 Chinese bitmap font (1.0 MB)
├── extracted_fonts/
│   ├── arrus.fnt                       # Standard Latin fonts from icons.lod
│   ├── ... (14 files)
│   └── chinese_fnt/
│       ├── 120                         # Copies of PE resources
│       ├── 121
│       ├── 123
│       └── 124
└── chinese.md                          # This file
```

## Other Available Asian Versions on Archive.org

| Game | Language | URL |
|------|----------|-----|
| MM6 | Chinese Simplified (alt) | https://archive.org/details/MM6Chs |
| MM7 | Chinese Simplified | https://archive.org/details/MM7Chs |
| MM7 | Korean | https://archive.org/details/magic-7-kor-1 |
| MM8 | Chinese Simplified | https://archive.org/details/MM8Chs |
| MM8 | Chinese Traditional | https://archive.org/details/8-2-1_202603 |

**Japanese versions** (MM6/7/8 PC) are **not available online**. They were published by
Imagineer as physical CDs and are now rare collector's items. They likely use the same
PE-embedded bitmap font approach with Shift-JIS or EUC-JP encoding.
