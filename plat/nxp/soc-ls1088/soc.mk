#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>


 # SoC-specific build parameters
SOC		:=	ls1088
PLAT_PATH	:=	plat/nxp
PLAT_SOC_PATH	:=	${PLAT_PATH}/soc-${SOC}
BOARD_PATH	:=	${PLAT_SOC_PATH}/${SOC}${BOARD}
SEPARATE_RW_AND_NOLOAD	:= 1

$(eval $(call assert_boolean,SEPARATE_RW_AND_NOLOAD))
$(eval $(call add_define,SEPARATE_RW_AND_NOLOAD))
$(eval $(call add_define,SEPARATE_RW_AND_NOLOAD))

 # get SoC-specific defnitions
include ${PLAT_SOC_PATH}/soc.def

 # common make across all platforms
include ${PLAT_PATH}/common/common.mk

PLAT_INCLUDES	+=	-I${PLAT_SOC_PATH}/include	\
				-I${BOARD_PATH}

PLAT_BL_COMMON_SOURCES	+=	${PLAT_SOC_PATH}/aarch64/${SOC}_helpers.S	\
				${PLAT_SOC_PATH}/soc.c \
				${PLAT_COMMON_PATH}/pmu.c

BL31_SOURCES	+=	${PLAT_SOC_PATH}/$(ARCH)/${SOC}.S	\
			${PLAT_COMMON_PATH}/$(ARCH)/bl31_data.S

ifeq (${TEST_BL31}, 1)
BL31_SOURCES	+=	${PLAT_SOC_PATH}/$(ARCH)/bootmain64.S  \
			${PLAT_SOC_PATH}/$(ARCH)/nonboot64.S
endif

BL2_LINKERFILE		:=	${PLAT_SOC_PATH}/bl2_el3_ls1088.ld.S
