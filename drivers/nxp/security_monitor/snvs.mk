#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${NEED_SNVS},)

NEED_SNVS	:=	1
$(info Adding SNVS driver)

SNVS_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/security_monitor

PLAT_INCLUDES		+= -I$(SNVS_DRIVERS_PATH)

PLAT_BL_COMMON_SOURCES	+= $(SNVS_DRIVERS_PATH)/snvs.c

endif
