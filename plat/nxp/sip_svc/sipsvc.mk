#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Rod Dorris <rod.dorris@nxp.com>
#
#------------------------------------------------------------------------------
#
# Select the SIP SVC files
#
# -----------------------------------------------------------------------------

SIPSVC_SOURCES	:=	${PLAT_SIPSVC_PATH}/sip_svc.c \
			${PLAT_SIPSVC_PATH}/$(ARCH)/sipsvc.S

# -----------------------------------------------------------------------------

