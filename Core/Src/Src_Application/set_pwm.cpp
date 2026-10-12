/***************************************************************************
 *  PWM-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#include "set_pwm.h"
#include "typedefs.h"
#include "main.h"
#include "trajectory.h"

/*****************************************************
* TREIBER-PHASENFAKTOREN (je Platinenvariante gesetzt)
* Definieren die im Header typedefs.h nur deklarierten
* Arrays die/der aktiven Driver (CONTROLLER_1/2/3).
*******************************************************/
#ifdef USE_TEST_MOTOR
const float PHASEFACTORA[3] ={0.2, 1.0, 1.2}; // Testmotor 23SSM8440
const float PHASEFACTORB[3] ={0.2, 1.0, 1.2}; // Testmotor 23SSM8440
#else
	#if defined(CONTROLLER_1)
	const float PHASEFACTORA[3] ={0.2, 1.0, 1.5}; // M1
	const float PHASEFACTORB[3] ={0.8, 1.0, 1.5}; // M2
	#elif defined(CONTROLLER_2)
	const float PHASEFACTORA[3] ={1.0, 1.0, 1.5}; // M3
	const float PHASEFACTORB[3] ={1.0, 1.0, 1.5}; // M4
	#elif defined(CONTROLLER_3)
	const float PHASEFACTORA[3] ={0.2, 1.0, 1.5}; // Original M5
	const float PHASEFACTORB[3] ={0.5, 0.8, 1.0}; // default
	#endif
#endif

set_pwm_class::set_pwm_class()
{
}

void set_pwm_class::init(int index, trajectory_class* trajectory)
{
	this->myTrajectory = trajectory;

	TIM_HandleTypeDef* htim;

	if (index == 0)  // Driver A -> TIM1
	{
		mDriver = 'A';
		htim = &htim1;
		HAL_GPIO_WritePin(MOT_A_M1_GPIO_Port, MOT_A_M1_Pin, GPIO_PIN_SET); 	// TODO: Fehler? Müssten die Pins nicht eher auf Low gesetzt werden?
		HAL_GPIO_WritePin(MOT_A_M2_GPIO_Port, MOT_A_M2_Pin, GPIO_PIN_SET);	// TODO: Fehler? Müssten die Pins nicht eher auf Low gesetzt werden?
		HAL_GPIO_WritePin(MOT_A_M3_GPIO_Port, MOT_A_M3_Pin, GPIO_PIN_SET);	// TODO: Fehler? Müssten die Pins nicht eher auf Low gesetzt werden?
		HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3);
		HAL_TIM_PWM_Start(htim, TIM_CHANNEL_4);
		HAL_TIM_PWM_Start(htim, TIM_CHANNEL_5);  // interner Trigger-Kanal für den ADC	
	}
	else if (index == 1)  // Driver B -> TIM20
	{
		mDriver = 'B';
		htim = &htim20;
		HAL_GPIO_WritePin(MOT_B_M1_GPIO_Port, MOT_B_M1_Pin, GPIO_PIN_SET);	// TODO: Fehler? Müssten die Pins nicht eher auf Low gesetzt werden?
		HAL_GPIO_WritePin(MOT_B_M2_GPIO_Port, MOT_B_M2_Pin, GPIO_PIN_SET);	// TODO: Fehler? Müssten die Pins nicht eher auf Low gesetzt werden?
		HAL_GPIO_WritePin(MOT_B_M3_GPIO_Port, MOT_B_M3_Pin, GPIO_PIN_SET);	// TODO: Fehler? Müssten die Pins nicht eher auf Low gesetzt werden?

		HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3);
		HAL_TIM_PWM_Start(htim, TIM_CHANNEL_4);
	}
	else
	{
		// TODO Error here
	}
}

void set_pwm_class::start()
{
	if(mDriver == 'A') // Driver A
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);  // Reset HIGH, Treiber aktiv
	}
	else if(mDriver == 'B') // Driver B
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	}
	else
	{
		// TODO Error here
	}
}

void set_pwm_class::stop()
{
	if(mDriver == 'A') // Driver A
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	}
	else if(mDriver == 'B') // Driver B
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	}
	else
	{
		// TODO Error here
	}
}

void set_pwm_class::run(float sin, float cos)
{
	if(mDriver == 'A')
	{
		const float pwm_scale = (float)__HAL_TIM_GET_AUTORELOAD(&htim1);   // 17000 bei 5 kHz

		this->phase_factor_a = PHASEFACTORA[this->myTrajectory->move_phase];

		this->isin_a = (int32_t) (this->phase_factor_a*(AXA_MAX_V/24.0)*pwm_scale*sin);
		this->icos_a = (int32_t) (this->phase_factor_a*(AXA_MAX_V/24.0)*pwm_scale*cos);

		if(isin_a >= 0)
		{
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, this->isin_a);  // Duty zwischen 0 und pwm_scale setzen
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
		}
		else
		{
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, -1*this->isin_a);
		}
		
		if(icos_a >= 0){
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, this->icos_a);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
		}
		else
		{
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, -1*this->icos_a);
		}

	}
	// Driver B
	else if(mDriver == 'B')
	{
		const float pwm_scale = (float)__HAL_TIM_GET_AUTORELOAD(&htim20);   // 17000 bei 5 kHz

		this->phase_factor_b = PHASEFACTORB[this->myTrajectory->move_phase];

		this->isin_b = (int32_t) (this->phase_factor_b*(AXB_MAX_V/24.0)*pwm_scale*sin);
		this->icos_b = (int32_t) (this->phase_factor_b*(AXB_MAX_V/24.0)*pwm_scale*cos);

		if(isin_b >= 0)
		{
			__HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_1, this->isin_b);
			__HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_2, 0);
		}
		else
		{
			__HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_1, 0);
			__HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_2, -1*this->isin_b);
		}
		if(icos_b >= 0){
			__HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_3, this->icos_b);
			__HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_4, 0);
		}
		else
		{
			__HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_3, 0);
			__HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_4, -1*this->icos_b);
		}
	}
}