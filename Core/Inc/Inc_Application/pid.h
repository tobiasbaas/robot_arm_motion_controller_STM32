/***************************************************************************
 *  PID-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#ifndef _PID_H
#define _PID_H

#include "parameter.h"
#include "axis.h"

// forward declaration
class axis_class;

class PID_class
{
	private:
		axis_class* 		myAxis;
		parameter_class* 	myParameter;
		error_class* 		myError;

	public:
		PID_class(/* args */);
		bool setParameter(CMD_STRUCT* cmd);
		bool refresh_parameter(void);
		void init(axis_class* Axis_ptr,parameter_class*  myParameter_ptr,error_class* myError_ptr);
		float PIDrun(float position, float slowed_target);
		float PIrun(float position, float slowed_target);
		bool servo_on_off(bool);

		bool servo_on; // flag for servo state

		#define MAX_NUM_PARAMETER_PID 5
		// parameter TYPES: _UINT64, _UINT32, _UINT16, _INT64, _INT32, _INT16, _FLOAT, _DOUBLE, _STRING, _BOOL, _INIT
		PARAM_DEV module_parameter[MAX_NUM_PARAMETER_PID] =
		{
			{0x01, _UINT16, MAX_NUM_PARAMETER_PID	, 0, 0, 	true, "Number of PID Module Parameters"}, // every Module needs this parameter with number of
			{0x10, _FLOAT, 	{.f32=50.0}				, 0, 10000, false, "P-Term"}, // every Module needs this parameter with number of
			{0x11, _FLOAT, 	{.f32=5.0}				, 0, 100, 	false, "I-Term (Tn)"}, // every Module needs this parameter with number of
			{0x12, _FLOAT, 	{.f32=0.0}				, 0, 100, 	false, "D-Term (Tv)"}, // every Module needs this parameter with number of
			{0x15, _FLOAT, 	{.f32=2.0}				, 0, 10, 	false, "Control Out Limit"} // every Module needs this parameter with number of
		};

		float	error;
		float	error_1;
		float	pTerm;
		float	iTerm;
		float	dTerm;
		float	pid_y_P;
		float	pid_y_I;
		float	pid_y_D;
		float	pid_y;
		float	pid_y_max;
};

#endif
