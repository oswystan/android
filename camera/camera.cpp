/*
 **************************************************************************************
 *       Filename:  camera.cpp
 *    Description:   source file
 *
 *        Version:  1.0
 *        Created:  2016-06-08 11:20:49
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */

#define LOG_TAG "CAMERA_TEST"
#include <stdio.h>
#include <cutils/memory.h>  
#include <Camera.h>
#include <CameraParameters.h>

#include <utils/Condition.h>
  
#include <binder/ProcessState.h>
#include <gui/Surface.h>  
#include <gui/SurfaceComposerClient.h>  
#include <gui/ISurfaceComposer.h>  
#include <android/native_window.h>  
#include "camera_st.h"

#undef logd
#undef logi
#undef logw
#undef loge
#define logd(fmt, ...) printf("[D/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)
#define logi(fmt, ...) printf("[I/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)
#define logw(fmt, ...) printf("[W/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)
#define loge(fmt, ...) printf("[E/" LOG_TAG "]" fmt "\n", ##__VA_ARGS__)
  
using namespace android;

struct camera_context {
    sp<SurfaceComposerClient> composer;
    sp<SurfaceControl> surface_ctrl;
    sp<Surface> surface;

    int previewWidth;
    int previewHeight;
    int picWidth;
    int picHeight;
    int winWidth;
    int winHeight;
    int frameRate;

    sp<Camera> cam;
    int id;
};

class CamListener : public CameraListener {

public:
    void notify(int32_t msgType, int32_t ext1, int32_t ext2) {
        logi("get msg %d, %d, %d", msgType, ext1, ext2);
        if(msgType == CAMERA_MSG_SHUTTER) {
            logd("signal shutter");
            Mutex::Autolock l(mutexShutter);
            condShutter.signal();
        } else if(msgType == CAMERA_MSG_ERROR) {
            logd("signal error");
            Mutex::Autolock l(mutexShutter);
            condShutter.signal();
            condJpeg.signal();
        }
    }
    void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata) {
        if (msgType != CAMERA_MSG_COMPRESSED_IMAGE) {
            return;
        }
        ssize_t offset;
        size_t size;
        sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
        logd("got jpeg data: off=%zd, size=%zu", offset, size);
        unsigned char *heapBase = (unsigned char*)heap->base();
        FILE* fp = fopen("/data/tmp.jpg", "wb");
        if (fp) {
            fwrite(heapBase+offset, size, 1, fp);
            fclose(fp);
            fp = NULL;
        }

        {
            Mutex::Autolock l(mutexJpeg);
            condJpeg.signal();
        }
    }
    void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr) {
        logi("post data timestamp msg %d", msgType);
    }

    int waitForShutter() {
        return condShutter.waitRelative(mutexShutter, s2ns(1));
    }

    int waitForJpeg() {
        return condJpeg.waitRelative(mutexJpeg, s2ns(1));
    }

private:
    Mutex     mutexShutter;
    Mutex     mutexJpeg;
    Condition condShutter;
    Condition condJpeg;
};


camera_context* createContext(int id, int preWidth, int preHeight, int picWidth, int picHeight, int fr) {
    camera_context* ret = new camera_context();
    ret->cam           = NULL;
    ret->id            = id;
    ret->frameRate     = fr;
    ret->previewWidth  = preWidth;
    ret->previewHeight = preHeight;
    ret->picWidth      = picWidth;
    ret->picHeight     = picHeight;

    ret->winWidth      = 720;
    ret->winHeight     = 540;
    return ret;
}
void dumpContext(camera_context* c) {
    logd("======================================");
    logd("id               = %d", c->id);
    logd("frameRate        = %d", c->frameRate);
    logd("previewWidth     = %d", c->previewWidth);
    logd("previewHeight    = %d", c->previewHeight);
    logd("picWidth         = %d", c->picWidth);
    logd("picHeight        = %d", c->picHeight);
    logd("winWidth         = %d", c->winWidth);
    logd("winHeight        = %d", c->winHeight);
    logd("======================================");
}

int connect(camera_context* c) {
    c->cam = Camera::connect(c->id, String16("camera"), Camera::USE_CALLING_UID);
    if(c->cam == NULL) {
        loge("fail to connect camera %d", c->id);
        return -1;
    }
    if(c->cam->getStatus() != NO_ERROR) {
        loge("invalid status of connected camera %d", c->id);
        return -1;
    }
    logi("camera %d connected ", c->id);
    return 0;
}

