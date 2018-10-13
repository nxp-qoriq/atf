#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#

NEED_FUSE	:= yes
$(eval $(call add_define, POLICY_FUSE_PROVISION))

FUSE_SOURCES	:= plat/nxp/common/fuse_io_storage.c

FUSE_FIP_NAME := fuse_fip.bin

fip_fuse: ${BUILD_PLAT}/${FUSE_FIP_NAME}

ifeq (${FUSE_PROV_FILE},)
else
ifeq (${TRUSTED_BOARD_BOOT},1)
FUSE_FIP_ARGS += --fuse-prov ${BUILD_PLAT}/${FUSE_PROV_FILE}.sb
FUSE_FIP_DEPS += ${BUILD_PLAT}/${FUSE_PROV_FILE}.sb
else
FUSE_FIP_ARGS += --fuse-prov ${FUSE_PROV_FILE}
FUSE_FIP_DEPS += ${FUSE_PROV_FILE}
endif
endif

ifeq (${FUSE_UP_FILE},)
else
ifeq (${TRUSTED_BOARD_BOOT},1)
FUSE_FIP_ARGS += --fuse-up ${BUILD_PLAT}/${FUSE_UP_FILE}.sb
FUSE_FIP_DEPS += ${BUILD_PLAT}/${FUSE_UP_FILE}.sb
else
FUSE_FIP_ARGS += --fuse-up ${FUSE_UP_FILE}
FUSE_FIP_DEPS += ${FUSE_UP_FILE}
endif
endif

ifeq (${TRUSTED_BOARD_BOOT},1)

ifeq (${MBEDTLS_DIR},)
else
  $(error Error: Trusted Board Boot with X509 certificates not supported with FUSE_PROG build option)
endif


# Max Size of CSF header. image will be appended at this offset of header
CSF_HDR_SZ	?= 0x4000

# Path to CST directory is required to generate the CSF header
# and prepend it to image before fip image gets generated
ifeq (${CST_DIR},)
  $(error Error: CST_DIR not set)
endif

ifeq (${FUSE_INPUT_FILE},)
FUSE_INPUT_FILE := plat/nxp/drivers/auth/csf_hdr_parser/${CSF_FILE}
endif

${BUILD_PLAT}/%.sb: %
	@echo " Generating CSF Header for $@ $<"
	$(CST_DIR)/create_hdr_esbc --in $< --out $@ --app_off ${CSF_HDR_SZ} \
					--app $< ${FUSE_INPUT_FILE}

endif

${BUILD_PLAT}/${FUSE_FIP_NAME}: fiptool ${FUSE_FIP_DEPS}
ifeq (${FUSE_FIP_DEPS},)
	$(error "Error: FUSE_PROV_FILE or/and FUSE_UP_FILE needs to point to the right file")
endif
	${FIPTOOL} create ${FUSE_FIP_ARGS} $@
	${FIPTOOL} info $@
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

