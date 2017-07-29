#######################################################################
##                     Copyright (C) 2017 wystan
##
##       filename: Android
##    description:
##        created: 2017-07-29 13:18:28
##         author: wystan
##
#######################################################################
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= main.cpp

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libbinder \
	libmedia

LOCAL_C_INCLUDES += \
    frameworks/av/include/

LOCAL_CFLAGS += -Wall -Wextra
LOCAL_MODULE:= audio_rec
include $(BUILD_EXECUTABLE)

#######################################################################
