QCOM camera releated information:

> qcom camera use ubwc(compress video data) as default video format, so if software want to access yuv data, you should disable ubwc first by the following commands:

```
adb shell setprop persist.camera.preview.ubwc 0 
adb shell setprop persist.camera.video.ubwc 0 
```



