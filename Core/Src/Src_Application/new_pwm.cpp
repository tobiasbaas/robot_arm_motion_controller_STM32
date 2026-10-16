/*
    Used motor driver: DRV8412
*/

#include <mutex>
@pragma std::once_flag

class set_pwm_class{
	public:
        set_pwm_class();
	
		void init(bool instance);
        void startPWM(void);
        // void stopPWM(void);
        void setDriverMode(void);   // (DRV8412)
		void enableDriver(void);    // to enable motor driver (DRV8412)
		void disableDriver(void);   // to disable motor driver (DRV8412)
		
        // setPhaseVoltage()
        // setPwmDuty()

    private:
        bool myInstance; // which axis instance is controlled

        TIM_HandleTypeDef* my_htim; // timer used for PWM

        float myMaxVoltage;   // max. voltage in [V]

        // aktuelle Phasen Spannung
        // aktuelle Stromrichtung
};