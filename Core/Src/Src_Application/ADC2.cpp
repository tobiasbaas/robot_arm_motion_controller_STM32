/*
 * ADC.cpp
 *
 *  Created on: Jan 12, 2024
 *      Author: nirut
 */

#include "ADC.hpp"


extern ADC_HandleTypeDef hadc1;

extern volatile float convVal1;
extern volatile float convVal2;
extern volatile float convVal3;


ADC_class::ADC_class()
{
}

void ADC_class::getADC_Val_BLDC_FOC(float &pADCValueA,float &pADCValueB,float &pADCValueC)
{
	pADCValueA = convVal1;
	pADCValueB = convVal3;
	pADCValueC = convVal2;
}


void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);


	uint32_t i_convVal1 = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1); // Read The Injected Channel Result
	uint32_t i_convVal2 = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2); // Read The Injected Channel Result
	uint32_t i_convVal3 = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3);


	convVal1 = float((i_convVal1 - ADC_OFFSET_CH1)) * conv+offset_c; // Read The Injected Channel Result
	convVal2 = float((i_convVal2 - ADC_OFFSET_CH2)) * conv+offset_c; // Read The Injected Channel Result
	convVal3 = float((i_convVal3 - ADC_OFFSET_CH3)) * conv+offset_c; // Read The Injected Channel Result

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}
