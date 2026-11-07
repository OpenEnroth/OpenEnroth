---
name: lod
description: Look inside the game's LOD, VID and SND archives with LodTool - list them, print or extract entries, convert images to PNG, identify a file's format. Use whenever you need to read game data stored in an archive, such as a map, an event script, a text table, an icon or a sprite.
---

`LodTool` reads the archives that MM6, MM7 and MM8 keep their data in. Build the `LodTool` target, the binary
lands at `<build>/src/Bin/LodTool/LodTool`, and the examples below call it by its bare name.

## Paths

Every command takes a path that can run into an archive: `mm7/DATA/GAMES.LOD/d01.blv` is entry `d01.blv` of
`GAMES.LOD`. The longest prefix that exists on disk is the file, the rest is the entry name. Archives are flat, so
an entry name is a single segment, and there are no archives inside archives.

- Folder and archive names on disk keep the case they shipped with, MM7 has `DATA/GAMES.LOD` and MM8 has
  `Data/games.lod`. Spell them exactly on a case-sensitive file system. Entry names inside an archive are
  case-insensitive.
- Entry names can contain spaces, `hit with arrow01l` in MM6's `Audio.snd`. Quote paths, and loop over `ls`
  output with `while IFS= read -r`, never with `for e in $(...)`.
- MM8 names are cut at 15 characters in the archive index, so `01female_wince.wav` is listed as
  `01female_wince.w` and must be read under that name.

## Commands

```sh
LodTool ls DIR_OR_ARCHIVE              # directory contents, or archive entries in archive order
LodTool cat [--raw] FILE                # write to stdout, decompressed unless --raw
LodTool extract [--raw] SRC [OUT]       # see below
LodTool convert [--palettes-lod-path BITMAPS_LOD] FILE OUT.png
LodTool magic FILE                      # print the format
```

`FILE` is an archive entry or a plain file on disk, `cat` and `magic` work the same on both, so an extracted
`--raw` entry decompresses later with `cat`.

`extract` follows `cp`. An archive always extracts into the directory `OUT`, which is created if needed. A single
file goes into `OUT` when `OUT` is an existing directory or ends with `/`, and is written to `OUT` otherwise. `OUT`
defaults to the current directory. Only decompression happens, names are kept as they are.

`convert` writes PNG only, and takes LOD images, LOD palettes, LOD sprites, LOD fonts (rendered as a 16x16 glyph
grid) and PCX images, decompressing first. A sprite only stores palette indices and the id of its default palette.
Without `--palettes-lod-path` pointing at the game's bitmaps archive it comes out grayscale, with indices as gray
levels. With it, it's drawn in the default palette, which for monsters has cyan and magenta in the recolored parts,
so it's not what the game shows. Read the resulting PNG to see the image.

`magic` prints one line, `LOD compressed pseudo image containing PCX image` for compressed data.

## What's where

From a full `magic` scan of every archive in all three games. Formats are what `magic` prints.

| Archive | MM6 | MM7 | MM8 |
|---|---|---|---|
| maps, `.blv` `.dlv` `.odm` `.ddm` | `data/Games.lod`, raw data | `DATA/GAMES.LOD`, compressed | `Data/games.lod`, compressed |
| events, strings, text tables, `.evt` `.str` `.txt` `.bin` | `data/icons.lod`, compressed | `DATA/events.lod`, compressed | `Data/EnglishT.lod`, raw data |
| UI images, PCX screens, fonts | `data/icons.lod` | `DATA/ICONS.LOD` | `Data/icons.lod` |
| textures and palettes `palNNN` | `data/BITMAPS.LOD` | `DATA/BITMAPS.LOD` | `Data/bitmaps.lod` |
| sprites | `data/SPRITES.LOD` | `DATA/SPRITES.LOD` | `Data/sprites.lod` |
| sounds | `Sounds/Audio.snd`, WAV | `SOUNDS/Audio.snd`, WAV | `Data/EnglishD.lod`, raw data |
| movies, Smacker and Bink | `Anims/Anims1.vid`, `Anims2.vid` | `Anims/Magic7.vid`, `Might7.vid` | `Anims/Magicdod.vid`, `mightdod.vid` |

Entries without an extension in the image archives are LOD images, palettes or sprites. Movies inside VID archives
are raw Smacker or Bink files, `cat` them out as they are.

`magic` says `Raw data` for two kinds of entries that are not raw, and `cat` passes them through unchanged:

- MM6 maps. Each is an 8-byte header with the compressed and the uncompressed size, then a zlib stream.
- MM8 `EnglishT.lod` and `EnglishD.lod` entries. Each starts with a header carrying the full entry name.

Past these, the scan found three stubs it can't name, MM6 `icons.lod/d4bm.evt` with 48 bytes, MM7
`SPRITES.LOD/pending` with 160 bytes, and MM8 `bitmaps.lod/pal857` with 816 bytes. Use
`cat --raw FILE | od -A d -t x1z | head` to look at the bytes when `magic` gives up.

## Recipes

```sh
# Decompress an MM7 map for a parser or a hex dump.
LodTool cat "$OPENENROTH_MM7_PATH/DATA/GAMES.LOD/d01.blv" > d01.blv

# Pull every MM7 event script out at once.
LodTool extract "$OPENENROTH_MM7_PATH/DATA/events.lod" evt/

# Look at a sprite in its default palette.
LodTool convert --palettes-lod-path "$OPENENROTH_MM7_PATH/DATA/BITMAPS.LOD" \
    "$OPENENROTH_MM7_PATH/DATA/SPRITES.LOD/arrowa0" arrowa0.png

# Count the formats in an archive.
A="$OPENENROTH_MM7_PATH/DATA/ICONS.LOD"
LodTool ls "$A" | while IFS= read -r e; do LodTool magic "$A/$e"; done | sort | uniq -c
```

Each call opens the archive anew, so a loop over a big archive like `SPRITES.LOD` takes minutes. Extract it once
and work on the files when you need more than a handful of entries.
