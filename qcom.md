QCOM camera releated information:

> qcom camera use ubwc(compress video data) as default video format, so if software want to access yuv data, you should disable ubwc first by the following commands:

```
adb shell setprop persist.camera.preview.ubwc 0 
adb shell setprop persist.camera.video.ubwc 0 
```

> 
> qcom yuv format limitations: `kernel/include/media/msm_media_info.h`
> 

```
 17     /* Venus NV12:
 18      * YUV 4:2:0 image with a plane of 8 bit Y samples followed
 19      * by an interleaved U/V plane containing 8 bit 2x2 subsampled
 20      * colour difference samples.
 21      *
 22      * <-------- Y/UV_Stride -------->
 23      * <------- Width ------->
 24      * Y Y Y Y Y Y Y Y Y Y Y Y . . . .  ^           ^
 25      * Y Y Y Y Y Y Y Y Y Y Y Y . . . .  |           |
 26      * Y Y Y Y Y Y Y Y Y Y Y Y . . . .  Height      |
 27      * Y Y Y Y Y Y Y Y Y Y Y Y . . . .  |          Y_Scanlines
 28      * Y Y Y Y Y Y Y Y Y Y Y Y . . . .  |           |
 29      * Y Y Y Y Y Y Y Y Y Y Y Y . . . .  |           |
 30      * Y Y Y Y Y Y Y Y Y Y Y Y . . . .  |           |
 31      * Y Y Y Y Y Y Y Y Y Y Y Y . . . .  V           |
 32      * . . . . . . . . . . . . . . . .              |
 33      * . . . . . . . . . . . . . . . .              |
 34      * . . . . . . . . . . . . . . . .              |
 35      * . . . . . . . . . . . . . . . .              V
 36      * U V U V U V U V U V U V . . . .  ^
 37      * U V U V U V U V U V U V . . . .  |
 38      * U V U V U V U V U V U V . . . .  |
 39      * U V U V U V U V U V U V . . . .  UV_Scanlines
 40      * . . . . . . . . . . . . . . . .  |
 41      * . . . . . . . . . . . . . . . .  V
 42      * . . . . . . . . . . . . . . . .  --> Buffer size alignment
 43      *
 44      * Y_Stride : Width aligned to 128
 45      * UV_Stride : Width aligned to 128
 46      * Y_Scanlines: Height aligned to 32
 47      * UV_Scanlines: Height/2 aligned to 16
 48      * Extradata: Arbitrary (software-imposed) padding
 49      * Total size = align((Y_Stride * Y_Scanlines
 50      *          + UV_Stride * UV_Scanlines
 51      *          + max(Extradata, Y_Stride * 8), 4096)
```

