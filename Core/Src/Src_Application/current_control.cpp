#include "current_control.h"
#include "axis.h"
#include "stm32g4xx_hal_adc_ex.h"

current_control_class::current_control_class()
{
    // constructor
}

void current_control_class::init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr, set_pwm_class* myPWM_ptr, trajectory_class* trajectory)
{
	this->myParameter 		= myParameter_ptr;
	this->myError			= myError_ptr;
	this->myAxis 			= Axis_ptr;
	this->myPWM				= myPWM_ptr;
    this->myTrajectory      = trajectory;

   	this->myPWM->init(myAxis->my_instance);
	this->myPWM->startPWM();
    this->myPWM->enableDriver();    // TODO move to startFOC

    if(myAxis->my_instance == 0) this->myADC = &hadc1;
    else if(myAxis->my_instance == 1) this->myADC = &hadc4;

    this->faultLatched = false;

    this->ocCount = 0;

    this->u_lim = myPWM->getVoltageLimit();

    this->PIController_id.error = 0.0;
    this->PIController_id.error_mem = 0.0;
    this->PIController_id.pidOut_max = u_lim;   

    this->PIController_iq.error = 0.0;
    this->PIController_iq.error_mem = 0.0;
    this->PIController_iq.pidOut_max = 0.0;


    this->phi_mech = 0.0;	// mechanical angle rotor; [rad]
    this->phi_el = 0.0;		// electrical angle rotor; [rad]

    this->sin_phi = 0.0;
    this->cos_phi = 0.0;

    this->i_alpha = 0;		// current phase 1/A; [A]
    this->i_beta = 0.0;		// current phase 2/B; [A]
    this->i_d = 0.0;			// current in the direction of the d-axis; [A]
    this->i_q = 0.0;			// torque-generating current; [A]
    this->i_d_set = 0.0;		// setpoint for id current; [A]
    this->i_q_set = 0.0;		// setpoint for iq current; [A]

    this->u_alpha = 0.0;		// voltage phase 1/A; [V]
    this->u_beta = 0.0;		// voltage phase 2/B; [V]

	this->ADC_Offset[0] = 0;
	this->ADC_Offset[1] = 0;
	this->ADC_Offset[2] = 0;
	this->ADC_Offset[3] = 0;


    // TODO fix
    if(this->myAxis->my_instance == 0)
	{
		this->myParameter->read_eeprom_parameter(&this->module_parameter[0], ADDR_FLASH_A1_CUR_CTL,0);
	}
	else
	{
		this->myParameter->read_eeprom_parameter(&this->module_parameter[0], ADDR_FLASH_A2_CUR_CTL,0);
	}
	this->refresh_parameter();
}

bool current_control_class::setParameter(CMD_STRUCT* cmd)
{
    bool ret;

	if(this->myAxis->enabled == false)
	{
		ret = this->myParameter->setParameter(cmd,&this->module_parameter[0]);
		this->refresh_parameter();
	}
	else
	{
		this->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_SVO_ON, __FILE__, __LINE__);
        ret = false;
	}

	return ret;
}

bool current_control_class::refresh_parameter(void)
{
	if(this->myAxis->enabled == false)
	{
        this->PIController_id.pTerm			= this->module_parameter[5].value.f32;
        const float tn_id                   = this->module_parameter[6].value.f32 * 0.001f; // ms -> s 
		this->PIController_id.iTerm         = (tn_id > 0.0f) ? current_control_const::TS / tn_id : 0.0f;

		this->PIController_iq.pTerm		    = this->module_parameter[7].value.f32;
        const float tn_iq                   = this->module_parameter[8].value.f32 * 0.001f; // ms -> s
		this->PIController_iq.iTerm		    = (tn_iq > 0.0f) ? current_control_const::TS / tn_iq : 0.0f;
	}
	else
	{
		this->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_SVO_ON, __FILE__, __LINE__);
        return false;
	}

	return true;
}

bool current_control_class::oldCurrentLoop()
{
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

	readADC();

	float u_sin;	// setting voltage
	float u_cos;	// setting voltage

	float angle = 0;
	if (myAxis->my_instance == 0)
	{
		angle = phi_mech * AXA_MOTORSTEP_COUNT / 4; // umrechnen von mech. Winkel zu elekt. Winkel
		float phase_factor_a = PHASEFACTORA[this->myAxis->myTrajectory->move_phase];
		
		u_sin = phase_factor_a*sinf(angle)*AXA_MAX_V;	// calc voltage
		u_cos = phase_factor_a*cosf(angle)*AXA_MAX_V;	// calc voltage
	}
	else if(myAxis->my_instance == 1)
	{
		angle = phi_mech * AXB_MOTORSTEP_COUNT / 4;
		float phase_factor_b = PHASEFACTORB[this->myAxis->myTrajectory->move_phase];

		u_sin = phase_factor_b*sinf(angle)*AXB_MAX_V;
		u_cos = phase_factor_b*cosf(angle)*AXB_MAX_V;
	}
	else 
	{
		// TODO Error here
	}

	this->myPWM->setPhaseVoltage(0, u_sin);
	this->myPWM->setPhaseVoltage(1, u_cos);

	return 0;
}

