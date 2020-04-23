/*
 * Copyright 2018-2020 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author York Sun <york.sun@nxp.com>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <errno.h>
#include <string.h>
#include <utils.h>
#include <platform_def.h>
#include <plat_common.h>
#include <io.h>
#include <soc.h>
#include <delay_timer.h>
#include <types.h>
#include <ddr.h>
#include <xlat_tables_v2.h>
#include "csr.h"
#include "ddr4fw.h"
#include "input.h"
#include "pie.h"

#ifdef DDR_PHY_DEBUG
#include "messages.h"
#endif

#define TIMEOUTDEFAULT 500
#define MAP_PHY_ADDR(pstate, n, instance, offset, c) \
		((((pstate * n) + instance + c) << 12) + offset)

static uint32_t map_phy_addr_space(uint32_t addr)
{
	/* 23 bit addressing */
	int pstate =     (addr & 0x700000) >> 20; /* bit 22:20 */
	int block_type = (addr & 0x0f0000) >> 16; /* bit 19:16 */
	int instance =   (addr & 0x00f000) >> 12; /* bit 15:12 */
	int offset =     (addr & 0x000fff);       /* bit 11:0 */

	switch (block_type) {
	case 0x0: /* 0x0 : ANIB */
		return MAP_PHY_ADDR(pstate, 12, instance, offset, 0);
	case 0x1: /* 0x1 : DBYTE */
		return MAP_PHY_ADDR(pstate, 10, instance, offset, 0x30);
	case 0x2: /* 0x2 : MASTER */
		return MAP_PHY_ADDR(pstate, 1, 0, offset, 0x58);
	case 0x4: /* 0x4 : ACSM */
		return MAP_PHY_ADDR(pstate, 1, 0, offset, 0x5c);
	case 0x5: /* 0x5 : μCTL Memory */
		return MAP_PHY_ADDR(pstate, 0, instance, offset, 0x60);
	case 0x7: /* 0x7 : PPGC */
		return MAP_PHY_ADDR(pstate, 0, 0, offset, 0x68);
	case 0x9: /* 0x9 : INITENG */
		return MAP_PHY_ADDR(pstate, 1, 0, offset, 0x69);
	case 0xc: /* 0xC : DRTUB */
		return MAP_PHY_ADDR(pstate, 0, 0, offset, 0x6d);
	case 0xd: /* 0xD : APB Only */
		return MAP_PHY_ADDR(pstate, 0, 0, offset, 0x6e);
	default:
		printf("ERR: Invalid block_type = 0x%x\n", block_type);
		return 0;
	}
}

static inline uint8_t get_board_rev(void)
{
	return (((*(volatile uint32_t*)(NXP_DCFG_ADDR + DCFG_SVR_OFFSET)) >> 4) & 0x3);
}

static inline uint16_t *phy_io_addr(void *phy, uint32_t addr)
{
	return phy + (map_phy_addr_space(addr) << 2);
}

static inline void phy_io_write16(uint16_t *phy, uint32_t addr, uint16_t data)
{
	out_le16(phy_io_addr(phy, addr), data);
#ifdef DEBUG_PHY_IO
	printf("0x%06x,0x%x\n", addr, data);
#endif
}

static inline uint16_t phy_io_read16(uint16_t *phy, uint32_t addr)
{
	uint16_t reg = in_le16(phy_io_addr(phy, addr));

#ifdef DEBUG_PHY_IO
	printf("R: 0x%06x,0x%x\n", addr, reg);
#endif

	return reg;
}

static void load_pieimage(uint16_t *phy,
			  enum dimm_types dimm_type)
{
	int i;
	int size;
	const struct pie *image = NULL;

	switch (dimm_type) {
	case UDIMM:
	case SODIMM:
	case NODIMM:
		image = pie_udimm;
		size = ARRAY_SIZE(pie_udimm);
		break;
	case RDIMM:
		image = pie_rdimm;
		size = ARRAY_SIZE(pie_rdimm);
		break;
	case LRDIMM:
		image = pie_lrdimm;
		size = ARRAY_SIZE(pie_lrdimm);
		break;
	default:
		printf("Unsupported DIMM type\n");
		break;
	}

	if (image) {
		for (i = 0; i < size; i++)
			phy_io_write16(phy, image[i].addr, image[i].data);
	}
}

static void prog_acsm_playback(uint16_t *phy,
			       const struct input *input, const void *msg)
{
	int vec;
	const struct ddr4r1d *msg_blk;
	uint16_t acsmplayback[2][3];
	uint32_t f0rc0a;
	uint32_t f0rc3x;
	uint32_t f0rc5x;

	if (input->basic.dimm_type != RDIMM)
		return;

	msg_blk = msg;
	f0rc0a = (msg_blk->f0rc0a_d0 & 0xf) | 0xa0;
	f0rc3x = (msg_blk->f0rc3x_d0 & 0xff) | 0x300;
	f0rc5x = (input->adv.phy_gen2_umctl_f0rc5x & 0xff) | 0x500;

	acsmplayback[0][0] = 0x3ff & f0rc0a;
	acsmplayback[1][0] = (0x1c00 & f0rc0a) >> 10;
	acsmplayback[0][1] = 0x3ff & f0rc3x;
	acsmplayback[1][1] = (0x1c00 & f0rc3x) >> 10;
	acsmplayback[0][2] = 0x3ff & f0rc5x;
	acsmplayback[1][2] = (0x1c00 & f0rc5x) >> 10;
	for (vec = 0; vec < 3; vec++) {
		phy_io_write16(phy, t_acsm | (csr_acsm_playback0x0_addr +
			       (vec << 1)), acsmplayback[0][vec]);
		phy_io_write16(phy, t_acsm | (csr_acsm_playback1x0_addr +
			       (vec << 1)), acsmplayback[1][vec]);
	}
}

static void prog_acsm_ctr(uint16_t *phy,
			  const struct input *input)
{
	if (input->basic.dimm_type != RDIMM)
		return;

	phy_io_write16(phy, t_acsm | csr_acsm_ctrl13_addr,
		       0xf << csr_acsm_cke_enb_lsb);

	phy_io_write16(phy, t_acsm | csr_acsm_ctrl0_addr,
		       csr_acsm_par_mode_mask | csr_acsm_2t_mode_mask);
}

static void prog_cal_rate_run(uint16_t *phy,
			  const struct input *input)
{
	int cal_rate;
	int cal_interval;
	int cal_once;
	uint32_t addr;

	cal_interval = input->adv.cal_interval;
	cal_once = input->adv.cal_once;
	cal_rate = 0x1 << csr_cal_run_lsb 		|
		   cal_once << csr_cal_once_lsb		|
		   cal_interval << csr_cal_interval_lsb;
	addr = t_master | csr_cal_rate_addr;
	phy_io_write16(phy, addr, cal_rate);
}

static void prog_seq0bdly0(uint16_t *phy,
		    const struct input *input)
{
	int ps_count[4];
	int frq;
	uint32_t addr;
	uint8_t board_rev;
	int lower_freq_opt = 0;

	frq = input->basic.frequency >> 1;
	ps_count[0] = frq >> 3; /* 0.5 * frq / 4*/
	if (input->basic.frequency < 400)
		lower_freq_opt = (input->basic.dimm_type == RDIMM) ? 7 : 3;
	else if (input->basic.frequency < 533)
		lower_freq_opt = (input->basic.dimm_type == RDIMM) ? 14 : 11;

	ps_count[1] = (frq >> 2) - lower_freq_opt; /* 1.0 * frq / 4 - lower_freq */
	ps_count[2] = (frq << 1) +  (frq >> 1); /* 10.0 * frq / 4 */

#ifdef  DDR_PLL_FIX
	board_rev = get_board_rev();
	if(board_rev == 1) {
		ps_count[0] = 0x520; /* seq0bdly0 */
		ps_count[1] = 0xa41; /* seq0bdly1 */
		ps_count[2] = 0x668a; /* seq0bdly2 */
	}
#endif
	if (frq > 266)
		ps_count[3] = 44;
	else if (frq > 200)
		ps_count[3] = 33;
	else
		ps_count[3] = 16;

	addr = t_master | csr_seq0bdly0_addr;
	phy_io_write16(phy, addr, ps_count[0]);

	debug("seq0bdly0 = 0x%x\n", phy_io_read16(phy, addr));

	addr = t_master | csr_seq0bdly1_addr;
	phy_io_write16(phy, addr, ps_count[1]);

	debug("seq0bdly1 = 0x%x\n", phy_io_read16(phy, addr));

	addr = t_master | csr_seq0bdly2_addr;
	phy_io_write16(phy, addr, ps_count[2]);

	debug("seq0bdly2 = 0x%x\n", phy_io_read16(phy, addr));

	addr = t_master | csr_seq0bdly3_addr;
	phy_io_write16(phy, addr, ps_count[3]);

	debug("seq0bdly3 = 0x%x\n", phy_io_read16(phy, addr));
}

