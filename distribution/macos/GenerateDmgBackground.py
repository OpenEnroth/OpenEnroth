#!/usr/bin/env python3
#
# Draws the dmg window background at 1x and 2x: a parchment ground, a title, and a gold arrow from the app to the
# Applications link. The layout matches dmg_settings.py, a 600x400 window with 128pt icons centered at (150, 190) and
# (450, 190). Needs Pillow and the DejaVu fonts.
#
# Usage: GenerateDmgBackground.py <output.png> <output@2x.png>

import math
import os
import random
import sys

from PIL import Image, ImageDraw, ImageFilter, ImageFont

S = 2 # Everything is drawn at 2x and downscaled for 1x.
W, H = 600 * S, 400 * S
APP = (150 * S, 190 * S)
APPLICATIONS = (450 * S, 190 * S)

GOLD = (206, 150, 34)
GOLD_LIGHT = (232, 188, 76)
INK = (64, 50, 34)
INK_SOFT = (128, 110, 86)

FONT_DIRS = [
    '/usr/share/fonts/truetype/dejavu',
    '/usr/share/fonts/TTF',
    '/usr/share/fonts/dejavu',
    os.path.expanduser('~/Library/Fonts'),
    '/Library/Fonts',
]


def font(name, size):
    for directory in FONT_DIRS:
        path = os.path.join(directory, name)
        if os.path.isfile(path):
            return ImageFont.truetype(path, size)
    sys.exit(f'{name} not found, install the DejaVu fonts.')


def lerp(a, b, t):
    return tuple(a[i] + (b[i] - a[i]) * t for i in range(len(a)))


def parchment():
    top, bottom = (251, 247, 237), (240, 231, 210)
    img = Image.new('RGB', (W, H))
    px = img.load()
    cx, cy = W / 2, H * 0.45
    rnd = random.Random(7) # Fixed seed, so the grain and the output files are reproducible.
    for y in range(H):
        base = lerp(top, bottom, y / (H - 1))
        for x in range(W):
            dx, dy = (x - cx) / (W / 2), (y - cy) / (H / 2)
            d = min(1.0, math.hypot(dx, dy) / 1.25)
            v = 1.0 - 0.045 * d ** 2.5 # Faint vignette.
            n = rnd.gauss(0, 1.3) # Paper grain.
            px[x, y] = tuple(max(0, min(255, int(round(c * v + n)))) for c in base)
    return img.filter(ImageFilter.GaussianBlur(0.5 * S))


def bezier(p0, p1, p2, steps):
    return [((1 - t) ** 2 * p0[0] + 2 * (1 - t) * t * p1[0] + t * t * p2[0],
             (1 - t) ** 2 * p0[1] + 2 * (1 - t) * t * p1[1] + t * t * p2[1])
            for t in (i / steps for i in range(steps + 1))]


def stroke(draw, points, offset, color_at, width_at):
    for i in range(len(points) - 1):
        t = i / (len(points) - 1)
        (ax, ay), (bx, by) = points[i], points[i + 1]
        draw.line([(ax, ay + offset), (bx, by + offset)], fill=color_at(t), width=max(1, int(width_at(t))))
        r = width_at(t) / 2
        draw.ellipse((bx - r, by + offset - r, bx + r, by + offset + r), fill=color_at(t))


def draw_arrow(img):
    x0, x1 = APP[0] + 92 * S, APPLICATIONS[0] - 104 * S
    y = APP[1] - 4 * S
    pts = bezier((x0, y), ((x0 + x1) / 2, y - 30 * S), (x1, y), 200)
    tip = pts[-1]
    ang = math.atan2(pts[-1][1] - pts[-6][1], pts[-1][0] - pts[-6][0])

    length, half = 17 * S, 10 * S
    back = (tip[0] - length * math.cos(ang), tip[1] - length * math.sin(ang))
    notch = (tip[0] - length * 0.62 * math.cos(ang), tip[1] - length * 0.62 * math.sin(ang))
    head = [tip,
            (back[0] + half * math.sin(ang), back[1] - half * math.cos(ang)),
            notch,
            (back[0] - half * math.sin(ang), back[1] + half * math.cos(ang))]
    shaft = pts[:-10]
    width_at = lambda t: (1.5 + 3.5 * t) * S

    shadow = Image.new('RGBA', img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(shadow)
    stroke(d, shaft, 3 * S, lambda t: (70, 45, 10, int(40 + 50 * t)), width_at)
    d.polygon([(x, y + 3 * S) for x, y in head], fill=(70, 45, 10, 90))
    img.alpha_composite(shadow.filter(ImageFilter.GaussianBlur(3.5 * S)))

    body = Image.new('RGBA', img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(body)
    stroke(d, shaft, 0,
           lambda t: tuple(int(c) for c in lerp(GOLD_LIGHT, GOLD, t)) + (int(70 + 185 * min(1.0, t * 1.4)),),
           width_at)
    d.polygon(head, fill=GOLD + (255,))
    img.alpha_composite(body)


def centered_text(img, y, text, text_font, fill):
    d = ImageDraw.Draw(img)
    w = d.textlength(text, font=text_font)
    d.text(((W - w) / 2, y), text, font=text_font, fill=fill)


def ornament(img, y):
    d = ImageDraw.Draw(img)
    cx, half, gap = W / 2, 84 * S, 11 * S
    for sign in (-1, 1):
        for i in range(int(half - gap)):
            a = int(200 * (1 - i / (half - gap)) ** 1.2)
            x = cx + sign * (gap + i)
            d.line([(x, y), (x + sign, y)], fill=GOLD + (a,), width=S)
    r = 4 * S
    d.polygon([(cx, y - r), (cx + r, y), (cx, y + r), (cx - r, y)], fill=GOLD + (235,))


def main():
    if len(sys.argv) != 3:
        sys.exit(f'Usage: {sys.argv[0]} output.png output@2x.png')

    img = parchment().convert('RGBA')
    draw_arrow(img)
    centered_text(img, 36 * S, 'Drag OpenEnroth into Applications', font('DejaVuSerif.ttf', 20 * S), INK + (255,))
    ornament(img, 76 * S)
    centered_text(img, H - 32 * S, 'Open-source engine for Might and Magic VI, VII and VIII',
                  font('DejaVuSans.ttf', 10 * S), INK_SOFT + (255,))

    img = img.convert('RGB')
    img.resize((W // S, H // S), Image.LANCZOS).save(sys.argv[1], optimize=True)
    img.save(sys.argv[2], optimize=True)


if __name__ == '__main__':
    main()
