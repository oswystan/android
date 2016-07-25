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

function akill() {
    adb root
    msg=`adb shell ps | grep $1`
    echo $msg
    echo $msg |cut -f 2 -d' '|xargs adb shell kill -9
    sleep 0.2
    msg=`adb shell ps | grep $1`
    echo $msg
}

function kmedia() {
    akill mediaserver
}

function logcat() {
    adb logcat -c && cls ; adb logcat 
}

function gpio() {
    if [ $# -ne 3 ]; then
        echo "usage: gpio <gpio_num direction value>"
        return -1
    fi
    adb shell "echo $1 >/sys/class/gpio/export"
    adb shell "echo $2 > /sys/class/gpio/gpio$1/direction"
    adb shell "echo $3 > /sys/class/gpio/gpio$1/value"
    adb shell "cat /sys/class/gpio/gpio$1/direction"
    adb shell "cat /sys/class/gpio/gpio$1/value"
}

##################################################################
