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

#define LOG_TAG "audio_rec"
#include <stdio.h>
#include <media/AudioRecord.h>

using namespace android;

#define logd(fmt, ...) printf("[D/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)
#define loge(fmt, ...) printf("[E/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)

//AUDIO_SOURCE_MIC
//AUDIO_FORMAT_PCM_SUB_16_BIT
typedef struct _audio_cfg {
    audio_source_t src;
    audio_format_t fmt;
    audio_channel_mask_t channels;
    uint32_t frame_cnt;
    uint32_t sample_rate;
} audio_cfg;

audio_cfg cfg = {
    src: AUDIO_SOURCE_MIC,
    fmt:  AUDIO_FORMAT_PCM_16_BIT,
    channels: AUDIO_CHANNEL_IN_STEREO,
    frame_cnt: 2048,
    sample_rate: 16000,
};

void data_callback(int event, void*, void *info) {
    AudioRecord::Buffer* buf = (AudioRecord::Buffer*)info;
    switch (event) {
        case AudioRecord::EVENT_MORE_DATA:
            logd("data callback : %d", buf->size);
            break;
        default:
            loge("invalid event type: %d", event);
    }
}

int main(int argc, const char *argv[]) {
    int time = 4;
    size_t min_cnt;
    if (argc >= 2) {
        time = atoi(argv[1]);
    }
    AudioRecord::getMinFrameCount(&min_cnt, cfg.sample_rate, cfg.fmt, cfg.channels);
    logd("get min frame count: %d", min_cnt);
    cfg.frame_cnt = min_cnt*2;

    sp<AudioRecord> rec = new AudioRecord(cfg.src, cfg.sample_rate, cfg.fmt,
                                       cfg.channels, cfg.frame_cnt, data_callback,
                                       NULL);
    if (rec.get() == NULL) {
        loge("low memory");
        return 1;
    }

    if (0 != rec->initCheck()) {
        loge("rec init check fail");
        rec.clear();
        return 1;
    }

    logd("start recording...");
    rec->start();
    sleep(time);
    logd("stop recording...");
    rec->stop();
    logd("stopped");

    rec.clear();
    return 0;
}

/********************************** END **********************************************/

