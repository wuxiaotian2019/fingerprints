#
# File included from device/<manufacture>/<>/<device>.mk
# Packages to include into the build
PRODUCT_PACKAGES += \
    android.hardware.biometrics.fingerprint@2.1-service \
    android.hardware.biometrics.fingerprint@2.1-service.rc \
    com.fingerprints.extension@1.0 \


# -----------------------------
# Add feature flags below
# ----------------------------
FPC_CONFIG_AFD_STAY_IN_SLEEP=1
FPC_CONFIG_FINGER_LOST_INTERRUPT=1
FPC_CONFIG_HW_AUTH=1
FPC_CONFIG_IDENTIFY_AT_ENROL=1
FPC_CONFIG_PN_CALIBRATION_DEBUG_PATH=/data/fpc/calibration_image_debug.pndat
FPC_CONFIG_PN_CALIBRATION_PATH=/data/fpc/calibration_image.pndat
FPC_CONFIG_SENSE_TOUCH_CALIBRATION_PATH=/data/fpc/calibration_sense_touch.dat
#FPC_CONFIG_SENSORTEST=1
FPC_CONFIG_TA_FS=1
FPC_CONFIG_NAVIGATION=1
FPC_TEE_RUNTIME=ISEE
LIBFPC_NAME=libfpc1028_debug.a
FPC_CONFIG_DEBUG=1
#FPC_CONFIG_ENGINEERING=1
SPI_DEVICE="/dev/spidev32765.0"
