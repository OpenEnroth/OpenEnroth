#!/usr/bin/env python3
#
# Draws the dmg window background at 1x and 2x, in the colors of the MM7 winner certificate: a sepia parchment ground
# with a title and a bronze arrow from the app to the Applications link. The layout matches dmg_settings.py, a 600x400
# window content area with 128pt icons centered at (150, 190) and (450, 190). Needs skia-python, Pillow and the DejaVu
# fonts.

import math
import os
import sys

import skia
from PIL import Image

W, H = 600, 400 # In points, every size below is in points too.
APP = (150, 190)
APPLICATIONS = (450, 190)
SEED = 7 # Fixed, so the noise and the output files are reproducible.

PAPER_TOP = 0xFFF2F0EB
PAPER_BOTTOM = 0xFFDBD4C4
PAPER_EDGE = 0xFFC9BDA5
BRONZE = 0xFF8C7B57
BRONZE_DARK = 0xFF60533A
INK = 0xFF252017

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
            return skia.Font(skia.Typeface.MakeFromFile(path), size)
    sys.exit(f'{name} not found, install the DejaVu fonts.')


def with_alpha(color, alpha):
    return (color & 0x00FFFFFF) | (alpha << 24)


def noise(canvas, frequency, octaves, strength):
    """Overlays gray fractal noise, which lightens and darkens the paper around its own color."""
    gray = skia.ColorFilters.Matrix([
        1, 0, 0, 0, 0,
        1, 0, 0, 0, 0,
        1, 0, 0, 0, 0,
        0, 0, 0, 0, strength,
    ])
    paint = skia.Paint(Shader=skia.PerlinNoiseShader.MakeFractalNoise(frequency, frequency, octaves, SEED),
                       ColorFilter=gray, BlendMode=skia.BlendMode.kOverlay)
    canvas.drawRect(skia.Rect(W, H), paint)


def parchment(canvas):
    canvas.drawRect(skia.Rect(W, H), skia.Paint(Shader=skia.GradientShader.MakeLinear(
        [(0, 0), (0, H)], [PAPER_TOP, PAPER_BOTTOM])))
    # Darkens toward the corners like aged paper.
    canvas.drawRect(skia.Rect(W, H), skia.Paint(Shader=skia.GradientShader.MakeRadial(
        (W / 2, H * 0.45), W * 0.65, [with_alpha(PAPER_EDGE, 0), with_alpha(PAPER_EDGE, 0), with_alpha(PAPER_EDGE, 150)],
        [0, 0.55, 1])))
    noise(canvas, 0.012, 2, 0.3) # Uneven aging, a few big patches.
    noise(canvas, 0.25, 2, 0.3) # Finer cloudiness.
    noise(canvas, 2.5, 1, 0.7) # Grain.


def arrow_paths():
    head_length, head_half_width = 17, 10
    notch_depth = head_length * 0.62 # How far the notch in the head's back edge sits from the tip.

    # The shaft is an arch that ends in the head's notch, and the head points along the arch's direction there.
    start = (APP[0] + 92, APP[1] - 4)
    notch = (APPLICATIONS[0] - 104 - notch_depth, APP[1] - 4)
    control = ((start[0] + notch[0]) / 2, start[1] - 30)
    shaft = skia.Path()
    shaft.moveTo(*start)
    shaft.quadTo(*control, *notch)

    ang = math.atan2(notch[1] - control[1], notch[0] - control[0])
    cos, sin = math.cos(ang), math.sin(ang)
    tip = (notch[0] + notch_depth * cos, notch[1] + notch_depth * sin)
    back = (tip[0] - head_length * cos, tip[1] - head_length * sin)
    head = skia.Path()
    head.moveTo(*tip)
    head.lineTo(back[0] + head_half_width * sin, back[1] - head_half_width * cos)
    head.lineTo(*notch)
    head.lineTo(back[0] - head_half_width * sin, back[1] + head_half_width * cos)
    head.close()
    return shaft, head


def arrow(canvas):
    shaft, head = arrow_paths()

    def draw(color, **extra):
        stroke = skia.Paint(AntiAlias=True, Color=color, Style=skia.Paint.kStroke_Style, StrokeWidth=4,
                            StrokeCap=skia.Paint.kRound_Cap, **extra)
        canvas.drawPath(shaft, stroke)
        canvas.drawPath(head, skia.Paint(AntiAlias=True, Color=color, **extra))

    canvas.save()
    canvas.translate(0, 3)
    draw(with_alpha(BRONZE_DARK, 70), MaskFilter=skia.MaskFilter.MakeBlur(skia.kNormal_BlurStyle, 3.5))
    canvas.restore()
    draw(BRONZE)


def centered_text(canvas, y, text, text_font, color):
    x = (W - text_font.measureText(text)) / 2
    canvas.drawString(text, x, y, text_font, skia.Paint(AntiAlias=True, Color=color))


def ornament(canvas, y):
    cx, half, gap = W / 2, 84, 11
    for sign in (-1, 1):
        start, end = (cx + sign * gap, y), (cx + sign * half, y)
        line = skia.Paint(AntiAlias=True, StrokeWidth=1, Shader=skia.GradientShader.MakeLinear(
            [start, end], [with_alpha(BRONZE, 210), with_alpha(BRONZE, 0)]))
        canvas.drawLine(*start, *end, line)
    diamond = skia.Path()
    diamond.moveTo(cx, y - 4)
    diamond.lineTo(cx + 4, y)
    diamond.lineTo(cx, y + 4)
    diamond.lineTo(cx - 4, y)
    diamond.close()
    canvas.drawPath(diamond, skia.Paint(AntiAlias=True, Color=with_alpha(BRONZE, 240)))


def render(scale):
    surface = skia.Surface(W * scale, H * scale)
    with surface as canvas:
        canvas.scale(scale, scale)
        parchment(canvas)
        arrow(canvas)
        centered_text(canvas, 56, 'Drag OpenEnroth into Applications', font('DejaVuSerif.ttf', 20), INK)
        ornament(canvas, 76)
        centered_text(canvas, H - 40, 'Open-source engine for Might and Magic VI, VII and VIII',
                      font('DejaVuSans.ttf', 10), BRONZE_DARK)
    return Image.fromarray(surface.makeImageSnapshot().toarray(colorType=skia.kRGBA_8888_ColorType)).convert('RGB')


def main():
    if len(sys.argv) != 3:
        sys.exit(f'Usage: {sys.argv[0]} <output.jpg> <output@2x.jpg>')
    for scale, path in ((1, sys.argv[1]), (2, sys.argv[2])):
        render(scale).save(path, format='JPEG', quality=92, subsampling=0, optimize=True)


if __name__ == '__main__':
    main()
