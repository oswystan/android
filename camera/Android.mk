# Copyright 2013 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	camera.cpp

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

LOCAL_MODULE:= camera
LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)

