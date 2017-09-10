
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







































#ifndef _VL53L1_HIST_ALGOS_GEN3_H_
#define _VL53L1_HIST_ALGOS_GEN3_H_

#include "vl53l1_types.h"
#include "vl53l1_ll_def.h"

#include "vl53l1_hist_private_structs.h"
#include "vl53l1_dmax_private_structs.h"

#ifdef __cplusplus
extern "C"
{
#endif









void VL53L1_FCTN_00016(
	VL53L1_hist_gen3_algo_private_data_t   *palgo);





















VL53L1_Error VL53L1_FCTN_00017(
	VL53L1_dmax_calibration_data_t         *pdmax_cal,
	VL53L1_hist_gen3_dmax_config_t         *pdmax_cfg,
	VL53L1_hist_post_process_config_t      *ppost_cfg,
	VL53L1_histogram_bin_data_t            *pbins,
	VL53L1_histogram_bin_data_t            *pxtalk,
	VL53L1_hist_gen3_algo_private_data_t   *palgo,
	VL53L1_hist_gen3_dmax_private_data_t   *pdmax_algo,
	VL53L1_range_results_t                 *presults);




























VL53L1_Error VL53L1_FCTN_00018(
	uint16_t                               ambient_threshold_events_scaler,
	int32_t                                ambient_threshold_sigma,
	int32_t                                min_ambient_threshold_events,
	uint8_t                                algo__crosstalk_compensation_enable,
	VL53L1_histogram_bin_data_t           *pbins,
	VL53L1_histogram_bin_data_t           *pxtalk,
	VL53L1_hist_gen3_algo_private_data_t  *palgo);















VL53L1_Error VL53L1_FCTN_00019(
	VL53L1_hist_gen3_algo_private_data_t  *palgo);















VL53L1_Error VL53L1_FCTN_00020(
	VL53L1_hist_gen3_algo_private_data_t  *palgo);













VL53L1_Error VL53L1_FCTN_00021(
	VL53L1_hist_gen3_algo_private_data_t  *palgo);













VL53L1_Error VL53L1_FCTN_00028(
	VL53L1_HistTargetOrder                target_order,
	VL53L1_hist_gen3_algo_private_data_t  *palgo);















VL53L1_Error VL53L1_FCTN_00022(
	uint8_t                                pulse_no,
	VL53L1_histogram_bin_data_t           *pbins,
	VL53L1_hist_gen3_algo_private_data_t  *palgo);
















VL53L1_Error VL53L1_FCTN_00027(
	uint8_t                                pulse_no,
	uint8_t                             clip_events,
	VL53L1_histogram_bin_data_t           *pbins,
	VL53L1_hist_gen3_algo_private_data_t  *palgo);



















VL53L1_Error VL53L1_FCTN_00030(
	int16_t                            VL53L1_PRM_00019,
	int16_t                            VL53L1_PRM_00024,
	uint8_t                            VL53L1_PRM_00030,
	uint8_t                            clip_events,
	VL53L1_histogram_bin_data_t       *pbins,
	uint32_t                          *pphase);

















VL53L1_Error VL53L1_FCTN_00023(
	uint8_t                                pulse_no,
	VL53L1_histogram_bin_data_t           *pbins,
	VL53L1_hist_gen3_algo_private_data_t  *palgo,
	int32_t                                pad_value,
	VL53L1_histogram_bin_data_t           *ppulse);














VL53L1_Error VL53L1_FCTN_00024(
	uint8_t                                pulse_no,
	VL53L1_histogram_bin_data_t           *ppulse,
	VL53L1_hist_gen3_algo_private_data_t  *palgo);















VL53L1_Error VL53L1_FCTN_00025(
	uint8_t                                pulse_no,
	uint16_t                               noise_threshold,
	VL53L1_hist_gen3_algo_private_data_t  *palgo);
















VL53L1_Error VL53L1_FCTN_00031(
	uint8_t   bin,
	int32_t   filta0,
	int32_t   filta1,
	uint8_t   VL53L1_PRM_00030,
	uint32_t *pmedian_phase);




















VL53L1_Error VL53L1_FCTN_00026(
	uint8_t                       bin,
	uint8_t                       sigma_estimator__sigma_ref_mm,
	uint8_t                       VL53L1_PRM_00030,
	uint8_t                       VL53L1_PRM_00055,
	uint8_t                       crosstalk_compensation_enable,
	VL53L1_histogram_bin_data_t  *phist_data_ap,
	VL53L1_histogram_bin_data_t  *phist_data_zp,
	VL53L1_histogram_bin_data_t  *pxtalk_hist,
	uint16_t                     *psigma_est);















void VL53L1_FCTN_00029(
	uint8_t                      range_id,
	uint8_t                      valid_phase_low,
	uint8_t                      valid_phase_high,
	uint16_t                     sigma_thres,
	VL53L1_histogram_bin_data_t *pbins,
	VL53L1_hist_pulse_data_t    *ppulse,
	VL53L1_range_data_t         *pdata);


#ifdef __cplusplus
}
#endif

#endif

