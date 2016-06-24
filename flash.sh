#!/bin/bash

function flash_it()
{
    p=`echo $1 | cut -d. -f1`
    echo $p $1
    sudo fastboot flash $p $1
}

####################################
images="boot.img system.img userdata.img"
if [ $# -ne 0 ]; then
    images=$*
fi

adb reboot bootloader
for f in $images
do
    flash_it $f
done
sudo fastboot reboot
