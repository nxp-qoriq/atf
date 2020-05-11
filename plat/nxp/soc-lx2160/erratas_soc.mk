#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform Errata Build flags.
# These should be enabled by the platform if the erratum workaround needs to be
# applied.

# Flag to apply erratum 50426 workaround during reset.
ERRATA_PLAT_A050426	?=0

# Process ERRATA_PLAT_A050426 flag
ifeq (${ERRATA_PLAT_A050426}, 1)
$(eval $(call add_define,ERRATA_PLAT_A050426))
endif

BL2_SOURCES	+= 	${PLAT_SOC_PATH}/erratas_soc.c
