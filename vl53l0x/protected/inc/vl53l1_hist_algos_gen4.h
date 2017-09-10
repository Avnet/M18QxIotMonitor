
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







































#ifndef _VL53L1_HIST_ALGOS_GEN4_H_
#define _VL53L1_HIST_ALGOS_GEN4_H_

#include "vl53l1_types.h"
#include "vl53l1_ll_def.h"

#include "vl53l1_hist_private_structs.h"
#include "vl53l1_dmax_private_structs.h"


#ifdef __cplusplus
extern "C"
{
#endif









void VL53L1_FCTN_00032(
	VL53L1_hist_gen4_algo_filtered_data_t  *palgo);






















VL53L1_Error VL53L1_FCTN_00033(
	VL53L1_dmax_calibration_data_t         *pdmax_cal,
	VL53L1_hist_gen3_dmax_config_t         *pdmax_cfg,
	VL53L1_hist_post_process_config_t      *ppost_cfg,
	VL53L1_histogram_bin_data_t            *pbins,
	VL53L1_histogram_bin_data_t            *pxtalk,
	VL53L1_hist_gen3_algo_private_data_t   *palgo,
	VL53L1_hist_gen4_algo_filtered_data_t  *pfiltered,
	VL53L1_hist_gen3_dmax_private_data_t   *pdmax_algo,
	VL53L1_range_results_t                 *presults);
















VL53L1_Error VL53L1_FCTN_00034(
	uint8_t                                pulse_no,
	VL53L1_histogram_bin_data_t           *ppulse,
	VL53L1_hist_gen3_algo_private_data_t  *palgo,
	VL53L1_hist_gen4_algo_filtered_data_t *pfiltered);
















VL53L1_Error VL53L1_FCTN_00035(
	uint8_t                                pulse_no,
	uint16_t                               noise_threshold,
	VL53L1_hist_gen4_algo_filtered_data_t *pfiltered,
	VL53L1_hist_gen3_algo_private_data_t  *palgo);





















VL53L1_Error VL53L1_FCTN_00036(
	uint8_t   bin,
	int32_t   VL53L1_PRM_00002,
	int32_t   VL53L1_PRM_00032,
	int32_t   VL53L1_PRM_00001,
	int32_t   ax,
	int32_t   bx,
	int32_t   cx,
	int32_t   VL53L1_PRM_00028,
	uint8_t   VL53L1_PRM_00030,
	uint32_t *pmedian_phase);


#ifdef __cplusplus
}
#endif

#endif

