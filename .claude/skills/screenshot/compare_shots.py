#!/usr/bin/env python3
"""Compare old vs new screenshots pixel by pixel. Usage: compare_shots.py OLD_DIR NEW_DIR OUT_DIR"""
import sys
import os
from PIL import Image, ImageChops

old_dir, new_dir, out_dir = sys.argv[1], sys.argv[2], sys.argv[3]
os.makedirs(out_dir, exist_ok=True)

names = sorted(n for n in os.listdir(old_dir) if n.endswith(".png"))
total_diff = 0
for name in names:
    new_path = os.path.join(new_dir, name)
    if not os.path.exists(new_path):
        print(f"{name}: MISSING in new dir")
        total_diff += 1
        continue
    a = Image.open(os.path.join(old_dir, name)).convert("RGB")
    b = Image.open(new_path).convert("RGB")
    if a.size != b.size:
        print(f"{name}: size mismatch {a.size} vs {b.size}")
        total_diff += 1
        continue
    diff = ImageChops.difference(a, b)
    bbox = diff.getbbox()
    if bbox is None:
        print(f"{name}: IDENTICAL")
        continue
    mask = diff.convert("L").point(lambda p: 255 if p else 0)
    count = sum(1 for p in mask.getdata() if p)
    total_diff += count
    print(f"{name}: {count} differing pixels, bbox={bbox}")
    # Side-by-side with diff highlighted in red on the old image.
    w, h = a.size
    combo = Image.new("RGB", (w * 3, h))
    combo.paste(a, (0, 0))
    combo.paste(b, (w, 0))
    marked = a.copy()
    red = Image.new("RGB", a.size, (255, 0, 0))
    marked.paste(red, (0, 0), mask)
    combo.paste(marked, (w * 2, 0))
    combo.save(os.path.join(out_dir, f"diff_{name}"))

print(f"TOTAL: {total_diff} differing pixels across {len(names)} screenshots")
sys.exit(0 if total_diff == 0 else 1)
