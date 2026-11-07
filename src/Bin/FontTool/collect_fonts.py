#!/usr/bin/env python3
"""
Collects font files out of a pile of archives into a content-addressed store, for `FontTool search`.

Every font is written once to `STORE/<sha256[:2]>/<sha256>.<ext>`, however many archives carry it, and
`STORE/index.tsv` maps each hash back to every place it was found. Nested zips are read in memory, other nested
archives (rar, 7z) are unpacked with `7z` into a scratch directory. Nothing in the archives is ever executed, and
archive member names never become output paths.

Usage: collect_fonts.py STORE ARCHIVE_OR_DIR... [--jobs N]
"""

import argparse
import hashlib
import io
import os
import shutil
import subprocess
import sys
import tempfile
import zipfile
from multiprocessing import Pool

FONT_EXTENSIONS = {'ttf', 'otf', 'ttc', 'otc', 'tt', 'pfb', 'pfa', 'fon', 'fnt', 'dfont', 'woff'}
ARCHIVE_EXTENSIONS = {'zip', 'rar', '7z'}
MAX_DEPTH = 4  # Nesting limit, so that a zip bomb of zips can't recurse forever.
MAX_FONT_SIZE = 64 << 20  # Bigger "fonts" are CJK monsters or garbage, neither is a bitmap game font's source.


def extension(name):
    _, dot, ext = name.rpartition('.')
    return ext.lower() if dot else ''


class Collector:
    def __init__(self, store):
        self.store = store
        self.lines = []
        self.errors = []

    def add_font(self, data, ext, origin):
        sha = hashlib.sha256(data).hexdigest()
        directory = os.path.join(self.store, sha[:2])
        path = os.path.join(directory, f'{sha}.{ext}')
        if not os.path.exists(path):
            os.makedirs(directory, exist_ok=True)
            temp = f'{path}.{os.getpid()}.tmp'
            with open(temp, 'wb') as f:
                f.write(data)
            os.replace(temp, path)  # Atomic, two workers finding the same font both end up with one good file.
        self.lines.append(f'{sha}\t{ext}\t{len(data)}\t{origin}\n')

    def add_member(self, data, name, origin, depth):
        ext = extension(name)
        if ext in FONT_EXTENSIONS:
            if 0 < len(data) <= MAX_FONT_SIZE:
                self.add_font(data, ext, origin)
        elif ext in ARCHIVE_EXTENSIONS and depth < MAX_DEPTH:
            self.add_nested(data, ext, origin, depth + 1)

    def add_nested(self, data, ext, origin, depth):
        try:
            if ext == 'zip' and zipfile.is_zipfile(io.BytesIO(data)):
                with zipfile.ZipFile(io.BytesIO(data)) as archive:
                    self.add_zip(archive, origin, depth, 0, 1)
            else:
                with tempfile.TemporaryDirectory(dir=self.store, prefix='.scratch-') as scratch:
                    source = os.path.join(scratch, f'archive.{ext}')
                    with open(source, 'wb') as f:
                        f.write(data)
                    self.add_7z(source, origin, depth)
        except Exception as e:
            self.errors.append(f'{origin}: {e}\n')

    def add_zip(self, archive, origin, depth, shard, shards):
        for i, info in enumerate(archive.infolist()):
            if i % shards != shard or info.is_dir():
                continue
            ext = extension(info.filename)
            if ext not in FONT_EXTENSIONS and ext not in ARCHIVE_EXTENSIONS:
                continue
            if ext in FONT_EXTENSIONS and info.file_size > MAX_FONT_SIZE:
                continue
            member = f'{origin}!{info.filename}'
            try:
                data = archive.read(info)
            except Exception as e:
                self.errors.append(f'{member}: {e}\n')
                continue
            self.add_member(data, info.filename, member, depth)

    def add_7z(self, source, origin, depth):
        with tempfile.TemporaryDirectory(dir=self.store, prefix='.scratch-') as scratch:
            target = os.path.join(scratch, 'out')
            os.mkdir(target)
            subprocess.run(['7z', 'x', '-y', '-bd', '-p', f'-o{target}', source], stdin=subprocess.DEVNULL,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=4 * 3600)
            for root, _, files in os.walk(target):
                for name in files:
                    path = os.path.join(root, name)
                    if os.path.islink(path) or not os.path.isfile(path):
                        continue
                    if extension(name) not in FONT_EXTENSIONS | ARCHIVE_EXTENSIONS:
                        continue
                    with open(path, 'rb') as f:
                        member = f.read()
                    self.add_member(member, name, f'{origin}!{os.path.relpath(path, target)}', depth)


