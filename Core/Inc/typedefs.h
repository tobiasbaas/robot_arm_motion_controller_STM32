#ifndef _TYPEDEFS_H
#define _TYPEDEFS_H

#include <stdint.h>
#include <stdbool.h>

#include "main.h"

/* AUSWAHL DER PLATINENVARIANTE (CONTROLLER_1 / CONTROLLER_2 / CONTROLLER_3)
 * Die Variante wird beim Bauen gesetzt, z.B. über das CMake-Preset:
 *   cmake --preset Board2 && cmake --build --preset Board2
 * HACK: Ist keins der drei Defines gesetzt, fällt die Firmware auf
 * CONTROLLER_1 (Board 1) zurück (siehe Treiberblock weiter unten).
 */

//#define CONTROLLER_1
//#define CONTROLLER_2
//#define CONTROLLER_3

#define SEND_OVER_USB // Information is sent using `send_string_USB`; comment this out to send using `send_string_DMA`

#define VERSION_STR 		"00.00.002"
#define CPY_STR 			"HKA Copyright 2026"

#define MAX_NUM_AXIS 		2
#define CMD_STR_LENGTH 		60
#define CMD_MAX_ARGS 		4	// maximum number of arguments in command


/*****************************************************
* FLASH LOCATION FOR PARAMETERS
*******************************************************/
#define ADDR_FLASH_DRC   		((uint32_t)0x08073000) /* Base @ of Page 230, 2 Kbytes */
#define ADDR_FLASH_A1   		((uint32_t)0x08073800) /* Base @ of Page 231, 2 Kbytes */
#define ADDR_FLASH_A1_TRJ  		((uint32_t)0x08074000) /* Base @ of Page 232, 2 Kbytes */
#define ADDR_FLASH_A1_POS   	((uint32_t)0x08074800) /* Base @ of Page 233, 2 Kbytes */
#define ADDR_FLASH_A1_PID_POS   ((uint32_t)0x08075000) /* Base @ of Page 234, 2 Kbytes */
#define ADDR_FLASH_A1_CUR_CTL  	((uint32_t)0x08075800) /* Base @ of Page 235, 2 Kbytes */
//#define ADDR_FLASH_PAGE_236   	((uint32_t)0x08076000) /* Base @ of Page 236, 2 Kbytes */
//#define ADDR_FLASH_PAGE_237   	((uint32_t)0x08076800) /* Base @ of Page 237, 2 Kbytes */
//#define ADDR_FLASH_PAGE_238   	((uint32_t)0x08077000) /* Base @ of Page 238, 2 Kbytes */
//#define ADDR_FLASH_PAGE_239  	((uint32_t)0x08077800) /* Base @ of Page 239, 2 Kbytes */
#define ADDR_FLASH_A2   		((uint32_t)0x08078000) /* Base @ of Page 240, 2 Kbytes */
#define ADDR_FLASH_A2_TRJ   	((uint32_t)0x08078800) /* Base @ of Page 241, 2 Kbytes */
#define ADDR_FLASH_A2_POS   	((uint32_t)0x08079000) /* Base @ of Page 242, 2 Kbytes */
#define ADDR_FLASH_A2_PID_POS   ((uint32_t)0x08079800) /* Base @ of Page 243, 2 Kbytes */
#define ADDR_FLASH_A2_CUR_CTL   ((uint32_t)0x0807a000) /* Base @ of Page 244, 2 Kbytes */
//#define ADDR_FLASH_PAGE_245   	((uint32_t)0x0807a800) /* Base @ of Page 245, 2 Kbytes */
//#define ADDR_FLASH_PAGE_246  	 ((uint32_t)0x0807b000) /* Base @ of Page 246, 2 Kbytes */
//#define ADDR_FLASH_PAGE_247   	((uint32_t)0x0807b800) /* Base @ of Page 247, 2 Kbytes */
//#define ADDR_FLASH_PAGE_248   	((uint32_t)0x0807c000) /* Base @ of Page 248, 2 Kbytes */
//#define ADDR_FLASH_PAGE_249   	((uint32_t)0x0807c800) /* Base @ of Page 249, 2 Kbytes */
#define ADDR_FLASH_PAGE_SYSTEM   ((uint32_t)0x0807d000) /* Base @ of Page 250, 2 Kbytes */
//#define ADDR_FLASH_PAGE_251   	((uint32_t)0x0807d800) /* Base @ of Page 251, 2 Kbytes */
//#define ADDR_FLASH_PAGE_252   	((uint32_t)0x0807e000) /* Base @ of Page 252, 2 Kbytes */
//#define ADDR_FLASH_PAGE_253   	((uint32_t)0x0807e800) /* Base @ of Page 253, 2 Kbytes */
//#define ADDR_FLASH_PAGE_254   	((uint32_t)0x0807f000) /* Base @ of Page 254, 2 Kbytes */
//#define ADDR_FLASH_PAGE_255   	((uint32_t)0x0807f800) /* Base @ of Page 255, 2 Kbytes */


