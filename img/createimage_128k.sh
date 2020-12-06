#!/bin/bash
# AUTHOR: stefan seifried
# MATRNR: 0925401
# create fat image with 128k mp3's
# the image will have sufficient space to fit in all mp3's 
# in the specified bitrate folder

dir=./mp3/128k 
image=image_128k.bin

[ -a "$dir" ] || echo "no 128k directory found, convert your mp3s first!" | exit 1
[ ! -d "$image" ] || rm "$image"

# get image size
size="`du -sb "$dir" | awk '{print $1}'`"
let size+=10000000							#add 10MB to directory size

# create image
dd if=/dev/zero of="$image" bs=1 count=1 seek="$size"
/sbin/mkfs.vfat "$image"

# copy every file to image
for i in "$dir"/*
do
	mcopy -i "$image" "$i" ::
done

mdir -i "$image"