/* Only RDIMM requires msg_blk */
static void i_load_pie(uint16_t **phy_ptr,
		       const struct input *input,
		       const void *msg)
{
	int i;
	uint16_t *phy;

	for (i = 0; i < NUM_OF_DDRC; i++) {
		phy = phy_ptr[i];
		if (!phy)
			continue;

		phy_io_write16(phy,
			       t_apbonly | csr_micro_cont_mux_sel_addr,
			       0);

		load_pieimage(phy, input->basic.dimm_type);

		prog_seq0bdly0(phy, input);
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag0_addr,
			       0x0000);
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag1_addr,
			       0x0173);
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag2_addr,
			       0x0060);
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag3_addr,
			       0x6110);
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag4_addr,
			       0x2152);
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag5_addr,
			       0xdfbd);
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag6_addr,
			       input->basic.dimm_type == RDIMM &&
			       input->adv.phy_gen2_umctl_opt == 1 ?
			       0x6000 : 0xffff);
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag7_addr,
			       0x6152);
		prog_acsm_playback(phy, input, msg);		/* rdimm */
		prog_acsm_ctr(phy, input);			/* rdimm */

		phy_io_write16(phy, t_master | csr_cal_zap_addr, 0x1);
		prog_cal_rate_run(phy, input);

		phy_io_write16(phy, t_drtub | csr_ucclk_hclk_enables_addr,
			       input->basic.dimm_type == RDIMM ? 0x2 : 0);

		phy_io_write16(phy, t_apbonly | csr_micro_cont_mux_sel_addr, 1);
	}
}

static void phy_gen2_init_input(struct input *input)
{
	int i;

	input->adv.dram_byte_swap		= 0;
	input->adv.ext_cal_res_val		= 0;
	input->adv.tx_slew_rise_dq		= 0xf;
	input->adv.tx_slew_fall_dq		= 0xf;
	input->adv.tx_slew_rise_ac		= 0xf;
	input->adv.tx_slew_fall_ac		= 0xf;
	input->adv.mem_alert_en			= 0;
	input->adv.mem_alert_puimp		= 5;
	input->adv.mem_alert_vref_level		= 0x29;
	input->adv.mem_alert_sync_bypass	= 0;
	input->adv.cal_interval			= 0x9;
	input->adv.cal_once			= 0;
	input->adv.dis_dyn_adr_tri		= 0;
	input->adv.is2ttiming			= 0;
	input->adv.d4rx_preamble_length		= 0;
	input->adv.d4tx_preamble_length		= 0;

	for (i = 0; i < 7; i++)
		debug("mr[%d] = 0x%x\n", i, input->mr[i]);

	debug("input->cs_d0 = 0x%x\n", input->cs_d0);
	debug("input->cs_d1 = 0x%x\n", input->cs_d1);
	debug("input->mirror = 0x%x\n", input->mirror);
	debug("PHY ODT impedance = %d ohm\n", input->adv.odtimpedance);
	debug("PHY DQ driver impedance = %d ohm\n", input->adv.tx_impedance);
	debug("PHY Addr driver impedance = %d ohm\n", input->adv.atx_impedance);

	for (i = 0; i < 4; i++)
		debug("odt[%d] = 0x%x\n", i, input->odt[i]);

	if (input->basic.dimm_type == RDIMM) {
		for (i = 0; i < 16; i++)
			debug("input->rcw[%d] = 0x%x\n", i, input->rcw[i]);
		debug("input->rcw3x = 0x%x\n", input->rcw3x);
	}
}

/*
 * All protocols share the same base structure of mesage block.
 * RDIMM and LRDIMM have more entries defined than UDIMM.
 * Create message blocks for 1D and 2D training.
 * Update len with message block size.
 */
static int phy_gen2_msg_init(void *msg_1d,
			     void *msg_2d,
			     const struct input *input)
{
	struct ddr4u1d *msg_blk = msg_1d;
	struct ddr4u2d *msg_blk_2d = msg_2d;
	struct ddr4r1d *msg_blk_r;
	struct ddr4lr1d *msg_blk_lr;

	switch (input->basic.dimm_type) {
	case UDIMM:
	case SODIMM:
	case NODIMM:
		msg_blk->dram_type	= 0x2;
		break;
	case RDIMM:
		msg_blk->dram_type	= 0x4;
		break;
	case LRDIMM:
		msg_blk->dram_type	= 0x5;
		break;
	default:
		ERROR("Unsupported DIMM type\n");
		return -EINVAL;
	}
	msg_blk->pstate			= 0;

	/*Enable quickRd2D, a substage of read deskew, to 1D training.*/
	msg_blk->reserved00		= 0x20;

	/*Enable High-Effort WrDQ1D.*/
	msg_blk->reserved00             |= 0x40;
	if (input->basic.dimm_type == LRDIMM)
		msg_blk->sequence_ctrl	= 0x3f1f;
	else
		msg_blk->sequence_ctrl	= 0x031f;
	msg_blk->phy_config_override	= 0;
#ifdef DDR_PHY_DEBUG
	msg_blk->hdt_ctrl		= 0x5;
#else
	msg_blk->hdt_ctrl		= 0xc9;
#endif
	msg_blk->msg_misc		= 0x0;
	msg_blk->dfimrlmargin		= 0x1;
	msg_blk->phy_vref		= input->vref ? input->vref : 0x61;
	msg_blk->cs_present		= input->cs_d0 | input->cs_d1;
	msg_blk->cs_present_d0		= input->cs_d0;
	msg_blk->cs_present_d1		= input->cs_d1;
	if (input->mirror)
		msg_blk->addr_mirror	= 0x0a;	/* odd CS are mirrored */
	msg_blk->share2dvref_result	= 1;

	msg_blk->acsm_odt_ctrl0		= input->odt[0];
	msg_blk->acsm_odt_ctrl1		= input->odt[1];
	msg_blk->acsm_odt_ctrl2		= input->odt[2];
	msg_blk->acsm_odt_ctrl3		= input->odt[3];
	msg_blk->enabled_dqs = (input->basic.num_active_dbyte_dfi0 +
				input->basic.num_active_dbyte_dfi1) * 8;
	msg_blk->x16present		= input->basic.dram_data_width == 0x10 ?
					  msg_blk->cs_present : 0;
	msg_blk->d4misc			= 0x1;
	msg_blk->cs_setup_gddec		= 0x1;
	msg_blk->rtt_nom_wr_park0	= 0;
	msg_blk->rtt_nom_wr_park1	= 0;
	msg_blk->rtt_nom_wr_park2	= 0;
	msg_blk->rtt_nom_wr_park3	= 0;
	msg_blk->rtt_nom_wr_park4	= 0;
	msg_blk->rtt_nom_wr_park5	= 0;
	msg_blk->rtt_nom_wr_park6	= 0;
	msg_blk->rtt_nom_wr_park7	= 0;
	msg_blk->mr0			= input->mr[0];
	msg_blk->mr1			= input->mr[1];
	msg_blk->mr2			= input->mr[2];
	msg_blk->mr3			= input->mr[3];
	msg_blk->mr4			= input->mr[4];
	msg_blk->mr5			= input->mr[5];
	msg_blk->mr6			= input->mr[6];
	if (msg_blk->mr4 & 0x1c0)
		ERROR("Setting DRAM CAL mode is not supported\n");

	msg_blk->alt_cas_l		= 0;
	msg_blk->alt_wcas_l		= 0;

	msg_blk->dramfreq		= input->basic.frequency * 2;
	msg_blk->pll_bypass_en		= input->basic.pll_bypass;
	msg_blk->dfi_freq_ratio		= input->basic.dfi_freq_ratio == 0 ? 1 :
					  input->basic.dfi_freq_ratio == 1 ? 2 :
					  4;
	msg_blk->bpznres_val		= input->adv.ext_cal_res_val;
	msg_blk->disabled_dbyte		= 0;

	debug("msg_blk->dram_type = 0x%x\n", msg_blk->dram_type);
	debug("msg_blk->sequence_ctrl = 0x%x\n", msg_blk->sequence_ctrl);
	debug("msg_blk->phy_cfg = 0x%x\n", msg_blk->phy_cfg);
	debug("msg_blk->x16present = 0x%x\n", msg_blk->x16present);
	debug("msg_blk->dramfreq = 0x%x\n", msg_blk->dramfreq);
	debug("msg_blk->pll_bypass_en = 0x%x\n", msg_blk->pll_bypass_en);
	debug("msg_blk->dfi_freq_ratio = 0x%x\n", msg_blk->dfi_freq_ratio);
	debug("msg_blk->phy_odt_impedance = 0x%x\n", msg_blk->phy_odt_impedance);
	debug("msg_blk->phy_drv_impedance = 0x%x\n", msg_blk->phy_drv_impedance);
	debug("msg_blk->bpznres_val = 0x%x\n", msg_blk->bpznres_val);
	debug("msg_blk->enabled_dqs = 0x%x\n", msg_blk->enabled_dqs);
	debug("msg_blk->acsm_odt_ctrl0 = 0x%x\n", msg_blk->acsm_odt_ctrl0);
	debug("msg_blk->acsm_odt_ctrl1 = 0x%x\n", msg_blk->acsm_odt_ctrl1);
	debug("msg_blk->acsm_odt_ctrl2 = 0x%x\n", msg_blk->acsm_odt_ctrl2);
	debug("msg_blk->acsm_odt_ctrl3 = 0x%x\n", msg_blk->acsm_odt_ctrl3);

	/* RDIMM only */
	if (input->basic.dimm_type == RDIMM ||
	    input->basic.dimm_type == LRDIMM) {
		msg_blk_r = (struct ddr4r1d *)msg_blk;
		if (msg_blk_r->cs_present_d0) {
			msg_blk_r->f0rc00_d0 = input->rcw[0];
			msg_blk_r->f0rc01_d0 = input->rcw[1];
			msg_blk_r->f0rc02_d0 = input->rcw[2];
			msg_blk_r->f0rc03_d0 = input->rcw[3];
			msg_blk_r->f0rc04_d0 = input->rcw[4];
			msg_blk_r->f0rc05_d0 = input->rcw[5];
			msg_blk_r->f0rc06_d0 = input->rcw[6];
			msg_blk_r->f0rc07_d0 = input->rcw[7];
			msg_blk_r->f0rc08_d0 = input->rcw[8];
			msg_blk_r->f0rc09_d0 = input->rcw[9];
			msg_blk_r->f0rc0a_d0 = input->rcw[10];
			msg_blk_r->f0rc0b_d0 = input->rcw[11];
			msg_blk_r->f0rc0c_d0 = input->rcw[12];
			msg_blk_r->f0rc0d_d0 = input->rcw[13];
			msg_blk_r->f0rc0e_d0 = input->rcw[14];
			msg_blk_r->f0rc0f_d0 = input->rcw[15];
			msg_blk_r->f0rc3x_d0 = input->rcw3x;
		}
		if (msg_blk_r->cs_present_d1) {
			msg_blk_r->f0rc00_d1 = input->rcw[0];
			msg_blk_r->f0rc01_d1 = input->rcw[1];
			msg_blk_r->f0rc02_d1 = input->rcw[2];
			msg_blk_r->f0rc03_d1 = input->rcw[3];
			msg_blk_r->f0rc04_d1 = input->rcw[4];
			msg_blk_r->f0rc05_d1 = input->rcw[5];
			msg_blk_r->f0rc06_d1 = input->rcw[6];
			msg_blk_r->f0rc07_d1 = input->rcw[7];
			msg_blk_r->f0rc08_d1 = input->rcw[8];
			msg_blk_r->f0rc09_d1 = input->rcw[9];
			msg_blk_r->f0rc0a_d1 = input->rcw[10];
			msg_blk_r->f0rc0b_d1 = input->rcw[11];
			msg_blk_r->f0rc0c_d1 = input->rcw[12];
			msg_blk_r->f0rc0d_d1 = input->rcw[13];
			msg_blk_r->f0rc0e_d1 = input->rcw[14];
			msg_blk_r->f0rc0f_d1 = input->rcw[15];
			msg_blk_r->f0rc3x_d1 = input->rcw3x;
		}
		if (input->basic.dimm_type == LRDIMM) {
			msg_blk_lr = (struct ddr4lr1d *)msg_blk;
			msg_blk_lr->bc0a_d0 = msg_blk_lr->f0rc0a_d0;
			msg_blk_lr->bc0a_d1 = msg_blk_lr->f0rc0a_d1;
			msg_blk_lr->f0bc6x_d0 = msg_blk_lr->f0rc3x_d0;
			msg_blk_lr->f0bc6x_d1 = msg_blk_lr->f0rc3x_d1;
		}
	}

	/* below is different for 1D and 2D message block */
	if (input->basic.train2d) {
		memcpy(msg_blk_2d, msg_blk, sizeof(struct ddr4u1d));
		/*High-Effort WrDQ1D is applicable to 2D traning also*/
		msg_blk_2d->reserved00		|= 0x40;
		msg_blk_2d->sequence_ctrl	= 0x0061;
		msg_blk_2d->rx2d_train_opt	= 1;
		msg_blk_2d->tx2d_train_opt	= 1;
		msg_blk_2d->share2dvref_result	= 1;
		msg_blk_2d->delay_weight2d	= 0x4;
		msg_blk_2d->voltage_weight2d	= 0x1;
		debug("rx2d_train_opt %d, tx2d_train_opt %d\n",
				msg_blk_2d->rx2d_train_opt,
				msg_blk_2d->tx2d_train_opt);
	}

	msg_blk->phy_cfg = ((msg_blk->mr3 & 0x8) || (msg_blk_2d->mr3 & 0x8)) ? 0
				: input->adv.is2ttiming;

	return 0;
}