/*****************************************************
* TREIBERBLOCK PRO PLATINENVARIANTE
* erklaert in der Kommentarzeile oben (CONTROLLER_1/2/3).
* Sichere Auswahl: BOARDS, deren Kennwerte in den
* Originalvarianten per Kommentar umgeschaltet wurden.
*******************************************************/
#if !defined(CONTROLLER_1) && !defined(CONTROLLER_2) && !defined(CONTROLLER_3)
	#define CONTROLLER_1 // Fallback: Board 1, wenn nichts gesetzt ist
#endif

#if defined(CONTROLLER_1) // ---------- Board 1 ----------
	#define CONTROLLER_NAME_STR "LIS-MC-Board1" // Labor Inform. Sys. - Motion Controller

	// Driver 1A
	#define AXA_MOTORSTEP_COUNT	200
	#define AXA_MAX_V			4.0

	// Driver 1B
	#define AXB_MOTORSTEP_COUNT	200
	#define AXB_MAX_V			4.0

#elif defined(CONTROLLER_2) // ---------- Board 2 ----------
	#define CONTROLLER_NAME_STR "LIS-MC-Board2" // Labor Inform. Sys. - Motion Controller

	// Driver 2A
	#define AXA_MOTORSTEP_COUNT	200
	#define AXA_MAX_V			4.0

	// Driver 2B
	#define AXB_MOTORSTEP_COUNT	200
	#define AXB_MAX_V			4.0

#elif defined(CONTROLLER_3) // ---------- Board 3 ----------
	#define CONTROLLER_NAME_STR "LIS-MC-Board3" // Labor Inform. Sys. - Motion Controller

	// Driver 3A
	#define AXA_MOTORSTEP_COUNT	200
	#define AXA_MAX_V			12.0	// Original M5

	// Driver 3B
	#define AXB_MOTORSTEP_COUNT	200
	#define AXB_MAX_V			4.0
#endif

/*****************************************************
* TESTMOTOR-OVERRIDE (fuer ALLE Boards und BEIDE Achsen)
* 23SSM8440-EC1000 (ACT; 1,8°; 2,4 V; 4,0 A/Phase; 0,6 Ohm; 2,5 mH)
* Auskommentieren -> Originalmotoren werden verwendet.
*******************************************************/
#define USE_TEST_MOTOR
#ifdef USE_TEST_MOTOR
	#undef  AXA_MAX_V
	#undef  AXB_MAX_V
	#define AXA_MAX_V	2.4	// Testmotor: begrenzt Phasenstrom auf ~4 A bei 0,6 Ohm
	#define AXB_MAX_V	2.4	// Testmotor: begrenzt Phasenstrom auf ~4 A bei 0,6 Ohm
#endif

/* Die PHASEFACTOR-Arrays pro Platine sind in set_pwm.cpp definiert (dort je
 * CONTROLLER_1/2/3 gesetzt). Hier nur Deklaration, damit mehrere C-/C++-TUs
 * keine Mehrfachdefinition (externe Bindung in C) erzeugen. */
extern const float PHASEFACTORA[3];
extern const float PHASEFACTORB[3];

/*****************************************************
* BOARD-KONFIGURATION (CANopen / Software)
*******************************************************/
#if defined(CONTROLLER_1) // ---------- Board 1 ----------
	#define CANOPEN_NODE_ID		2                       // CANopen Node-ID
	#define JOINT0_SP_SCALE		(5.0/1000000.0)         // Achse 0: setpoint = OD_x6001 * SCALE (Orig: /1e6 * 5.0)
	#define JOINT1_SP_SCALE		(-5.0/1000000.0)        // Achse 1: setpoint = OD_x6002 * SCALE (Orig: /1e6 * -5.0)
	#define AXIS1_ENABLED		1                       // 1 = Achse 1 aktiv, 0 = deaktiviert
	#define RPDO1_COB_ID		0x00000202              // PDO1 COB-ID (Direktzugriff auf joint1/x6001)
	#define RPDO2_COB_ID		0x00000203              // PDO2 COB-ID (Direktzugriff auf joint2/x6002)

