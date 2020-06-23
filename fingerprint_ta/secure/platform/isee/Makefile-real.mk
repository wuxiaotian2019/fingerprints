#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#

FINGERPRINT_TA_TOP = ../..
FINGERPRINT_TA_PLATFORM_TOP = ..
TARGET = fp_server/
include default_config.mk


INCLUDE_DIRS += \
	$(FINGERPRINT_TA_TOP)/inc \
	$(FINGERPRINT_TA_TOP)/inc/fs \
	$(FINGERPRINT_TA_TOP)/../interface \
	$(FINGERPRINT_TA_TOP)/../../common/include \
	$(FINGERPRINT_TA_TOP)/lib \
	$(FINGERPRINT_TA_PLATFORM_TOP)/isee/inc

SRC_C = $(FINGERPRINT_TA_TOP)/src/fpc_ta_router.c \
	$(FINGERPRINT_TA_TOP)/src/fpc_crypto.c \
	$(FINGERPRINT_TA_TOP)/src/fpc_ta_bio.c \
	$(FINGERPRINT_TA_TOP)/src/fpc_ta_common.c \
	$(FINGERPRINT_TA_TOP)/src/fpc_ta_sensor.c \
	$(FINGERPRINT_TA_TOP)/src/fpc_db.c \
	$(FINGERPRINT_TA_TOP)/src/kpi/fpc_ta_kpi.c \
	$(FINGERPRINT_TA_PLATFORM_TOP)/isee/src/utils.c \
	$(FINGERPRINT_TA_PLATFORM_TOP)/isee/src/fpc_spi.c \
	$(FINGERPRINT_TA_PLATFORM_TOP)/isee/src/fpc_mem.c \
	$(FINGERPRINT_TA_PLATFORM_TOP)/isee/src/fpc_ta_main.c \
	$(FINGERPRINT_TA_PLATFORM_TOP)/isee/src/fpc_ut_crypto.c



ifdef FPC_CONFIG_ENGINEERING
LOCAL_CFLAGS     += -DFPC_CONFIG_ENGINEERING
SRC_C  += $(FINGERPRINT_TA_TOP)/src/engineering/fpc_ta_engineering.c
INCLUDE_DIRS += $(FINGERPRINT_TA_TOP)/inc/engineering
endif

ifdef FPC_CONFIG_SENSORTEST
LOCAL_CFLAGS     += -DFPC_CONFIG_SENSORTEST
SRC_C  += $(FINGERPRINT_TA_TOP)/src/sensortest/fpc_ta_sensortest.c
INCLUDE_DIRS += $(FINGERPRINT_TA_TOP)/inc/sensortest
endif

ifdef FPC_CONFIG_HW_AUTH
LOCAL_CFLAGS    += -DFPC_CONFIG_HW_AUTH
SRC_C += $(FINGERPRINT_TA_TOP)/src/hw_auth/fpc_ta_hw_auth.c
SRC_C += $(FINGERPRINT_TA_PLATFORM_TOP)/isee/src/hw_auth/fpc_ta_hw_auth_ut.c
INCLUDE_DIRS += $(FINGERPRINT_TA_TOP)/inc/hw_auth
endif

ifdef FPC_CONFIG_TA_DB_BLOB
LOCAL_CFLAGS    += -DFPC_CONFIG_TA_DB_BLOB
SRC_C += $(FINGERPRINT_TA_TOP)/src/db_blob/fpc_ta_db_blob.c
endif

ifdef FPC_CONFIG_TA_FS
LOCAL_CFLAGS += -DFPC_CONFIG_TA_FS
SRC_C += $(FINGERPRINT_TA_TOP)/src/fs/fpc_ta_fs.c
SRC_C += $(FINGERPRINT_TA_PLATFORM_TOP)/isee/src/fpc_ta_fs_ut.c
endif

ifeq ($(FPC_CONFIG_LIVENESS_DETECTION),on)
LOCAL_CFLAGS += -DFPC_CONFIG_LIVENESS_DETECTION_ENABLED
endif

ifeq ($(FPC_CONFIG_LIVENESS_DETECTION),app_only)
LOCAL_CFLAGS += -DFPC_CONFIG_LIVENESS_DETECTION_APP_ONLY
endif

ifdef FPC_CONFIG_LOGGING_IN_RELEASE
LOCAL_CFLAGS     += -DFPC_CONFIG_LOGGING_IN_RELEASE
endif

ifdef FPC_CONFIG_NAVIGATION
LOCAL_CFLAGS += -DFPC_CONFIG_NAVIGATION
SRC_C        += $(FINGERPRINT_TA_TOP)/src/navigation/fpc_ta_navigation.c
INCLUDE_DIRS += $(FINGERPRINT_TA_TOP)/inc/navigation
endif

ifdef FPC_CONFIG_PRIV_HEAP_DEBUG
LOCAL_CFLAGS += -DFPC_CONFIG_PRIV_HEAP_DEBUG
SRC_C        += $(FINGERPRINT_TA_TOP)/src/debug/fpc_debug.c
INCLUDE_DIRS += $(FINGERPRINT_TA_TOP)/inc/debug
endif

ifdef FPC_CONFIG_IDENTIFY_AT_ENROL
LOCAL_CFLAGS     += -DFPC_CONFIG_IDENTIFY_AT_ENROL
endif

ifdef FPC_CONFIG_DEBUG
LOCAL_CFLAGS += -DFPC_CONFIG_DEBUG
endif

TOOL_CHAINS_VENDOR = LINARO

MY_CFLAGS += \
	-DCURRENT_TIME="\"`date '+%Y-%m-%d %H:%M:%S'`"\" \
	-DLOG_LV_INFO -DFPC_CONFIG_TEE_ISEE -DFPC_TA_OEM_TEMPLATE_CRYPTO -DLOG_TAG='"fpc_ta"' -Wall -Wextra \
	-mfloat-abi=softfp \
	$(LOCAL_CFLAGS) \
	$(addprefix -I , $(INCLUDE_DIRS)) \
	$(CFLAGS_SYS_BASE) \
	$(CFLAGS_PF_CRYPTO) \
	$(CFLAGS_PF_FP) \
	$(CFLAGS_PF_KM) \
	$(CFLAGS_PF_SPI) \
	$(CFLAGS_PF_TIME) \
	$(CFLAGS_PF_TS) \
	$(CFLAGS_APP_BTA) \


MY_LDFLAGS += \
	-L$(FINGERPRINT_TA_TOP)/lib -l$(subst lib,,$(subst .a,,$(LIBFPC_NAME))) \
	$(LDFLAGS_PF_CRYPTO) \
	$(LDFLAGS_PF_FP) \
	$(LDFLAGS_PF_KM) \
	$(LDFLAGS_PF_SPI) \
	$(LDFLAGS_PF_TIME) \
	$(LDFLAGS_PF_TS) \
	$(LDFLAGS_APP_BTA) \

include $(UT_SDK_HOME)/build/prog.mk
