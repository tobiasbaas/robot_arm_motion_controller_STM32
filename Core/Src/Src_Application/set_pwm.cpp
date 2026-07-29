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

set_pwm_class::set_pwm_class()
{
}

void set_pwm_class::init(int index, trajectory_class* trajectory)
{
	this->myTrajectory = trajectory;

	// init gpios and set modes for drv8412 drivers
	if(index == 0)// Driver A
	{
		mDriver = 'A';

		// Driver A
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_TIM1_CLK_ENABLE();  // Timer 1 aktivieren

		GPIO_InitTypeDef GPIO_InitStructA = {0};

		// PC0-PC3 als Alternativfunktion (PWM) konfigurieren
		GPIO_InitStructA.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
		GPIO_InitStructA.Mode = GPIO_MODE_AF_PP;  // Alternativfunktion für PWM
		GPIO_InitStructA.Pull = GPIO_NOPULL;
		GPIO_InitStructA.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStructA.Alternate = GPIO_AF2_TIM1;  // Timer 1 zuordnen
		HAL_GPIO_Init(GPIOC, &GPIO_InitStructA);

		// Timer 1 für PWM konfigurieren
		htim1.Instance = TIM1;
		htim1.Init.Prescaler = 5 - 1;  // Teilt den Takt für eine PWM-Frequenz um ~1 kHz
		htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim1.Init.Period = 1000 - 1;  // 1000 Schritte für 100% Duty Cycle
		htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		HAL_TIM_PWM_Init(&htim1);

		TIM_OC_InitTypeDef sConfigOCA = {0};
		sConfigOCA.OCMode = TIM_OCMODE_PWM1;
		sConfigOCA.Pulse = 0;
		sConfigOCA.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOCA.OCFastMode = TIM_OCFAST_DISABLE;

		// PWM-Kanäle für PC0-PC3 konfigurieren
		HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOCA, TIM_CHANNEL_1);  // PC0
		HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOCA, TIM_CHANNEL_2);  // PC1
		HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOCA, TIM_CHANNEL_3);  // PC2
		HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOCA, TIM_CHANNEL_4);  // PC3

		// PWM starten (HW-PWM aktiv)
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  // PC0
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);  // PC1
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);  // PC2
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);  // PC3


		HAL_GPIO_WritePin(MOT_A_M1_GPIO_Port, MOT_A_M1_Pin, GPIO_PIN_SET);  // Mode1 LOW
		HAL_GPIO_WritePin(MOT_A_M2_GPIO_Port, MOT_A_M2_Pin, GPIO_PIN_SET);  // Mode2 LOW
		HAL_GPIO_WritePin(MOT_A_M3_GPIO_Port, MOT_A_M3_Pin, GPIO_PIN_SET);  // Mode3 LOW
	}
	else if(index == 1)// Driver B
	{
		mDriver = 'B';

		// Driver B
		__HAL_RCC_GPIOF_CLK_ENABLE();
		__HAL_RCC_TIM20_CLK_ENABLE();  // Timer 20 aktivieren

		GPIO_InitTypeDef GPIO_InitStructB = {0};

		// PF12-PF15 als Alternativfunktion (PWM) konfigurieren
		GPIO_InitStructB.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
		GPIO_InitStructB.Mode = GPIO_MODE_AF_PP;  // Alternativfunktion für PWM
		GPIO_InitStructB.Pull = GPIO_NOPULL;
		GPIO_InitStructB.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStructB.Alternate = GPIO_AF2_TIM20;  // Timer 20 zuordnen
		HAL_GPIO_Init(GPIOF, &GPIO_InitStructB);

		// Timer 20 für PWM konfigurieren
		htim20.Instance = TIM20;
		htim20.Init.Prescaler = 5 - 1;  // Teilt den Takt für eine PWM-Frequenz um ~1 kHz
		htim20.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim20.Init.Period = 1000 - 1;  // 1000 Schritte für 100% Duty Cycle
		htim20.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		HAL_TIM_PWM_Init(&htim20);

		TIM_OC_InitTypeDef sConfigOCB = {0};
		sConfigOCB.OCMode = TIM_OCMODE_PWM1;
		sConfigOCB.Pulse = 0;
		sConfigOCB.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOCB.OCFastMode = TIM_OCFAST_DISABLE;

		// PWM-Kanäle für PC0-PC3 konfigurieren
		HAL_TIM_PWM_ConfigChannel(&htim20, &sConfigOCB, TIM_CHANNEL_1);  // PF12
		HAL_TIM_PWM_ConfigChannel(&htim20, &sConfigOCB, TIM_CHANNEL_2);  // PF13
		HAL_TIM_PWM_ConfigChannel(&htim20, &sConfigOCB, TIM_CHANNEL_3);  // PF14
		HAL_TIM_PWM_ConfigChannel(&htim20, &sConfigOCB, TIM_CHANNEL_4);  // PF15

		// PWM starten (HW-PWM aktiv)
		HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_1);  // PF12
		HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_2);  // PF13
		HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_3);  // PF14
		HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_4);  // PF15


		HAL_GPIO_WritePin(MOT_B_M1_GPIO_Port, MOT_B_M1_Pin, GPIO_PIN_SET);  // Mode1 LOW
		HAL_GPIO_WritePin(MOT_B_M2_GPIO_Port, MOT_B_M2_Pin, GPIO_PIN_SET);  // Mode2 LOW
		HAL_GPIO_WritePin(MOT_B_M3_GPIO_Port, MOT_B_M3_Pin, GPIO_PIN_SET);  // Mode3 LOW
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
		this->phase_factor_a = PHASEFACTORA[this->myTrajectory->move_phase];

		this->isin_a = (int32_t) (this->phase_factor_a*(AXA_MAX_V/24.0)*1000.0*sin);
		this->icos_a = (int32_t) (this->phase_factor_a*(AXA_MAX_V/24.0)*1000.0*cos);

		if(isin_a >= 0)
		{
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, this->isin_a);  // Duty zwischen 0 und 1000 setzen
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
		this->phase_factor_b = PHASEFACTORB[this->myTrajectory->move_phase];

		this->isin_b = (int32_t) (this->phase_factor_b*(AXB_MAX_V/24.0)*1000.0*sin);
		this->icos_b = (int32_t) (this->phase_factor_b*(AXB_MAX_V/24.0)*1000.0*cos);

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

