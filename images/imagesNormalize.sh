#!/bin/bash
declare -a arr1[2]
count=0
for i in *-0.tiff; do
	f=${i/-0.tiff}
	g=${f/cam--}
	arr1[$count]=$g
	count=$(($count+1))
done
echo ${arr1[*]}
for i in *.tiff; do
	f=${i/.tiff}
	g=${f/cam--}
	h=${g/-*}
	j=${g/$h}
	if [ "$h" = "${arr1[0]}" ] 
	then
		mv "$i" "cam--0$j.tiff"
	fi
	if [ "$h" = "${arr1[1]}" ] 
	then
		mv "$i" "cam--1$j.tiff"
	fi
	echo
done