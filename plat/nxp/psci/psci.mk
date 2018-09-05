#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#
#------------------------------------------------------------------------------
#
# Select the PSCI files
#
# -----------------------------------------------------------------------------

PSCI_SOURCES	:=	${PLAT_COMMON_PATH}/ls_topology.c		\
					${PLAT_PSCI_PATH}/plat_psci.c	\
					${PLAT_PSCI_PATH}/$(ARCH)/psci_utils.S	\
					plat/common/plat_psci_common.c

# -----------------------------------------------------------------------------