static void prog_tx_pre_drv_mode(uint16_t *phy,
				 const struct input *input)
{
	int lane, byte, b_addr, c_addr, p_addr;
	int tx_slew_rate, tx_pre_p, tx_pre_n;
	int tx_pre_drv_mode = 0x2;
	uint32_t addr;

	/* Program TxPreDrvMode with 0x2 */
	/* FIXME: TxPreDrvMode depends on DramType? */
	tx_pre_p = input->adv.tx_slew_rise_dq;
	tx_pre_n = input->adv.tx_slew_fall_dq;
	tx_slew_rate = tx_pre_drv_mode << csr_tx_pre_drv_mode_lsb	|
		     tx_pre_p << csr_tx_pre_p_lsb			|
		     tx_pre_n << csr_tx_pre_n_lsb;
	p_addr = 0;
	for (byte = 0; byte < input->basic.num_dbyte; byte++) {
		c_addr = byte << 12;
		for (lane = 0; lane <= 1; lane++) {
			b_addr = lane << 8;
			addr = p_addr | t_dbyte | c_addr | b_addr |
					csr_tx_slew_rate_addr;
			phy_io_write16(phy, addr, tx_slew_rate);
		}
	}
}

static void prog_atx_pre_drv_mode(uint16_t *phy,
				  const struct input *input)
{
	int anib, c_addr;
	int atx_slew_rate, atx_pre_p, atx_pre_n, atx_pre_drv_mode,
		ck_anib_inst[2];
	uint32_t addr;

	atx_pre_n = input->adv.tx_slew_fall_ac;
	atx_pre_p = input->adv.tx_slew_rise_ac;

	if (input->basic.num_anib == 8) {
		ck_anib_inst[0] = 1;
		ck_anib_inst[1] = 1;
	} else if (input->basic.num_anib == 10 || input->basic.num_anib == 12 ||
	    input->basic.num_anib == 13) {
		ck_anib_inst[0] = 4;
		ck_anib_inst[1] = 5;
	} else {
		ERROR("Invalid number of aNIBs: %d\n", input->basic.num_anib);
		return;
	}

	for (anib = 0; anib < input->basic.num_anib; anib++) {
		c_addr = anib << 12;
		if (anib == ck_anib_inst[0] || anib == ck_anib_inst[1])
			atx_pre_drv_mode = 0;
		else
			atx_pre_drv_mode = 3;
		atx_slew_rate = atx_pre_drv_mode << csr_atx_pre_drv_mode_lsb |
				atx_pre_n << csr_atx_pre_n_lsb		     |
				atx_pre_p << csr_atx_pre_p_lsb;
		addr = t_anib | c_addr | csr_atx_slew_rate_addr;
		phy_io_write16(phy, addr, atx_slew_rate);
	}
}

static void prog_enable_cs_multicast(uint16_t *phy,
				     const struct input *input)
{
	uint32_t addr = t_master | csr_enable_cs_multicast_addr;

	if (input->basic.dimm_type != RDIMM &&
	    input->basic.dimm_type != LRDIMM)
		return;

	phy_io_write16(phy, addr, input->adv.cast_cs_to_cid);
}

static void prog_dfi_rd_data_cs_dest_map(uint16_t *phy,
					 unsigned int ip_rev,
					 const struct input *input,
					 const struct ddr4lr1d *msg)
{
	const struct ddr4lr1d *msg_blk;
	uint16_t dfi_xxdestm0 = 0;
	uint16_t dfi_xxdestm1 = 0;
	uint16_t dfi_xxdestm2 = 0;
	uint16_t dfi_xxdestm3 = 0;
	uint16_t dfi_rd_data_cs_dest_map;
	uint16_t dfi_wr_data_cs_dest_map;


#ifdef NXP_ERRATUM_A011396
	/* Only apply to DDRC 5.05.00 */
	if (ip_rev == 0x50500) {
		phy_io_write16(phy, t_master | csr_dfi_rd_data_cs_dest_map_addr, 0);
		return;
	}
#endif

	msg_blk = msg;

	switch (input->basic.dimm_type) {
	case UDIMM:
	case SODIMM:
	case NODIMM:
		if ((msg_blk->msg_misc & 0x40) != 0) {
			dfi_rd_data_cs_dest_map = 0xa0;
			dfi_wr_data_cs_dest_map = 0xa0;

			phy_io_write16(phy,
				t_master | csr_dfi_rd_data_cs_dest_map_addr,
				dfi_rd_data_cs_dest_map);
			phy_io_write16(phy,
				t_master | csr_dfi_wr_data_cs_dest_map_addr,
				dfi_wr_data_cs_dest_map);
		}
		break;
	case LRDIMM:
		if (msg->cs_present_d1) {
			dfi_xxdestm2 = 1;
			dfi_xxdestm3 = 1;
		}

		dfi_rd_data_cs_dest_map = dfi_xxdestm0 << csr_dfi_rd_destm0_lsb	|
			dfi_xxdestm1 << csr_dfi_rd_destm1_lsb	|
			dfi_xxdestm2 << csr_dfi_rd_destm2_lsb	|
			dfi_xxdestm3 << csr_dfi_rd_destm3_lsb;
		dfi_wr_data_cs_dest_map = dfi_xxdestm0 << csr_dfi_wr_destm0_lsb	|
			dfi_xxdestm1 << csr_dfi_wr_destm1_lsb	|
			dfi_xxdestm2 << csr_dfi_wr_destm2_lsb	|
			dfi_xxdestm3 << csr_dfi_wr_destm3_lsb;
		phy_io_write16(phy, t_master | csr_dfi_rd_data_cs_dest_map_addr,
				dfi_rd_data_cs_dest_map);
		phy_io_write16(phy, t_master | csr_dfi_wr_data_cs_dest_map_addr,
				dfi_wr_data_cs_dest_map);

		break;
	default:
		break;
	}
}