void release(camera_context* c) {
    if(NULL == c || c->cam.get() == NULL) {
        logw("invalid parameters");
        return;
    }
    c->cam->disconnect();
    c->cam.clear();
    //c->id = 0;
    c->composer->dispose();
    c->surface.clear();
    c->surface_ctrl.clear();
    c->composer.clear();
    logi("camera %d released", c->id);
}

int createSurface(camera_context* c) {
    sp<ProcessState> proc(ProcessState::self());  
    ProcessState::self()->startThreadPool();  
  
    c->composer = new SurfaceComposerClient;
    c->surface_ctrl = c->composer->createSurface(String8("camera-test"), c->winWidth, c->winHeight, PIXEL_FORMAT_RGB_565, 0);
    SurfaceComposerClient::openGlobalTransaction();
    c->surface_ctrl->setLayer(100000);
    c->surface_ctrl->setPosition(0, 0);
    c->surface_ctrl->show();
    SurfaceComposerClient::closeGlobalTransaction();
    c->surface = c->surface_ctrl->getSurface();

#if 0
    ANativeWindow_Buffer outBuffer;
    ARect inOutDirtyBounds;
    c->surface->lock(&outBuffer, &inOutDirtyBounds);
    char* buffer = reinterpret_cast<char*>(outBuffer.bits);
    logi("buffer=%p stride=%d height=%d bytesPerPixe;=%d", buffer, outBuffer.stride, outBuffer.height, bytesPerPixel(outBuffer.format));
    memset(buffer, 0xAF, outBuffer.stride * bytesPerPixel(outBuffer.format) * outBuffer.height);
    c->surface->unlockAndPost();
#endif

    logi("create surface successfully");
    return 0;
}

int runCamera(camera_context* c) {

    int ret = createSurface(c);
    if (ret != 0) {
        loge("fail to create surface");
        return -1;
    }

    ret = connect(c);
    if (ret != 0) {
        loge("fail to connect camera device");
        return -1;
    }

    sp<Camera> cam = c->cam;    
    sp<CamListener> listener = new CamListener();
    cam->setListener(listener);

    CameraParameters camPara;
    camPara.unflatten(cam->getParameters());
    camPara.setPreviewSize(c->previewWidth, c->previewHeight);
    camPara.setPictureSize(c->picWidth, c->picHeight);
    camPara.setPreviewFrameRate(c->frameRate);
    ret = cam->setParameters(camPara.flatten());
    if(ret != 0) {
        loge("fail to set parameters ret = %d", ret);
        return ret;
    }

    ret = cam->setPreviewTarget(c->surface->getIGraphicBufferProducer());
    if(ret != 0) {
        loge("fail to set preview target ret = %d", ret);
        return ret;
    }

    ret = cam->startPreview();
    if(ret != 0) {
        loge("fail to start preview ret = %d", ret);
        return ret;
    }
    sleep(2);
    int msg = CAMERA_MSG_SHUTTER | CAMERA_MSG_COMPRESSED_IMAGE;
    ret = cam->takePicture(msg);
    if(ret != 0) {
        loge("fail to take picture ret = %d", ret);
    }

    ret = listener->waitForShutter();
    if (ret != 0) {
        loge("fail to wait for shutter %d", ret);
        goto EXIT;
    }
    ret = listener->waitForJpeg();
    if (ret != 0) {
        loge("fail to wait for shutter %d", ret);
        goto EXIT;
    }
    logi("camera %d preview and jpeg test OK", c->id);

EXIT:
    cam->stopPreview();
    release(c);

    return ret;
}


int main(int argc, const char *argv[]) {

    std::string str = "hello, world";
    logi("str=%s", str.c_str());
    return 0;
    camera_context* ctx0 = createContext(0, 1440, 1080, 4160, 3120, 30);
    camera_context* ctx1 = createContext(1, 1440, 1080, 4160, 3120, 30);
    camera_context* ctx2 = createContext(2, 1440, 1080, 3264, 2448, 30);
    if(NULL == ctx0 || NULL == ctx2) {
        loge("fail to create context");
        return -1;
    }

    camera_context* ctx[3];
    ctx[0] = ctx0;
    ctx[1] = ctx1;
    ctx[2] = ctx2;
    
    for (unsigned int i = 0; i < sizeof(ctx)/sizeof(ctx[0]); i++) {
        logi("run camera id %d\n", ctx[i]->id);
        dumpContext(ctx[i]);
        runCamera(ctx[i]);
    }

    return 0;
}

/********************************** END **********************************************/
