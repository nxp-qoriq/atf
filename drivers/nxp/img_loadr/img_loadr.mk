#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

IMG_LOADR_DRIVERS_PATH        :=  ${PLAT_DRIVERS_PATH}/img_loadr

BL2_SOURCES		+=  $(IMG_LOADR_DRIVERS_PATH)/load_img.c
PLAT_INCLUDES		+= -I$(IMG_LOADR_DRIVERS_PATH)