static void prog_pll_ctrl(uint16_t *phy,
			   const struct input *input)
{
	uint32_t addr;
	int pll_ctrl1 = 0x21; /* 000100001b */
	int pll_ctrl4 = 0x17f; /* 101111111b */
	int pll_test_mode = 0x24; /* 00100100b */

	addr = t_master | csr_pll_ctrl1_addr;
	phy_io_write16(phy, addr, pll_ctrl1);

	debug("pll_ctrl1 = 0x%x\n", phy_io_read16(phy, addr));

	addr = t_master | csr_pll_test_mode_addr;
	phy_io_write16(phy, addr, pll_test_mode);

	debug("pll_test_mode = 0x%x\n", phy_io_read16(phy, addr));

	addr = t_master | csr_pll_ctrl4_addr;
	phy_io_write16(phy, addr, pll_ctrl4);

	debug("pll_ctrl4 = 0x%x\n", phy_io_read16(phy, addr));
}

static void prog_pll_ctrl2(uint16_t *phy,
			   const struct input *input)
{
	int pll_ctrl2;
	uint32_t addr = t_master | csr_pll_ctrl2_addr;

	if (input->basic.frequency / 2 < 235)
		pll_ctrl2 = 0x7;
	else if (input->basic.frequency / 2 < 313)
		pll_ctrl2 = 0x6;
	else if (input->basic.frequency / 2 < 469)
		pll_ctrl2 = 0xb;
	else if (input->basic.frequency / 2 < 625)
		pll_ctrl2 = 0xa;
	else if (input->basic.frequency / 2 < 938)
		pll_ctrl2 = 0x19;
	else if (input->basic.frequency / 2 < 1067)
		pll_ctrl2 = 0x18;
	else
		pll_ctrl2 = 0x19;

	phy_io_write16(phy, addr, pll_ctrl2);

	debug("pll_ctrl2 = 0x%x\n", phy_io_read16(phy, addr));
}

static void prog_pll_pwr_dn(uint16_t *phy,
			   const struct input *input)
{
	uint32_t addr;

	addr = t_master | csr_pll_pwr_dn_addr;
	phy_io_write16(phy, addr, 0);

	debug("pll_pwrdn = 0x%x\n", phy_io_read16(phy, addr));
}

static void prog_ard_ptr_init_val(uint16_t *phy,
				  const struct input *input)
{
	int ard_ptr_init_val;
	uint32_t addr = t_master | csr_ard_ptr_init_val_addr;

	if (input->basic.frequency >= 933)
		ard_ptr_init_val = 0x2;
	else
		ard_ptr_init_val = 0x1;

	phy_io_write16(phy, addr, ard_ptr_init_val);
}

static void prog_dqs_preamble_control(uint16_t *phy,
				      const struct input *input)
{
	int data;
	uint32_t addr = t_master | csr_dqs_preamble_control_addr;
	const int wdqsextension = 0;
	const int lp4sttc_pre_bridge_rx_en = 0;
	const int lp4postamble_ext = 0;
	const int lp4tgl_two_tck_tx_dqs_pre = 0;
	const int position_dfe_init = 2;
	const int dll_rx_preamble_mode = 1;
	int two_tck_tx_dqs_pre = input->adv.d4tx_preamble_length;
	int two_tck_rx_dqs_pre = input->adv.d4rx_preamble_length;

	data = wdqsextension << csr_wdqsextension_lsb			|
	       lp4sttc_pre_bridge_rx_en << csr_lp4sttc_pre_bridge_rx_en_lsb |
	       lp4postamble_ext << csr_lp4postamble_ext_lsb		|
	       lp4tgl_two_tck_tx_dqs_pre << csr_lp4tgl_two_tck_tx_dqs_pre_lsb |
	       position_dfe_init << csr_position_dfe_init_lsb		|
	       two_tck_tx_dqs_pre << csr_two_tck_tx_dqs_pre_lsb		|
	       two_tck_rx_dqs_pre << csr_two_tck_rx_dqs_pre_lsb;
	phy_io_write16(phy, addr, data);

	data = dll_rx_preamble_mode << csr_dll_rx_preamble_mode_lsb;
	addr = t_master | csr_dbyte_dll_mode_cntrl_addr;
	phy_io_write16(phy, addr, data);
}

static void prog_proc_odt_time_ctl(uint16_t *phy,
				   const struct input *input)
{
	int proc_odt_time_ctl;
	uint32_t addr = t_master | csr_proc_odt_time_ctl_addr;

	if (input->adv.wdqsext) {
		proc_odt_time_ctl = 0x3;
	} else if (input->basic.frequency <= 933) {
		proc_odt_time_ctl = 0xa;
	} else if (input->basic.frequency <= 1200) {
		if (input->adv.d4rx_preamble_length == 1)
			proc_odt_time_ctl = 0x2;
		else
			proc_odt_time_ctl = 0x6;
	} else {
		if (input->adv.d4rx_preamble_length == 1)
			proc_odt_time_ctl = 0x3;
		else
			proc_odt_time_ctl = 0x7;
	}
	phy_io_write16(phy, addr, proc_odt_time_ctl);
}

static const struct impedance_mapping map[] = {
	{	29,	0x3f	},
	{	31,	0x3e	},
	{	33,	0x3b	},
	{	36,	0x3a	},
	{	39,	0x39	},
	{	42,	0x38	},
	{	46,	0x1b	},
	{	51,	0x1a	},
	{	57,	0x19	},
	{	64,	0x18	},
	{	74,	0x0b	},
	{	88,	0x0a	},
	{	108,	0x09	},
	{	140,	0x08	},
	{	200,	0x03	},
	{	360,	0x02	},
	{	481,	0x01	},
	{}
};

static int map_impedance(int strength)
{
	const struct impedance_mapping *tbl = map;
	int val = 0;

	if (strength == 0)
		return 0;

	while (tbl->ohm) {
		if (strength < tbl->ohm) {
			val = tbl->code;
			break;
		}
		tbl++;
	}

	return val;
}

static int map_odtstren_p(int strength, int hard_macro_ver)
{
	int val = -1;

	if (hard_macro_ver == 4) {
		if (strength == 0)
			val = 0;
		else if (strength == 120)
			val = 0x8;
		else if (strength == 60)
			val = 0x18;
		else if (strength == 40)
			val = 0x38;
		else
			printf("error: unsupported ODTStrenP %d\n", strength);
	} else {
		val = map_impedance(strength);
	}

	return val;
}

static void prog_tx_odt_drv_stren(uint16_t *phy,
				  const struct input *input)
{
	int lane, byte, b_addr, c_addr;
	int tx_odt_drv_stren;
	int odtstren_p, odtstren_n;
	uint32_t addr;

	odtstren_p = map_odtstren_p(input->adv.odtimpedance,
				input->basic.hard_macro_ver);
	if (odtstren_p < 0)
		return;

	odtstren_n = 0;	/* always high-z */
	tx_odt_drv_stren = odtstren_n << csr_odtstren_n_lsb | odtstren_p;
	for (byte = 0; byte < input->basic.num_dbyte; byte++) {
		c_addr = byte << 12;
		for (lane = 0; lane <= 1; lane++) {
			b_addr = lane << 8;
			addr = t_dbyte | c_addr | b_addr |
				csr_tx_odt_drv_stren_addr;
			phy_io_write16(phy, addr, tx_odt_drv_stren);
		}
	}
}

static int map_drvstren_fsdq_p(int strength, int hard_macro_ver)
{
	int val = -1;

	if (hard_macro_ver == 4) {
		if (strength == 0)
			val = 0x07;
		else if (strength == 120)
			val = 0x0F;
		else if (strength == 60)
			val = 0x1F;
		else if (strength == 40)
			val = 0x3F;
		else
			printf("error: unsupported drv_stren_fSDq_p %d\n",
			       strength);
	} else {
		val = map_impedance(strength);
	}

	return val;
}

static int map_drvstren_fsdq_n(int strength, int hard_macro_ver)
{
	int val = -1;

	if (hard_macro_ver == 4) {
		if (strength == 0)
			val = 0x00;
		else if (strength == 120)
			val = 0x08;
		else if (strength == 60)
			val = 0x18;
		else if (strength == 40)
			val = 0x38;
		else
			printf("error: unsupported drvStrenFSDqN %d\n",
			       strength);
	} else {
		val = map_impedance(strength);
	}

	return val;
}

