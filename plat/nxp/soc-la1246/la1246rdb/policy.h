/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Parth Girishkumar Bera <parthgirishkumar.bera@nxp.com>
 */

/*---------------------------------------------------------------------------*/

#ifndef _POLICY_H
#define	_POLICY_H

 // the following defines affect the PLATFORM SECURITY POLICY

 // set this to 0x0 if the platform is not using/responding to ECC errors
 // set this to 0x1 if ECC is being used (we have to do some init)
#define  POLICY_USING_ECC 0x0

 // Set this to 0x0 to leave the default SMMU page size in sACR
 // Set this to 0x1 to change the SMMU page size to 64K
#define POLICY_SMMU_PAGESZ_64K 0x1

//-----------------------------------------------------------------------------

#endif // _POLICY_H
