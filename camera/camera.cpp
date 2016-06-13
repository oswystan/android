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
#include <cutils/memory.h>  
#include <utils/Log.h>
#include <Camera.h>
#include <CameraParameters.h>

#include <utils/Log.h>  
  
#include <binder/ProcessState.h>
#include <gui/Surface.h>  
#include <gui/SurfaceComposerClient.h>  
#include <gui/ISurfaceComposer.h>  
#include <android/native_window.h>  
  
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
    void notify(int32_t msgType, int32_t ext1, int32_t ext2) {
        ALOGI("get msg %d, %d, %d", msgType, ext1, ext2);
    }
    void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata) {
        ALOGI("post data msg %d metadata = %p", msgType, metadata);
        if (msgType != CAMERA_MSG_COMPRESSED_IMAGE) {
            return;
        }
        ssize_t offset;
        size_t size;
        sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
        ALOGV("copyAndPost: off=%zd, size=%zu", offset, size);
        unsigned char *heapBase = (unsigned char*)heap->base();
        FILE* fp = fopen("/data/tmp.jpg", "wb");
        if (fp) {
            fwrite(heapBase+offset, size, 1, fp);
            fclose(fp);
            fp = NULL;
        }

    }
    void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr) {
        ALOGI("post data timestamp msg %d", msgType);
    }
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
    ALOGI("======================================");
    ALOGI("id               = %d", c->id);
    ALOGI("frameRate        = %d", c->frameRate);
    ALOGI("previewWidth     = %d", c->previewWidth);
    ALOGI("previewHeight    = %d", c->previewHeight);
    ALOGI("picWidth         = %d", c->picWidth);
    ALOGI("picHeight        = %d", c->picHeight);
    ALOGI("winWidth         = %d", c->winWidth);
    ALOGI("winHeight        = %d", c->winHeight);
    ALOGI("======================================");
}

int connect(camera_context* c) {
    c->cam = Camera::connect(c->id, String16("camera"), Camera::USE_CALLING_UID);
    if(c->cam == NULL) {
        ALOGE("fail to connect camera %d", c->id);
        return -1;
    }
    if(c->cam->getStatus() != NO_ERROR) {
        ALOGE("invalid status of connected camera %d"), c->id;
        return -1;
    }
    ALOGI("####### camera %d connected #######", c->id);
    return 0;
}

void release(camera_context* c) {
    if(NULL == c || c->cam.get() == NULL) {
        ALOGW("invalid parameters");
        return;
    }
    c->cam->disconnect();
    c->cam.clear();
    c->id = 0;
    ALOGI("------- camera %d released -------", c->id);
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
    ALOGI("buffer=%p stride=%d height=%d bytesPerPixe;=%d", buffer, outBuffer.stride, outBuffer.height, bytesPerPixel(outBuffer.format));
    memset(buffer, 0xAF, outBuffer.stride * bytesPerPixel(outBuffer.format) * outBuffer.height);
    c->surface->unlockAndPost();
#endif

    ALOGI("create surface successfully");
    return 0;
}

int runCamera(camera_context* c) {

    int ret = createSurface(c);
    if (ret != 0) {
        ALOGE("fail to create surface");
        return -1;
    }

    ret = connect(c);
    if (ret != 0) {
        ALOGE("fail to connect camera device");
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
        ALOGE("fail to set parameters ret = %d", ret);
        return ret;
    }

    ret = cam->setPreviewTarget(c->surface->getIGraphicBufferProducer());
    if(ret != 0) {
        ALOGE("fail to set preview target ret = %d", ret);
        return ret;
    }

    ret = cam->startPreview();
    if(ret != 0) {
        ALOGE("fail to start preview ret = %d", ret);
        return ret;
    }
    sleep(2);
    int msg = CAMERA_MSG_SHUTTER | CAMERA_MSG_COMPRESSED_IMAGE;
    ALOGI("msg = %08X", msg);
    ret = cam->takePicture(msg);
    if(ret != 0) {
        ALOGE("fail to take picture ret = %d", ret);
    }

    sleep(2);
    cam->stopPreview();
    return 0;
}

int main(int argc, const char *argv[]) {

    camera_context* ctx = createContext(0, 1440, 1080, 4160, 3120, 30);
    if(NULL == ctx) {
        ALOGE("fail to create context");
        return -1;
    }
    dumpContext(ctx);

    runCamera(ctx);

    release(ctx);
    return 0;
}


/********************************** END **********************************************/
