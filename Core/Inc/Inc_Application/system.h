/*
 * System.h
 *
 *  Created on: Mar 29, 2025
 *      Author: tobiasbaas
 */

#ifndef INC_CPPCODE_SYSTEM_H_
#define INC_CPPCODE_SYSTEM_H_

#include "typedefs.h"
#include "parameter.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



class system_class
{	private:


	public:
		bool init(void);
		system_class(parameter_class * myParameter_ptr);
		bool refresh_parameter(void);
		bool setParameter(CMD_STRUCT* cmd);

		char 				device_name[8];
		uint16_t			device_serial_number;
		parameter_class * 	myParameter;

		#define MAX_NUM_PARAMETER_SYSTEM 5
		PARAM_DEV module_parameter[MAX_NUM_PARAMETER_SYSTEM] =
		{
			{0x01, _UINT16, 	MAX_NUM_PARAMETER_SYSTEM, 	MAX_NUM_PARAMETER_SYSTEM, 	MAX_NUM_PARAMETER_SYSTEM, 	true, "Number of Parameters in System Module"}, // every Module needs this parameter with number of
			{0x10, _STRING, 	{.str8="NEW"}, 				0, 							1, 							false, "Controller Name (8 Signs Max.)"},
			{0x11, _UINT32, 	{.ui32=0}, 					0, 							999999, 					false, "Device Serial Number"},
			{0x12, _UINT16, 	{.ui16=MAX_NUM_AXIS}, 		MAX_NUM_AXIS, 				MAX_NUM_AXIS, 				true, "Number of Controller Axis"},
			{0x13, _FLOAT, 		{.f32=SERVO_AXIS_FRQ}, 		1000, 						20000, 						true, "Axis Loop Frequency"}
		};
};

#endif /* INC_CPPCODE_SYSTEM_H_ */
