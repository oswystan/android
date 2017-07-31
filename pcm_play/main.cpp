/*
 **************************************************************************************
 *       Filename:  main.cpp
 *    Description:   source file
 *
 *        Version:  1.0
 *        Created:  2017-07-29 13:04:42
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */

#define LOG_TAG "pcm_play"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <media/AudioTrack.h>

using namespace android;

#define logd(fmt, ...) printf("[D/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)
#define loge(fmt, ...) printf("[E/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)

typedef struct _audio_cfg {
    audio_stream_type_t     type;
    uint32_t                sample_rate;
    audio_format_t          format;
    audio_channel_mask_t    channels;
} audio_cfg;

audio_cfg cfg = {
    .type        = AUDIO_STREAM_MUSIC,
    .sample_rate = 44100,
    .format      = AUDIO_FORMAT_PCM_16_BIT,
    .channels    = AUDIO_CHANNEL_OUT_STEREO
};

void play_pcm(const char* fn) {

    char buf[1024];
    int ret = 0;

    FILE* fp = fopen(fn, "r");
    if (!fp) {
        loge("fail to open file: %s[%s]", fn, strerror(errno));
        return;
    }

    sp<AudioTrack> t = new AudioTrack(cfg.type, cfg.sample_rate, cfg.format, cfg.channels);
    if (t.get() == NULL) {
        loge("low memory");
        goto OUT;
    }
    t->start();
    while(!feof(fp)) {
        ret = fread(buf, 1, sizeof(buf), fp);
        if (ret <= 0) {
            break;
        }
        t->write(buf, ret);
    }
    logd("done.");

    t->flush();
    t->stop();
    t.clear();

OUT:
    fclose(fp);
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        loge("usage: %s <pcm_data_file>", argv[0]);
        return 1;
    }
    play_pcm(argv[1]);
    return 0;
}

/********************************** END **********************************************/

