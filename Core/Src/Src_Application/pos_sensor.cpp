/***************************************************************************
 *  Position Sensor-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#include "pos_sensor.h"

pos_sensor_class::pos_sensor_class(void)
{
	this->position 				= 0.0;
	this->velocity 				= 0.0;
	this->velocity_filtered 	= 0.0;
	this->velocity_filtered_1	= 0.0;
	this->position_1 			= 0.0;
	this->sensor_raw			= 0;
	this->sensor_raw_1			= 0;
	this->numerator				= 0;
	this->denominator			= 0;
	this->position_offset		= 0;
}

void pos_sensor_class::init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr)
{
	this->myParameter	= myParameter_ptr;
	this->myError		= myError_ptr;
	this->myAxis 		= Axis_ptr;
	this->refresh_parameter();
}

bool pos_sensor_class::setParameter(CMD_STRUCT* cmd)
{
	bool ret;

	if(this->myAxis->enabled == false)
	{
		ret = this->myParameter->setParameter(cmd, &this->module_parameter[0]);
		this->refresh_parameter();
	}
	else
	{
		this->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_AXS_ON, __FILE__, __LINE__); // Module Parameter Locked - Servo is Running
	}
	return ret;
}

bool pos_sensor_class::refresh_parameter(void)
{
	bool ret = true;

	// refresh parameter, only if no trajectory_class is performed
	if(this->myAxis->enabled == false)
	{
		this->numerator			= this->module_parameter[1].value.ui32;
		this->denominator		= this->module_parameter[2].value.ui32;
	}
	else
	{
		this->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_AXS_ON, __FILE__, __LINE__); // Parameter update not allowed, motion in progress
	}
	return ret;
}

bool pos_sensor_class::get_position(void)
{
	bool ret = true;
	ret = pos_sensor_class::get_value_AB(); // get raw sensor value in "sensor_raw"


	this->position = (float)this->sensor_raw * (float)this->numerator / (float)this->denominator + this->position_offset;
	this->velocity = (this->position - this->position_1) * SERVO_FRQ;
	this->velocity_filtered_1 = this->velocity_filtered;
	this->velocity_filtered = 0.98 *this->velocity_filtered_1 + 0.02 * this->velocity;
	this->position_1 = this->position;
	
	return ret;
}

// if no position sensor available
bool pos_sensor_class::get_stepper_position(void)
{
	this->position = myAxis->myCurrent_Control->getPosition(); // z. B. in Rad
	this->velocity = (this->position - this->position_1) * SERVO_FRQ;
	this->velocity_filtered_1 = this->velocity_filtered;
	this->velocity_filtered = 0.98 *this->velocity_filtered_1 + 0.02 * this->velocity;
	this->position_1 = this->position;

	return 0;
}

bool pos_sensor_class::get_value_AB(void)
{
	bool ret = true;
	// get value from Hardware ... need to be adapted to HARDWARE Register TIMER?
	this->sensor_raw_1 = this->sensor_raw;
	this->sensor_raw =  __HAL_TIM_GetCounter(&htim2);
	return ret;
}

bool pos_sensor_class::get_value_SinCos(void)
{
	bool ret = true;
	// get value from Hardware (TW29 by SPI)
	this->sensor_raw = 0;

	return ret;
}
