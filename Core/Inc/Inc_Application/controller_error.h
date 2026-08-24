/***************************************************************************
 *  Error-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#ifndef _CONTROLLER_ERROR_H
#define _CONTROLLER_ERROR_H

#include "typedefs.h"
#include <string.h>
#include "main.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

extern "C" void send_string(const char* str, bool new_line);


#define MAX_NUM_OF_ERRORS_IN_LIST 20    //define length of the error list


// Error struct
typedef struct
{
    int16_t error_value;
    char description[60];
}ERRORS;

//DATARECORDER, AXIS, P_PID, C_PID, ENCODER, TRAJECTORY, PWM, PARAMETER, COMMAND

typedef struct
{
	union
	{
		uint16_t all;
		struct
		{
			bool 		err 		: 1;		// Error active
			bool 		warn 		: 1;		// Warning active
			bool 		run 		: 1;		// firmware is running
			bool 		bsy			: 1;		// FW is busy
			uint16_t 	res0		: 4;		// Bits reserved
			bool 		a0en		: 1;		// Axis 1 enabled
			bool 		a0so		: 1;		// Axis 1 servo on
			bool 		a0ont		: 1;		// Axis 1 on target
			bool 		res1		: 1;		// Bits reserved
			bool 		a1en		: 1;		// Axis 2 enabled
			bool 		a1so		: 1;		// Axis 2 servo on
			bool 		a1ont		: 1;		// Axis 2 on target
			bool 		res2		: 1;		// Bits reserved
		};
	};
}STATUS_WORD;

//SYSTEM, DATARECORDER, AXIS, P_PID, C_PID, ENCODER, TRAJECTORY, PWM, PARAMETER, COMMAND

typedef enum
{
	ERR_SERVO_LOAD_2_HIGH 	= -1,
	ERR_NO_ERROR 			= 0,
	ERR_UNKNOWN_DEV_CLASS 	= 1,
	ERR_UNKNOWN_CMD 		= 2,
	ERR_CMD_LIST_ERROR 		= 3,
	ERR_MOD_NAME_UNKNOWN 	= 4,
	ERR_PAM_NON_EXIST 		= 5,
	ERR_UNKNOWN_MOD_ID 		= 6,
	ERR_UNKNOWN_INSTANCE 	= 7,
	ERR_UNKNOWN_PAM_ID 		= 8,
	ERR_UNKNOWN_PAM_TYPE	= 9,
	ERR_PAM_VALUE_INVALID 	= 10,
	ERR_PAM_LOCKED_MO_ON 	= 11,
	ERR_MOD_NO_PAM	 		= 12,
	ERR_MOD_NO_HLP			= 13,
	ERR_NO_CMD_END			= 14,
	ERR_PAM_LOCKED_AXS_ON 	= 100,
	ERR_AXS_DISABLED 		= 101,
	ERR_PAM_LOCKED_SVO_ON 	= 200,
	ERR_SVO_OFF 			= 201,
	ERR_SVO_ON 				= 202,
	ERR_PAM_READ_ONLY		= 203,
	ERR_PAM_OUT_OF_RANGE	= 204,
	ERR_VALUE_ERR			= 300,
	ERR_VARIABLE_STR_CONV	= 301,
	ERR_UNKNOWN_ERROR		= 1000
}ERROR_ID_NUM;

class error_class
{

    private:

		#define MAX_NUM_ERRORS 26
		const ERRORS my_sys_error_list[MAX_NUM_ERRORS] =
		{
				{ERR_SERVO_LOAD_2_HIGH	, "Warning, Servo-Load over 70%"}, 			// WARNING 	-> negative error number
				{ERR_NO_ERROR			, "No Error"},								// ERROR 	-> positive error number and zero
				{ERR_UNKNOWN_DEV_CLASS	, "Unknown DEVICE CLASS"},
				{ERR_UNKNOWN_CMD		, "Unknown Command"},
				{ERR_CMD_LIST_ERROR		, "Internal Command List Error"},
				{ERR_MOD_NAME_UNKNOWN	, "Module Name Unknown"},
				{ERR_PAM_NON_EXIST		, "Parameter does Not Exist"},
				{ERR_UNKNOWN_MOD_ID		, "Unknown Module Identifier"},
				{ERR_UNKNOWN_INSTANCE	, "Unknown Instance"},
				{ERR_UNKNOWN_PAM_ID		, "Unknown Parameter ID"},
				{ERR_UNKNOWN_PAM_TYPE	, "Unknown Parameter Type"},
				{ERR_PAM_VALUE_INVALID	, "Parameter Value not Valid"},
				{ERR_PAM_LOCKED_MO_ON	, "Parameter update not allowed, motion in progress"},
				{ERR_MOD_NO_PAM			, "Module has No Parameter"},
				{ERR_MOD_NO_HLP			, "Module Has No Help Entries"},
				{ERR_NO_CMD_END			, "No Semicolon at CMD End"},
				{ERR_PAM_LOCKED_AXS_ON	, "Module Parameter Locked - Axis Enabled"},		// from 100 Axis Parameter Errors
				{ERR_AXS_DISABLED		, "Axis Disabled"},
				{ERR_PAM_LOCKED_SVO_ON	, "Module Parameter Locked - Servo Running"},		// from 200 Servo Control Errors
				{ERR_SVO_OFF			, "Servo OFF"},
				{ERR_SVO_ON				, "Servo ON"},
				{ERR_PAM_READ_ONLY		, "Parameter is Read Only"},
				{ERR_PAM_OUT_OF_RANGE	, "Parameter Out of Range"},
				{ERR_VALUE_ERR			, "Invalid Value Format"},
				{ERR_VARIABLE_STR_CONV	, "Variable to String Convertion faild"	},
				{ERR_UNKNOWN_ERROR		, "Unknown Error"}
		};

        uint8_t cnt;

        // Error struct
        typedef struct
        {
            DEV_CLASS dev_class;
            int16_t error_value;
            char file[30];
            uint16_t line;
            char cmd_str[CMD_STR_LENGTH];
        }ERROR_LIST;

        ERROR_LIST myErrors[MAX_NUM_OF_ERRORS_IN_LIST];
        bool debug_out;

    public:
        DEV_CLASS_STRINGS dev_class_str[NUM_OF_DEV_CLASSES] =
        {
        		{"NON" ,"None"},
				{"SYS" ,"System"},
				{"DRC" ,"Data Recorder"},
				{"AXS" ,"Axis"},
				{"MCO" ,"Motion Controller"},
				{"CCO" ,"Current Controller"},
				{"ENC" ,"Encoder"},
				{"TRJ" ,"Trajectory Generator"},
				{"PWM" ,"Pulse Width Modulation"},
				{"PAM" ,"Parameter"},
				{"CMD" ,"Command Interpreter"},
				{"ERR" ,"Error Handler"},
        };

        error_class();
        bool add(DEV_CLASS dev_class, int16_t error, const char *file, uint16_t line);
        bool print_error_list();

        //const char dev_class_str[NUM_OF_DEV_CLASSES][4] = DEV_CLASS_STR // Module description strings
        bool print_error_ids(void);

        // status word
        STATUS_WORD  status;
        char last_cmd[CMD_STR_LENGTH]= {0};
        char* last_cmd_ptr;

		#define MAX_NUM_PARAMETER_AXIS 1
        // parameter TYPES: _UINT64, _UINT32, _UINT16, _INT64, _INT32, _INT16, _FLOAT, _DOUBLE, _STRING, _BOOL, _INIT
        PARAM_DEV module_parameter[MAX_NUM_PARAMETER_AXIS] =
        {
            {0x01, _UINT16, 	MAX_NUM_PARAMETER_AXIS, 0, 0, true, "Number of Axis Module Parameters"}, // every Module needs this parameter with number of
        };
};

#endif
