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
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "data_recorder.h"


extern "C" void send_string_DMA(const char* str, bool new_line);
extern "C" void send_string_USB(const char* str, bool new_line);



class ComInt_class
{
    public:

     // class pointer
		axis_class * 		myAxis[MAX_NUM_AXIS];
        error_class * 		myError;
        parameter_class * 	myParameter;
        data_recorder_class *	myDR;



        // Constructor
        ComInt_class();
        ComInt_class(error_class* myError_ptr);
        // Prototypes
        bool interpreter(char* str);


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
#define MAX_NUM_COMANDS 19
        const CMDTAB cmd_tab[MAX_NUM_COMANDS] = 
        {
        	{"APD", &ComInt_class::find_phase, 			"Auto Phase Angle Detection"},
        	{"CLT", &ComInt_class::start_current_test,	"Start Current Loop Test"},
        	{"EAX", &ComInt_class::enable_disable_axis,	"Enable/Disable Axis"},
            {"EXT", &ComInt_class::exit, 				"Exit"},
            {"ERR", &ComInt_class::get_error_list, 		"Get Error List"},
			{"GPL", &ComInt_class::get_parameter_hlp, 	"Get Parameter List"},
            {"GPV", &ComInt_class::get_parameter, 		"Get Parameter Value"},
            {"HLP", &ComInt_class::help, 				"Help "},
            {"IDN", &ComInt_class::get_dev_id, 			"Get Device Identification"},
            {"MOV", &ComInt_class::mov, 				"Set position for axis"},
			{"POS", &ComInt_class::qPOS, 				"Get position for axis"},
            {"RCL", &ComInt_class::run_current_loop, 	"Run Current Loop"},
            {"REC", &ComInt_class::rec, 				"Start data recorder immediately"},
            {"RRT", &ComInt_class::read_record_table, 	"Read Record Table"},
			{"SPA", &ComInt_class::set_parameter, 		"Set parameter value"},
            {"STA", &ComInt_class::status, 				"Get status"},
			{"STP", &ComInt_class::stop, 				"Stop Any Motion"},
			{"SVO", &ComInt_class::servo_on_off, 		"Enable/Disable Servo Control of Axis"},
        	{"SVT", &ComInt_class::start_svm_test, 		"Start SVM Test of Axis"}
        };

        PARAM_DEV return_parameter; // return parameter 
        CMD_STRUCT myCmd;

        // Helper functions
		bool seperate_cmd(char *str);
		DEV_CLASS arg2module(char* str);
		void print_cmd_list(void);
		void cpy_CMD_arguments_4_parameters(void);
		bool print_param_value(PARAM_DEV* param);
		bool convert_value_from_string(PARAM_DEV* param);
		bool get_parameter_hlp(void);
		void print_module_list(void);
		// Commands
		bool exit(void);
		bool get_parameter(void);
		bool help(void);
		bool get_dev_id(void);
		bool mov(void);
		bool rec(void);
		bool set_parameter(void);
		bool status(void);
		bool set_eeprom_parameter(void);
		bool get_error_list(void);
		bool qPOS(void);
		bool enable_disable_axis(void);
		bool servo_on_off(void);
		bool read_record_table(void);
		bool start_current_test(void);
		bool start_svm_test(void);
		bool find_phase(void);
		bool run_current_loop(void);
		bool stop(void);

};

#endif

