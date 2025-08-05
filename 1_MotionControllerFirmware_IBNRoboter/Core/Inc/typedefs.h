#ifndef _TYPEDEFS_H
#define _TYPEDEFS_H

#include <stdint.h>
#include <stdbool.h>

#include "main.h"

//#define CONTROLLER_1
//#define CONTROLLER_2
//#define CONTROLLER_3

#define VERSION_STR 		"00.00.002"
#define CONTROLLER_NAME_STR "LIS-MC-U1A2" // Labor Inform. Sys. - Motion Controller - Universal: no1 Axis: 2
#define CPY_STR 			"HKA Copyright 2025"

#define MAX_NUM_AXIS 		2
#define CMD_STR_LENGTH 		60
#define CMD_MAX_ARGS 		4	// maximum number of arguments in command

// Driver 1A
#define AXA_MOTORSTEP_COUNT	200
#define AXA_MAX_V			4.0
const float PHASEFACTORA[3] ={0.2, 1.0, 1.5}; // M1

// Driver 1B
#define AXB_MOTORSTEP_COUNT	200
#define AXB_MAX_V			4.0
const float PHASEFACTORB[3] ={0.8, 1.0, 1.5}; // M2

/* choose depending on which motion controller you want to flash
// Driver 2A
#define AXA_MOTORSTEP_COUNT	200
#define AXA_MAX_V			4.0
const float PHASEFACTORA[3] ={1.0, 1.0, 1.5}; // M3

// Driver 2B
#define AXB_MOTORSTEP_COUNT	200
#define AXB_MAX_V			4.0
const float PHASEFACTORB[3] ={1.0, 1.0, 1.5}; // M4

// Driver 3A
#define AXA_MOTORSTEP_COUNT	200
#define AXA_MAX_V			12.0
const float PHASEFACTORA[3] ={0.2, 1.0, 1.5}; // M5

// Driver 3B
#define AXB_MOTORSTEP_COUNT	200
#define AXB_MAX_V			4.0
const float PHASEFACTORB[3] ={0.5, 0.8, 1.0}; // default*/

// Definition servo time
#define	SERVO_FRQ 			10000.0		// Servo Frequenz
#define	SERVO_AXIS_FRQ 		SERVO_FRQ /	MAX_NUM_AXIS	// Servo Frequenz
#define	SERVO_AXIS_TIME		MAX_NUM_AXIS / SERVO_FRQ 	// Servo Time

#define	SYSTEM_SERVO_TIME 	0.0001 		// Servo Zyklus Zeit
#define TIMER_COUTER_MAX	160000		// Max counter for timer of servo frq (Timer clock 160 MHz)

// PI
#define M_PI 	3.141592654		// PI
#define M_PI_2 	1.570796327		// PI/2
#define M_PI_3 	1.047197551		// PI/3
#define M_2PI 	6.283185307		// 2 * PI

#define SQRT_3 	1.732050808		// SQRT(3)

#define RAD2DEG 180/M_PI	// Radiant to Degree
#define DEG2RAD M_PI/180	// Degree to Radiant

#define	UINT_32_MAX	2^32
#define	INT_32_MAX	2^31
#define	INT_32_MIN	-2^31

#define	UINT_16_MAX	2^16
#define	INT_16_MAX	2^15
#define	INT_16_MIN	-2^15


#define Enable_CH1_L6230_Pin 		GPIO_PIN_10
#define Enable_CH1_L6230_GPIO_Port 	GPIOC
#define Enable_CH2_L6230_Pin 		GPIO_PIN_11
#define Enable_CH2_L6230_GPIO_Port 	GPIOC
#define Enable_CH3_L6230_Pin 		GPIO_PIN_12
#define Enable_CH3_L6230_GPIO_Port 	GPIOC


/*****************************************************
* DEFINITIONS FOR DEFAULT VALUES IN FIRMAWARE
*******************************************************/

// Trajectory Parameter
#define DEFAULT_MAX_TRAVELRANGE 	100.0
#define DEFAULT_MIN_TRAVELRANGE 	-100.0
#define DEFAULT_MAX_VELOCITY 		10.0
#define DEFAULT_MAX_ACCELERATION 	10.0



/*****************************************************
 *  ENUM for classifying CONTROLLER MODULES
 *****************************************************/

#define NUM_OF_DEV_CLASSES 12

//#define DEV_CLASS_STR{{"NON"},{"SYS"},{"DRC"},{"AXS"},{"MCO"},{"CCO"},{"ENC"},{"TRJ"},{"PWM"},{"PAM"},{"CMD"},{"ERR"}};

typedef struct
{
	const char cmd[4];
	const char* description;

}DEV_CLASS_STRINGS;

typedef enum
{
    NON_MOD, SYSTEM_MOD, DATARECORDER_MOD, AXIS_MOD, MOTION_PID_MOD, C_PID_MOD, ENCODER_MOD, TRAJECTORY_MOD, PWM_MOD, PARAMETER_MOD, COMMAND_MOD, ERROR_MOD
}DEV_CLASS;

/*****************************************************
 *  ENUM for classifying PARAMETER TYPES
 *****************************************************/

#define NUM_OF_PARAM_TYPES 11
#define PARAM_TYPE_STR {{"UINT64"},{"UINT32"},{"UINT16"},{"INT64"},{"INT32"},{"INT16"},{"FLOAT"},{"DOUBLE"},{"STRING"},{"BOOL"},{"POSINT"}};
typedef enum
{
    _UINT64, _UINT32, _UINT16, _INT64, _INT32, _INT16, _FLOAT, _DOUBLE, _STRING, _BOOL, POS_INT
}PARAM_TYPE;


/*****************************************************
 *  struct for UNION_VALUES
 *****************************************************/
typedef union
{
	int64_t 	i64;
    int32_t 	i32;
    int16_t 	i16;
    uint16_t	ui16;
    uint64_t 	ui64;
    uint32_t 	ui32;
    float 		f32;
    double 		d64;
    char 		str8[8];
    bool 		_bool;
}UNION_VALUE;

/*****************************************************
 * PARAMETER STRUCT
 *****************************************************/
typedef struct
{
    uint16_t 		adress;
    PARAM_TYPE 		type;
    UNION_VALUE 	value;
    int32_t 		minimum;
    int32_t 		maximum;
    bool			read_only;
    const char* description;
}PARAM_DEV;

/*****************************************************
 * CMD STRUCT
 *****************************************************/
typedef struct
{
    char        cmd_str[4];     // command
    char        module_str[4];     // module string (with instance)
    uint8_t    	i_instance;     // instance number
    DEV_CLASS   dv_module;       // module number
    uint16_t   	i_address;       // Address number
    uint32_t	i_numerator;
    uint32_t	i_denominator;
    UNION_VALUE value;
    char argument[CMD_MAX_ARGS+2][20];       // CMD arguments (need 2 more, DMC and Module_ID String)
}CMD_STRUCT;

#endif
