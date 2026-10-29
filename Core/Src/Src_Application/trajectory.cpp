/***************************************************************************
 *  trajectory_class-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#include "trajectory.h"
#include "math.h"

trajectory_class::trajectory_class(void)
{
	trajectory_class::move_phase = PHASE_STANDSTILL;

	// initialisierung mit default values
	trajectory_class::delta_t 			= SYSTEM_SERVO_TIME;
	trajectory_class::delta_t2 			= SYSTEM_SERVO_TIME * SYSTEM_SERVO_TIME;
	trajectory_class::motion_on 		= false;
	trajectory_class::setpoint			= 0.0;
	trajectory_class::slowed_target		= 0.0;
	trajectory_class::vel 				= 0.0;
	trajectory_class::acc 				= 0.0;
	trajectory_class::s_delta 			= 0.0;
	trajectory_class::s_br 				= 0.0;
	trajectory_class::d_s 				= 0.0;
	trajectory_class::dd_s				= 0.0;
	trajectory_class::max_travel_limit 	= 0.0;
	trajectory_class::min_travel_limit 	= 0.0;
}


void trajectory_class::init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr)
{
	this->myAxis 		= Axis_ptr;
	this->myParameter	= myParameter_ptr;
	this->myError		= myError_ptr;
	this->motion_on 	= false;
	this->refresh_parameter();
}


bool trajectory_class::refresh_parameter(void)
{
	bool ret = true;

	// refresh parameter, only if no trajectory_class is performed
	if(this->motion_on == false)
	{
		this->vel_max			= this->module_parameter[1].value.f32;
		this->acc_max			= this->module_parameter[2].value.f32;
		this->dec_max			= this->module_parameter[3].value.f32;
		this->max_travel_limit	= this->module_parameter[4].value.f32;
		this->min_travel_limit	= this->module_parameter[5].value.f32;

		//Max values
		this->d_s_max 			= this->vel_max * delta_t;
		this->dd_s_max_acc 		= this->acc_max * delta_t2;
		this->dd_s_max_dec 		= this->dec_max * delta_t2;


		// initial values
		this->motion_init();
	}
	else
	{
		this->myAxis->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_MO_ON, __FILE__, __LINE__); // Parameter update not allowed, motion in progress
	}

	return ret;
}

void trajectory_class::motion_init(void)
{
	// initial values
	this->setpoint		= this->myAxis->myPos->mech_position;	 
	this->slowed_target	= this->setpoint;	
	this->vel 			= 0.0;
	this->acc 			= 0.0;
	this->s_delta 		= 0.0;
	this->s_br 			= 0.0;
	this->d_s 			= 0.0;
	this->dd_s			= 0.0;
	this->motion_on 	= false;
}

bool trajectory_class::setParameter(CMD_STRUCT* cmd)
{
	bool ret;

	if(this->motion_on == false)
	{
		ret = this->myParameter->setParameter(cmd, &this->module_parameter[0]);
		this->refresh_parameter();
	}
	else
	{
		this->myAxis->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_MO_ON,__FILE__, __LINE__); // Parameter update not allowed, motion in progress
	}
	return ret;
}

bool trajectory_class::set_target(float target)
{
	 bool ret = true;

	 // proof for target to be within limits
	 if((target <= this->max_travel_limit )  && (target >= this->min_travel_limit))
	 {
		this->setpoint = target; // set new target --> ! directly used...
	 }

	 return ret;
}


bool trajectory_class::calc(void)
{
	bool ret = true;

	//  Bewegungsrichtung - Wegstrecke bis zum Ziel:
	trajectory_class::s_delta = trajectory_class::setpoint - trajectory_class::slowed_target;

	if(trajectory_class::s_delta == 0.0f)
	{
		trajectory_class::motion_on = false;
		trajectory_class::ont 		= true;

		trajectory_class::move_phase = PHASE_STANDSTILL;
	}
	else
	{
		trajectory_class::motion_on = true;
		trajectory_class::ont 		= false;

		//  aktueller Bremsweg
		// Schutz gegen Division durch 0 (Parameter dec_max darf 0 sein) - laeuft im Servo-ISR!
		float dd_s_max_dec_tmp = trajectory_class::dd_s_max_dec;
		if(dd_s_max_dec_tmp == 0.0f)
		{
			dd_s_max_dec_tmp = 1.0e-9f;	// verhindert inf/NaN im Servoloop
		}
		trajectory_class::s_br = roundf(trajectory_class::d_s / dd_s_max_dec_tmp)  * 0.5f * (trajectory_class::d_s);

		// Geschwindigkeitsberechnung
		if(trajectory_class::s_delta >= 0.0f )						// positive Bewegungsrichtung
		{
			trajectory_class::move_phase = PHASE_ACCELERATE;

			if(fabsf(trajectory_class::s_delta) > fabsf(trajectory_class::s_br))		// Beschleunigung
			{
				trajectory_class::dd_s = trajectory_class::dd_s_max_acc;
			}
			else                            	// Verzögerung
			{
				trajectory_class::dd_s = -trajectory_class::dd_s_max_dec;
			}
		}
		else                                	// negative Bewegungsrichtung
		{
			trajectory_class::move_phase = PHASE_ACCELERATE;

			if(fabsf(trajectory_class::s_delta) > fabsf(trajectory_class::s_br))     	// Beschleunigung
			{
				trajectory_class::dd_s = -trajectory_class::dd_s_max_acc;
			}
			else                            	// Verzögerung
			{
				trajectory_class::dd_s = trajectory_class::dd_s_max_dec;
			}
		}

		trajectory_class::d_s = trajectory_class::d_s + trajectory_class::dd_s;

		// Begrenzung der Geschwindigkeit
		if (trajectory_class::d_s > trajectory_class::d_s_max) 							// konstante Fahrt ohen weitere Beschleunigung (positiv)
		{
			trajectory_class::move_phase = PHASE_MOVE;

			trajectory_class::d_s = trajectory_class::d_s_max;
			trajectory_class::dd_s = 0.0;
		}

		if(trajectory_class::d_s < -trajectory_class::d_s_max ) 						// konstante Fahrt ohen weitere Beschleunigung (negativ)
		{
			trajectory_class::move_phase = PHASE_MOVE;

			trajectory_class::d_s = -trajectory_class::d_s_max;
			trajectory_class::dd_s = 0.0;
		}

		// letzte Iteration vor erreichen des Ziels...
		if(fabsf(trajectory_class::s_delta) <= fabsf(trajectory_class::d_s))
		{
			trajectory_class::move_phase = PHASE_ACCELERATE; // change to "PHASE_STANDSTILL"?

			trajectory_class::slowed_target = trajectory_class::setpoint;
			trajectory_class::dd_s = 0.0;
			trajectory_class::d_s = 0.0;
		}
		else
		{
			// Streckenberechnung
			trajectory_class::slowed_target = trajectory_class::slowed_target + trajectory_class::d_s;
		}


		// Rückgabewerte umgerechnet auf Sekunde
		trajectory_class::vel = trajectory_class::d_s / delta_t;
		trajectory_class::acc = trajectory_class::dd_s / delta_t2;
	}
	return ret;
}




