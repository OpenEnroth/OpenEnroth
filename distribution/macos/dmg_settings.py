# dmgbuild settings for the OpenEnroth disk image, see make_dmg.sh. Values passed with -D arrive in `defines`.
import os.path

application = defines['app']

format = 'ULMO'
files = [application]
symlinks = {'Applications': '/Applications'}
icon = defines['icon']

# The background is drawn for this window size and these icon positions.
background = defines['background']
window_rect = ((200, 120), (600, 400))
icon_size = 128
text_size = 12
icon_locations = {
    os.path.basename(application): (150, 190),
    'Applications': (450, 190),
}
