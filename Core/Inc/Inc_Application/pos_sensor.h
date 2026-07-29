/***************************************************************************
 *  Position Sensor-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#ifndef _POS_SENSOR_H
#define _POS_SENSOR_H

#include "parameter.h"
#include "axis.h"
#include "tim.h"

// forward declaration
class axis_class;
class parameter_class;

class pos_sensor_class
{
	private:
		axis_class* 		myAxis;
		parameter_class* 	myParameter;
		error_class* 		myError;
		bool refresh_parameter(void);

	public:
		pos_sensor_class();

		bool setParameter(CMD_STRUCT* cmd);
		bool get_position(void);
		bool get_stepper_position(void);
		bool get_value_AB(void);
		bool get_value_SinCos(void);
		void init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr);

		#define MAX_NUM_PAM_SENSOR 3
		// parameter TYPES: _UINT64, _UINT32, _UINT16, _INT64, _INT32, _INT16, _FLOAT, _DOUBLE, _STRING, _BOOL, _INIT

		PARAM_DEV module_parameter[MAX_NUM_PAM_SENSOR] =
		{
			{0x01, _UINT16, 	MAX_NUM_PAM_SENSOR, 0, 0,			true,	 "Number of Axis Module Parameters"}, // every Module needs this parameter with number of
			{0x10, _UINT32, 	1, 					0, UINT_32_MAX,	false,	 "Numerator"},
			{0x11, _UINT32, 	8192, 		 		0, UINT_32_MAX,	false,	 "Denominator"},
		};


		float		position;
		float		position_offset;
		float		velocity;
		float		velocity_filtered;
		float		velocity_filtered_1;
		float		position_1;
		int32_t 	sensor_raw;	
		int32_t 	sensor_raw_1;
		uint32_t	numerator;
		uint32_t 	denominator;
};

#endif
