
/*
* This file is part of VL53L1 Platform
*
* Copyright (c) 2016, STMicroelectronics - All Rights Reserved
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/





















#include <stdio.h>

#include <string.h>
#include <stdarg.h>
#include <malloc.h>

#include "vl53l1_platform_log.h"
#include "vl53l1_platform_user_config.h"

#ifdef VL53L1_LOG_ENABLE

	char * _trace_filename = NULL;
	FILE *_tracefile = NULL;

	uint32_t _trace_level     = VL53L1_TRACE_LEVEL_WARNING;
	uint32_t _trace_modules   = VL53L1_TRACE_MODULE_NONE;
	uint32_t _trace_functions = VL53L1_TRACE_FUNCTION_ALL;

	int8_t VL53L1_trace_config(
		char *filename,
		uint32_t modules,
		uint32_t level,
		uint32_t functions)
	{
		int8_t status = 0;




























		if (((filename != NULL) && (_tracefile == NULL)) && strcmp(filename,""))
		{
			_tracefile = fopen(filename, "w+");




			if ( _tracefile != NULL )
			{
				_trace_filename = (char*)malloc((strlen(filename) + 1) * sizeof(char));
				strcpy(_trace_filename, filename);
			}
			else
			{
				printf("VL53L1_trace_config(): failed to open log file (%s)\n", filename);
				status = 1;
			}
		}

		_trace_modules   = modules;
		_trace_level     = level;
		_trace_functions = functions;

		return status;
	}

	void VL53L1_trace_print_module_function(uint32_t module, uint32_t level, uint32_t function, const char *format, ...)
	{
		if ( ((level <=_trace_level) && ((module & _trace_modules) > 0))
			|| ((function & _trace_functions) > 0) )
		{
			va_list arg_list;
			char message[VL53L1_MAX_STRING_LENGTH];

			va_start(arg_list, format);
			vsnprintf(message, VL53L1_MAX_STRING_LENGTH-1, format, arg_list);

			va_end(arg_list);

			if (_tracefile != NULL)
			{
				fprintf(_tracefile, message);

			}
			else
			{
				printf(message);

			}









		}

	}


	uint32_t VL53L1_get_trace_functions(void)
	{
		return _trace_functions;
	}


	void VL53L1_set_trace_functions(uint32_t function)
	{
		_trace_functions = function;
	}


	uint32_t VL53L1_clock(void)
	{


		uint32_t tick_count_ms = (uint32_t)clock();
		return tick_count_ms;
	}
#endif


