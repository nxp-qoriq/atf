# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#
#------------------------------------------------------------------------------
#
# Select the CORE files
#
# -----------------------------------------------------------------------------

CPU_LIBS		:=	lib/cpus/${ARCH}/aem_generic.S

ifeq (,$(filter $(CORE_TYPE),a53 a55 a57 a72 a75))
$(error "CORE_TYPE not specified or incorrect")
else
CPU_LIBS		+=	lib/cpus/${ARCH}/cortex_$(CORE_TYPE).S
endif

# -----------------------------------------------------------------------------

