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
    adb logcat -c && cls ; adb logcat | tee _`date '+%Y%m%d.%H%M%S'`.log
}
function logk() {
    adb shell cat /proc/kmsg |tee k.log
}

function gpio() {
    # show current status
    if [ $# -eq 1 ]; then
        adb shell "echo $1 >/sys/class/gpio/export"
        adb shell "cat /sys/class/gpio/gpio$1/direction"
        adb shell "cat /sys/class/gpio/gpio$1/value"
        adb shell "echo $1 >/sys/class/gpio/unexport"
    # set gpio configuration
    elif [ $# -eq 3 ]; then
        adb shell "echo $1 >/sys/class/gpio/export"
        adb shell "echo $2 > /sys/class/gpio/gpio$1/direction"
        adb shell "echo $3 > /sys/class/gpio/gpio$1/value"
        adb shell "cat /sys/class/gpio/gpio$1/direction"
        adb shell "cat /sys/class/gpio/gpio$1/value"
        adb shell "echo $1 >/sys/class/gpio/unexport"
    else
        echo "usage: gpio <gpio_num direction value>"
        return -1
    fi
}
function dbgfs() {
    adb remount
    adb shell mkdir /mnt/dbg
    adb shell mount -t debugfs none /mnt/dbg
}

function mkboot() {
    make -C kernel O=../out/target/product/le_$1/obj/KERNEL_OBJ ARCH=arm64 CROSS_COMPILE=aarch64-linux-android- KCFLAGS=-mno-android -j 10
    acp out/target/product/le_$1/obj/KERNEL_OBJ/arch/arm64/boot/Image.gz-dtb out/target/product/le_$1/kernel

    out/host/linux-x86/bin/mkbootimg  --kernel out/target/product/le_$1/kernel --ramdisk out/target/product/le_$1/ramdisk.img --cmdline "console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 androidboot.hardware=qcom user_debug=31 msm_rtb.filter=0x237 ehci-hcd.park=3 lpm_levels.sleep_disabled=1 cma=32M@0-0xffffffff" --base 0x80000000 --pagesize 4096  --output out/target/product/le_$1/boot.img

    out/host/linux-x86/bin/boot_signer /boot out/target/product/le_$1/boot.img build/target/product/security/verity.pk8 build/target/product/security/verity.x509.pem out/target/product/le_$1/boot.img
}

##################################################################
