################################################
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= camera_st.cpp camera_cmds.cpp

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	libcamera_metadata \
	libcamera_client \
	libgui \
	libui \
	libbinder

LOCAL_C_INCLUDES += \
	frameworks/av/include/camera 

LOCAL_C_INCLUDES += \
    external/skia/include/core  

LOCAL_CFLAGS += -Wall -Wextra
LOCAL_MODULE:= camera_st
LOCAL_MODULE_TAGS := tests
include $(BUILD_EXECUTABLE)

################################################
