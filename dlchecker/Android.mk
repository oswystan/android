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

LOCAL_SRC_FILES:= main.c

LOCAL_SHARED_LIBRARIES := libdl

LOCAL_CFLAGS += -Wall -Wextra

LOCAL_MODULE:= dlchecker
include $(BUILD_EXECUTABLE)

#######################################################################
