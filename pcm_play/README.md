## decode mp3 to pcm

```
## mp3=>PCM
ffmpeg -i 1.mp3 -acodec pcm_s16le -ar 44100 -ac 2 -f s16le 44100_2_16.pcm

## PCM=>wav/mp3
ffmpeg -f s16le -ar 44100 -ac 2 -i 44100_2_16.pcm out.mp3
ffmpeg -f s16le -ar 44100 -ac 2 -i 44100_2_16.pcm out.wav

## get file information
ffprobe a.mp3

```