def run_task(task):
    store, path, shard, shards = task
    collector = Collector(store)
    ext = extension(path)
    try:
        if ext == 'zip':
            try:
                archive = zipfile.ZipFile(path)
                archive.infolist()[0].filename.encode()
            except Exception:
                # zipfile chokes on some zip64 directories and on names that aren't valid UTF-8, 7z reads both.
                archive = None
                if shard == 0:
                    collector.add_7z(path, path, 0)
            if archive:
                with archive:
                    collector.add_zip(archive, path, 0, shard, shards)
        elif ext in ARCHIVE_EXTENSIONS:
            collector.add_7z(path, path, 0)
        else:
            with open(path, 'rb') as f:
                collector.add_member(f.read(), path, path, 0)
    except Exception as e:
        collector.errors.append(f'{path}: {e}\n')

    part = os.path.join(store, 'parts', f'{hashlib.sha1(os.fsencode(path)).hexdigest()}.{shard}')
    with open(f'{part}.tsv', 'w') as f:
        f.writelines(collector.lines)
    with open(f'{part}.err', 'w') as f:
        f.writelines(collector.errors)
    return path, shard, len(collector.lines), len(collector.errors)


def main():
    parser = argparse.ArgumentParser(description='Collect fonts from archives into a deduplicated store.')
    parser.add_argument('store')
    parser.add_argument('inputs', nargs='+')
    parser.add_argument('--jobs', type=int, default=os.cpu_count())
    args = parser.parse_args()

    if not shutil.which('7z'):
        sys.exit('7z is required for rar and 7z archives')

    store = os.path.abspath(args.store)
    os.makedirs(os.path.join(store, 'parts'), exist_ok=True)

    files = []
    for input in args.inputs:
        if os.path.isdir(input):
            for root, dirs, names in os.walk(input):
                dirs[:] = [d for d in dirs if os.path.abspath(os.path.join(root, d)) != store]
                files += [os.path.join(root, name) for name in names]
        else:
            files.append(input)
    files = [os.path.abspath(f) for f in files
             if extension(f) in FONT_EXTENSIONS | ARCHIVE_EXTENSIONS and os.path.isfile(f)]

    # Big zips are split into shards by member index, so that one 22 GB zip doesn't run on a single core.
    shard_size = 1 << 30
    tasks = []
    for path in sorted(files, key=os.path.getsize, reverse=True):
        shards = max(1, os.path.getsize(path) // shard_size) if extension(path) == 'zip' else 1
        tasks += [(store, path, shard, shards) for shard in range(shards)]

    with Pool(args.jobs) as pool:
        for done, (path, shard, count, errors) in enumerate(pool.imap_unordered(run_task, tasks), 1):
            print(f'[{done}/{len(tasks)}] {path} #{shard}: {count} fonts, {errors} errors', flush=True)

    with open(os.path.join(store, 'index.tsv'), 'w') as index:
        for name in sorted(os.listdir(os.path.join(store, 'parts'))):
            if name.endswith('.tsv'):
                with open(os.path.join(store, 'parts', name)) as part:
                    shutil.copyfileobj(part, index)


if __name__ == '__main__':
    main()
