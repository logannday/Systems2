#!/bin/bash

gcc filesec.c

rm output.txt
rm rec_test.txt
touch rec_test.txt
touch output.txt

for n in a b c;
do
    echo "asdfghjkl;" >> rec_test.txt
    ./filesec -e "rec_test.txt"
done
