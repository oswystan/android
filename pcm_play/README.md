## decode mp3 to pcm

```
ffmpeg -i 1.mp3 -acodec pcm_s16le -ar 44100 -ac 2 -f s16le a.raw
```
