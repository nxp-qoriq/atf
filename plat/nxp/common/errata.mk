#
# Copyright 2018, 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>

# Platform Errata Build flags.
# These should be enabled by the platform if the erratum workaround needs to be
# applied.

# Flag to apply erratum 8850 workaround during reset.
ERRATA_PLAT_A008850	?=0

# Process ERRATA_PLAT_A008850 flag
$(eval $(call assert_boolean,ERRATA_PLAT_A008850))
$(eval $(call add_define,ERRATA_PLAT_A008850))

# Flag to apply erratum 9660 workaround during reset.
ERRATA_PLAT_A009660	?=0

# Process ERRATA_PLAT_A009660 flag
$(eval $(call assert_boolean,ERRATA_PLAT_A009660))
$(eval $(call add_define,ERRATA_PLAT_A009660))

ERRATA_PLAT_A010539	?=0

# Process ERRATA_PLAT_A010539 flag
$(eval $(call assert_boolean,ERRATA_PLAT_A010539))
$(eval $(call add_define,ERRATA_PLAT_A010539))

# Flag to apply erratum 50426 workaround during reset.
ERRATA_PLAT_A050426	?=0

# Process ERRATA_PLAT_A050426 flag
ifeq (${ERRATA_PLAT_A050426}, 1)
$(eval $(call add_define,ERRATA_PLAT_A050426))
endif

BL2_SOURCES	+= 	${PLAT_COMMON_PATH}/errata.c
