#!/bin/bash
# AUTHOR: stefan seifried
# MATRNR: 0925401
# create id3info.txt used on fat images for mp3 player
# usage: ./createid3info.sh folder
# output: filename;artist;title; -> tokenize for comma and you will get the data for each file ;)

file="$1"/id3info.txt

[ ! -d "$file" ] || rm "$file"
touch "$file"
for i in "$1"/*.mp3
do 	
	o=${i//"$1/"/}
	o=${o//"/"/}
	o=${o//".mp3"/}
	
	printf %-8.8s $o >> "$file"
	printf %-55.55s"\n" "`./readid3.sh "$i"`" >> "$file"
done