static void prog_tx_impedance_ctrl1(uint16_t *phy,
				    const struct input *input)
{
	int lane, byte, b_addr, c_addr;
	int tx_impedance_ctrl1;
	int drv_stren_fsdq_p, drv_stren_fsdq_n;
	uint32_t addr;

	drv_stren_fsdq_p = map_drvstren_fsdq_p(input->adv.tx_impedance,
					input->basic.hard_macro_ver);
	drv_stren_fsdq_n = map_drvstren_fsdq_n(input->adv.tx_impedance,
					input->basic.hard_macro_ver);
	tx_impedance_ctrl1 = drv_stren_fsdq_n << csr_drv_stren_fsdq_n_lsb |
			   drv_stren_fsdq_p << csr_drv_stren_fsdq_p_lsb;
	for (byte = 0; byte < input->basic.num_dbyte; byte++) {
		c_addr = byte << 12;
		for (lane = 0; lane <= 1; lane++) {
			b_addr = lane << 8;
			addr = t_dbyte | c_addr | b_addr |
				csr_tx_impedance_ctrl1_addr;
			phy_io_write16(phy, addr, tx_impedance_ctrl1);
		}
	}
}

static int map_adrv_stren_p(int strength, int hard_macro_ver)
{
	int val = -1;

	if (hard_macro_ver == 4) {
		if (strength == 120)
			val = 0x1c;
		else if (strength == 60)
			val = 0x1d;
		else if (strength == 40)
			val = 0x1f;
		else
			printf("error: unsupported aDrv_stren_p %d\n",
			       strength);
	} else {
		if (strength == 120)
			val = 0x00;
		else if (strength == 60)
			val = 0x01;
		else if (strength == 40)
			val = 0x03;
		else if (strength == 30)
			val = 0x07;
		else if (strength == 24)
			val = 0x0f;
		else if (strength == 20)
			val = 0x1f;
		else
			printf("error: unsupported aDrv_stren_p %d\n",
			       strength);
	}

	return val;
}

static int map_adrv_stren_n(int strength, int hard_macro_ver)
{
	int val = -1;

	if (hard_macro_ver == 4) {
		if (strength == 120)
			val = 0x00;
		else if (strength == 60)
			val = 0x01;
		else if (strength == 40)
			val = 0x03;
		else
			printf("Error: unsupported ADrvStrenP %d\n", strength);
	} else {
		if (strength == 120)
			val = 0x00;
		else if (strength == 60)
			val = 0x01;
		else if (strength == 40)
			val = 0x03;
		else if (strength == 30)
			val = 0x07;
		else if (strength == 24)
			val = 0x0f;
		else if (strength == 20)
			val = 0x1f;
		else
			printf("Error: unsupported ADrvStrenP %d\n", strength);
	}

	return val;
}

static void prog_atx_impedance(uint16_t *phy,
			       const struct input *input)
{
	int anib, c_addr;
	int atx_impedance;
	int adrv_stren_p;
	int adrv_stren_n;
	uint32_t addr;

	if (input->basic.hard_macro_ver == 4 &&
	    input->adv.atx_impedance == 20) {
		printf("Error:ATxImpedance has to be 40 for HardMacroVer 4\n");
		return;
	}

	adrv_stren_p = map_adrv_stren_p(input->adv.atx_impedance,
					input->basic.hard_macro_ver);
	adrv_stren_n = map_adrv_stren_n(input->adv.atx_impedance,
					input->basic.hard_macro_ver);
	atx_impedance = adrv_stren_n << csr_adrv_stren_n_lsb		|
		       adrv_stren_p << csr_adrv_stren_p_lsb;
	for (anib = 0; anib < input->basic.num_anib; anib++) {
		c_addr = anib << 12;
		addr = t_anib | c_addr | csr_atx_impedance_addr;
		phy_io_write16(phy, addr, atx_impedance);
	}
}

static void prog_dfi_mode(uint16_t *phy,
			  const struct input *input)
{
	int dfi_mode;
	uint32_t addr;

	if (input->basic.dfi1exists == 1)
		dfi_mode = 0x5;	/* DFI1 exists but disabled */
	else
		dfi_mode = 0x1;	/* DFI1 does not physically exists */
	addr = t_master | csr_dfi_mode_addr;
	phy_io_write16(phy, addr, dfi_mode);
}

static void prog_dfi_camode(uint16_t *phy,
			    const struct input *input)
{
	int dfi_camode = 2;
	uint32_t addr = t_master | csr_dfi_camode_addr;

	phy_io_write16(phy, addr, dfi_camode);
}

static void prog_cal_drv_str0(uint16_t *phy,
			      const struct input *input)
{
	int cal_drv_str0;
	int cal_drv_str_pd50;
	int cal_drv_str_pu50;
	uint32_t addr;

	cal_drv_str_pu50 = input->adv.ext_cal_res_val;
	cal_drv_str_pd50 = cal_drv_str_pu50;
	cal_drv_str0 = cal_drv_str_pu50 << csr_cal_drv_str_pu50_lsb |
			cal_drv_str_pd50;
	addr = t_master | csr_cal_drv_str0_addr;
	phy_io_write16(phy, addr, cal_drv_str0);
}

static void prog_cal_uclk_info(uint16_t *phy,
			       const struct input *input)
{
	int cal_uclk_ticks_per1u_s;
	uint32_t addr;

	cal_uclk_ticks_per1u_s = input->basic.frequency >> 1;
	if (cal_uclk_ticks_per1u_s < 24)
		cal_uclk_ticks_per1u_s = 24;

	addr = t_master | csr_cal_uclk_info_addr;
	phy_io_write16(phy, addr, cal_uclk_ticks_per1u_s);
}

static void prog_cal_rate(uint16_t *phy,
			  const struct input *input)
{
	int cal_rate;
	int cal_interval;
	int cal_once;
	uint32_t addr;

	cal_interval = input->adv.cal_interval;
	cal_once = input->adv.cal_once;
	cal_rate = cal_once << csr_cal_once_lsb		|
		  cal_interval << csr_cal_interval_lsb;
	addr = t_master | csr_cal_rate_addr;
	phy_io_write16(phy, addr, cal_rate);
}

static void prog_vref_in_global(uint16_t *phy,
				const struct input *input,
				const struct ddr4u1d *msg)
{
	int vref_in_global;
	int global_vref_in_dac = 0;
	int global_vref_in_sel = 0;
	uint32_t addr;

	/*
	 * phy_vref_prcnt = msg->phy_vref / 128.0
	 *  global_vref_in_dac = (phy_vref_prcnt - 0.345) / 0.005;
	 */
	global_vref_in_dac = (msg->phy_vref * 1000 - 345 * 128 + 320) /
			     (5 * 128);

	vref_in_global = global_vref_in_dac << csr_global_vref_in_dac_lsb |
		       global_vref_in_sel;
	addr = t_master | csr_vref_in_global_addr;
	phy_io_write16(phy, addr, vref_in_global);
}

static void prog_dq_dqs_rcv_cntrl(uint16_t *phy,
				  const struct input *input)
{
	int lane, byte, b_addr, c_addr;
	int dq_dqs_rcv_cntrl;
	int gain_curr_adj_defval = 0xb;
	int major_mode_dbyte = 3;
	int dfe_ctrl_defval = 0;
	int ext_vref_range_defval = 0;
	int sel_analog_vref = 1;
	uint32_t addr;

	dq_dqs_rcv_cntrl = gain_curr_adj_defval << csr_gain_curr_adj_lsb |
			major_mode_dbyte << csr_major_mode_dbyte_lsb	|
			dfe_ctrl_defval << csr_dfe_ctrl_lsb		|
			ext_vref_range_defval << csr_ext_vref_range_lsb	|
			sel_analog_vref << csr_sel_analog_vref_lsb;
	for (byte = 0; byte < input->basic.num_dbyte; byte++) {
		c_addr = byte << 12;
		for (lane = 0; lane <= 1; lane++) {
			b_addr = lane << 8;
			addr = t_dbyte | c_addr | b_addr |
					csr_dq_dqs_rcv_cntrl_addr;
			phy_io_write16(phy, addr, dq_dqs_rcv_cntrl);
		}
	}
}

static void prog_mem_alert_control(uint16_t *phy,
				   const struct input *input)
{
	int mem_alert_control;
	int mem_alert_control2;
	int malertpu_en;
	int malertrx_en;
	int malertvref_level;
	int malertpu_stren;
	int malertsync_bypass;
	int malertdisable_val_defval = 1;
	uint32_t addr;

	if (input->basic.dram_type ==  DDR4 && input->adv.mem_alert_en == 1) {
		malertpu_en = 1;
		malertrx_en = 1;
		malertpu_stren = input->adv.mem_alert_puimp;
		malertvref_level = input->adv.mem_alert_vref_level;
		malertsync_bypass = input->adv.mem_alert_sync_bypass;
		mem_alert_control = malertdisable_val_defval << 14	|
				  malertrx_en << 13		|
				  malertpu_en << 12		|
				  malertpu_stren << 8		|
				  malertvref_level;
		mem_alert_control2 = malertsync_bypass <<
					csr_malertsync_bypass_lsb;
		addr = t_master | csr_mem_alert_control_addr;
		phy_io_write16(phy, addr, mem_alert_control);
		addr = t_master | csr_mem_alert_control2_addr;
		phy_io_write16(phy, addr, mem_alert_control2);
	}
}

