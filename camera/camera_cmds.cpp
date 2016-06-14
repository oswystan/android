/*
 **************************************************************************************
 *       Filename:  camera_cmds.cpp
 *    Description:   source file
 *
 *        Version:  1.0
 *        Created:  2016-06-14 14:03:08
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

using namespace android;

#define BUILD_CMD(name) { #name, cmd_##name }
#define CHECK_CTX(c) \
    do {\
        if ( NULL == c ) {\
            loge("should connect camera first");\
            return -1;\
        }\
    }while(0)

#define CHECK_ARG(arg, cnt) \
    do {\
        if ( arg.size() != cnt ) {\
            loge("invalid argument size %ld", arg.size());\
            return -1;\
        }\
    }while(0)


class CamListener : public CameraListener {

public:
    void notify(int32_t msgType, int32_t ext1, int32_t ext2) {
        //logi("get msg %d, %d, %d", msgType, ext1, ext2);
        if(msgType == CAMERA_MSG_SHUTTER) {
            //logd("signal shutter");
            Mutex::Autolock l(mutexShutter);
            condShutter.signal();
        } else if(msgType == CAMERA_MSG_FOCUS) {
            logd("get auto focus %d %d", ext1, ext2);
            Mutex::Autolock l(mutexAF);
            condAF.signal();
        } else if(msgType == CAMERA_MSG_ERROR) {
            //logd("signal error");
            Mutex::Autolock l(mutexShutter);
            condShutter.signal();
            condAF.signal();
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
        //logd("got jpeg data: off=%zd, size=%zu", offset, size);
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

    int waitForAF() {
        return condAF.waitRelative(mutexAF, s2ns(2));
    }
    int waitForJpeg() {
        return condJpeg.waitRelative(mutexJpeg, s2ns(1));
    }

private:
    Mutex     mutexShutter;
    Mutex     mutexAF;
    Mutex     mutexJpeg;
    Condition condShutter;
    Condition condAF;
    Condition condJpeg;
};

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
    sp<CamListener> listener;
    int id;
};
int cmd_connect(stc_t* stc, std::vector<std::string>& arg) {
    CHECK_ARG(arg, 1);

    camera_context* c = new camera_context;
    c->winWidth = 720;
    c->winHeight = 640;
    c->id = atoi(arg[0].c_str());

    sp<ProcessState> proc(ProcessState::self());  
    ProcessState::self()->startThreadPool();  
  
    c->composer = new SurfaceComposerClient;
    c->surface_ctrl = c->composer->createSurface(String8("camera-test"), c->winWidth, c->winHeight, PIXEL_FORMAT_RGB_565, 0);
    SurfaceComposerClient::openGlobalTransaction();
    c->surface_ctrl->setLayer(1000000);
    c->surface_ctrl->setPosition(0, 0);
    c->surface_ctrl->show();
    SurfaceComposerClient::closeGlobalTransaction();
    c->surface = c->surface_ctrl->getSurface();

    c->cam = Camera::connect(c->id, String16("camera"), Camera::USE_CALLING_UID);
    if(c->cam == NULL) {
        loge("fail to connect camera %d", c->id);
        return -1;
    }
    if(c->cam->getStatus() != NO_ERROR) {
        loge("invalid status of connected camera %d", c->id);
        return -1;
    }
    stc->priv = c;
    return 0;
}
int cmd_config(stc_t* stc, std::vector<std::string>& arg) {
    camera_context* c = (camera_context*)stc->priv;
    CHECK_CTX(c);
    CHECK_ARG(arg, 3);

    int ret = 0;

    //TODO parse arg to get this information
    for (unsigned int i = 0; i < arg.size(); i++) {
        char tag[64];
        char val[64];
        memset(tag, 0x00, sizeof(tag));
        memset(val, 0x00, sizeof(val));
        sscanf(arg[i].c_str(), "%[a-z]=%[a-z|0-9]", tag, val);
        if (strcmp(tag, "preview") == 0) {
            sscanf(val, "%dx%d", &c->previewWidth, &c->previewHeight);
        } else if (strcmp(tag, "picture") == 0) {
            sscanf(val, "%dx%d", &c->picWidth, &c->picHeight);
        } else if (strcmp(tag, "fr") == 0) {
            c->frameRate = atoi(val);
        }
    }

    //c->frameRate     = 30;
    //c->previewWidth  = 1440;
    //c->previewHeight = 1080;
    //c->picWidth      = 4160;
    //c->picHeight     = 3120;

    sp<Camera> cam = c->cam;    
    sp<CamListener> listener = new CamListener();
    cam->setListener(listener);
    c->listener = listener;

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

    return 0;
}
int cmd_preview(stc_t* stc, std::vector<std::string>& arg) {
    camera_context* c = (camera_context*)stc->priv;
    CHECK_CTX(c);

    sp<Camera> cam = c->cam;
    int ret = cam->setPreviewTarget(c->surface->getIGraphicBufferProducer());
    if(ret != 0) {
        loge("fail to set preview target ret = %d", ret);
        return ret;
    }

    ret = cam->startPreview();
    if(ret != 0) {
        loge("fail to start preview ret = %d", ret);
        return ret;
    }

    return 0;
}
int cmd_delay(stc_t* stc, std::vector<std::string>& arg) {
    CHECK_ARG(arg, 1);
    int ms = atoi(arg[0].c_str());
    usleep(ms*1000);
    return 0;
}
int cmd_autofocus(stc_t* stc, std::vector<std::string>& arg) {
    camera_context* c = (camera_context*)stc->priv;
    CHECK_CTX(c);

    sp<Camera> cam = c->cam;
    sp<CamListener> listener = c->listener.get();
    int ret = cam->autoFocus();
    if (ret != 0) {
        loge("fail to do autofocus %d", ret);
        return -1;
    }
    
    ret = listener->waitForAF();
    if (ret != 0) {
        loge("fail to wait for autofocus %d", ret);
        return -1;
    }
    return 0;
}
int cmd_zoom(stc_t* stc, std::vector<std::string>& arg) {
    camera_context* c = (camera_context*)stc->priv;
    CHECK_CTX(c);
    return -1;
}
int cmd_capture(stc_t* stc, std::vector<std::string>& arg) {
    camera_context* c = (camera_context*)stc->priv;
    CHECK_CTX(c);

    int msg = CAMERA_MSG_SHUTTER | CAMERA_MSG_COMPRESSED_IMAGE;
    sp<Camera> cam = c->cam;
    sp<CamListener> listener = c->listener.get();
    int ret = cam->takePicture(msg);
    if(ret != 0) {
        loge("fail to take picture ret = %d", ret);
        goto EXIT;
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

EXIT:
    return ret;
}
int cmd_release(stc_t* stc, std::vector<std::string>& arg) {
    camera_context* c = (camera_context*)stc->priv;
    CHECK_CTX(c);

    c->id = 0;
    c->cam->disconnect();
    c->cam.clear();
    c->composer->dispose();
    c->surface.clear();
    c->surface_ctrl.clear();
    c->composer.clear();
    stc->priv = NULL;

    return 0;
}
int cmd_start_recording(stc_t* stc, std::vector<std::string>& arg) {
    camera_context* c = (camera_context*)stc->priv;
    CHECK_CTX(c);
    return -1;
}
int cmd_stop_recording(stc_t* stc, std::vector<std::string>& arg) {
    camera_context* c = (camera_context*)stc->priv;
    CHECK_CTX(c);
    return -1;
}

cmd_handler_t g_handlers[] = {
    BUILD_CMD(connect),
    BUILD_CMD(config),
    BUILD_CMD(preview),
    BUILD_CMD(delay),
    BUILD_CMD(autofocus),
    BUILD_CMD(zoom),
    BUILD_CMD(capture),
    BUILD_CMD(release),
    BUILD_CMD(start_recording),
    BUILD_CMD(stop_recording),
    { NULL, NULL}
};


/********************************** END **********************************************/

