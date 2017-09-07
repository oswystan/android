/*
 **************************************************************************************
 *       Filename:  jpeg_enc.cpp
 *    Description:   source file
 *
 *        Version:  1.0
 *        Created:  2017-09-06 10:43:06
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */

#include <pthread.h>
#include <errno.h>
#include "log.h"

extern "C" {
#include "mm_jpeg_interface.h"
#include <media/msm_media_info.h>

#include <linux/msm_ion.h>

typedef struct  {
  struct ion_fd_data ion_info_fd;
  struct ion_allocation_data alloc;
  int p_pmem_fd;
  size_t size;
  int ion_fd;
  uint8_t *addr;
} buffer_t;
void* buffer_allocate(buffer_t *p_buffer, int cached);
int buffer_deallocate(buffer_t *p_buffer);

}

typedef struct _jpeg_enc_t {
    uint32_t handle;
    uint32_t job_id;
    uint32_t session_id;
    uint32_t out_size;

    buffer_t                out;
    buffer_t                in;
    mm_dimension            size;
    pthread_mutex_t         lock;
    pthread_cond_t          cond;
    mm_jpeg_ops_t           ops;
    mm_jpeg_job_t           job;
    mm_jpeg_encode_params_t params;

    jpeg_job_status_t       status;
} jpeg_enc_t;

static jpeg_enc_t jpeg;

unsigned long get_ms() {
    struct timeval tv; 
    struct timespec ts;
    unsigned long ret = 0;

    if (0 == gettimeofday(&tv, NULL)) {
        ret = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }   

    return ret;
}

/* Static constants */
/*  default Luma Qtable */
uint8_t DEFAULT_QTABLE_0[QUANT_SIZE] = {
  16, 11, 10, 16, 24, 40, 51, 61,
  12, 12, 14, 19, 26, 58, 60, 55,
  14, 13, 16, 24, 40, 57, 69, 56,
  14, 17, 22, 29, 51, 87, 80, 62,
  18, 22, 37, 56, 68, 109, 103, 77,
  24, 35, 55, 64, 81, 104, 113, 92,
  49, 64, 78, 87, 103, 121, 120, 101,
  72, 92, 95, 98, 112, 100, 103, 99
};

/*  default Chroma Qtable */
uint8_t DEFAULT_QTABLE_1[QUANT_SIZE] = {
  17, 18, 24, 47, 99, 99, 99, 99,
  18, 21, 26, 66, 99, 99, 99, 99,
  24, 26, 56, 99, 99, 99, 99, 99,
  47, 66, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99
};

unsigned n1 = 0;
unsigned n2 = 0;

static void jpegenc_callback(jpeg_job_status_t status, 
        uint32_t handle, 
        uint32_t job, 
        mm_jpeg_output_t* out, 
        void* cookie) {
    logi("do jpeg callback.");
    if (cookie != &jpeg) {
        loge("invalid cookie[%p != %p]", cookie, &jpeg);
        return;
    }

    jpeg.status = status;
    jpeg.out_size = out->buf_filled_len;

    n2 = get_ms();
    logd("====in callback %d", n2-n1);

    pthread_mutex_lock(&jpeg.lock);
    pthread_cond_signal(&jpeg.cond);
    pthread_mutex_unlock(&jpeg.lock);
}

int jpegenc_init(int w, int h) {
    memset(&jpeg, 0x00, sizeof(jpeg));
    jpeg.size.w = w;
    jpeg.size.h = h;

    pthread_mutex_init(&jpeg.lock, NULL);
    pthread_cond_init(&jpeg.cond, NULL);

    jpeg.handle = jpeg_open(&jpeg.ops, NULL, jpeg.size, NULL);
    if (0 == jpeg.handle) {
        loge("fail to call jpeg_open");
        return -EINVAL;
    }

    /* init parameters */
    jpeg.out.size = w * h * 3 / 2;
    jpeg.out.addr = (uint8_t*)buffer_allocate(&jpeg.out, 0);
    if (NULL == jpeg.out.addr) {
        jpeg.ops.close(jpeg.handle);
        jpeg.handle = 0;
        loge("fail to allocate buffer");
        return -EINVAL;
    }
    jpeg.in.size = w * h * 3 / 2;
    jpeg.in.addr = (uint8_t*)buffer_allocate(&jpeg.out, 0);
    if (NULL == jpeg.in.addr) {
        jpeg.ops.close(jpeg.handle);
        jpeg.handle = 0;
        loge("fail to allocate buffer");
        return -EINVAL;
    }

    return 0;
}