static void prog_dfi_freq_ratio(uint16_t *phy,
				const struct input *input)
{
	int dfi_freq_ratio;
	uint32_t addr = t_master | csr_dfi_freq_ratio_addr;

	dfi_freq_ratio = input->basic.dfi_freq_ratio;
	phy_io_write16(phy, addr, dfi_freq_ratio);
}

static void prog_tristate_mode_ca(uint16_t *phy,
				  const struct input *input)
{
	int tristate_mode_ca;
	int dis_dyn_adr_tri;
	int ddr2tmode;
	int ck_dis_val_def = 1;
	uint32_t addr = t_master | csr_tristate_mode_ca_addr;

	dis_dyn_adr_tri = input->adv.dis_dyn_adr_tri;
	ddr2tmode = input->adv.is2ttiming;
	tristate_mode_ca = ck_dis_val_def << csr_ck_dis_val_lsb	|
			 ddr2tmode << csr_ddr2tmode_lsb		|
			 dis_dyn_adr_tri << csr_dis_dyn_adr_tri_lsb;
	phy_io_write16(phy, addr, tristate_mode_ca);
}

static void prog_dfi_xlat(uint16_t *phy,
			  const struct input *input)
{
	uint16_t loop_vector;
	int dfifreqxlat_dat;
	int pllbypass_dat;
	uint32_t addr;

	/* fIXME: Shall unused P1, P2, P3 be bypassed? */
	pllbypass_dat = input->basic.pll_bypass; /* only [0] is used */
	for (loop_vector = 0; loop_vector < 8; loop_vector++) {
		if (loop_vector == 0)
			dfifreqxlat_dat = pllbypass_dat + 0x5555;
		else if (loop_vector == 7)
			dfifreqxlat_dat = 0xf000;
		else
			dfifreqxlat_dat = 0x5555;
		addr = t_master | (csr_dfi_freq_xlat0_addr + loop_vector);
		phy_io_write16(phy, addr, dfifreqxlat_dat);
	}
}

static void prog_dbyte_misc_mode(uint16_t *phy,
				 const struct input *input,
				 const struct ddr4u1d *msg)
{
	int dbyte_misc_mode;
	int dq_dqs_rcv_cntrl1;
	int dq_dqs_rcv_cntrl1_1;
	int byte, c_addr;
	uint32_t addr;

	dbyte_misc_mode = 0x1 << csr_dbyte_disable_lsb;
	dq_dqs_rcv_cntrl1 = 0x1ff << csr_power_down_rcvr_lsb		|
			 0x1 << csr_power_down_rcvr_dqs_lsb	|
			 0x1 << csr_rx_pad_standby_en_lsb;
	dq_dqs_rcv_cntrl1_1 = (0x100 << csr_power_down_rcvr_lsb |
			csr_rx_pad_standby_en_mask) ;
	for (byte = 0; byte < input->basic.num_dbyte; byte++) {
		c_addr = byte << 12;
		if (byte <= input->basic.num_active_dbyte_dfi0 - 1) {
			/* disable RDBI lane if not used. */
			if ((input->basic.dram_data_width != 4) &&
				(((msg->mr5 >> 12) & 0x1) == 0)) {
				addr = t_dbyte | c_addr | csr_dq_dqs_rcv_cntrl1_addr;
				phy_io_write16(phy, addr, dq_dqs_rcv_cntrl1_1);
			}
		} else {
			addr = t_dbyte | c_addr | csr_dbyte_misc_mode_addr;
			phy_io_write16(phy, addr, dbyte_misc_mode);
			addr = t_dbyte | c_addr | csr_dq_dqs_rcv_cntrl1_addr;
			phy_io_write16(phy, addr, dq_dqs_rcv_cntrl1);
		}
	}
}

static void prog_master_x4config(uint16_t *phy,
				 const struct input *input)
{
	int master_x4config;
	int x4tg;
	uint32_t addr = t_master | csr_master_x4config_addr;

	x4tg = input->basic.dram_data_width == 4 ? 0xf : 0;
	master_x4config = x4tg << csr_x4tg_lsb;
	phy_io_write16(phy, addr, master_x4config);
}

static void prog_dmipin_present(uint16_t *phy,
				const struct input *input,
				const struct ddr4u1d *msg)
{
	int dmipin_present;
	uint32_t addr = t_master | csr_dmipin_present_addr;

	dmipin_present = (msg->mr5 >> 12) & 0x1;
	phy_io_write16(phy, addr, dmipin_present);
}

static void prog_dfi_phyupd(uint16_t *phy,
			  const struct input *input)
{
	int dfiphyupd_dat;
	uint32_t addr;

	addr = t_master | (csr_dfiphyupd_addr);
	dfiphyupd_dat = phy_io_read16(phy, addr) &
				~csr_dfiphyupd_threshold_mask;

	phy_io_write16(phy, addr, dfiphyupd_dat);
}

static void prog_cal_misc2(uint16_t *phy,
			  const struct input *input)
{
	int cal_misc2_dat, cal_offsets_dat;
	uint32_t addr;

	addr = t_master | (csr_cal_misc2_addr);
	cal_misc2_dat = phy_io_read16(phy, addr) |
			(1 << csr_cal_misc2_err_dis);

	phy_io_write16(phy, addr, cal_misc2_dat);


	addr = t_master | (csr_cal_offsets_addr);
	cal_offsets_dat = (phy_io_read16(phy, addr) & csr_cal_offset_pdc_mask)
			| 0x9;

	phy_io_write16(phy, addr, cal_offsets_dat);
}

static int c_init_phy_config(uint16_t **phy_ptr,
			     unsigned int ip_rev,
			     const struct input *input,
			     const void *msg)
{
	int i;
	uint16_t *phy;
	uint8_t board_rev;

	for (i = 0; i < NUM_OF_DDRC; i++) {
		phy = phy_ptr[i];
		if (!phy)
			continue;

		debug("Initialize PHY %d config\n", i);
		prog_tx_pre_drv_mode(phy, input);
		prog_atx_pre_drv_mode(phy, input);
		prog_enable_cs_multicast(phy, input);	/* rdimm and lrdimm */
		prog_dfi_rd_data_cs_dest_map(phy, ip_rev, input, msg);
		prog_pll_ctrl(phy, input);
		prog_pll_ctrl2(phy, input);
#ifdef  DDR_PLL_FIX
		board_rev = get_board_rev();
		debug("board_rev = %x\n", board_rev);
		if (board_rev == 1) {
			prog_pll_pwr_dn(phy, input);

			/*Enable FFE aka TxEqualizationMode for rev1 SI*/
			phy_io_write16(phy, 0x010048, 0x1);
		}
#endif
		prog_ard_ptr_init_val(phy, input);
		prog_dqs_preamble_control(phy, input);
		prog_proc_odt_time_ctl(phy, input);
		prog_tx_odt_drv_stren(phy, input);
		prog_tx_impedance_ctrl1(phy, input);
		prog_atx_impedance(phy, input);
		prog_dfi_mode(phy, input);
		prog_dfi_camode(phy, input);
		prog_cal_drv_str0(phy, input);
		prog_cal_uclk_info(phy, input);
		prog_cal_rate(phy, input);
		prog_vref_in_global(phy, input, msg);
		prog_dq_dqs_rcv_cntrl(phy, input);
		prog_mem_alert_control(phy, input);
		prog_dfi_freq_ratio(phy, input);
		prog_tristate_mode_ca(phy, input);
		prog_dfi_xlat(phy, input);
		prog_dbyte_misc_mode(phy, input, msg);
		prog_master_x4config(phy, input);
		prog_dmipin_present(phy, input, msg);
		prog_dfi_phyupd(phy, input);
		prog_cal_misc2(phy, input);
	}

	return 0;
}

static uint32_t get_mail(uint16_t *phy, int stream)
{
	int timeout;
	uint32_t mail = 0;

	timeout = TIMEOUTDEFAULT;
	while (--timeout &&
	       (phy_io_read16(phy, t_apbonly | csr_uct_shadow_regs)
		& uct_write_prot_shadow_mask)) {
		mdelay(10);
	}
	if (!timeout) {
		ERROR("Timeout getting mail from PHY\n");
		return 0xFFFF;
	}

	mail = phy_io_read16(phy, t_apbonly |
			     csr_uct_write_only_shadow);
	if (stream) {
		mail |= phy_io_read16(phy, t_apbonly |
				      csr_uct_dat_write_only_shadow) << 16;
	}

	/* Ack */
	phy_io_write16(phy, t_apbonly | csr_dct_write_prot, 0);

	timeout = TIMEOUTDEFAULT;
	while (--timeout &&
	       !(phy_io_read16(phy, t_apbonly | csr_uct_shadow_regs)
		 & uct_write_prot_shadow_mask)) {
		mdelay(1);
	}
	if (!timeout)
		ERROR("Timeout ack PHY mail\n");

	/* completed */
	phy_io_write16(phy, t_apbonly | csr_dct_write_prot, 1);

	return mail;
}

