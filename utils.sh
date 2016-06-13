##################################################################
##
## some useful scripts for android development.
##
##################################################################
function bk() {
    count=1
    if [ $# -gt 0 ]; then
        count=$1
    fi  

    while [ $count -gt 0 ] 
    do
        adb shell input keyevent 4
        count=`expr $count - 1`
    done
}

function kmedia() {
    adb shell ps |grep mediaserver
    adb shell ps |grep mediaserver |cut -f 6 -d' '|xargs adb shell kill -9
    sleep 0.2 
    adb shell ps |grep mediaserver
}

function logcat() {
    adb logcat -c && cls ; adb logcat 
}

##################################################################
