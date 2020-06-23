#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := fpc

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := FingerprintExtensionService

ifneq ($(filter 7.%,$(PLATFORM_VERSION)),)
FPC_SYSTEM_DIR := true
endif

ifneq ($(filter 6.%,$(PLATFORM_VERSION)),)
FPC_SYSTEM_DIR := true
endif

ifeq ($(FPC_SYSTEM_DIR),)
# In Android O we should place the library in /vendor/app insteadof /system/app/
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES := \
    com.fingerprints.extension

LOCAL_DEX_PREOPT := false

#LOCAL_PRIVATE_PLATFORM_APIS := true

include $(BUILD_PACKAGE)