#ifdef DDR_PHY_DEBUG
static const char * lookup_msg(uint32_t index, int train2d)
{
	int i;
	int size;
	const struct phy_msg *messages;
	const char *ptr = NULL;

	if (train2d) {
		messages = messages_2d;
		size = ARRAY_SIZE(messages_2d);
	} else {
		messages = messages_1d;
		size = ARRAY_SIZE(messages_1d);
	}
	for (i = 0; i < size; i++) {
		if (messages[i].index == index) {
			ptr = messages[i].msg;
			break;
		}
	}

	return ptr;
}
#endif

#define MAX_ARGS 32
static void decode_stream_message(uint16_t *phy, int train2d)
{
	uint32_t index __unused;
	__unused const char *format;
	__unused uint32_t args[MAX_ARGS];
	__unused int i;

#ifdef DDR_PHY_DEBUG
	index = get_mail(phy, 1);
	if ((index & 0xffff) > MAX_ARGS)	/* up to MAX_ARGS args so far */
		printf("Program error in %s\n", __func__);
	for (i = 0; i < (index & 0xffff) && i < MAX_ARGS; i++)
		args[i] = get_mail(phy, 1);

	format = lookup_msg(index, train2d);
	if (format) {
		printf("0x%08x: ", index);
		printf(format, args[0], args[1], args[2], args[3], args[4],
		       args[5], args[6], args[7], args[8], args[9], args[10],
		       args[11], args[12], args[13], args[14], args[15],
		       args[16], args[17], args[18], args[19], args[20],
		       args[21], args[22], args[23], args[24], args[25],
		       args[26], args[27], args[28], args[29], args[30],
		       args[31]);
	}
#endif
}

static int wait_fw_done(uint16_t *phy, int train2d)
{
	uint32_t mail = 0;

	while (mail == 0x0) {
		mail = get_mail(phy, 0);
		switch (mail) {
		case 0x7:
			debug("%s Training completed\n", train2d ? "2D" : "1D");
			break;
		case 0xff:
			debug("%s Training failure\n", train2d ? "2D" : "1D");
			break;
		case 0x0:
			debug("End of initialization\n");
			mail = 0;
			break;
		case 0x1:
			debug("End of fine write leveling\n");
			mail = 0;
			break;
		case 0x2:
			debug("End of read enable training\n");
			mail = 0;
			break;
		case 0x3:
			debug("End of read delay center optimization\n");
			mail = 0;
			break;
		case 0x4:
			debug("End of write delay center optimization\n");
			mail = 0;
			break;
		case 0x5:
			debug("End of 2D read delay/voltage center optimization\n");
			mail = 0;
			break;
		case 0x6:
			debug("End of 2D write delay /voltage center optimization\n");
			mail = 0;
			break;
		case 0x8:
			decode_stream_message(phy, train2d);
			mail = 0;
			break;
		case 0x9:
			debug("End of max read latency training\n");
			mail = 0;
			break;
		case 0xa:
			debug("End of read dq deskew training\n");
			mail = 0;
			break;
		case 0xc:
			debug("End of LRDIMM Specific training (DWL, MREP, MRD and MWD)\n");
			mail = 0;
			break;
		case 0xd:
			debug("End of CA training\n");
			mail = 0;
			break;
		case 0xfd:
			debug("End of MPR read delay center optimization\n");
			mail = 0;
			break;
		case 0xfe:
			debug("End of Write leveling coarse delay\n");
			mail = 0;
			break;
		case 0xffff:
			debug("Timed out\n");
			break;
		default:
			mail = 0;
			break;
		}
	}

	if (mail == 0x7)
		return 0;
	else if (mail == 0xff)
		return -EIO;
	else if (mail == 0xffff)
		return -ETIMEDOUT;

	debug("PHY_GEN2 FW: Unxpected mail = 0x%x\n", mail);

	return -EINVAL;
}

static int g_exec_fw(uint16_t **phy_ptr, int train2d)
{
	int ret = -EINVAL;
	int i;
	uint16_t *phy;

	for (i = 0; i < NUM_OF_DDRC; i++) {
		phy = phy_ptr[i];
		if (!phy)
			continue;

		phy_io_write16(phy,
			       t_apbonly | csr_micro_cont_mux_sel_addr,
			       0x1);
		phy_io_write16(phy,
			       t_apbonly | csr_micro_reset_addr,
			       csr_reset_to_micro_mask |
			       csr_stall_to_micro_mask);
		phy_io_write16(phy,
			       t_apbonly | csr_micro_reset_addr,
			       csr_stall_to_micro_mask);
		phy_io_write16(phy,
			       t_apbonly | csr_micro_reset_addr,
			       0);

		/* Enable clocks in case they were disabled. */
		phy_io_write16(phy, t_drtub | csr_ucclk_hclk_enables_addr, 3);

		ret = wait_fw_done(phy, train2d);
		if (ret == -ETIMEDOUT) {
			ERROR("Timed out while waiting for firmware execution on PHY %d\n",
			      i);
		}

		phy_io_write16(phy,
			       t_apbonly | csr_micro_reset_addr,
			       csr_stall_to_micro_mask);
	}

	return ret;
}

static inline int send_fw(uint16_t *phy,
			   uint32_t dst,
			   uint16_t *img,
			   uint32_t size)
{
	int i;

	if (size % 2) {
		ERROR("Wrong image size 0x%x\n", size);
		return -EINVAL;
	}

	for (i = 0; i < size / 2; i++)
		phy_io_write16(phy, dst + i, *(img + i));

	return 0;
}

static int load_fw(uint16_t **phy_ptr,
		   struct input *input,
		   int train2d,
		   void *msg,
		   size_t len)
{
	uint32_t imem_id, dmem_id;
	uintptr_t image_buf;
	uint32_t size;
	int ret;
	int i;
	uint16_t *phy;

	switch(input->basic.dimm_type) {
	case UDIMM:
	case SODIMM:
	case NODIMM:
		imem_id = train2d ? DDR_IMEM_UDIMM_2D_IMAGE_ID :
			  DDR_IMEM_UDIMM_1D_IMAGE_ID;
		dmem_id = train2d ? DDR_DMEM_UDIMM_2D_IMAGE_ID :
			  DDR_DMEM_UDIMM_1D_IMAGE_ID;
		break;
	case RDIMM:
		imem_id = train2d ? DDR_IMEM_RDIMM_2D_IMAGE_ID :
			  DDR_IMEM_RDIMM_1D_IMAGE_ID;
		dmem_id = train2d ? DDR_DMEM_RDIMM_2D_IMAGE_ID :
			  DDR_DMEM_RDIMM_1D_IMAGE_ID;
		break;
	default:
		ERROR("Unsupported DIMM type\n");
		return -EINVAL;
	}

	size = PHY_GEN2_MAX_IMAGE_SIZE;
	image_buf = (uintptr_t)PHY_GEN2_FW_IMAGE_BUFFER;
	mmap_add_dynamic_region(PHY_GEN2_FW_IMAGE_BUFFER,
			PHY_GEN2_FW_IMAGE_BUFFER,
			PHY_GEN2_MAX_IMAGE_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE);
	ret = load_img(imem_id, &image_buf, &size);
	if (ret) {
		ERROR("Failed to load %d firmware.\n", imem_id);
		return ret;
	}
	debug("Loaded Imaged id %d of size %x at address %lx\n", imem_id, size, image_buf);

	for (i = 0; i < NUM_OF_DDRC; i++) {
		phy = phy_ptr[i];
		if (!phy)
			continue;

		/* Initilaizing firmware mailbox */
		phy_io_write16(phy, t_apbonly | csr_dct_write_prot, 0x1);
		phy_io_write16(phy, t_drtub | csr_uct_write_prot, 0x1);

		if (train2d == 0) {
			phy_io_write16(phy, t_master | csr_mem_reset_l_addr,
				       csr_protect_mem_reset_mask);
		}
		/* Enable access to the internal CSRs */
		phy_io_write16(phy, t_apbonly | csr_micro_cont_mux_sel_addr, 0);

		/* Enable clocks in case they were disabled. */
		phy_io_write16(phy, t_drtub | csr_ucclk_hclk_enables_addr, 3);

		ret = send_fw(phy, PHY_GEN2_IMEM_ADDR,
			      (uint16_t *)image_buf, size);
		if (ret)
			return ret;
	}

	size = PHY_GEN2_MAX_IMAGE_SIZE;
	image_buf = (uintptr_t)PHY_GEN2_FW_IMAGE_BUFFER;
	ret = load_img(dmem_id, &image_buf, &size);
	if (ret) {
		ERROR("Failed to load %d firmware.\n", dmem_id);
		return ret;
	}
	debug("Loaded Imaged id %d of size %x at address %lx\n", dmem_id, size, image_buf);
	image_buf += len;
	size -= len;

	for (i = 0; i < NUM_OF_DDRC; i++) {
		phy = phy_ptr[i];
		if (!phy)
			continue;

		ret = send_fw(phy, PHY_GEN2_DMEM_ADDR, msg, len);
		if (ret)
			return ret;

		ret = send_fw(phy, PHY_GEN2_DMEM_ADDR + len / 2,
			      (uint16_t *)image_buf, size);
		if (ret)
			return ret;
	}

	return ret;
}

