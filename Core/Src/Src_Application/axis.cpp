/***************************************************************************
 *  Axis-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#include "axis.h"

axis_class::axis_class()
{
	 axis_class::my_instance 		= 100;	// if instance is 100, axix not initialised...
	 axis_class::myError 			= NULL;
	 axis_class::myTrajectory 		= NULL;
	 axis_class::myPos		 		= NULL;
	 axis_class::myPid_Pos			= NULL;
	 axis_class::myCurrent_Control	= NULL;
	 axis_class::enabled			= false;
	 //axis_class::position			= 0;
}

bool axis_class::init(uint8_t i, error_class* myError, trajectory_class* trajectory, pos_sensor_class* pos_sensor, PID_class* pid_pos, current_control_class* current_control,data_recorder_class* DR,parameter_class* myParameter)
{
	axis_class::my_instance 		= i;
	axis_class::myError 			= myError;
	axis_class::myTrajectory 		= trajectory;
	axis_class::myPos		 		= pos_sensor;
	axis_class::myPid_Pos			= pid_pos;
	axis_class::myCurrent_Control	= current_control;
	axis_class::myDR				= DR;
	axis_class::myParameter			= myParameter;

	// get ADC Offset
	

	return true;
}

bool axis_class::setParameter(CMD_STRUCT* cmd)
{
	bool ret;

	if(this->enabled == false)
	{
		ret = this->myParameter->setParameter(cmd, &this->module_parameter[0]);
	}
	else
	{
		this->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_AXS_ON,__FILE__, __LINE__); // Parameter update not allowed, motion in progress
	}

	return ret;
}

bool axis_class::refresh_parameter(void)
{
	bool ret = true;

	// refresh parameter, only if no trajectory_class is performed
	if(this->enabled == false)
	{
		// General
	}
	else
	{
		this->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_AXS_ON,__FILE__, __LINE__); // Parameter update not allowed, motion in progress
	}
	return ret;
}


void axis_class::enable(void)
{
	this->enabled = true;								// enable axis in standstill

	this->myTrajectory->motion_init();					// initialize trajectory variables for standstill
	this->myPid_Pos->servo_on = false;					// set servo off
}

void axis_class::disable(void)
{
	this->enabled = false;

	this->myTrajectory->motion_init();					// set trajectory variables for standstill
	this->myPid_Pos->servo_on = false;					// set servo off
}

bool axis_class::run(void)
{
	bool ret = true;

	#ifdef USE_FOC
		// FOC 
		// Setpoints für i_d und i_q berrechnen
		// Trajectorie berrechnen
		// Positionsregler
		// ...
	#else
		// open loop
		ret = this->myPos->get_position_SPI(); // just called for data rec. for testing
		ret = this->myPos->get_stepper_position();
		ret = this->myTrajectory->calc();

		if(this->enabled == true)
		{
			if(this->myPid_Pos->servo_on == true)
			{
				float pid_output = this->myPid_Pos->PIDrun(this->myPos->position, this->myTrajectory->slowed_target);

				float angle_step = (this->myPos->position + 0.02f * pid_output);

				this->myCurrent_Control->setMechAngle(angle_step);
			}
			this->myCurrent_Control->oldCurrentLoop();
		}
	#endif
	
	return ret;
}



