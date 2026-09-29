#!/usr/bin/env python3
#
# Draws the dmg window background at 1x and 2x, in the colors of the MM7 winner certificate: a sepia parchment ground
# with a bronze double frame, a title, and a bronze arrow from the app to the Applications link. The layout matches
# dmg_settings.py, a 600x400 window with 128pt icons centered at (150, 190) and (450, 190). Needs Pillow and the DejaVu
# fonts.
#
# Usage: GenerateDmgBackground.py <output.jpg> <output@2x.jpg>

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


def bezier(p0, p1, p2, p3, steps):
    def at(t, i):
        u = 1 - t
        return u ** 3 * p0[i] + 3 * u * u * t * p1[i] + 3 * u * t * t * p2[i] + t ** 3 * p3[i]
    return [(at(t, 0), at(t, 1)) for t in (i / steps for i in range(steps + 1))]


def supersampled(draw):
    """Runs draw(ImageDraw, scale) on a canvas 4x the image size and returns it scaled down, which anti-aliases it."""
    scale = 4
    big = Image.new('RGBA', (W * scale, H * scale), (0, 0, 0, 0))
    draw(ImageDraw.Draw(big), scale)
    return big.convert('RGBa').resize((W, H), Image.LANCZOS).convert('RGBA') # Premultiplied, so edges don't darken.


def stroke(draw, points, color, width_at):
    for i in range(len(points) - 1):
        t = i / (len(points) - 1)
        (ax, ay), (bx, by) = points[i], points[i + 1]
        draw.line([(ax, ay), (bx, by)], fill=color, width=max(1, int(width_at(t))))
        r = width_at(t) / 2
        draw.ellipse((bx - r, by - r, bx + r, by + r), fill=color)


def draw_arrow(img):
    length, half = 17 * S, 10 * S
    notch_depth = length * 0.62 # How far the notch in the head's back edge sits from the tip.

    # The shaft rises from the app and levels out into the head's notch, and the head points along the curve's
    # direction there, so the shaft enters the head on its center line.
    x0, x1 = APP[0] + 92 * S, APPLICATIONS[0] - 104 * S - notch_depth
    y = APP[1] - 4 * S
    d = x1 - x0
    c1, c2, end = (x0 + 0.25 * d, y - 24 * S), (x1 - 0.5 * d, y - 17 * S), (x1, y)
    shaft = bezier((x0, y), c1, c2, end, 400)
    ang = math.atan2(end[1] - c2[1], end[0] - c2[0])

    tip = (end[0] + notch_depth * math.cos(ang), end[1] + notch_depth * math.sin(ang))
    back = (tip[0] - length * math.cos(ang), tip[1] - length * math.sin(ang))
    head = [tip,
            (back[0] + half * math.sin(ang), back[1] - half * math.cos(ang)),
            end,
            (back[0] - half * math.sin(ang), back[1] + half * math.cos(ang))]

    def arrow(color, offset):
        def draw(d, scale):
            moved = lambda points: [(x * scale, (y + offset) * scale) for x, y in points]
            stroke(d, moved(shaft), color, lambda t: (1.5 + 3.5 * t) * S * scale)
            d.polygon(moved(head), fill=color)
        return draw

    shadow = supersampled(arrow(BRONZE_DARK + (70,), 3 * S))
    img.alpha_composite(shadow.filter(ImageFilter.GaussianBlur(3.5 * S)))
    img.alpha_composite(supersampled(arrow(BRONZE + (255,), 0)))


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
        sys.exit(f'Usage: {sys.argv[0]} <output.jpg> <output@2x.jpg>')

    img = parchment().convert('RGBA')
    frame(img)
    draw_arrow(img)
    centered_text(img, 36 * S, 'Drag OpenEnroth into Applications', font('DejaVuSerif.ttf', 20 * S), INK + (255,))
    ornament(img, 76 * S)
    centered_text(img, H - 44 * S, 'Open-source engine for Might and Magic VI, VII and VIII',
                  font('DejaVuSans.ttf', 10 * S), BRONZE_DARK + (255,))

    img = img.convert('RGB')
    jpeg ={'format': 'JPEG', 'quality': 92, 'subsampling': 0, 'optimize': True}
    img.resize((W // S, H // S), Image.LANCZOS).save(sys.argv[1], **jpeg)
    img.save(sys.argv[2], **jpeg)


if __name__ == '__main__':
    main()
