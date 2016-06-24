#!/bin/bash

base_dir="usr/project/le/turbo/src/bsp/out/target/product/le_turbo/"
#base_dir="usr/project/le/turbo/src/smt/out/target/product/le_turbo/"

usr="andbase"
srv="10.142.129.149"
images="boot.img system.img userdata.img"

function get_file()
{
    if [ $# -ne 1 ]; then
        echo "Need a file name"
        return 1
    fi
    scp $usr@$srv:$base_dir/$1 ./
}

####################################
if [ $# -ne 0 ]; then
    images=$*
fi

for f in $images
do
    echo get $f
    get_file $f
done
