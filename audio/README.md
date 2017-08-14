## 分层正确性确认

- 驱动层：使用tinycap和tinyplay验证
- HAL层：使用audio_rec验证

## 驱动xrun调试

打开音频驱动的SND_PCM_XRUN_DEBUG选项，这样在<BR>
`/proc/asound/card*/pcm*/xrun_debug`，<BR>
然后通过<BR>
`echo 1> xrun_debug` 
