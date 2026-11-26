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
#include <cmath>

axis_class::axis_class()
{
	 axis_class::my_instance 		= 100;	// if instance is 100, axix not initialised...
	 axis_class::myError 			= NULL;
	 axis_class::myTrajectory 		= NULL;
	 axis_class::myPos		 		= NULL;
	 axis_class::myPid_Pos			= NULL;
	 axis_class::myCurrent_Control	= NULL;
	 axis_class::enabled			= false;
	 axis_class::v_soll				= axis_const::V_SOLL_DEFAULT;
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
	
	theta_ref = 0.0f;
	lag = 0.0f;
	firstLoop = true;

	this->refresh_parameter();	// P-Term (kp_iq) aus Default/Parameter uebernehmen

	return true;
}

bool axis_class::setParameter(CMD_STRUCT* cmd)
{
	bool ret = false;

	if(this->enabled == false)
	{
		ret = this->myParameter->setParameter(cmd, &this->module_parameter[0]);
		this->refresh_parameter();
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
		kp_iq = this->module_parameter[1].value.f32;

		float v = this->module_parameter[2].value.f32;
		if(!std::isfinite(v)) v = axis_const::V_SOLL_DEFAULT;
		if(v >  axis_const::V_SOLL_MAX) v =  axis_const::V_SOLL_MAX;
		if(v < -axis_const::V_SOLL_MAX) v = -axis_const::V_SOLL_MAX;
		this->v_soll = v;
		this->module_parameter[2].value.f32 = v;
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

	this->lag = 0.0f;
	this->firstLoop = true;								// Rampe beim naechsten Einschalten auf Istposition setzen
}

void axis_class::disable(void)
{
	this->enabled = false;

	this->myTrajectory->motion_init();					// set trajectory variables for standstill
	this->myPid_Pos->servo_on = false;					// set servo off

	if(this->myCurrent_Control != NULL)
	{
		this->myCurrent_Control->updateFOCSetpoints(0.0f, 0.0f);	// Strom-Sollwert sicher auf 0
	}
	this->lag = 0.0f;
	this->firstLoop = true;
}

bool axis_class::run(void)
{
	bool ret = true;

	#ifdef USE_FOC
		if (this->myPos->is_encoder_ready())
		{
			// FOC 
			// Setpoints für i_d und i_q berrechnen
			// Trajectorie berrechnen
			// Positionsregler
			// ...

			const float mech_position = myPos->mt6835_result.angle_rad;

			if((this->enabled == true) && (this->myPid_Pos->servo_on == true))
			{
				if(firstLoop)								// Rampe beim Einschalten auf Istposition synchronisieren
				{
					theta_ref = mech_position;
					firstLoop = false;
				}

				theta_ref += this->v_soll * SERVO_AXIS_TIME;                         // 1. Rampe für Sollwert
				lag = theta_ref - mech_position;            // 2. Vorhalt begrenzen
				if (lag >  axis_const::LEAD_MAX) lag =  axis_const::LEAD_MAX;
				if (lag < -axis_const::LEAD_MAX) lag = -axis_const::LEAD_MAX;
				theta_ref = mech_position + lag;

				// P-Regler
				float iq_set = kp_iq * lag;                           // 3. P-Regler (kein "*-1", siehe unten)
				if (iq_set >  axis_const::IQ_MAX) iq_set =  axis_const::IQ_MAX;
				if (iq_set < -axis_const::IQ_MAX) iq_set = -axis_const::IQ_MAX;

				myCurrent_Control->updateFOCSetpoints(0.0f, iq_set);
			}
			else if(firstLoop == false)						// gerade von aktiv -> inaktiv gewechselt
			{
				// Strom-Sollwert sicher auf 0, Rampe fuer naechstes Einschalten neu synchronisieren
				myCurrent_Control->updateFOCSetpoints(0.05f, 0.0f);
				lag = 0.0f;
				firstLoop = true;
			}

			return ret;
		}
		// Encoder-Init fehlgeschlagen -> Open-Loop-Fallback
	#endif

		// open loop
		//ret = this->myPos->get_el_position(); // just called for data rec. for testing
		//	this->myCurrent_Control->readADC();	// just called for data rec. for testing
		ret = this->myPos->get_stepper_position();
		ret = this->myTrajectory->calc();

		if(this->enabled == true)
		{
			if(this->myPid_Pos->servo_on == true)
			{
				float pid_output = this->myPid_Pos->PIDrun(this->myPos->mech_position, this->myTrajectory->slowed_target);

				float angle_step = (this->myPos->mech_position + 0.02f * pid_output);

				this->myCurrent_Control->setMechAngle(angle_step);
			}
			this->myCurrent_Control->oldCurrentLoop();
		}
	
	return ret;
}