int jpegenc_encode(void* addr, int in_fd, void** out, int* size) {

    mm_jpeg_encode_params_t* p      = &jpeg.params;
    mm_jpeg_encode_job_t* job       = &jpeg.job.encode_job;
    unsigned long t0 = get_ms();
    unsigned long t1 = 0;
    unsigned long t2 = 0;
    //memcpy(jpeg.in.addr, addr, jpeg.size.w * jpeg.size.h * 3 / 2);
    t1 = get_ms();
    n1 = t1;

    int stride;
    int scanline;

    p->src_main_buf[0].buf_size     = jpeg.size.w * jpeg.size.h * 3 / 2;
    p->src_main_buf[0].buf_vaddr    = (uint8_t*)jpeg.in.addr;
    p->src_main_buf[0].fd           = jpeg.in.p_pmem_fd;
    p->src_main_buf[0].buf_vaddr    = (uint8_t*)addr;
    p->src_main_buf[0].fd           = in_fd;
    p->src_main_buf[0].index        = 0;
    p->src_main_buf[0].format       = MM_JPEG_FMT_YUV;
    p->src_main_buf[0].offset.num_planes = 2;
    stride = VENUS_Y_STRIDE(COLOR_FMT_NV21, jpeg.size.w);
    scanline = VENUS_Y_SCANLINES(COLOR_FMT_NV21, jpeg.size.h);
    p->src_main_buf[0].offset.mp[0].len         = stride * scanline;
    p->src_main_buf[0].offset.mp[0].stride      = stride;
    p->src_main_buf[0].offset.mp[0].scanline    = scanline;
    p->src_main_buf[0].offset.mp[0].width       = jpeg.size.w;
    p->src_main_buf[0].offset.mp[0].height      = jpeg.size.h;
    p->src_main_buf[0].offset.mp[0].offset      = 0;
    p->src_main_buf[0].offset.mp[0].offset_x    = 0;
    p->src_main_buf[0].offset.mp[0].offset_y    = 0;
    stride = VENUS_UV_STRIDE(COLOR_FMT_NV21, jpeg.size.w);
    scanline = VENUS_UV_SCANLINES(COLOR_FMT_NV21, jpeg.size.h);
    p->src_main_buf[0].offset.mp[1].len         = stride * scanline;
    p->src_main_buf[0].offset.mp[1].stride      = stride;
    p->src_main_buf[0].offset.mp[1].scanline    = scanline;
    p->src_main_buf[0].offset.mp[1].width       = jpeg.size.w;
    p->src_main_buf[0].offset.mp[1].height      = jpeg.size.h/2;
    p->src_main_buf[0].offset.mp[1].offset      = 0;
    p->src_main_buf[0].offset.mp[1].offset_x    = 0;
    p->src_main_buf[0].offset.mp[1].offset_y    = 0;

    //p->src_thumb_buf[0].buf_size     = jpeg.size.w * jpeg.size.h * 3 / 2;
    //p->src_thumb_buf[0].buf_vaddr    = (uint8_t*)addr;
    //p->src_thumb_buf[0].fd           = in_fd;
    //p->src_thumb_buf[0].index        = 0;
    //p->src_thumb_buf[0].format       = MM_JPEG_FMT_YUV;
    //p->src_thumb_buf[0].offset.mp[0].len         = jpeg.size.w * jpeg.size.h;
    //p->src_thumb_buf[0].offset.mp[0].stride      = jpeg.size.w;
    //p->src_thumb_buf[0].offset.mp[0].scanline    = jpeg.size.h;
    //p->src_thumb_buf[0].offset.mp[1].len         = jpeg.size.w * jpeg.size.h / 2;
    p->num_src_bufs = 1;
    
    p->dest_buf[0].buf_size = jpeg.out.size;
    p->dest_buf[0].buf_vaddr = jpeg.out.addr;
    p->dest_buf[0].fd = jpeg.out.p_pmem_fd;
    p->dest_buf[0].index = 0;
    p->num_dst_bufs = 1;

    p->jpeg_cb  = jpegenc_callback;
    p->userdata = &jpeg;
    p->color_format = MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;  //FIXME
    p->thumb_color_format = MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;  //FIXME
    p->encode_thumbnail = 0;
    p->quality = 80;     //FIXME;
    p->thumb_quality = 80;     //FIXME;

    job->dst_index = 0;
    job->src_index = 0;
    job->rotation = 0;

    /* main dimension */
    job->main_dim.src_dim.width = jpeg.size.w;
    job->main_dim.src_dim.height = jpeg.size.h;
    job->main_dim.dst_dim.width = jpeg.size.w;
    job->main_dim.dst_dim.height = jpeg.size.h;
    job->main_dim.crop.top = 0;
    job->main_dim.crop.left = 0;
    job->main_dim.crop.width = jpeg.size.w;
    job->main_dim.crop.height = jpeg.size.h;
    p->main_dim  = job->main_dim;

    job->thumb_dim.src_dim.width = jpeg.size.w;
    job->thumb_dim.src_dim.height = jpeg.size.h;
    job->thumb_dim.dst_dim.width = 320;
    job->thumb_dim.dst_dim.height = 240;
    job->thumb_dim.crop.top = 0;
    job->thumb_dim.crop.left = 0;
    job->thumb_dim.crop.width = 0;
    job->thumb_dim.crop.height = 0;
    p->thumb_dim = job->thumb_dim;

    job->exif_info.numOfEntries = 0;
    p->burst_mode = 0;

    /* Qtable */
    job->qtable[0].eQuantizationTable =
        OMX_IMAGE_QuantizationTableLuma;
    job->qtable[1].eQuantizationTable =
        OMX_IMAGE_QuantizationTableChroma;

    for (int i = 0; i < QUANT_SIZE; i++) {
        job->qtable[0].nQuantizationMatrix[i] = DEFAULT_QTABLE_0[i];
        job->qtable[1].nQuantizationMatrix[i] = DEFAULT_QTABLE_1[i];
    }

    job->qtable_set[0] = 1;
    job->qtable_set[1] = 1;

    int ret = jpeg.ops.create_session(jpeg.handle, &jpeg.params, &jpeg.session_id);
    if (0 == jpeg.session_id) {
        loge("failed to create session: %d", ret);
        ret = -EINVAL;
        goto OUT;
    }

    jpeg.job.job_type = JPEG_JOB_TYPE_ENCODE;
    jpeg.job.encode_job.src_index = 0;
    jpeg.job.encode_job.dst_index = 0;
    ret = jpeg.ops.start_job(&jpeg.job, &jpeg.job_id);
    if (ret != 0) {
        loge("fail to start jpeg job");
        ret = -EINVAL;
        goto OUT;
    }

    pthread_mutex_lock(&jpeg.lock);
    pthread_cond_wait(&jpeg.cond, &jpeg.lock);
    pthread_mutex_unlock(&jpeg.lock);
    t2 = get_ms();
    logd("====jpeg: %lu %lu", t2-t1, t1-t0);


    if (jpeg.status == JPEG_JOB_STATUS_ERROR) {
        loge("encode error");
        ret = -EINVAL;
        goto OUT;
    }

    *out = jpeg.out.addr;
    *size = jpeg.out_size;

OUT:
    if (jpeg.ops.destroy_session && 0 != jpeg.session_id) {
        jpeg.ops.destroy_session(jpeg.session_id);
        jpeg.session_id = 0;
    }

    return ret;
}

int jpegenc_alloc(buffer_t* buf) {

    return 0;
}

int jpegenc_deinit() {
    if (jpeg.ops.destroy_session && 0 != jpeg.session_id) {
        jpeg.ops.destroy_session(jpeg.session_id);
        jpeg.session_id = 0;
    }

    if (jpeg.out.addr) {
        buffer_deallocate(&jpeg.out);
    }

    if (jpeg.ops.close && 0 != jpeg.handle) {
        jpeg.ops.close(jpeg.handle);
        jpeg.handle = 0;
    }

    return 0;
}

/********************************** END **********************************************/

