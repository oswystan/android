#!/bin/bash
##########################################################################
##
## used for push libs that updated from the last push operation.
## subcommands:
##      t           - update the timestamp file
##      push_last   - push the last file(s) which pushed last time.
##
##########################################################################
out_dir="/Users/winner/usr/project/le/turbo/src/daily/out/target/product/le_turbo"
out_file="$out_dir/_last_push_"
push_list=""

function get_push_list() {
    if [ ! -f $out_file ]; then
        touch $out_file
        return;
    fi
    old_dir=`pwd`
    cd $out_dir
    push_list=`find system -newer $out_file | grep -v test`
    cd $old_dir
    echo $push_list > $out_file
}
function t() {
    touch $out_file
}
function push_last() {
    touch $out_file
    push_list=`cat $out_file`
    adb root
    adb remount
    for one in $push_list
    do
        echo $one
        adb push $out_dir/$one $one
    done
    adb shell sync
}

function main() {
    get_push_list
    adb root
    adb remount
    for one in $push_list
    do
        echo $one
        adb push $out_dir/$one $one
    done
    adb shell sync
}

if [ $# -eq 0 ]; then
    main
else
    $*
fi

