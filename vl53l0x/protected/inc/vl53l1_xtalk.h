
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









































#ifndef _VL53L1_XTALK_H_
#define _VL53L1_XTALK_H_

#include "vl53l1_types.h"
#include "vl53l1_ll_def.h"
#include "vl53l1_xtalk_private_structs.h"

#ifdef __cplusplus
extern "C" {
#endif
















VL53L1_Error VL53L1_xtalk_calibration_process_data(
	VL53L1_xtalk_range_results_t        *pxtalk_ranges,
	VL53L1_xtalk_histogram_data_t       *pxtalk_shape,
	VL53L1_xtalk_calibration_results_t  *pxtalk_cal);


















VL53L1_Error VL53L1_FCTN_00047(
		VL53L1_histogram_bin_data_t        *pavg_bins,
		VL53L1_xtalk_algo_data_t           *pdebug,
		VL53L1_xtalk_range_data_t          *pxtalk_data,
		uint8_t                             histogram__window_start,
		uint8_t                             histogram__window_end,
		VL53L1_xtalk_histogram_shape_t     *pxtalk_shape);

















VL53L1_Error VL53L1_FCTN_00045(
	VL53L1_xtalk_range_results_t  *pxtalk_results,
	VL53L1_xtalk_algo_data_t      *pdebug,
	int16_t                       *xgradient,
	int16_t                       *ygradient);
















VL53L1_Error VL53L1_FCTN_00046(
	VL53L1_xtalk_range_data_t *pxtalk_data,
	VL53L1_xtalk_algo_data_t  *pdebug,
	uint16_t                  *xtalk_mean_offset_kcps);


















VL53L1_Error VL53L1_FCTN_00051(
	VL53L1_histogram_bin_data_t	   *phist_data,
	VL53L1_xtalk_range_data_t      *pxtalk_data,
	VL53L1_xtalk_algo_data_t       *pdebug,
	VL53L1_xtalk_histogram_shape_t *pxtalk_histo);
























VL53L1_Error VL53L1_FCTN_00040(
	uint16_t                       mean_offset,
	int16_t                        xgradient,
	int16_t                        ygradient,
	int8_t                         centre_offset_x,
	int8_t                         centre_offset_y,
	uint16_t                       roi_effective_spads,
	uint8_t                        roi_centre_spad,
	uint8_t                        roi_xy_size,
	uint32_t                      *xtalk_rate_kcps);

















VL53L1_Error VL53L1_FCTN_00041(
	VL53L1_histogram_bin_data_t    *phist_data,
	VL53L1_xtalk_histogram_shape_t *pxtalk_data,
	uint32_t                        xtalk_rate_kcps,
	VL53L1_histogram_bin_data_t    *pxtalkcount_data);















VL53L1_Error VL53L1_FCTN_00053(
	VL53L1_histogram_bin_data_t   *phist_data,
	VL53L1_histogram_bin_data_t   *pxtalk_data,
	uint8_t                        xtalk_bin_offset);

















VL53L1_Error VL53L1_FCTN_00050(
	VL53L1_histogram_bin_data_t       *pxtalk_data,
	uint32_t                           amb_threshold,
	uint8_t                            VL53L1_PRM_00019,
	uint8_t                            VL53L1_PRM_00024);



















VL53L1_Error VL53L1_FCTN_00052(
	VL53L1_customer_nvm_managed_t *pcustomer,
	VL53L1_dynamic_config_t       *pdyn_cfg,
	VL53L1_xtalk_histogram_data_t *pxtalk_shape,
	VL53L1_histogram_bin_data_t   *pip_hist_data,
	VL53L1_histogram_bin_data_t   *pop_hist_data,
	VL53L1_histogram_bin_data_t   *pxtalk_count_data);
















VL53L1_Error VL53L1_FCTN_00049(
		uint8_t                      sigma_mult,
		int32_t                      VL53L1_PRM_00028,
		uint32_t                    *ambient_noise);



















VL53L1_Error VL53L1_generate_dual_reflectance_xtalk_samples (
	VL53L1_xtalk_range_results_t *pxtalk_results,
	uint16_t                      expected_target_distance_mm,
	uint8_t                       higher_reflectance,
	VL53L1_histogram_bin_data_t  *pxtalk_avg_samples
);































VL53L1_Error VL53L1_FCTN_00048(
		VL53L1_histogram_bin_data_t			*pzone_avg_1,
		VL53L1_histogram_bin_data_t 		*pzone_avg_2,
		uint16_t                             expected_target_distance,
		uint8_t                              subtract_amb,
		uint8_t                              higher_reflectance,
		VL53L1_histogram_bin_data_t			*pxtalk_output
);

#ifdef __cplusplus
}
#endif

#endif


