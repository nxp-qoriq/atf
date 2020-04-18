#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

###############################################################################
# Flow begins in BL2 at EL3 mode
BL2_AT_EL3			:= 1

# Though one core is powered up by default, there are
# platform specific ways to release more than one core
COLD_BOOT_SINGLE_CPU		:= 0

PROGRAMMABLE_RESET_ADDRESS	:= 1

USE_COHERENT_MEM		:= 0

PLAT_XLAT_TABLES_DYNAMIC	:= 0

ENABLE_SVE_FOR_NS		:= 0

ENABLE_STACK_PROTECTOR		:= 0

ERROR_DEPRECATED		:= 0

LS_DISABLE_TRUSTED_WDOG		:= 1

# On ARM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
SEPARATE_CODE_AND_RODATA	:= 1

# Enable new version of image loading on ARM platforms
LOAD_IMAGE_V2			:= 1

RCW				:= ""

ifneq (${SPD},none)
$(eval $(call add_define, NXP_LOAD_BL32))
endif

ifeq (${TRUSTED_BOARD_BOOT},1)
ifeq ($(SECURE_BOOT),)
SECURE_BOOT := yes
endif
endif

###############################################################################

PLAT_DRIVERS_PATH	:=	drivers/nxp
PLAT_TOOL_PATH		:=	tools/nxp
PLAT_COMMON_PATH	:=	${PLAT_PATH}/common
PLAT_PSCI_PATH		:=	${PLAT_PATH}/psci
PLAT_SIPSVC_PATH	:=	${PLAT_PATH}/sip_svc

PLAT_INCLUDES		+=	-Iinclude/plat/arm/common			\
				-Iinclude/drivers/arm   			\
				-Iinclude/lib					\
				-Iinclude/drivers/io			\
				-I${PLAT_COMMON_PATH}/include	\
				-I${PLAT_PSCI_PATH}/include	\
				-I${PLAT_SIPSVC_PATH}/include   \
				-Ilib/psci

include $(PLAT_DRIVERS_PATH)/dcfg/dcfg.mk
include $(PLAT_DRIVERS_PATH)/timer/timer.mk
include ${PLAT_COMMON_PATH}/core.mk
include ${PLAT_COMMON_PATH}/interconnect.mk
include ${PLAT_COMMON_PATH}/gic.mk
include ${PLAT_COMMON_PATH}/security.mk
include ${PLAT_COMMON_PATH}/console.mk
include ${PLAT_PSCI_PATH}/psci.mk
include ${PLAT_SIPSVC_PATH}/sipsvc.mk

ifeq (${FUSE_PROG}, 1)
include plat/nxp/common/fuse.mk
IMG_LOADR	?= 1
endif

ifeq (${TEST_BL31}, 1)
$(eval $(call add_define,TEST_BL31))
PLAT_TEST_PATH		:=	${PLAT_PATH}/test
include ${PLAT_TEST_PATH}/test.mk
endif

PLAT_BL_COMMON_SOURCES	+=	${CONSOLE_SOURCES}

ifeq (${BOOT_MODE}, flexspi_nor)
$(eval $(call add_define,FLEXSPI_NOR_BOOT))
include $(PLAT_DRIVERS_PATH)/flexspi/nor/flexspi_nor.mk
else ifeq (${BOOT_MODE}, sd)
$(eval $(call add_define,SD_BOOT))
BOOT_DEV_SOURCES		= ${PLAT_DRIVERS_PATH}/sd/sd_mmc.c \
					drivers/io/io_block.c
PLAT_INCLUDES			+= -I$(PLAT_DRIVERS_PATH)/sd
else ifeq (${BOOT_MODE}, emmc)
$(eval $(call add_define,EMMC_BOOT))
BOOT_DEV_SOURCES		= ${PLAT_DRIVERS_PATH}/sd/sd_mmc.c \
					drivers/io/io_block.c
PLAT_INCLUDES			+= -I$(PLAT_DRIVERS_PATH)/sd
endif

# DDR driver needs to be enabled by default
override DDR_DRIVER			:= yes

ifeq ($(DDR_DRIVER),yes)
$(eval $(call add_define, DDR_INIT))
endif

ifeq ($(DDR_DRIVER),yes)
# define DDR_CNTRL_SOURCES
ifeq ($(DDRCNTLR),MMDC)
include $(PLAT_DRIVERS_PATH)/ddr/fsl-mmdc/ddr.mk
else
include $(PLAT_DRIVERS_PATH)/ddr/nxp-ddr/ddr.mk
endif # DDR_CNTRL_SOURCES
ifeq ($(PLAT_DDR_PHY), PHY_GEN2)
PLAT_DDR_PHY_DIR		:= phy-gen2
PHY_SOURCES			:= ${PLAT_COMMON_PATH}/ddr_io_storage.c
IMG_LOADR			?= 1
endif
endif

ifeq (${IMG_LOADR}, 1)
include $(PLAT_DRIVERS_PATH)/img_loadr/img_loadr.mk
endif

ifeq ($(I2C_DRIVER),yes)
$(eval $(call add_define, I2C_INIT))
include $(PLAT_DRIVERS_PATH)/i2c/i2c.mk
endif

ifeq ($(NEED_FUSE),yes)
BL2_SOURCES	+= ${FUSE_SOURCES}
endif
###############################################################################

PLAT_INCLUDES		+=	-Iinclude/common/tbbr

PLAT_BL_COMMON_SOURCES	+= 	${CPU_LIBS}				\
				${INTERCONNECT_SOURCES}			\
				${SECURITY_SOURCES}

PLAT_BL_COMMON_SOURCES	+=	plat/nxp/common/${ARCH}/ls_helpers.S	\
				plat/nxp/common/ls_err.c		\
				plat/nxp/common/ls_common.c

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES		+=	${PLAT_COMMON_PATH}/ls_stack_protector.c
endif

include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

BL2_SOURCES		+=	drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				common/desc_image_load.c 		\
				${BOOT_DEV_SOURCES}			\
				${DDR_CNTLR_SOURCES}			\
				${PHY_SOURCES}				\
				plat/nxp/common/ls_image_load.c		\
				plat/nxp/common/ls_io_storage.c		\
				plat/nxp/common/ls_bl2_el3_setup.c	\
				plat/nxp/common/${ARCH}/ls_bl2_mem_params_desc.c

BL31_SOURCES	+=	plat/nxp/common/ls_bl31_setup.c	\
				${GIC_SOURCES}	\
				${PSCI_SOURCES}	\
				${SIPSVC_SOURCES}


ifeq (${TEST_BL31}, 1)
BL31_SOURCES	+=	${TEST_SOURCES}
endif

# Verify build config
# -------------------

ifneq (${LOAD_IMAGE_V2}, 1)
  $(error Error: Layerscape needs LOAD_IMAGE_V2=1)
else
$(eval $(call add_define,LOAD_IMAGE_V2))
endif

include $(PLAT_TOOL_PATH)/create_pbl.mk
