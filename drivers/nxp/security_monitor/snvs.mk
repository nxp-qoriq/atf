#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

SNVS_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/security_monitor

PLAT_INCLUDES		+= -I$(SNVS_DRIVERS_PATH)

PLAT_BL_COMMON_SOURCES	+= $(SNVS_DRIVERS_PATH)/snvs.c

