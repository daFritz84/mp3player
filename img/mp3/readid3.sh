#!/bin/bash
# AUTHOR: stefan seifried
# MATRNR: 0925401
# read id3 info, thanks to fancybone
# >> http://www.linuxquestions.org/questions/linux-general-1/bash-script-for-sorting-and-renaming-multiple-mp3-files-by-id3-tags-602105/

TITLE="`id3info "$1" | grep '^=== TIT2' | sed -e 's/.*: //g'`"
ARTIST="`id3info "$1" | grep '^=== TPE1' | sed -e 's/.*: //g'`"

echo "$ARTIST-$TITLE"
