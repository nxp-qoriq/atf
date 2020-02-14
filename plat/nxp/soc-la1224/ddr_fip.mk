#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#

ifeq (${DDR_IMEM_UDIMM_1D},)
    DDR_IMEM_UDIMM_1D	:=	ddr4_pmu_train_imem.bin
endif

ifeq (${DDR_IMEM_UDIMM_2D},)
    DDR_IMEM_UDIMM_2D	:=	ddr4_2d_pmu_train_imem.bin
endif

ifeq (${DDR_DMEM_UDIMM_1D},)
    DDR_DMEM_UDIMM_1D	:=	ddr4_pmu_train_dmem.bin
endif

ifeq (${DDR_DMEM_UDIMM_2D},)
    DDR_DMEM_UDIMM_2D	:=	ddr4_2d_pmu_train_dmem.bin
endif

ifeq (${DDR_IMEM_RDIMM_1D},)
    DDR_IMEM_RDIMM_1D	:=	ddr4_rdimm_pmu_train_imem.bin
endif

ifeq (${DDR_IMEM_RDIMM_2D},)
    DDR_IMEM_RDIMM_2D	:=	ddr4_rdimm2d_pmu_train_imem.bin
endif

ifeq (${DDR_DMEM_RDIMM_1D},)
    DDR_DMEM_RDIMM_1D	:=	ddr4_rdimm_pmu_train_dmem.bin
endif

ifeq (${DDR_DMEM_RDIMM_2D},)
    DDR_DMEM_RDIMM_2D	:=	ddr4_rdimm2d_pmu_train_dmem.bin
endif

fip_ddr: fiptool
	./tools/fiptool/fiptool create  --ddr-immem-udimm-1d ${DDR_IMEM_UDIMM_1D} \
				        --ddr-immem-udimm-2d ${DDR_IMEM_UDIMM_2D} \
					--ddr-dmmem-udimm-1d ${DDR_DMEM_UDIMM_1D} \
					--ddr-dmmem-udimm-2d ${DDR_DMEM_UDIMM_2D} \
					--ddr-immem-rdimm-1d ${DDR_IMEM_RDIMM_1D} \
				        --ddr-immem-rdimm-2d ${DDR_IMEM_RDIMM_2D} \
					--ddr-dmmem-rdimm-1d ${DDR_DMEM_RDIMM_1D} \
					--ddr-dmmem-rdimm-2d ${DDR_DMEM_RDIMM_2D} $@.bin

ifeq (${TRUSTED_BOARD_BOOT},1)
	
UDIMM_DEPS = ${DDR_IMEM_UDIMM_1D}.sb ${DDR_IMEM_UDIMM_2D}.sb ${DDR_DMEM_UDIMM_1D}.sb ${DDR_DMEM_UDIMM_2D}.sb
RDIMM_DEPS = ${DDR_IMEM_RDIMM_1D}.sb ${DDR_IMEM_RDIMM_2D}.sb ${DDR_DMEM_RDIMM_1D}.sb ${DDR_DMEM_RDIMM_2D}.sb

fip_ddr_sec: fiptool ${RDIMM_DEPS} ${UDIMM_DEPS}
	./tools/fiptool/fiptool create  --ddr-immem-udimm-1d ${DDR_IMEM_UDIMM_1D}.sb \
				        --ddr-immem-udimm-2d ${DDR_IMEM_UDIMM_2D}.sb \
					--ddr-dmmem-udimm-1d ${DDR_DMEM_UDIMM_1D}.sb \
					--ddr-dmmem-udimm-2d ${DDR_DMEM_UDIMM_2D}.sb \
					--ddr-immem-rdimm-1d ${DDR_IMEM_RDIMM_1D}.sb \
				        --ddr-immem-rdimm-2d ${DDR_IMEM_RDIMM_2D}.sb \
					--ddr-dmmem-rdimm-1d ${DDR_DMEM_RDIMM_1D}.sb \
					--ddr-dmmem-rdimm-2d ${DDR_DMEM_RDIMM_2D}.sb $@.bin

# Max Size of CSF header. image will be appended at this offset of header
CSF_HDR_SZ	?= 0x3000

# Path to CST directory is required to generate the CSF header
# and prepend it to image before fip image gets generated
ifeq (${CST_DIR},)
  $(error Error: CST_DIR not set)
endif

ifeq (${DDR_INPUT_FILE},)
DDR_INPUT_FILE:= plat/nxp/drivers/auth/csf_hdr_parser/${CSF_FILE}
endif

${DDR_IMEM_UDIMM_1D}_sb: ${DDR_IMEM_UDIMM_1D}
	@echo " Generating CSF Header for $@ $<"
	$(CST_DIR)/create_hdr_esbc --in $< --out $@ --app_off ${CSF_HDR_SZ} \
					--app $< ${DDR_INPUT_FILE}


%.sb: %
	@echo " Generating CSF Header for $@ $<"
	$(CST_DIR)/create_hdr_esbc --in $< --out $@ --app_off ${CSF_HDR_SZ} \
					--app $< ${DDR_INPUT_FILE}

endif
