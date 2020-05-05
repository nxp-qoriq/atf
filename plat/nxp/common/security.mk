#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#------------------------------------------------------------------------------
#
# Select the SECURITY files
#
# -----------------------------------------------------------------------------

ifeq ($(TZC_ID), TZC400)
SECURITY_SOURCES 	+=	drivers/arm/tzc/tzc400.c
else ifeq ($(TZC_ID), NONE)
    $(info -> No TZC present on platform)
else
    $(error -> TZC type not set!)
endif

#GET SEC_SOURCES defined
include $(PLAT_DRIVERS_PATH)/crypto/caam/caam.mk

SECURITY_SOURCES	+= ${SEC_SOURCES}

ifeq ($(CHASSIS), 3_2)
CSF_FILE		:=	input_blx_ch3
BL2_CSF_FILE		:=	input_bl2_ch${CHASSIS}
PBI_CSF_FILE		:=	input_pbi_ch${CHASSIS}
$(eval $(call add_define, CSF_HDR_CH3))
else
    $(error -> CHASSIS not set!)
endif

# Trusted Boot configuration
ifeq (${TRUSTED_BOARD_BOOT},1)

$(eval $(call add_define, NXP_SFP_ENABLED))
include $(PLAT_DRIVERS_PATH)/sfp/sfp.mk

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
    BL2_INPUT_FILE:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${BL2_CSF_FILE}
    endif

    ifeq (${PBI_INPUT_FILE},)
    PBI_INPUT_FILE:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${PBI_CSF_FILE}
    endif


endif #TRUSTED_BOARD_BOOT

# -----------------------------------------------------------------------------

