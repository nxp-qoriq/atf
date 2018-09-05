/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Ruchika Gupta <ruchika.gupta@nxp.com>
 */

#include <platform_def.h>
#include <gicv3.h>
#include <plat_common.h>
#include <platform.h>

static const interrupt_prop_t ls_interrupt_props[] = {
	PLAT_LS_G1S_IRQ_PROPS(INTR_GROUP1S),
	PLAT_LS_G0_IRQ_PROPS(INTR_GROUP0)
};

static uintptr_t target_mask_array[PLATFORM_CORE_COUNT];

static unsigned int plat_mpidr_to_core_pos(u_register_t mpidr)
{
	return plat_core_pos_by_mpidr(mpidr);
}

static struct gicv3_driver_data ls_gic_data = {
	.gicd_base = NXP_GICD_ADDR,
	.gicr_base = NXP_GICR_ADDR,
	.interrupt_props = ls_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(ls_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = target_mask_array,
	.mpidr_to_core_pos = plat_mpidr_to_core_pos,
};

/******************************************************************************
 * NXP common helper to initialize the GICv3 only driver.
 *****************************************************************************/
void plat_ls_gic_driver_init(void)
{
	gicv3_driver_init(&ls_gic_data);
}

void plat_ls_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * NXP common helper to enable the GICv3 CPU interface
 *****************************************************************************/
void plat_ls_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * NXP common helper to disable the GICv3 CPU interface
 *****************************************************************************/
void plat_ls_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

/******************************************************************************
 * NXP common helper to initialize the per cpu distributor interface in GICv3
 *****************************************************************************/
void plat_ls_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * Stubs for Redistributor power management. Although GICv3 doesn't have
 * Redistributor interface, these are provided for the sake of uniform GIC API
 *****************************************************************************/
void plat_ls_gic_redistif_on(void)
{
}

void plat_ls_gic_redistif_off(void)
{
}
