/***************************************************************************
 *  Current Control-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#include "current_control.h"
#include "typedefs.h"
#include <math.h>


current_control_class::current_control_class()
{
}

void current_control_class::init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr, set_pwm_class* myPWM_ptr, trajectory_class* trajectory)
{
	this->myParameter 		= myParameter_ptr;
	this->myError			= myError_ptr;
	this->myAxis 			= Axis_ptr;
	this->myPWM				= myPWM_ptr;

	this->myPWM->init(myAxis->my_instance, trajectory);
	this->myPWM->start();

}

bool current_control_class::setParameter(CMD_STRUCT* cmd)
{
	bool ret;

	if(this->myAxis->enabled == false)
	{
		ret = this->myParameter->setParameter(cmd,&this->module_parameter[0]);
		// this->refresh_parameter();
	}
	else
	{
		this->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_AXS_ON, __FILE__, __LINE__); // Module Parameter Locked - Servo is Running
	}

	return ret;
}

bool current_control_class::refresh_parameter(void)
{
	return true;
}


bool current_control_class::setAngle(float angle)
{
	mAngle = angle;
	return 0;
}

float current_control_class::getPosition(void)
{
    return mAngle; // Annahme: Sollwert wird immer gehalten
}

bool current_control_class::run(void)
{
	float angle = 0;
	if (myAxis->my_instance == 0)
		{
			angle = mAngle * AXA_MOTORSTEP_COUNT / 4; // umrechnen von mech. Winkel zu elekt. Winkel
		}
		else
		{
			angle = mAngle * AXB_MOTORSTEP_COUNT / 4;
		}


	this->myPWM->run(sinf(angle), cosf(angle));

	return 0;
}