#elif defined(CONTROLLER_2) // ---------- Board 2 ----------
	#define CANOPEN_NODE_ID		3
	#define JOINT0_SP_SCALE		(-5.0/1000000.0)        // Achse 0: setpoint = OD_x6001 * SCALE (Orig: /1e6 * -5.0)
	#define JOINT1_SP_SCALE		(5.0/1000000.0)         // Achse 1: setpoint = OD_x6002 * SCALE (Orig: /1e6 * 5.0)
	#define AXIS1_ENABLED		1
	#define RPDO1_COB_ID		0x00000204              // PDO1 COB-ID
	#define RPDO2_COB_ID		0x00000205              // PDO2 COB-ID

#elif defined(CONTROLLER_3) // ---------- Board 3 ----------
	#define CANOPEN_NODE_ID		4
	#define JOINT0_SP_SCALE		(1.0/300.0)             // Achse 0: setpoint = OD_x6001 * SCALE
	#define JOINT1_SP_SCALE		(-5.0/1000000.0)        // ungenutzt (AXIS1_ENABLED = 0)
	#define AXIS1_ENABLED		0                       // Platine 3 hat keine 2. Achse
	#define RPDO1_COB_ID		0x00000206              // PDO1 COB-ID
	#define RPDO2_COB_ID		0x80000000              // PDO2 ungueltig (Platine 3 hat nur RPDO1)
#endif

// Definition Voltages
#define BOARD_VCC 24.0	// supply voltage in [V]

// Definition servo time
#define	SERVO_FRQ 			10000.0		                // Servo Frequenz
#define	SERVO_AXIS_FRQ 		SERVO_FRQ /	MAX_NUM_AXIS	// Servo Frequenz
#define	SERVO_AXIS_TIME		MAX_NUM_AXIS / SERVO_FRQ 	// Servo Time

#define	SYSTEM_SERVO_TIME 	0.0001 		// Servo Zyklus Zeit in [??]
#define TIMER_COUTER_MAX	160000		// Max counter for timer of servo frq (Timer clock 160 MHz) //TODO Not 170 MHz?

// PI
#define M_PI 	3.141592654		// PI
#define M_PI_2 	1.570796327		// PI/2
#define M_PI_3 	1.047197551		// PI/3
#define M_2PI 	6.283185307		// 2 * PI

#define SQRT_3 	1.732050808		// SQRT(3)

#define RAD2DEG 180/M_PI	// Radiant to Degree
#define DEG2RAD M_PI/180	// Degree to Radiant

#define	UINT_32_MAX	0x7FFFFFFF
#define	INT_32_MAX	0x7FFFFFFF
#define	INT_32_MIN	0x80000000

#define	UINT_16_MAX	0xFFFF
#define	INT_16_MAX	0x7FFF
#define	INT_16_MIN	0x8000

/*
#define Enable_CH1_L6230_Pin 		GPIO_PIN_10
#define Enable_CH1_L6230_GPIO_Port 	GPIOC
#define Enable_CH2_L6230_Pin 		GPIO_PIN_11
#define Enable_CH2_L6230_GPIO_Port 	GPIOC
#define Enable_CH3_L6230_Pin 		GPIO_PIN_12
#define Enable_CH3_L6230_GPIO_Port 	GPIOC
*/

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
    _UINT64 = 0, _UINT32 = 1, _UINT16 = 2, _INT64 = 3, _INT32 = 4, _INT16 = 5, _FLOAT = 6, _DOUBLE = 7, _STRING = 8, _BOOL = 9, POS_INT = 10
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
    const char*     description;
}PARAM_DEV;


/*****************************************************
 * CMD STRUCT   
 *****************************************************/
typedef struct
{
    char        cmd_str[4];     // command
    char        module_str[4];  // module string (with instance)
    uint8_t    	i_instance;     // instance number
    DEV_CLASS   dv_module;      // module number
    uint16_t   	i_address;      // Address number
    uint32_t	i_numerator;
    uint32_t	i_denominator;
    UNION_VALUE value;
    char        argument[CMD_MAX_ARGS+2][20]; // CMD arguments (need 2 more, DMC and Module_ID String)
}CMD_STRUCT;

#endif
