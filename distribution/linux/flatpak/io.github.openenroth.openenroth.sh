#!/bin/bash
DATAFILE="magic7.vid"
SAVEDIR="$XDG_DATA_HOME/mm7/saves"
GAMETITLE="Might & Magic 7"

export OPENENROTH_MM6_PATH="$XDG_DATA_HOME/mm6/data"
export OPENENROTH_MM7_PATH="$XDG_DATA_HOME/mm7/data"
export OPENENROTH_MM8_PATH="$XDG_DATA_HOME/mm8/data"

if [[ ! $(find "$OPENENROTH_MM7_PATH" -iname $DATAFILE) ]]; then
    # Create game data/save paths if needed to take at least that step off the user
    if [[ ! -d "$OPENENROTH_MM7_PATH" ]]; then mkdir -p "$OPENENROTH_MM7_PATH"; fi
    if [[ ! -d "$SAVEDIR" ]]; then mkdir -p "$SAVEDIR"; fi
    # Finally, fail with error message.
    zenity --error \
        --text "<b>Could not find $GAMETITLE game data!</b>\\n\\nPlease copy the game data files for $GAMETITLE to <tt><b>$OPENENROTH_MM7_PATH</b></tt>." \
        --ok-label 'Quit'
    exit 1
fi
exec OpenEnroth --user-path "${SAVEDIR}"
