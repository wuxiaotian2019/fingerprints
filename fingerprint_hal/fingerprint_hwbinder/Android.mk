#
# Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# TODO: Update LOCAL_MODULE to match the correct project setup
LOCAL_MODULE := fingerprint.fpc.default
LOCAL_MULTILIB := 64
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/vendor/lib64/
LOCAL_MULTILIB := first
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := fpc
LOCAL_CFLAGS := -std=c99 -Werror\
    -DLOG_TAG='"fpc_fingerprint_hal"' \
    -DFPC_TZ_APP_NAME="\"fpctzappfingerprint\""

LOCAL_PROPRIETARY_MODULE := true
LOCAL_HEADER_LIBRARIES += libhardware_headers

LOCAL_SRC_FILES := fpc_legacy_hal.c

ifeq ($(FPC_CONFIG_DEBUG),1)
LOCAL_CFLAGS += -DFPC_DEBUG_LOGGING
endif

ifeq ($(FPC_CONFIG_HW_AUTH),1)
LOCAL_CFLAGS    += -DFPC_CONFIG_HW_AUTH
endif


LOCAL_C_INCLUDES += $(LOCAL_PATH)/ \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/ \
                    $(LOCAL_PATH)/../../fingerprint_tac/interface/

LOCAL_SHARED_LIBRARIES := liblog libutils  libcutils libhidlbase libhidltransport \
				libutils android.hardware.biometrics.fingerprint@2.1 \
				com.fingerprints.extension@1.0


LOCAL_STATIC_LIBRARIES += fpc_hal_common fpc_tac  fpc_hal_extension

ifeq ($(FPC_TEE_RUNTIME), TBASE)
LOCAL_SHARED_LIBRARIES += libMcClient
else ifeq ($(FPC_TEE_RUNTIME), QSEE)
LOCAL_SHARED_LIBRARIES += libQSEEComAPI libion
else ifeq ($(FPC_TEE_RUNTIME), ANDROID)
LOCAL_SHARED_LIBRARIES += lib_fpc_ta_shared
else ifeq ($(FPC_TEE_RUNTIME), ISEE)
LOCAL_SHARED_LIBRARIES += libteec
endif

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
