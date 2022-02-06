#! /bin/bash

filename=$1

echo "file is $filename"
filename_shuffled=${filename}_shuffled

echo "shuffle file is $filename_shuffled"

shuf $filename >> $filename_shuffled

lines=$(wc -l < $filename_shuffled)
first_lines=$((${lines} * 2 / 3))
split -l $first_lines $filename_shuffled

mv xaa train.data
mv xab test.data

rm $filename_shuffled
