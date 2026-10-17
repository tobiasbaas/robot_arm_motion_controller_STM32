/*
    Used motor driver: DRV8412
*/

#pragma once
#include "main.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim20;

class set_pwm_class{
	public:
        set_pwm_class();
	
		void init(bool instance);
        void startPWM(void);
        // void stopPWM(void);
        void setDriverMode(void);   // (DRV8412)
		void enableDriver(void);    // to enable motor driver (DRV8412)
		void disableDriver(void);   // to disable motor driver (DRV8412)
		
        bool setPhaseVoltage(uint8_t phase, float voltage); // set voltage for given phase
        bool isPhaseCurrentNegative(bool phase, bool &phaseSign);   // phaseSign false -> sign is '+'; pahseSign true -> sign is '-' 

        
    private:
        bool myInstance; // which axis instance is controlled

        TIM_HandleTypeDef* my_htim; // timer used for PWM

        float myMaxVoltage;   // max. voltage in [V]

        bool driverEnabled;
        bool myPhase1_isNegative;
        bool myPhase2_isNegative;
        // aktuelle Phasen Spannung
        // aktuelle Stromrichtung
};