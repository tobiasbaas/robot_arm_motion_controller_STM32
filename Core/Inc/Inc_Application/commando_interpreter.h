/***************************************************************************
 *  Commandointerpreter-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#ifndef _COMMANDO_INTERPRETER_H
#define _COMMANDO_INTERPRETER_H

//#include "main.h"
#include "axis.h"
#include "parameter.h"
#include "controller_error.h"
#include "typedefs.h"
#include "data_recorder.h"
#include "stm32g4xx_hal_def.h"
#include "system.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" void send_string(const char* str, bool new_line);


class ComInt_class
{
    public:

        // class pointer
		axis_class * 			myAxis[MAX_NUM_AXIS];
        error_class * 			myError;
        parameter_class * 		myParameter;
        data_recorder_class *	myDR;
        system_class *			mySystem;

        // Constructor
        ComInt_class();
        ComInt_class(error_class* myError_ptr);
        
        // Prototypes
        bool interpreter(char* str);

        // Module Parameter
		#define MAX_NUM_PARAMETER_CMD_INT 1
		PARAM_DEV module_parameter[MAX_NUM_PARAMETER_CMD_INT] =
		{
			{0x01, _UINT16, 	MAX_NUM_PARAMETER_CMD_INT, MAX_NUM_PARAMETER_CMD_INT, MAX_NUM_PARAMETER_CMD_INT, true, "Number of Parameters in Command Interpreter Module"}, // every Module needs this parameter with number of
		};

    private:
        typedef bool (ComInt_class::* method_function)(void);

        typedef struct 
        {
            const char*			cmd;
            method_function		meth_fnc;
            const char*			description;
        }CMDTAB;
    
        // MÜSSEN APHABETISCH SORTIERT SEIN!!!!!!!
        // CMD-String, command function pinter, description
        #define MAX_NUM_COMANDS 26
        const CMDTAB cmd_tab[MAX_NUM_COMANDS] = 
        {
        	{"APD", &ComInt_class::find_phase, 			"Auto Phase Angle Detection"},
        	{"CLT", &ComInt_class::start_current_test,	"Start Current Loop Test"},
        	{"EAX", &ComInt_class::enable_disable_axis,	"Enable/Disable Axis"},
            {"EXT", &ComInt_class::exit, 				"Exit"},
            {"ERR", &ComInt_class::get_error_list, 		"Get Error List"},
			{"GPL", &ComInt_class::get_parameter_hlp, 	"Get Parameter List"},
            {"GPV", &ComInt_class::get_parameter, 		"Get Parameter Value"},
			{"GPE", &ComInt_class::get_eeprom_parameter,"Get all Module Parameter from EEPROM"},
            {"HLP", &ComInt_class::help, 				"Help "},
            {"IDN", &ComInt_class::get_dev_id, 			"Get Device Identification"},
            {"MOV", &ComInt_class::mov, 				"Set Target Position for Axis"},
			{"MVR", &ComInt_class::mvr, 				"Set Relative Target Position for Axis"},
			{"POS", &ComInt_class::qPOS, 				"Get position for axis"},
			{"RBT", &ComInt_class::reboot, 				"Reboot System"},
            {"RCL", &ComInt_class::run_current_loop, 	"Run Current Loop"},
            {"REC", &ComInt_class::rec, 				"Start data recorder immediately"},
			{"RPE", &ComInt_class::get_eeprom_parameter,"Reload all Module Parameter from EEPROM"},
            {"RRT", &ComInt_class::read_record_table, 	"Read Record Table"},
			{"RTB", &ComInt_class::read_record_table_binary, 	"Read Record Table Binary"},//NEW
			{"SPE", &ComInt_class::set_eeprom_parameter,"Write all Module Parameter to EEPROM"},
			{"SPV", &ComInt_class::set_parameter, 		"Set parameter value"},
            {"STA", &ComInt_class::status, 				"Get status"},
			{"STP", &ComInt_class::stop, 				"Stop Any Motion"},
			{"SVO", &ComInt_class::servo_on_off, 		"Enable/Disable Servo Control of Axis"},
        	{"SVT", &ComInt_class::start_svm_test, 		"Start SVM Test of Axis"},
        	{"ZXF", &ComInt_class::clear_FLASH, 		"RESET ALL FLASH VALUES PW: 1234"}
        };

        CMD_STRUCT myCmd;	// struct with CMD information

        // Helper functions
        bool seperate_cmd(char *str);
        DEV_CLASS arg2module(char* str);
        void print_cmd_list(void);
        void cpy_CMD_arguments_4_parameters(void);
        //bool print_param_value(PARAM_DEV* param);
        bool convert_value_from_string(PARAM_DEV* param);
        bool get_parameter_hlp(void);
        void print_module_list(void);
        bool write_data_to_flash(uint64_t *flash_buffer_w,uint16_t size,uint32_t Address);
        uint32_t GetPage(uint32_t Address);
        uint32_t flash_addr_axis(uint8_t instance, uint32_t addr_a1, uint32_t addr_a2);

        // Commands
        bool exit(void);
        bool get_parameter(void);
        bool help(void);
        bool get_dev_id(void);
        bool mov(void);
        bool mvr(void);
        bool rec(void);
        bool set_parameter(void);
        bool status(void);
        bool set_eeprom_parameter(void);
        bool get_eeprom_parameter(void);
        bool get_error_list(void);
        bool qPOS(void);
        bool enable_disable_axis(void);
        bool servo_on_off(void);
        bool read_record_table(void);
        bool read_record_table_binary(void);//NEW
        bool start_current_test(void);
        bool start_svm_test(void);
        bool find_phase(void);
        bool run_current_loop(void);
        bool stop(void);
        bool reboot(void);
        bool clear_FLASH(void);
};

#endif

