################################################
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= camera_st.cpp camera_cmds.cpp jpeg_enc.cpp

LOCAL_CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter
LOCAL_CFLAGS += -D_ANDROID_
LOCAL_CFLAGS += -DSYSTEM_HEADER_PREFIX=sys
LOCAL_CFLAGS += -DUSE_ION

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	libcamera_metadata \
	libcamera_client \
	libgui \
	libui \
	libbinder

LOCAL_C_INCLUDES += \
	frameworks/av/include/camera \
	frameworks/native/include/media/hardware \
	frameworks/native/include/media/openmax \
	hardware/qcom/media/libstagefrighthw

LOCAL_C_INCLUDES += \
    external/skia/include/core  


########### QCOM JPEG ENCODER ##############

OMX_HEADER_DIR := frameworks/native/include/media/openmax
OMX_CORE_DIR := hardware/qcom/camera/mm-image-codec

LOCAL_C_INCLUDES += $(OMX_HEADER_DIR)
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qexif
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qomx_core
LOCAL_C_INCLUDES += hardware/qcom/camera/QCamera2/stack/mm-jpeg-interface/inc \
					hardware/qcom/camera/QCamera2/stack/common \
					out/target/product/msm8996/obj/KERNEL_OBJ/usr/include

LOCAL_SHARED_LIBRARIES += libmmjpeg_interface

############################################


LOCAL_CFLAGS += -Wall -Wextra
LOCAL_MODULE:= camera_st
LOCAL_MODULE_TAGS := tests
LOCAL_32_BIT_ONLY := true
include $(BUILD_EXECUTABLE)

################################################
