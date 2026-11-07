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
  case-insensitive, and LodTool prints LOD entry names in lowercase.
- Entry names can contain spaces, `hit with arrow01l` in MM6's `Audio.snd`. Quote paths, and loop over `ls`
  output with `while IFS= read -r`, never with `for e in $(...)`.
- Entry names in the English releases' LODs are cut at 16 characters. Only MM8's `EnglishD.lod` has longer names,
  so `01female_wince.wav` is listed as `01female_wince.w` and must be read under that name.

## Commands

```sh
LodTool ls DIR_OR_ARCHIVE               # directory contents, or archive entries, sorted
LodTool cat [--raw] FILE                # write to stdout, decompressed unless --raw
LodTool extract [--raw] ARCHIVE OUT     # every entry into directory OUT
LodTool convert [--palettes-lod-path BITMAPS_LOD] FILE OUT.png
LodTool magic FILE                      # print the format
```

`FILE` is an archive entry or a plain file on disk, `cat` and `magic` work the same on both, so an extracted
`--raw` entry decompresses later with `cat`. `--raw` only matters for LOD entries, SND entries always come out as
plain WAV.

`extract` takes a LOD, VID or SND archive only, and creates `OUT` if needed. Only decompression happens, and
entries are written under the names `ls` prints. To save a single entry, redirect `cat` into a file.

`convert` writes PNG only, and takes LOD images, LOD palettes, LOD sprites, LOD fonts (rendered as a 16x16 glyph
grid) and PCX images, decompressing first. A sprite only stores palette indices and the id of its default palette.
Without `--palettes-lod-path` pointing at the game's bitmaps archive it comes out grayscale, with indices as gray
levels. With it, it's drawn in the default palette. For monsters that palette has cyan as the transparent
background and magenta in the recolored parts, so it's not what the game shows. Read the resulting PNG to see the
image.

`magic` prints one line. For compressed data it names the format inside too, as in
`LOD compressed file containing raw data` for an MM7 map.

## What's where

From a full `magic` scan of every archive in all three games. "Compressed" means `magic` prints
`LOD compressed file` or `LOD compressed pseudo image`, and `cat` decompresses it.

| Data | MM6 | MM7 | MM8 |
|---|---|---|---|
| maps, `.blv` `.dlv` `.odm` `.ddm` | `data/Games.lod`, see below | `DATA/GAMES.LOD`, compressed | `Data/games.lod`, compressed |
| events, strings, text tables, `.evt` `.str` `.txt` `.bin` | `data/icons.lod`, compressed | `DATA/events.lod`, compressed | `Data/EnglishT.lod`, see below |
| UI images, PCX screens, fonts | `data/icons.lod` | `DATA/ICONS.LOD` | `Data/icons.lod`, `EnglishT.lod`, `EnglishD.lod` |
| textures and palettes `palNNN` | `data/BITMAPS.LOD` | `DATA/BITMAPS.LOD` | `Data/bitmaps.lod` |
| sprites | `data/SPRITES.LOD` | `DATA/SPRITES.LOD` | `Data/sprites.lod` |
| sounds | `Sounds/Audio.snd`, WAV | `SOUNDS/Audio.snd`, WAV | `Data/EnglishD.lod`, see below |
| movies | `Anims/Anims1.vid`, `Anims2.vid`, Smacker | `Anims/Magic7.vid`, `Might7.vid`, Smacker and Bink | `Anims/Magicdod.vid`, `mightdod.vid`, Smacker and Bink |

Entries without an extension in the image archives are LOD images, palettes or sprites.

MM7's `ICONS.LOD` and MM8's `icons.lod` also hold stale copies of a few text tables. In MM7, `dsft.bin`,
`spells.txt`, `mapstats.txt` and five more differ from the copies in `events.lod`. OpenEnroth reads `events.lod`,
so read the table from there.

`magic` says `Raw data` for two kinds of entries that are compressed in a way LodTool doesn't know, and `cat`
passes them through unchanged:

- MM6 maps. Each is a little-endian u32 compressed size and a u32 uncompressed size, then a zlib stream. `cd3.blv`
  and `d08.blv` have bad adler32 checksums, so inflate them raw, as in Python's
  `zlib.decompressobj(-15).decompress(data[10:])`.
- MM8 `EnglishT.lod` and `EnglishD.lod` entries. Each has a 96-byte header, the full entry name in the first 64
  bytes, the compressed size as a u32 at byte 68 and the uncompressed size as a u32 at byte 88. A zlib stream
  follows. `EnglishD.lod` sounds inflate to WAV.

Past these, the scan found four entries `magic` can't name. MM6 `icons.lod/d4bm.evt` is a 48-byte stub, and MM7
`SPRITES.LOD/pending` is a 160-byte stub. MM7 `SOUNDS/Audio.snd/02flame01` is a WAV that fails its checksum and
comes out truncated. MM8 `bitmaps.lod/pal857` has 816 bytes, a header naming `pal857.pal` and what looks like a
palette. Use `cat --raw FILE | od -A d -t x1z | head` to look at the bytes when `magic` gives up.

## Recipes

Here `mm7` is the MM7 game data folder.

```sh
# Decompress an MM7 map for a parser or a hex dump.
LodTool cat "mm7/DATA/GAMES.LOD/d01.blv" > d01.blv

# Pull every MM7 event script, string table and text table out at once.
LodTool extract "mm7/DATA/events.lod" evt/

# Look at a sprite in its default palette.
LodTool convert --palettes-lod-path "mm7/DATA/BITMAPS.LOD" \
    "mm7/DATA/SPRITES.LOD/arrowa0" arrowa0.png

# Count the formats in an archive.
A="mm7/DATA/ICONS.LOD"
LodTool ls "$A" | while IFS= read -r e; do LodTool magic "$A/$e"; done | sort | uniq -c
```

Each call opens the archive anew, so a loop over a big archive like `SPRITES.LOD` takes a minute or two. Extract
it once and work on the files when you need more than a handful of entries.
