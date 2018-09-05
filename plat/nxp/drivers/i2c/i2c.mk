#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

I2C_DRIVERS_PATH        :=      plat/nxp/drivers/i2c

BL2_SOURCES		+=  $(I2C_DRIVERS_PATH)/i2c.c
PLAT_INCLUDES		+= -I$(I2C_DRIVERS_PATH)
