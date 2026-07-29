/***************************************************************************
 *  PWM-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#ifndef _SET_PWM_H
#define _SET_PWM_H

#include "trajectory.h"


class trajectory_class;

class set_pwm_class{
	private:
		/* data */
		char mDriver;
		int32_t isin_a = 0;
		int32_t isin_b = 0;
		int32_t icos_a = 0;
		int32_t icos_b = 0;
		float phase_factor_a = 0.0;
		float phase_factor_b = 0.0;
	public:
		trajectory_class* 			myTrajectory;

		set_pwm_class();

		void init(int index, trajectory_class* trajectory);
		void start(); // to enable drivers
		void stop(); // to disable drivers
		void run(float sin, float cos); // to set new values (cyclic)
};
#endif
