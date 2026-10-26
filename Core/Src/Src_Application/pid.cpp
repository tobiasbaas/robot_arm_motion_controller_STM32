/***************************************************************************
 *  PID-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#include "pid.h"

PID_class::PID_class(/* args */)
{
	PID_class::servo_on 	= false;
	PID_class::error 		= 0.0;
	PID_class::error_1 		= 0.0;
	PID_class::pTerm 		= 1.0;
	PID_class::iTerm 		= 1.0;
	PID_class::dTerm 		= 1.0;
	PID_class::pid_y_P 		= 0.0;
	PID_class::pid_y_I 		= 0.0;
	PID_class::pid_y_D 		= 0.0;
	PID_class::pid_y 		= 0.0;
	PID_class::myAxis 		= NULL;
	PID_class::pid_y_max	= 0.0;
}

void PID_class::init(axis_class* Axis_ptr,parameter_class*  myParameter_ptr,error_class* myError_ptr)
{
	this->myAxis 		= Axis_ptr;
	this->myParameter	= myParameter_ptr;
	this->myError		= myError_ptr;
	this->refresh_parameter();
}

bool PID_class::servo_on_off(bool state)
{
	bool ret = false;

	PID_class::pid_y 	= 0.0;
	PID_class::pid_y_I 	= 0.0;
	PID_class::error	= 0.0;
	PID_class::error_1 	= 0.0;
	this->myAxis->myTrajectory->motion_init();

	if(state == true)
	{
		if(this->myAxis->enabled == true)
		{
			this->myAxis->myCurrent_Control->resetFaultLatch();
			//this->myAxis->myCurrent_Control->start_foc_run();
			PID_class::servo_on = true;
			ret = true;
		}
		else
		{
			this->myAxis->myError->add(PARAMETER_MOD, ERR_AXS_DISABLED, __FILE__, __LINE__); // Axis Disabled
			ret = true;
		}

	}
	else if(state == false)
	{
		PID_class::servo_on = false;
		ret = true;
	}
	return ret;
}

bool PID_class::setParameter(CMD_STRUCT* cmd)
{
	bool ret = false;

	if(this->servo_on == false)
	{
		ret = this->myParameter->setParameter(cmd, &this->module_parameter[0]);
		this->refresh_parameter();
	}
	else
	{
		this->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_SVO_ON, __FILE__, __LINE__); // Module Parameter Locked - Servo is Running
	}

	return ret;
}

bool PID_class::refresh_parameter(void)
{
	bool ret = true;

	// refresh parameter, only if no trajectory_class is performed
	if(this->myAxis->enabled == false)
	{
		this->pTerm 		= this->module_parameter[1].value.f32;
		// i-Term, disable for zero
		float tmp 			= this->module_parameter[2].value.f32;
		if(tmp==0)
		{
			this->iTerm 	= 0.0;
		}
		else
		{
			this->iTerm 	= SERVO_AXIS_TIME / tmp; // TN for iterm with servo time
		}
		this->dTerm 		= SERVO_AXIS_FRQ * this->module_parameter[3].value.f32;
		this->pid_y_max		= this->module_parameter[4].value.f32;
	}


	return ret;
}


float PID_class::PIDrun(float position, float slowed_target)
{
	//error
	PID_class::error = slowed_target - position;

	// P-Term
	PID_class::pid_y_P = PID_class::error * PID_class::pTerm;

	// I-Term
	float iTerm_tmp = PID_class::error * PID_class::iTerm;
	PID_class::pid_y_I = PID_class::pid_y_I + iTerm_tmp;

	// D-Term
	PID_class::pid_y_D = (PID_class::error - PID_class::error_1) * PID_class::dTerm;

	// Control out
	PID_class::pid_y = PID_class::pid_y_P + PID_class::pid_y_D + PID_class::pid_y_I;

	// Control out Limiter
		// Positive Limit
	if(PID_class::pid_y > PID_class::pid_y_max)
	{
		PID_class::pid_y = PID_class::pid_y_max;
		// anti Windup (limit i-term in case of contol out limit reached...)
		PID_class::pid_y_I -= iTerm_tmp;
	} // Negative Limit
	else if (PID_class::pid_y < (-1.0 * PID_class::pid_y_max) )
	{
		PID_class::pid_y = -1 * PID_class::pid_y_max;
		// anti Windup (limit i-term in case of contol out limit reached...)
		PID_class::pid_y_I -= iTerm_tmp;
	}

	// last error storage
	PID_class::error_1 = PID_class::error;

	return PID_class::pid_y;
}


