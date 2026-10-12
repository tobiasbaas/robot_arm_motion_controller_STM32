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
#include "axis.h"
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

	this->myADC_Values[0] = 0;
	this->myADC_Values[1] = 0;
	this->myADC_Values[2] = 0;
	this->myADC_Values[3] = 0;

	this->myADC_Offset[0] = 0;
	this->myADC_Offset[1] = 0;
	this->myADC_Offset[2] = 0;
	this->myADC_Offset[3] = 0;


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
	readADC();

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

// ADC init() // Offset messen etc, Injected ADCs einstellen, dann im DR auslesen, ADC kalibireren

// ADC auslesen; Richtung ermitteln anhand der gesetzten PWM
void current_control_class::readADC(void)
{
    this->myADC_Values[0] = rawToAmps(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1), this->myADC_Offset[0]);
    this->myADC_Values[1] = rawToAmps(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2), this->myADC_Offset[1]);
    this->myADC_Values[2] = rawToAmps(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3), this->myADC_Offset[2]);
    this->myADC_Values[3] = rawToAmps(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_4), this->myADC_Offset[3]);
}


// Ermittelt die ADC-Werte bei 0 A für alle 4 Kanäle (Rank 1..4).
// Rückgabe true = ok, offset_out[] ist gültig. Bei false bleibt offset_out unverändert.
bool current_control_class::calibrateADC_Offsets(ADC_HandleTypeDef* hadc)
{
	constexpr uint32_t CAL_SAMPLES     = 256;     // Mittelung über 256 Sequenzen
	constexpr uint16_t OFFSET_EXPECTED = 1861;    // 1,5 V / 3,3 V * 4095	
	constexpr uint16_t OFFSET_TOL      = 200;     // ≈ ±0,16 V; weiter weg = Hardwarefehler oder Strom != 0

    uint32_t sum[4] = {0, 0, 0, 0};

    __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_JEOS);          // alte Ergebnisse verwerfen

    for (uint32_t n = 0; n < CAL_SAMPLES; n++)
    {
		// 256 mal messen und später mittelwert bilden

        // Auf das Ende der nächsten Injected-Sequenz warten (kommt einmal pro PWM-Periode)
        const uint32_t t0 = HAL_GetTick();
        while (!__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_JEOS))
        {
            if (HAL_GetTick() - t0 > 5)
			{
				// TODO: Error
				return false;   // kein Trigger: ADC/Timer läuft nicht
			} 
        }

        sum[0] += HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
        sum[1] += HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_2);
        sum[2] += HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_3);
        sum[3] += HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_4);

        __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_JEOS);
    }

    // Mittelwert (gerundet) und Plausibilitätsprüfung
    uint16_t result[4];
    for (int i = 0; i < 4; i++)
    {
        result[i] = (uint16_t)((sum[i] + CAL_SAMPLES / 2) / CAL_SAMPLES);	// Mittelwert + runden

        const int32_t dev = (int32_t)result[i] - (int32_t)OFFSET_EXPECTED;
        if (dev > OFFSET_TOL || dev < -(int32_t)OFFSET_TOL)
		{
			// TODO: Error
			 return false;	// Offset unplausibel bzw. zu weit weg
		}
    }

    for (int i = 0; i < 4; i++) this->myADC_Offset[i] = result[i];
    return true;
}

// Rohwert -> Strom in A, mit dem ermittelten Offset des jeweiligen Kanals
float current_control_class::rawToAmps(uint16_t raw, uint16_t offset)
{
    return ((int32_t)raw - (int32_t)offset) * current_sens::AMPS_PER_COUNT;
}


void current_control_class::setCurrentToZero()
{
	this->myPWM->run(0, 0); // PWM auf LOW
	return;
}
