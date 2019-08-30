#
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#

###############################################################################
# Flow begins in BL2 at EL3 mode
BL2_AT_EL3			:= 1

# TODO - Below defines for warm boot -- Needs to be tested

# Though one core is powered up by default, there are
# platform specific ways to release more than one core
# TODO - Ideally based on above thought this value should be 1
# to be corrected once PSCI flow comes in
COLD_BOOT_SINGLE_CPU		:= 0

# TODO - RVBAR_EL3 is programmable on our cores. Not tested with warm boot
PROGRAMMABLE_RESET_ADDRESS	:= 1

# TODO - Rod to check .. We will use the latest PSCI API's
ENABLE_PLAT_COMPAT		:= 0

# TODO - Rod to check - Related with coherent memory usage in PSCI
# do not use coherent memory
USE_COHERENT_MEM		:= 0

# Use generic OID definition (tbbr_oid.h)
USE_TBBR_DEFS			:= 1

# We are not using dynamic memory mapping
PLAT_XLAT_TABLES_DYNAMIC	:= 0

# do not enable SVE
ENABLE_SVE_FOR_NS		:= 0

ENABLE_STACK_PROTECTOR		:= 1

ERROR_DEPRECATED		:= 0

MULTI_CONSOLE_API		:= 1

# Process LS_DISABLE_TRUSTED_WDOG flag
# TODO:Temparally disabled it on development phase, not implemented yet
LS_DISABLE_TRUSTED_WDOG		:= 1

# On ARM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
SEPARATE_CODE_AND_RODATA	:= 1

# Enable new version of image loading on ARM platforms
LOAD_IMAGE_V2			:= 1

RCW				:= ""

ifneq (${SPD},none)
$(eval $(call add_define, LS_LOAD_BL32))
endif

ifeq (${TRUSTED_BOARD_BOOT},1)
ifeq ($(SECURE_BOOT),)
SECURE_BOOT := yes
endif
endif

$(eval $(call assert_boolean,POLICY_OTA))

ifeq (${POLICY_OTA},1)
$(eval $(call add_define,POLICY_OTA))
ifeq (${BOOT_MODE}, sd)
$(error Error: SD Boot is invalid for OTA)
endif
ifeq (${BOOT_MODE}, emmc)
$(error Error: EMMC Boot is invalid for OTA)
endif
ifeq (${BOOT_MODE}, nand)
$(error Error: NAND Boot is invalid for OTA)
endif
endif

###############################################################################

PLAT_TOOL_PATH		:=	${PLAT_PATH}/tools
PLAT_COMMON_PATH	:=	${PLAT_PATH}/common
PLAT_DRIVERS_PATH	:=	${PLAT_PATH}/drivers
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

include ${PLAT_COMMON_PATH}/core.mk
include ${PLAT_COMMON_PATH}/interconnect.mk
include ${PLAT_COMMON_PATH}/gic.mk
include ${PLAT_COMMON_PATH}/security.mk
include ${PLAT_COMMON_PATH}/console.mk
include ${PLAT_PSCI_PATH}/psci.mk
include ${PLAT_SIPSVC_PATH}/sipsvc.mk
# Enable workarounds for platform specific errata
include ${PLAT_COMMON_PATH}/errata.mk
ifeq (${FUSE_PROG}, 1)
include plat/nxp/common/fuse.mk
endif

ifeq (${TEST_BL31}, 1)
$(eval $(call add_define,TEST_BL31))
PLAT_TEST_PATH		:=	${PLAT_PATH}/test
include ${PLAT_TEST_PATH}/test.mk
endif

#linker file for BL2
# Keep it in sync with bl2/bl2_el3_plat.ld.S
# A Loadable section  limit assert added
BL2_LINKERFILE		?=	plat/nxp/common/bl2_el3_plat.ld.S

TIMER_SOURCES		:=	drivers/delay_timer/delay_timer.c	\
				plat/nxp/common/timer.c

