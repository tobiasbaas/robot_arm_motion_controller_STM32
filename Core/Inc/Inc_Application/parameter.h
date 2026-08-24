/***************************************************************************
 *  Parameter-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/


#ifndef _PARAMETER_H
#define _PARAMETER_H

#include "main.h"
#include "typedefs.h"
#include "controller_error.h"

extern "C" void send_string(const char* str, bool new_line);


class parameter_class
{     
    public:
        //constructor
        parameter_class();
        bool getParameter(uint16_t* param_id, PARAM_DEV* module_parameter);
        bool setParameter(CMD_STRUCT* cmd, PARAM_DEV* module_parameter);
        bool sendParameterHelp(uint16_t* param_id, PARAM_DEV* module_parameter);
        bool read_eeprom_parameter(PARAM_DEV *module_parameter, uint32_t address, int print);
        bool clear_parameter_flash(void);
        bool write_eeprom_parameter(PARAM_DEV *module_parameter, uint32_t address);
        void init_param(error_class* ErrorClass);
        bool print_param_value_raw(PARAM_TYPE type, UNION_VALUE value);



        error_class* myError;

        /* parameter TYPES: UINT64, UINT32, UINT16, INT64, INT32, INT16, FLOAT, DOUBLE, STRING, BOOL
         *
         * If you add a Parameter,
         */

        #define MAX_NUM_PARAMETER_PARM 1
        PARAM_DEV module_parameter[MAX_NUM_PARAMETER_PARM] =
        {
        	{0x01, _UINT16, MAX_NUM_PARAMETER_PARM, MAX_NUM_PARAMETER_PARM, MAX_NUM_PARAMETER_PARM, true, "Number of Parameters in Parameter Module"} // every Module needs this parameter with number of
        };
        
    private:
        const char param_type_str[NUM_OF_PARAM_TYPES][7] = PARAM_TYPE_STR // Module description strings
		bool convert_value_from_string(UNION_VALUE* value, PARAM_TYPE type, char* str, int32_t min, int32_t max);
        bool write_data_to_flash(uint64_t *flash_buffer_w,uint16_t size,uint32_t Address);
        uint32_t GetPage(uint32_t Addr);
};
#endif
