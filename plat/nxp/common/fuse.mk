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
FUSE_FIP_ARGS += --fuse-prov ${FUSE_PROV_FILE}
FUSE_FIP_DEPS += ${FUSE_PROV_FILE}
endif

ifeq (${FUSE_UP_FILE},)
else
FUSE_FIP_ARGS += --fuse-up ${FUSE_UP_FILE}
FUSE_FIP_DEPS += ${FUSE_UP_FILE}
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

