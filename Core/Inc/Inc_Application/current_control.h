#pragma once

#include "set_pwm.h"
#include "typedefs.h"
#include <cstdint>
#include <cmath>

// forward declarations
class axis_class;
class parameter_class;
class error_class;
class trajectory_class;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc4;

namespace current_control_const 
{
	constexpr float TS = 1/30000.0f;	// cycle time of the current control loop [s]
};

namespace current_sens
{
	constexpr float VREF            = 3.3f;       // ADC-Referenz in V (anpassen, falls anders)
	constexpr float ADC_FS          = 4095.0f;    // 12 Bit
	constexpr float GAIN            = 20.0f;	  // Verstärkung Strommessung
	constexpr float R_SHUNT         = 0.010f;     // 10 mOhm
	constexpr float AMPS_PER_COUNT  = (VREF / ADC_FS) / (GAIN * R_SHUNT);   // ≈ 4,03 mA pro Count

	constexpr uint8_t OC_DEBOUNCE = 3;	// used for overcurrent shutdown; 
										// specifies the number of loop cycles after which a overcurrent fault is triggered
};

typedef struct
{
	float	error;
	float	error_mem;
	float	pTerm;
	float	iTerm;
	float	pidOut_max;	// max output; TODO = AXA_MAX?
	float	pidOut_P;	// output after p-term; = Kp
	float	pidOut_I;	// output after i-term	= 1/Ki (or response time Tn)
	float	pidOut;		// output PI controller
}PI_FOC;

class current_control_class
{
	public:
		current_control_class();
		void init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr, set_pwm_class* myPWM_ptr, trajectory_class* trajectory);
		bool setParameter(CMD_STRUCT* cmd);
		bool refresh_parameter(void);

		bool oldCurrentLoop(void);				// current loop used bevore FOC
		void setMechAngle(float mech_angle); 	// used for old current loop
		float getMechAngle();					// used for old current loop

		void currentControlLoop();
		void updateFOCSetpoints(float i_d_set, float i_q_set); // updates the setpoints for id and iq

		void readADC(void);	// read values from injected ADC Channels
		float rawToAmps(uint16_t raw, uint16_t offset);	// in [A]
		bool calibrateADC_Offsets(ADC_HandleTypeDef* hadc);	// get ADC offset when current is 0 A
		bool checkOvercurrent();
		bool isFaultLatched();
		void resetFaultLatch();

		void setAllCurrentsToZero();	// set both pwm dutys to zero
		
		void PIController(float error, PI_FOC& PI);	// parameter 'PI' is the structure for the respective controller

		void parkTransformation();	// Park-Transform; uses this->i_alpha and this->i_beta as currents
		void inverseParkTransformation(float voltage_d, float voltage_q);


		#define MAX_NUM_PARAMETER_CURRENT_CON 9
		PARAM_DEV module_parameter[MAX_NUM_PARAMETER_CURRENT_CON] =
		{
			{0x01, 		_UINT16, 		MAX_NUM_PARAMETER_CURRENT_CON, 			MAX_NUM_PARAMETER_CURRENT_CON, MAX_NUM_PARAMETER_CURRENT_CON, 		true,		"Number of Parameters in Current Control Module"}, //[0]
			{0x10, 		_FLOAT, 		{.f32=24.0}, 							0, 48,			false,	"Intermediate Voltage"},	// TODO needed -> remove?
			{0x20, 		_FLOAT, 		{.f32=4.0}, 							0, 6, 			false,	"Maximum Current"},	// TODO needed -> remove?
			{0x21, 		_FLOAT, 		{.f32=5.0}, 							0, 6, 			false,	"Peak Current"},	// ODO needed -> remove?; 6A s the maximum current the measurement circuit can measure
			{0x22, 		_UINT16, 		{.ui32=1024}, 							0, UINT_32_MAX, false,	"Number Counts per electrical Pitch"}, // TODO Counts from Encoder?
			{0x50, 		_FLOAT, 		{.f32=1.0}, 							0, 10000, 		false,	"P-Term FOC D"},							
			{0x51, 		_FLOAT, 		{.f32=2.0}, 							0, 100, 		false,	"TN (I-Term) FOC D"},	// [ms]
			{0x60, 		_FLOAT, 		{.f32=1.0}, 							0, 10000, 		false,	"P-Term FOC Q"},
			{0x61, 		_FLOAT, 		{.f32=2.0}, 							0, 100, 		false,	"TN (I-Term) FOC Q"}	// [ms]
		};
 

		float i_alpha;		// current phase 1/A; [A]
		float i_beta;		// current phase 2/B; [A]

		set_pwm_class* myPWM;

    private:
		axis_class* myAxis;
		error_class* myError;
		parameter_class* myParameter;
		trajectory_class* myTrajectory;

		ADC_HandleTypeDef* myADC;	// what adc is used

		bool faultLatched;	// true if over current event triggered

		uint8_t ocCount;		// counter for overcurrent detection
		float u_lim;		// max voltage per phase, comes from set_pwm_class; [V]

		PI_FOC PIController_id;	// values for PI controller controlling the id current
		PI_FOC PIController_iq;	// values for PI controller controlling the iq current

		float phi_mech;		// mechanical angle rotor; [rad]
		float phi_el;		// electrical angle rotor; [rad]

		float sin_phi;		// calculated sine for the given electrical angle
		float cos_phi;		// calculated cosine for the given electrical angle

		float i_d;			// current in the direction of the d-axis; [A]
		float i_q;			// torque-generating current; [A]
		volatile float i_d_set;		// setpoint for id current; [A]
		volatile float i_q_set;		// setpoint for iq current; [A]

		float u_alpha;		// voltage phase 1/A; [V]
		float u_beta;		// voltage phase 2/B; [V]

		uint16_t ADC_Offset[4];	// Offsets der ADC Channels bei 0A, also welcher Rohwert wird bei 0A gelesen; Erwarteter Wert 1861 (für +1,5V Offset bei 12Bit)

		volatile uint32_t dbg_last[6];   // letzter Wert je Abschnitt (in Takten)
		volatile uint32_t dbg_max[6];    // größter bisher gesehener Wert
};