void current_control_class::setMechAngle(float mech_angle)
{
    phi_mech = mech_angle;
}

float current_control_class::getMechAngle()
{
    return phi_mech;    
}

// control loop for FOC and current control
void current_control_class::currentControlLoop()
{
    if(faultLatched || !myPWM->isDriverEnabled()) return;  
    readADC();
    if(checkOvercurrent()) return;  // Overcurrent protection

    // Encoder lesen
    // Encoder Winkel umrechnen

    parkTransformation(); // Park-Transformation using this->i_alpha and this->i_beta
    
    PIController((this->i_d_set - this->i_d), this->PIController_id); // PI controller for id; uses u_lim as maximum, as id is regulated to 0 and u_d is therefore relatively small
    const float ud = PIController_id.pidOut;
    PIController_iq.pidOut_max = sqrtf(fmaxf(u_lim * u_lim - ud * ud, 0.0f));   // calculates the maximum output for u_d so that the magnitude of the voltage vector does not exceed u_lim
    PIController((this->i_q_set - this->i_q), this->PIController_iq); // PI controller for iq

    inverseParkTransformation(this->PIController_id.pidOut, this->PIController_iq.pidOut); // inverse Park-Transformation

    myPWM->setPhaseVoltage(0, this->u_alpha);   // set Phase 1/A
    myPWM->setPhaseVoltage(1, this->u_beta);   // set Phase 2/B
}

// setFOC
// resetFOC

void current_control_class::updateFOCSetpoints(float id_set, float iq_set)
{
    // limit to maximum current
    const float i_lim = module_parameter[2].value.f32;  // e.g 5.5 A
    id_set = fmaxf(-i_lim, fminf(i_lim, id_set));       // d should almost be 0A
    const float iq_lim = sqrtf(fmaxf(i_lim*i_lim - id_set*id_set, 0.0f)); // calc iq_limit, keeping the magnitude of the current vector below i_limit
    iq_set = fmaxf(-iq_lim, fminf(iq_lim, iq_set));

    this->i_d_set = id_set;
    this->i_q_set = iq_set;
}

void current_control_class::readADC(void)
{
    bool signPhase1;    // 0 -> '+'; 1 -> '-' 
    bool signPhase2;    // 0 -> '+'; 1 -> '-' 

    if(myPWM->isDriverEnabled())
    {
        myPWM->isPhaseCurrentNegative(0, signPhase1);   // get sign
        myPWM->isPhaseCurrentNegative(1, signPhase2);   // get sign

        // get current for Phase 1/A
        if(!signPhase1) // current flows form half-bridge A to halfbridge B 
        {
            this->i_alpha = rawToAmps(HAL_ADCEx_InjectedGetValue(myADC, ADC_INJECTED_RANK_2), this->ADC_Offset[1]); // Shunt B; Rank 2
        }
        else // current flows form half-bridge B to halfbridge A
        {
            this->i_alpha = -1.0 * rawToAmps(HAL_ADCEx_InjectedGetValue(myADC, ADC_INJECTED_RANK_1), this->ADC_Offset[0]); // Shunt A; Rank 1
        }

        // get current for Phase 2/B
        if(!signPhase2) // current flows form half-bridge C to halfbridge D 
        {
            this->i_beta = rawToAmps(HAL_ADCEx_InjectedGetValue(myADC, ADC_INJECTED_RANK_4), this->ADC_Offset[3]); // Shunt D; Rank 4
        }
        else // current flows form half-bridge D to halfbridge C
        {
            this->i_beta = -1.0 * rawToAmps(HAL_ADCEx_InjectedGetValue(myADC, ADC_INJECTED_RANK_3), this->ADC_Offset[2]); // Shunt C; Rank 3
        }
    }
    else 
    {
        // TODO Error here
    }
    
	return;
}

float current_control_class::rawToAmps(uint16_t raw, uint16_t offset)
{
    return ((int32_t)raw - (int32_t)offset) * current_sens::AMPS_PER_COUNT;
}

