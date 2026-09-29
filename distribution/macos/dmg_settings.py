# dmgbuild settings for the OpenEnroth disk image, see make_dmg.sh. Values passed with -D arrive in `defines`.
import os.path
import struct

application = defines['app']
background = defines['background']

format = 'ULMO'
files = [application]
symlinks = {'Applications': '/Applications'}
icon = defines['icon']

# The window takes the size of the background, and the icons sit where GenerateDmgBackground.py draws around them.
with open(background, 'rb') as f:
    width, height = struct.unpack('>II', f.read(24)[16:24]) # PNG IHDR, right after the 8-byte signature and chunk header.
window_rect = ((200, 120), (width, height))
icon_size = 128
text_size = 12
icon_locations = {
    os.path.basename(application): (width // 4, int(height * 0.475)),
    'Applications': (width * 3 // 4, int(height * 0.475)),
}
