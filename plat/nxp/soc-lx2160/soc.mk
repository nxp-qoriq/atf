#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#


 # SoC-specific build parameters
I2C_DRIVER	:=	yes
SOC		:=	lx2160
PLAT_PATH	:=	plat/nxp
PLAT_SOC_PATH	:=	${PLAT_PATH}/soc-${SOC}
BOARD_PATH	:=	${PLAT_SOC_PATH}/${BOARD}

ifneq ($(BOARD),lx2160a_hpcsom)
NXP_WDOG_RESTART:=	yes
endif

 # get SoC-specific defnitions
include ${PLAT_SOC_PATH}/soc.def

ifeq ($(WARM_BOOT),yes)
NXP_NV_SW_MAINT_LAST_EXEC_DATA := yes
$(eval $(call add_define,NXP_WARM_BOOT))
PLAT_BL_COMMON_SOURCES		+=	plat/nxp/common/plat_warm_reset.c
endif

ifeq (${NXP_WDOG_RESTART}, yes)
NXP_NV_SW_MAINT_LAST_EXEC_DATA := yes
LS_EL3_INTERRUPT_HANDLER := yes
$(eval $(call add_define, NXP_WDOG_RESTART))
endif

ifeq (${LS_EL3_INTERRUPT_HANDLER}, yes)
$(eval $(call add_define, LS_EL3_INTERRUPT_HANDLER))
endif

ifeq (${GENERATE_COT},1)
# Save Keys to be used by another FIP image
SAVE_KEYS=1
endif


 # common make across all platforms
include ${PLAT_PATH}/common/common.mk

PLAT_INCLUDES	+=	-I${PLAT_SOC_PATH}/include	\
				-I${BOARD_PATH}

PLAT_BL_COMMON_SOURCES	+=	${PLAT_SOC_PATH}/aarch64/${SOC}_helpers.S	\
				${PLAT_SOC_PATH}/soc.c \
				${PLAT_COMMON_PATH}/pmu.c

BL31_SOURCES	+=	${PLAT_SOC_PATH}/$(ARCH)/${SOC}.S	\
			${PLAT_COMMON_PATH}/$(ARCH)/bl31_data.S

ifeq (${WARM_BOOT}, yes)
NXP_NV_SW_MAINT_LAST_EXEC_DATA := yes
BL31_SOURCES	+=	${PLAT_SOC_PATH}/$(ARCH)/${SOC}_warm_rst.S
endif

ifeq (${TEST_BL31}, 1)
BL31_SOURCES	+=	${PLAT_SOC_PATH}/$(ARCH)/bootmain64.S  \
			${PLAT_SOC_PATH}/$(ARCH)/nonboot64.S
endif

include ${PLAT_SOC_PATH}/ddr_fip.mk
include ${PLAT_SOC_PATH}/erratas_soc.mk
