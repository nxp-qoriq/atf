#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#
#------------------------------------------------------------------------------
#
# Select the SECURITY files
#
# -----------------------------------------------------------------------------

ifeq ($(TZC_ID), TZC380)
SECURITY_SOURCES 	+=	${PLAT_DRIVERS_PATH}/tzc/src/tzc380.c
PLAT_INCLUDES		+=	-I${PLAT_DRIVERS_PATH}/tzc/include
else ifeq ($(TZC_ID), TZC400)
SECURITY_SOURCES 	+=	drivers/arm/tzc/tzc400.c
else ifeq ($(TZC_ID), NONE)
    $(info -> No TZC present on platform)
else
    $(error -> TZC type not set!)
endif

SECURITY_SOURCES	+=	${PLAT_COMMON_PATH}/secure_boot.c	\
				${PLAT_DRIVERS_PATH}/sfp/sfp.c

PLAT_INCLUDES		+= 	-I${PLAT_DRIVERS_PATH}/sfp		\

#GET SEC_SOURCES defined
include $(PLAT_DRIVERS_PATH)/fsl_sec/sec.mk

SECURITY_SOURCES	+= ${SEC_SOURCES}

ifeq ($(CHASSIS), 2)
SECURITY_SOURCES 	+=	${PLAT_DRIVERS_PATH}/csu/csu.c
PLAT_INCLUDES		+=	-I${PLAT_DRIVERS_PATH}/csu
CSF_FILE		:=	input_blx_ch${CHASSIS}
BL2_CSF_FILE		:=	input_bl2_ch${CHASSIS}
else
ifeq ($(CHASSIS), 3)
CSF_FILE		:=	input_blx_ch${CHASSIS}
BL2_CSF_FILE		:=	input_bl2_ch${CHASSIS}
PBI_CSF_FILE		:=	input_pbi_ch${CHASSIS}
$(eval $(call add_define, CSF_HDR_CH3))
else
ifeq ($(CHASSIS), 3_2)
CSF_FILE		:=	input_blx_ch3
BL2_CSF_FILE		:=	input_bl2_ch${CHASSIS}
PBI_CSF_FILE		:=	input_pbi_ch${CHASSIS}
$(eval $(call add_define, CSF_HDR_CH3))
else
    $(error -> CHASSIS not set!)
endif
endif
endif

# Trusted Boot configuration
ifeq (${TRUSTED_BOARD_BOOT},1)

include ${PLAT_COMMON_PATH}/tbbr.mk

ifeq (${MBEDTLS_DIR},)
    # Generic image processing filters to prepend CSF header
    ifeq (${BL33_INPUT_FILE},)
    BL33_INPUT_FILE:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${CSF_FILE}
    endif

    ifeq (${BL31_INPUT_FILE},)
    BL31_INPUT_FILE:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${CSF_FILE}
    endif

    ifeq (${BL32_INPUT_FILE},)
    BL32_INPUT_FILE:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${CSF_FILE}
    endif

    ifeq (${FUSE_INPUT_FILE},)
    FUSE_INPUT_FILE:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${CSF_FILE}
    endif

endif #MBEDTLS_DIR

    ifeq (${BL2_INPUT_FILE},)
    BL2_INPUT_FILE:= plat/nxp/drivers/auth/csf_hdr_parser/${BL2_CSF_FILE}
    endif

    ifeq (${PBI_INPUT_FILE},)
    PBI_INPUT_FILE:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${PBI_CSF_FILE}
    endif


endif #TRUSTED_BOARD_BOOT

# -----------------------------------------------------------------------------

