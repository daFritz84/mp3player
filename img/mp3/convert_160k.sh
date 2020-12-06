#!/bin/bash
# AUTHOR: stefan seifried
# MATRNR: 0925401
# converts all files in original to 160k

[ -a 160k ] || mkdir 160k
cnt=0

for i in ./original/*.mp3
do 
	let cnt+=1
	o=./160k/"$cnt".mp3	
	ffmpeg -i "$i" -ab 160k "$o"
	#originally tried to use "-map_meta_data 0:0,s0" to copy id3-tag info
	#but didn't work to well though: information got displayed at the output file
	#during conversion but then id3info could not get artist e.g.
	id3cp "$i" "$o"
done
