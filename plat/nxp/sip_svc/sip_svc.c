/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author Sumit Garg <sumit.garg@nxp.com>
 */

#include <string.h>
#include <assert.h>
#include <console.h>
#include <debug.h>
#include <sipsvc.h>
#include <runtime_svc.h>
#include <uuid.h>
#include <fsl_sec.h>
#include <io.h>
#include <mmio.h>
#include <plat_common.h>

/* Layerscape SiP Service UUID */
DEFINE_SVC_UUID(ls_sip_svc_uid,
		0x871de4ef, 0xedfc, 0x4209, 0xa4, 0x23,
		0x8d, 0x23, 0x75, 0x9d, 0x3b, 0x9f);

#pragma weak ls_plat_sip_handler
uint64_t ls_plat_sip_handler(uint32_t smc_fid,
				uint64_t x1,
				uint64_t x2,
				uint64_t x3,
				uint64_t x4,
				void *cookie,
				void *handle,
				uint64_t flags)
{
	ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

uint64_t el2_2_aarch32(u_register_t smc_id, u_register_t start_addr,
			u_register_t parm1, u_register_t parm2);

uint64_t prefetch_disable(u_register_t smc_id, u_register_t mask);
uint64_t bl31_get_porsr1(void);

static void clean_top_32b_of_param(uint32_t smc_fid,
				   uint64_t *px1,
				   uint64_t *px2,
				   uint64_t *px3,
				   uint64_t *px4)
{
	/* if parameters from SMC32. Clean top 32 bits */
	if (GET_SMC_CC(smc_fid) == SMC_32) {
		*px1 = *px1 & SMC32_PARAM_MASK;
		*px2 = *px2 & SMC32_PARAM_MASK;
		*px3 = *px3 & SMC32_PARAM_MASK;
		*px4 = *px4 & SMC32_PARAM_MASK;
	}
}

/* This function handles Layerscape defined SiP Calls */
uint64_t ls_sip_handler(uint32_t smc_fid,
			uint64_t x1,
			uint64_t x2,
			uint64_t x3,
			uint64_t x4,
			void *cookie,
			void *handle,
			uint64_t flags)
{
	uint32_t ns;
	uint64_t ret;
	dram_regions_info_t *info_dram_regions;

	/* if parameter is sent from SMC32. Clean top 32 bits */
	clean_top_32b_of_param(smc_fid, &x1, &x2, &x3, &x4);

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns) {
		/* SiP SMC service secure world's call */
		;
	} else {
		/* SiP SMC service normal world's call */
		;
	}

	switch (smc_fid & SMC_FUNC_MASK) {
	case SIP_SVC_RNG:
		if (CHECK_SEC_DISABLED != 0) {
			NOTICE("SEC is disabled.\n");
			SMC_RET1(handle, SMC_UNK);
		}

		/* Return zero on failure */
		ret = get_random((int)x1);
		if (ret) {
			SMC_RET2(handle, SMC_OK, ret);
		} else {
			SMC_RET1(handle, SMC_UNK);
		}
	case SIP_SVC_HUK:
		if (CHECK_SEC_DISABLED != 0) {
			INFO("SEC is disabled.\n");
			SMC_RET1(handle, SMC_UNK);
		}
		ret = get_hw_unq_key_blob_hw((uint8_t *) x1, (uint32_t) x2);

		if (ret == SMC_OK) {
			SMC_RET1(handle, SMC_OK);
		} else {
			SMC_RET1(handle, SMC_UNK);
		}
	case SIP_SVC_MEM_BANK:
		VERBOSE("Handling SMC SIP_SVC_MEM_BANK.\n");
		info_dram_regions = get_dram_regions_info();

		if (x1 == -1) {
			SMC_RET2(handle, SMC_OK,
					info_dram_regions->total_dram_size);
		} else if (x1 >= info_dram_regions->num_dram_regions) {
			SMC_RET1(handle, SMC_UNK);
		} else {
			SMC_RET3(handle, SMC_OK,
				info_dram_regions->region[x1].addr,
				info_dram_regions->region[x1].size);
		}
	case SIP_SVC_PREFETCH_DIS:
		VERBOSE("In SIP_SVC_PREFETCH_DIS call\n");
		ret = prefetch_disable(smc_fid, x1);
		if (ret == SMC_OK) {
			SMC_RET1(handle, SMC_OK);
		} else {
			SMC_RET1(handle, SMC_UNK);
		}
	case SIP_SVC_2_AARCH32:
		ret = el2_2_aarch32(smc_fid, x1, x2, x3);

		/* In success case, control should not reach here. */
		NOTICE("SMC: SIP_SVC_2_AARCH32 Failed.\n");
		SMC_RET1(handle, SMC_UNK);
	case SIP_SVC_PORSR1:
		ret = bl31_get_porsr1();
		SMC_RET2(handle, SMC_OK, ret);
	default :
		return ls_plat_sip_handler(smc_fid, x1, x2, x3, x4,
				cookie, handle, flags);
	}
}

/* This function is responsible for handling all SiP calls */
uint64_t sip_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	switch (smc_fid & SMC_FUNC_MASK) {
	case SIP_SVC_CALL_COUNT:
		/* Return the number of Layerscape SiP Service Calls. */
		SMC_RET1(handle, LS_COMMON_SIP_NUM_CALLS);
		break;
	case SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, ls_sip_svc_uid);
		break;
	case SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, LS_SIP_SVC_VERSION_MAJOR,
			 LS_SIP_SVC_VERSION_MINOR);
		break;
	default:
		return ls_sip_handler(smc_fid, x1, x2, x3, x4,
				      cookie, handle, flags);
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	ls_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	sip_smc_handler
);
