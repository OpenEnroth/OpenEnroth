#!/usr/bin/env python3
#
# Draws the dmg window background at 1x and 2x, in the colors of the MM7 winner certificate: a sepia parchment ground
# with a bronze double frame, a title, and a bronze arrow from the app to the Applications link. The layout matches
# dmg_settings.py, a 600x400 window with 128pt icons centered at (150, 190) and (450, 190). Needs Pillow and the DejaVu
# fonts.
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

PAPER_TOP = (242, 240, 235)
PAPER_BOTTOM = (219, 212, 196)
PAPER_EDGE = (201, 189, 165)
BRONZE = (140, 123, 87)
BRONZE_LIGHT = (185, 170, 139)
BRONZE_DARK = (96, 83, 58)
INK = (37, 32, 23)

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


def mottle(rnd, cells_x, cells_y, blur):
    """Smooth random field in -1..1, made by blowing up a small grid of random values."""
    small = Image.new('L', (cells_x, cells_y))
    small.putdata([rnd.randint(0, 255) for _ in range(cells_x * cells_y)])
    return small.resize((W, H), Image.BICUBIC).filter(ImageFilter.GaussianBlur(blur))


def parchment():
    rnd = random.Random(7) # Fixed seed, so the texture and the output files are reproducible.
    blotches = mottle(rnd, 24, 16, 18 * S).load() # Uneven aging, a few big patches.
    fibers = mottle(rnd, 300, 200, 0.75 * S).load() # Finer cloudiness.

    img = Image.new('RGB', (W, H))
    px = img.load()
    cx, cy = W / 2, H * 0.45
    for y in range(H):
        base = lerp(PAPER_TOP, PAPER_BOTTOM, y / (H - 1))
        for x in range(W):
            dx, dy = (x - cx) / (W / 2), (y - cy) / (H / 2)
            edge = min(1.0, math.hypot(dx, dy) / 1.3) ** 3 * 0.55 # Darkens toward the corners like aged paper.
            c = lerp(base, PAPER_EDGE, edge)
            n = (blotches[x, y] - 128) / 128 * 9 + (fibers[x, y] - 128) / 128 * 5 + rnd.gauss(0, 4.5)
            # Darker spots also turn a little browner, the way old paper stains.
            px[x, y] = (max(0, min(255, int(round(c[0] + n)))),
                        max(0, min(255, int(round(c[1] + n * 1.08)))),
                        max(0, min(255, int(round(c[2] + n * 1.25)))))
    return img.filter(ImageFilter.GaussianBlur(0.15 * S))


def frame(img):
    d = ImageDraw.Draw(img)
    outer, inner = 10 * S, 14 * S
    d.rectangle((outer, outer, W - 1 - outer, H - 1 - outer), outline=BRONZE + (200,), width=2 * S)
    d.rectangle((inner, inner, W - 1 - inner, H - 1 - inner), outline=BRONZE + (130,), width=S)


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
    stroke(d, shaft, 3 * S, lambda t: BRONZE_DARK + (int(30 + 45 * t),), width_at)
    d.polygon([(x, y + 3 * S) for x, y in head], fill=BRONZE_DARK + (80,))
    img.alpha_composite(shadow.filter(ImageFilter.GaussianBlur(3.5 * S)))

    body = Image.new('RGBA', img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(body)
    stroke(d, shaft, 0,
           lambda t: tuple(int(c) for c in lerp(BRONZE_LIGHT, BRONZE, t)) + (int(80 + 175 * min(1.0, t * 1.4)),),
           width_at)
    d.polygon(head, fill=BRONZE + (255,))
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
            a = int(210 * (1 - i / (half - gap)) ** 1.2)
            x = cx + sign * (gap + i)
            d.line([(x, y), (x + sign, y)], fill=BRONZE + (a,), width=S)
    r = 4 * S
    d.polygon([(cx, y - r), (cx + r, y), (cx, y + r), (cx - r, y)], fill=BRONZE + (240,))


def main():
    if len(sys.argv) != 3:
        sys.exit(f'Usage: {sys.argv[0]} <output.png> <output@2x.png>')

    img = parchment().convert('RGBA')
    frame(img)
    draw_arrow(img)
    centered_text(img, 36 * S, 'Drag OpenEnroth into Applications', font('DejaVuSerif.ttf', 20 * S), INK + (255,))
    ornament(img, 76 * S)
    centered_text(img, H - 44 * S, 'Open-source engine for Might and Magic VI, VII and VIII',
                  font('DejaVuSans.ttf', 10 * S), BRONZE_DARK + (255,))

    img = img.convert('RGB')
    img.resize((W // S, H // S), Image.LANCZOS).save(sys.argv[1], optimize=True)
    img.save(sys.argv[2], optimize=True)


if __name__ == '__main__':
    main()
