#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#
# =============================================================================
# microtrust specific includes for the TAC
# Note that LOCAL_PATH is set to the location of the normal/Android.mk that
# includes this file
# =============================================================================
LOCAL_PATH_PLATFORM := ./platform/isee
LOCAL_PATH_PLATFORM_IF := ../include/platform/isee

# =============================================================================
LOCAL_C_INCLUDES += \
		$(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM)/inc \
		$(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM_IF) \
                $(TOP)/vendor/mediatek/proprietary/trustzone/trustkernel/source/client/public

LOCAL_SRC_FILES += \
		$(LOCAL_PATH_PLATFORM)/src/fpc_ut_tac.c

ifdef FPC_CONFIG_HW_AUTH
LOCAL_SRC_FILES += $(LOCAL_PATH_PLATFORM)/src/hw_auth/fpc_tee_hw_auth_ut.c
endif


LOCAL_CFLAGS += -DFPC_REE_DEVICE_ALIAS_FILE='"modalias"'
LOCAL_CFLAGS += -DFPC_REE_DEVICE_NAME='"fpc_finger"'
LOCAL_CFLAGS += -DFPC_REE_DEVICE_PATH='"/sys/bus/platform/devices"'
    
