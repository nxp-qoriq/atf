#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

SD_DRIVERS_PATH		:=  ${PLAT_DRIVERS_PATH}/sd

SD_MMC_BOOT_SOURCES	+= ${SD_DRIVERS_PATH}/sd_mmc.c \
			   drivers/io/io_block.c

PLAT_SD_MMC_INCLUDES	+= -I$(SD_DRIVERS_PATH)