static void parse_odt(const unsigned int val,
		       const int read,
		       const int i,
		       const unsigned int cs_d0,
		       const unsigned int cs_d1,
		       unsigned int *odt)
{
	int shift = read ? 4 : 0;
	int j;

	if (i < 0 || i > 3) {
		printf("Error: invalid chip-select value\n");
	}
	switch (val) {
	case DDR_ODT_CS:
		odt[i] |= (1 << i) << shift;
		break;
	case DDR_ODT_ALL_OTHER_CS:
		for (j = 0; j < DDRC_NUM_CS; j++) {
			if (i == j)
				continue;
			if (!((cs_d0 | cs_d1) & (1 << j)))
				continue;
			odt[j] |= (1 << i) << shift;
		}
		break;
	case DDR_ODT_CS_AND_OTHER_DIMM:
		odt[i] |= (1 << i) << 4;
		/* fall through */
	case DDR_ODT_OTHER_DIMM:
		for (j = 0; j < DDRC_NUM_CS; j++) {
			if (((cs_d0 & (1 << i)) && (cs_d1 & (1 << j))) ||
			    ((cs_d1 & (1 << i)) && (cs_d0 & (1 << j))))
				odt[j] |= (1 << i) << shift;
		}
		break;
	case DDR_ODT_ALL:
		for (j = 0; j < DDRC_NUM_CS; j++) {
			if (!((cs_d0 | cs_d1) & (1 << j)))
				continue;
			odt[j] |= (1 << i) << shift;
		}
		break;
	case DDR_ODT_SAME_DIMM:
		for (j = 0; j < DDRC_NUM_CS; j++) {
			if (((cs_d0 & (1 << i)) && (cs_d0 & (1 << j))) ||
			    ((cs_d1 & (1 << i)) && (cs_d1 & (1 << j))))
				odt[j] |= (1 << i) << shift;
		}
		break;
	case DDR_ODT_OTHER_CS_ONSAMEDIMM:
		for (j = 0; j < DDRC_NUM_CS; j++) {
			if (i == j)
				continue;
			if (((cs_d0 & (1 << i)) && (cs_d0 & (1 << j))) ||
			    ((cs_d1 & (1 << i)) && (cs_d1 & (1 << j))))
				odt[j] |= (1 << i) << shift;
		}
		break;
	case DDR_ODT_NEVER:
		break;
	default:
		break;
	}
}

int compute_ddr_phy(struct ddr_info *priv)
{
	const unsigned long clk = priv->clk;
	const struct memctl_opt *popts = &priv->opt;
	const struct ddr_conf *conf = &priv->conf;
	const struct dimm_params *dimm_param = &priv->dimm;
	struct ddr_cfg_regs *regs = &priv->ddr_reg;
	int ret;
	static struct input input;
	static struct ddr4u1d msg_1d;
	static struct ddr4u2d msg_2d;
	int i;
	unsigned int odt_rd, odt_wr;

	if (!dimm_param) {
		ERROR("Empty DIMM parameters.\n");
		return -EINVAL;
	}

	zeromem(&input, sizeof(input));
	zeromem(&msg_1d, sizeof(msg_1d));
	zeromem(&msg_2d, sizeof(msg_2d));

	input.basic.dram_type = DDR4;
	/* FIXME: Add condition for LRDIMM */
	input.basic.dimm_type = dimm_param->rdimm ?
				 RDIMM : UDIMM;
	input.basic.num_dbyte = dimm_param->primary_sdram_width / 8 +
				 dimm_param->ec_sdram_width / 8;
	input.basic.num_active_dbyte_dfi0 = input.basic.num_dbyte;
	input.basic.num_rank_dfi0 = dimm_param->n_ranks + 1;
	input.basic.dram_data_width = dimm_param->device_width;
	input.basic.hard_macro_ver	= 0xa;
	input.basic.num_pstates	= 1;
	input.basic.dfi_freq_ratio	= 1;
	input.basic.num_anib		= 0xc;
	input.basic.train2d		= popts->skip2d ? 0 : 1;
	input.basic.frequency = (int) (clk / 2000000ul);
	debug("frequency = %dMHz\n", input.basic.frequency);
	input.cs_d0 = conf->cs_on_dimm[0];
#if DDRC_NUM_DIMM > 1
	input.cs_d1 = conf->cs_on_dimm[1];
#endif
	input.mirror = dimm_param->mirrored_dimm;
	input.mr[0] = regs->sdram_mode[0] & 0xffff;
	input.mr[1] = regs->sdram_mode[0] >> 16;
	input.mr[2] = regs->sdram_mode[1] >> 16;
	input.mr[3] = regs->sdram_mode[1] & 0xffff;
	input.mr[4] = regs->sdram_mode[8] >> 16;
	input.mr[5] = regs->sdram_mode[8] & 0xffff;
	input.mr[6] = regs->sdram_mode[9] >> 16;
	input.vref = popts->vref_phy;
	debug("Vref_phy = %d percent\n", (input.vref * 100) >> 7);
	for (i = 0; i < DDRC_NUM_CS; i++) {
		if (!(regs->cs[i].config & SDRAM_CS_CONFIG_EN))
			continue;
		odt_rd = (regs->cs[i].config >> 20) & 0x7;
		odt_wr = (regs->cs[i].config >> 16) & 0x7;
		parse_odt(odt_rd, true, i, input.cs_d0, input.cs_d1,
			   input.odt);
		parse_odt(odt_wr, false, i, input.cs_d0, input.cs_d1,
			   input.odt);
	}

	/* Do not set sdram_cfg[RD_EN] or sdram_cfg2[RCW_EN] for RDIMM */
	if (dimm_param->rdimm) {
		regs->sdram_cfg[0] &= ~(1 << 28);
		regs->sdram_cfg[1] &= ~(1 << 2);
		input.rcw[0] = (regs->sdram_rcw[0] >> 28) & 0xf;
		input.rcw[1] = (regs->sdram_rcw[0] >> 24) & 0xf;
		input.rcw[2] = (regs->sdram_rcw[0] >> 20) & 0xf;
		input.rcw[3] = (regs->sdram_rcw[0] >> 16) & 0xf;
		input.rcw[4] = (regs->sdram_rcw[0] >> 12) & 0xf;
		input.rcw[5] = (regs->sdram_rcw[0] >> 8) & 0xf;
		input.rcw[6] = (regs->sdram_rcw[0] >> 4) & 0xf;
		input.rcw[7] = (regs->sdram_rcw[0] >> 0) & 0xf;
		input.rcw[8] = (regs->sdram_rcw[1] >> 28) & 0xf;
		input.rcw[9] = (regs->sdram_rcw[1] >> 24) & 0xf;
		input.rcw[10] = (regs->sdram_rcw[1] >> 20) & 0xf;
		input.rcw[11] = (regs->sdram_rcw[1] >> 16) & 0xf;
		input.rcw[12] = (regs->sdram_rcw[1] >> 12) & 0xf;
		input.rcw[13] = (regs->sdram_rcw[1] >> 8) & 0xf;
		input.rcw[14] = (regs->sdram_rcw[1] >> 4) & 0xf;
		input.rcw[15] = (regs->sdram_rcw[1] >> 0) & 0xf;
		input.rcw3x = (regs->sdram_rcw[2] >> 8) & 0xff;
	}

	input.adv.odtimpedance = popts->odt ? popts->odt : 60;
	input.adv.tx_impedance = popts->phy_tx_impedance ? popts->phy_tx_impedance : 28;
	input.adv.atx_impedance = popts->phy_atx_impedance ? popts->phy_atx_impedance : 30;

	debug("Initializing input adv data structure\n");
	phy_gen2_init_input(&input);

	debug("Initializing message block\n");
	ret = phy_gen2_msg_init(&msg_1d, &msg_2d, &input);
	if (ret) {
		ERROR("Init msg failed (error code %d)\n", ret);
		return ret;
	}

	ret = c_init_phy_config(priv->phy, priv->ip_rev, &input, &msg_1d);
	if (ret) {
		ERROR("Init PHY failed (error code %d)\n", ret);
		return ret;
	}

	debug("Load 1D firmware\n");
	ret = load_fw(priv->phy, &input, 0, &msg_1d, sizeof(struct ddr4u1d));
	if (ret) {
		ERROR("Loading firmware failed (error code %d)\n", ret);
		return ret;
	}

	debug("Execute firmware\n");
	ret = g_exec_fw(priv->phy, 0);
	if (ret)
		ERROR("Execution FW failed (error code %d)\n", ret);

	if (!ret && input.basic.train2d) {		/* 2D training starts here */
		debug("Load 2D firmware\n");
		ret = load_fw(priv->phy, &input, 1, &msg_2d,
			      sizeof(struct ddr4u2d));
		if (ret) {
			ERROR("Loading firmware failed (error code %d)\n", ret);
		} else {
			debug("Execute 2D firmware\n");
			ret = g_exec_fw(priv->phy, 1);
		}
	}

	if (!ret) {
		debug("Load PIE\n");
		i_load_pie(priv->phy, &input, &msg_1d);

		NOTICE("DDR4 %s with %d-rank %d-bit bus (x%d)\n",
		       input.basic.dimm_type == RDIMM ? "RDIMM" :
		       input.basic.dimm_type == LRDIMM ? "LRDIMM" :
		       "UDIMM",
		       dimm_param->n_ranks,
		       dimm_param->primary_sdram_width,
		       dimm_param->device_width);
	}

	return ret;
}
