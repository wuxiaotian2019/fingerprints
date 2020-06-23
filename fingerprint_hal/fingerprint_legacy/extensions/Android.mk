#
# Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := fpc_hal_extension_legacy

LOCAL_CFLAGS := -Wall -Werror \
    -DLOG_TAG='"fpc_hal_extension"' \

LOCAL_SRC_FILES  += authenticator/FingerprintAuthenticator.cpp \
                    authenticator/IFingerprintAuthenticator.cpp \
                    calibration/FingerprintCalibration.cpp \
                    calibration/IFingerprintCalibration.cpp \
                    engineering/FingerprintEngineering.cpp \
                    engineering/IFingerprintEngineering.cpp \
                    navigation/FingerprintNavigation.cpp \
                    navigation/IFingerprintNavigation.cpp \
                    recalibration/FingerprintRecalibration.cpp \
                    recalibration/IFingerprintRecalibration.cpp \
                    sensetouch/FingerprintSenseTouch.cpp \
                    sensetouch/IFingerprintSenseTouch.cpp \
                    sensortest/FingerprintSensorTest.cpp \
                    sensortest/IFingerprintSensorTest.cpp

COMMON_INCLUDE_PATH := $(LOCAL_PATH)/../../fingerprint_common

LOCAL_C_INCLUDES += $(COMMON_INCLUDE_PATH)/ \
                    $(COMMON_INCLUDE_PATH)/nav \
                    $(COMMON_INCLUDE_PATH)/pn \
                    $(COMMON_INCLUDE_PATH)/sensetouch \
                    $(COMMON_INCLUDE_PATH)/../include \
                    system/core/base/include

include $(BUILD_STATIC_LIBRARY)
