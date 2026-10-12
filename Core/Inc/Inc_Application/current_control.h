/***************************************************************************
 *  Current Control-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/
#include "typedefs.h"
#include "axis.h"
#include "trajectory.h"
#include "main.h"
#include "set_pwm.h"

#ifndef _CURRENT_CONTROL_H
#define _CURRENT_CONTROL_H

extern ADC_HandleTypeDef hadc1;

// --- Messkette ---
namespace current_sens
{
	constexpr float VREF            = 3.3f;       // ADC-Referenz in V (anpassen, falls anders)
	constexpr float ADC_FS          = 4095.0f;    // 12 Bit
	constexpr float GAIN            = 20.0f;	  // Verstärkung Strommessung
	constexpr float R_SHUNT         = 0.010f;     // 10 mOhm
	constexpr float AMPS_PER_COUNT  = (VREF / ADC_FS) / (GAIN * R_SHUNT);   // ≈ 4,03 mA pro Count
}

typedef struct
{
	float	error;
	float	error_mem;
	float	pTerm;
	float	iTerm;
	float	pid_y_P;
	float	pid_y_I;
	float	pid_y;
	float	pid_y_max;
}PI_FOC;

class axis_class;
class set_pwm_class;
class trajectory_class;

class current_control_class : public parameter_class
{
	private:
		axis_class* myAxis;
		set_pwm_class* myPWM;
		error_class* myError;
		parameter_class* myParameter;

		float mAngle = 0.0;

	public:
		current_control_class();
		bool setParameter(CMD_STRUCT* cmd);
		bool refresh_parameter(void);
		bool run(void);
		void init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr, set_pwm_class* myPWM_ptr, trajectory_class* trajectory);
		bool setAngle(float angle);
		float getPosition(void);

		void readADC(void);
		bool calibrateADC_Offsets(ADC_HandleTypeDef* hadc);
		float rawToAmps(uint16_t raw, uint16_t offset);

		void setCurrentToZero();


		float myADC_Values[4];	// float only for testreasons to be able to use the data recorder
		uint16_t myADC_Offset[4];	// Offsets der ADC Channels bei 0A, also welcher Rohwert wird bei 0A gelesen; Erwarteter Wert 1861 (für +1,5V Offset bei 12Bit)
};

#endif