PLAT_BL_COMMON_SOURCES	+=	${CONSOLE_SOURCES}

ifeq (${BOOT_MODE}, nor)
$(eval $(call add_define,NOR_BOOT))
BOOT_DEV_SOURCES		=	${PLAT_DRIVERS_PATH}/ifc/nor/ifc_nor.c
else ifeq (${BOOT_MODE}, nand)
$(eval $(call add_define,NAND_BOOT))
BOOT_DEV_SOURCES		=	${PLAT_DRIVERS_PATH}/ifc/nand/nand.c	\
					drivers/io/io_block.c
else ifeq (${BOOT_MODE}, qspi)
$(eval $(call add_define,QSPI_BOOT))
BOOT_DEV_SOURCES		=	${PLAT_DRIVERS_PATH}/qspi/qspi.c
else ifeq (${BOOT_MODE}, sd)
$(eval $(call add_define,SD_BOOT))
BOOT_DEV_SOURCES		=	${PLAT_DRIVERS_PATH}/sd/sd_mmc.c	\
					drivers/io/io_block.c
else ifeq (${BOOT_MODE}, emmc)
$(eval $(call add_define,SD_BOOT))
BOOT_DEV_SOURCES		=	${PLAT_DRIVERS_PATH}/sd/sd_mmc.c	\
					drivers/io/io_block.c
else ifeq (${BOOT_MODE}, flexspi_nor)
$(eval $(call add_define,FLEXSPI_NOR_BOOT))
BOOT_DEV_SOURCES		= ${PLAT_DRIVERS_PATH}/flexspi/nor/flexspi_nor.c
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
PLAT_INCLUDES		+= -I$(PLAT_DRIVERS_PATH)/ddr/fsl-mmdc
else
include $(PLAT_DRIVERS_PATH)/ddr/nxp-ddr/ddr.mk
PLAT_INCLUDES		+= -I$(PLAT_DRIVERS_PATH)/ddr/nxp-ddr
endif # DDR_CNTRL_SOURCES
include $(PLAT_DRIVERS_PATH)/i2c/i2c.mk
ifeq ($(PLAT_DDR_PHY), phy-gen2)
PHY_SOURCES			:= ${PLAT_COMMON_PATH}/ddr_io_storage.c
endif
endif

ifeq ($(NEED_FUSE),yes)
BL2_SOURCES	+= ${FUSE_SOURCES}
endif

###############################################################################

PLAT_INCLUDES		+=	-Iinclude/common/tbbr

PLAT_BL_COMMON_SOURCES	+= 	${CPU_LIBS}				\
				${INTERCONNECT_SOURCES}			\
				${TIMER_SOURCES}			\
				${SECURITY_SOURCES}

PLAT_BL_COMMON_SOURCES	+=	plat/nxp/common/${ARCH}/ls_helpers.S	\
				plat/nxp/common/ls_err.c		\
				plat/nxp/common/ls_common.c

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES		+=	${PLAT_COMMON_PATH}/ls_stack_protector.c
endif

include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

ifeq (${POLICY_OTA}, 1)
PLAT_INCLUDES		+=	-I$(PLAT_DRIVERS_PATH)/sd
BL2_SOURCES		+=	plat/nxp/common/ls_ota.c
BOOT_DEV_SOURCES	+=	${PLAT_DRIVERS_PATH}/sd/sd_mmc.c        \
				drivers/io/io_block.c
ifeq ($(CHASSIS), 2)
BOOT_DEV_SOURCES	+=	${PLAT_DRIVERS_PATH}/wdt/wdt.c
else
BOOT_DEV_SOURCES	+=	${PLAT_DRIVERS_PATH}/wdt/sp805_wdt.c
endif
endif

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
				${PLAT_COMMON_PATH}/load_img.c		\
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
endif

include $(PLAT_TOOL_PATH)/create_pbl.mk
