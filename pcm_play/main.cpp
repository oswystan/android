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
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <media/AudioTrack.h>

using namespace android;

#define log(fmt, ...) do{ printf(fmt, ##__VA_ARGS__); fflush(stdout); } while(0)
#define logd(fmt, ...) printf("[D/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)
#define loge(fmt, ...) printf("[E/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)

static char buf[1024*88];

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

int get_cfg_by_name(const char* fn, audio_cfg* c) {
    const char* ptr = basename(fn);
    int ret = 0;
    int sample_rate = 0;
    int channels = 0;
    int bits = 0;
    ret = sscanf(ptr, "%d_%d_%d.pcm", &sample_rate, &channels, &bits);
    if (ret != 3) {
        loge("invalid file name format: %s(example: 16000_2_16.pcm)", ptr);
        return -1;
    }

    logd("sample_rate: %d, channels: %d, bits: %d", sample_rate, channels, bits);
    c->sample_rate = sample_rate;
    switch (bits) {
        case 16:
            c->format = AUDIO_FORMAT_PCM_16_BIT;
            break;
        case 8:
            c->format = AUDIO_FORMAT_PCM_8_BIT;
            break;
        case 32:
            c->format = AUDIO_FORMAT_PCM_32_BIT;
            break;
        default:
            loge("invalid bits: %d", bits);
            return -1;
    }
    c->channels = channels;
    return 0;
}

void play_pcm(const char* fn) {
    int ret = 0;
    if (get_cfg_by_name(fn, &cfg) != 0) {
        return;
    }

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
    logd("play ...");
    t->start();
    while(!feof(fp)) {
        ret = fread(buf, 1, sizeof(buf), fp);
        if (ret <= 0) {
            break;
        }
        log(".");
        t->write(buf, ret);
    }
    log("\n");
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

