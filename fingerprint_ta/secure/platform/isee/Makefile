#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#

# At customer platform, we can use this one.
#ANDROID_DIR=../../../../../..

ANDROID_DIR=$(ANDROID_SYSTEM_ROOT)
TEEI_DIR=$(ANDROID_DIR)/vendor/mediatek/proprietary/trustzone/microtrust/source/platform/mt6797/teei
build:
	@make -f Makefile-real.mk
	$(shell if [ -f $(TEEI_DIR)/fp_server ]; then echo "cp -rf obj/fp_server $(TEEI_DIR)"; fi;)
	@echo
	@echo "\033[32mbuild finished ... \033[0m"

clean:
	@make -f Makefile-real.mk clean
	@-rm -rf isee
	@-rm -rf obj
	@-rm -rf ../src
	@echo
	@echo "\033[32mclean finished ... \033[0m"



install:
	-adb wait-for-device root
	-adb wait-for-device remount
	-adb push obj/fp_server /system/thh/fp_server
	-adb push obj/fp_server /data/thh/tee_05/tee
	@echo
	@echo "\033[32minstall finished ... \033[0m"


all:clean build install