// Determines the ADC values at 0 A for all 4 channels (Rank 1–4).
// Returns `true` = OK; `offset_out[]` is valid. If `false` is returned, `offset_out` remains unchanged.
bool current_control_class::calibrateADC_Offsets(ADC_HandleTypeDef* hadc)
{
	constexpr uint32_t CAL_SAMPLES     = 256;     // Average of 256 measurements
	constexpr uint16_t OFFSET_EXPECTED = 1861;    // 1,5 V / 3,3 V * 4095	
	constexpr uint16_t OFFSET_TOL      = 200;     // ≈ ±0,16 V; further away = hardware error or current != 0

    uint32_t sum[4] = {0, 0, 0, 0};

    __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_JEOS);          // Discard old results

    for (uint32_t n = 0; n < CAL_SAMPLES; n++)
    {
		// Take 256 measurements and calculate the average later

        // Wait for the end of the next injected sequence (occurs once per PWM period)
        const uint32_t t0 = HAL_GetTick();
        while (!__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_JEOS))
        {
            if (HAL_GetTick() - t0 > 5)
			{
				// TODO: Error
				return false;   // No trigger: ADC/timer is not running
			} 
        }

        sum[0] += HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
        sum[1] += HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_2);
        sum[2] += HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_3);
        sum[3] += HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_4);

        __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_JEOS);
    }

    // Mean (rounded) and plausibility check
    uint16_t result[4];
    for (int i = 0; i < 4; i++)
    {
        result[i] = (uint16_t)((sum[i] + CAL_SAMPLES / 2) / CAL_SAMPLES);	// Average + round

        const int32_t dev = (int32_t)result[i] - (int32_t)OFFSET_EXPECTED;
        if (dev > OFFSET_TOL || dev < -(int32_t)OFFSET_TOL)
		{
			// TODO: Error
			return false;	// Offset is implausible or too far away
		}
    }

    for (int i = 0; i < 4; i++) this->ADC_Offset[i] = result[i];
    return true;
}


bool current_control_class::checkOvercurrent()
{
    const float i_peak = module_parameter[3].value.f32;   // Peak Current
    if (fabsf(i_alpha) > i_peak || fabsf(i_beta) > i_peak) 
    {
        if (++ocCount >= current_sens::OC_DEBOUNCE) {                 // 
            myPWM->disableDriver();                     // RESET-Pin -> hochohmig
            i_d_set = i_q_set = 0.0f;                   // setpoint to zero 
            PIController_id.error_mem = PIController_iq.error_mem = 0.0f;
            faultLatched = true;    

            return true;
        }
    } 
    else ocCount = 0;

    return false;
}

bool current_control_class::isFaultLatched()
{
    return faultLatched;
}

void current_control_class::resetFaultLatch()
{
    this->faultLatched = false;
}

void current_control_class::setAllCurrentsToZero()
{
	this->myPWM->setPhaseVoltage(0, 0); // set PWM LOW
	this->myPWM->setPhaseVoltage(1, 0); // set PWM LOW
	return;
}


// Encoder


void current_control_class::PIController(float error, PI_FOC& PI)
{
    // parameter 'PI' is the structure for the respective controller

	PI.error = error;

	PI.pidOut_P = error * PI.pTerm;  // P component

    const float ki = PI.pTerm * PI.iTerm;      // iTerm = Ts/Tn
	if (ki > 0.0f) 
    {
        PI.error_mem += error;
        PI.pidOut_I   = ki * PI.error_mem;
    } 
    else // integrator deactivated
    {
        PI.error_mem = 0.0f;
        PI.pidOut_I  = 0.0f;
    }

	float pid_y = PI.pidOut_P + PI.pidOut_I;    // calc output

    // Anti-Windup for positive and negative maximum
    if (pid_y > PI.pidOut_max || pid_y < -PI.pidOut_max) {
        pid_y = (pid_y > 0.0f) ? PI.pidOut_max : -PI.pidOut_max;
        if (ki > 0.0f) {                       // set I such that P + I = y
            PI.pidOut_I  = pid_y - PI.pidOut_P;
            PI.error_mem = PI.pidOut_I / ki;
        }
    }

	PI.pidOut = pid_y;

	return;
}

void current_control_class::parkTransformation()
{
    this->cos_phi = cosf(this->phi_el);
    this->sin_phi = sinf(this->phi_el);

    this->i_d =  this->i_alpha * this->cos_phi + this->i_beta * this->sin_phi;  
    this->i_q = -this->i_alpha * this->sin_phi + this->i_beta * this->cos_phi;

    return;
}

void current_control_class::inverseParkTransformation(float voltage_d, float voltage_q)
{
    this->u_alpha = voltage_d * this->cos_phi - voltage_q * this->sin_phi;
    this->u_beta  = voltage_d * this->sin_phi + voltage_q * this->cos_phi;

    return;
}
