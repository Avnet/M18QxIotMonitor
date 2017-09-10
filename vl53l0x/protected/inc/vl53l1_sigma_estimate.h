
/*
* This file is part of VL53L1 Protected
*
* Copyright (C) 2016, STMicroelectronics - All Rights Reserved
*
* License terms: STMicroelectronics Proprietary in accordance with licensing
* terms at www.st.com/sla0044
*
* STMicroelectronics confidential
* Reproduction and Communication of this document is strictly prohibited unless
* specifically authorized in writing by STMicroelectronics.
*
*/






































#ifndef _VL53L1_SIGMA_ESTIMATE_H_
#define _VL53L1_SIGMA_ESTIMATE_H_

#include "vl53l1_types.h"
#include "vl53l1_ll_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  VL53L1_DEF_00002  0xFFFF




















uint16_t VL53L1_FCTN_00042(
	uint8_t	 sigma_estimator__effective_pulse_width_ns,
	uint8_t  sigma_estimator__effective_ambient_width_ns,
	uint8_t	 sigma_estimator__sigma_ref_mm,
	VL53L1_range_data_t  *pdata);



















uint16_t VL53L1_FCTN_00044(
	uint8_t	 sigma_estimator__effective_pulse_width_ns,
	uint8_t  sigma_estimator__effective_ambient_width_ns,
	uint8_t	 sigma_estimator__sigma_ref_mm,
	VL53L1_range_data_t *pdata);



























VL53L1_Error  VL53L1_FCTN_00014(
	uint8_t	      sigma_estimator__sigma_ref_mm,
	uint32_t      VL53L1_PRM_00002,
	uint32_t      VL53L1_PRM_00032,
	uint32_t      VL53L1_PRM_00001,
	uint32_t      a_zp,
	uint32_t      c_zp,
	uint32_t      bx,
	uint32_t      ax_zp,
	uint32_t      cx_zp,
	uint32_t      VL53L1_PRM_00028,
	uint16_t      fast_osc_frequency,
	uint16_t      *psigma_est);

















uint32_t VL53L1_FCTN_00043(
	uint32_t  VL53L1_PRM_00002,
	uint32_t  VL53L1_PRM_00032);


#ifdef __cplusplus
}
#endif

#endif


