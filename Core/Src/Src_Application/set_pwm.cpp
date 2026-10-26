#include "set_pwm.h"
#include "typedefs.h"
//#include <cstdint>
#include <cmath>

set_pwm_class::set_pwm_class()
{
    // default values
    myInstance = 0;
    my_htim = &htim1;
    myMaxVoltage = 0;
    driverEnabled = false;
    myPhase1_isNegative = false;
    myPhase2_isNegative = false;
}

void set_pwm_class::init(bool instance)
{
    myInstance = instance;

    if(instance == 0)
    {
        my_htim = &htim1; // use TIM1 -> axis 1 or "A"
        myMaxVoltage = AXA_MAX_V;
    }
    else if(instance == 1)
    {
        my_htim = &htim20;  // use TIM20 -> axis 2 or "B"
        myMaxVoltage = AXB_MAX_V;
    }
    else
    {
        // TODO Error here
    }

    //startPWM();
    setDriverMode();
    //enableDriver();

    return;
}

void set_pwm_class::startPWM(void)
{
    HAL_TIM_PWM_Start(my_htim, TIM_CHANNEL_1); // half bridge A
    HAL_TIM_PWM_Start(my_htim, TIM_CHANNEL_2); // half bridge B
    HAL_TIM_PWM_Start(my_htim, TIM_CHANNEL_3); // half bridge C
    HAL_TIM_PWM_Start(my_htim, TIM_CHANNEL_4); // half bridge D
    HAL_TIM_PWM_Start(my_htim, TIM_CHANNEL_5); // internal trigger channel for the injected ADC	

    return;
}

void set_pwm_class::setDriverMode(void)
{
    // Mode 001 -> Dual full brige with OC latching shutdown; currently the oc shutdown case is not solved by SW

    if(myInstance == 0)
    {
        // Axis 1/A:
        HAL_GPIO_WritePin(MOT_A_M1_GPIO_Port, MOT_A_M1_Pin, GPIO_PIN_SET); 	
		HAL_GPIO_WritePin(MOT_A_M2_GPIO_Port, MOT_A_M2_Pin, GPIO_PIN_RESET);	
		HAL_GPIO_WritePin(MOT_A_M3_GPIO_Port, MOT_A_M3_Pin, GPIO_PIN_RESET);	
    }
    else if(myInstance == 1)
    {
        // Axis 2/B:
        HAL_GPIO_WritePin(MOT_B_M1_GPIO_Port, MOT_B_M1_Pin, GPIO_PIN_SET);	
		HAL_GPIO_WritePin(MOT_B_M2_GPIO_Port, MOT_B_M2_Pin, GPIO_PIN_RESET);	
		HAL_GPIO_WritePin(MOT_B_M3_GPIO_Port, MOT_B_M3_Pin, GPIO_PIN_RESET);
    }

    return;
}

void set_pwm_class::enableDriver(void)
{
    if(myInstance == 0) // Driver 1/A
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    // reset pin HIGH -> motordriver activ/ both H-bridges active
	}
	else if(myInstance == 1) // Driver 2/B
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET); // reset pin HIGH -> motordriver activ/ both H-bridges active
	}
	else
	{
		// TODO Error here
        return;
	}

    driverEnabled = true;

    return;
}

void set_pwm_class::disableDriver(void)
{
    __HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_4, 0);


    if(myInstance == 0) // Driver 1/A
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);    // reset pin LOW -> reset both H-bridges 
	}
	else if(myInstance == 1) // Driver 2/B
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET); // reset pin LOW -> reset both H-bridges 
	}
	else
	{
		// TODO Error here
        return;
	}

    driverEnabled = false;

    return;
}

bool set_pwm_class::setPhaseVoltage(uint8_t phase, float voltage)
{
    float pwmMax = (float)__HAL_TIM_GET_AUTORELOAD(my_htim);   // e.g. 17000 for 5 kHz

    bool isNegative = (voltage < 0.0f);   // get sign
    voltage = fabsf(voltage); // remove sign

    if(voltage > myMaxVoltage) voltage = myMaxVoltage;  // clamp voltage for voltage limitation

    int32_t duty = (int32_t)((voltage / BOARD_VCC) * pwmMax);  // calc duty cycle for pwm

    // TODO Grenze für PWM, Siehe Datenblatt wegen Bootstrapkondensator etc.; duty muss < Arr sein
    pwmMax -= pwm_const::DUTY_LIMIT;
    if(duty > pwmMax) duty = pwmMax;

    if(phase == 0)  // half-bridge A and B
    {
        if(!isNegative) // A -> B
        {
            __HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_2, 0);
            __HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_1, duty);  
        }
        else if(isNegative) // B -> A
        {
            __HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_1, 0);  
			__HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_2, duty);
        }

        myPhase1_isNegative = isNegative;   // save sign; used for current measurement
    }
    else if(phase == 1) // half-bridge C and D
    {
        if(!isNegative) // C -> D
        {
			__HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_4, 0);
            __HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_3, duty); 
        }
        else if(isNegative) // D -> C
        {
            __HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_3, 0);  
			__HAL_TIM_SET_COMPARE(my_htim, TIM_CHANNEL_4, duty);
        }

        myPhase2_isNegative = isNegative;   // save sign; used for current measurement
    }
    else 
    {
        // TODO Error here
        return false;
    }

    return true;
}

bool set_pwm_class::isPhaseCurrentNegative(bool phase, bool &phaseSign)
{
    if(phase == 0) // half-bridge A and B
    {
        phaseSign = myPhase1_isNegative;
    }
    else if(phase == 1) // half-bridge C and D
    {
        phaseSign = myPhase2_isNegative;
    }
    else
    {
        // TODO Error here
        return false;
    }

    return true;
}

float set_pwm_class::getVoltageLimit() const
{
    const float arr   = (float)__HAL_TIM_GET_AUTORELOAD(my_htim);
    const float u_dty = BOARD_VCC * (arr - pwm_const::DUTY_LIMIT) / arr;   // max Voltage at max duty
    return fminf(myMaxVoltage, u_dty);
}

bool set_pwm_class::isDriverEnabled()
{
    return driverEnabled;
}