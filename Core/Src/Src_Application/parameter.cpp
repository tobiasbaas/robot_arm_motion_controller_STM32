/***************************************************************************
 *  Parameter-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#include "parameter.h"

parameter_class::parameter_class()
{
    parameter_class::myError = 0;
}

void parameter_class::init_param(error_class* ErrorClass)
{
    parameter_class::myError = ErrorClass;
}

bool parameter_class::getParameter(PARAM_DEV* call_param, uint16_t* param_id, PARAM_DEV* module_parameter)
{
    bool ret = false;

    uint16_t len = module_parameter[0].value.ui16; // table length

    //send_string_DMA("GET PARAMEER of PARAMETER UNIT",true);
    //send_string_USB("GET PARAMEER of PARAMETER UNIT",true);

    for(uint16_t i = 0; i < len; i++)
    {
        if(*param_id == module_parameter[i].adress)
        {
            call_param->adress 		= module_parameter[i].adress;
            //call_param->numerator   = module_parameter[i].numerator;
            //call_param->denominator = module_parameter[i].denominator;
            call_param->type 		= module_parameter[i].type;
            call_param->value 		= module_parameter[i].value;
            ret = true;
            i = len; // end for loop
        }
    }

    if(ret == false)
    {
        parameter_class::myError->add(PARAMETER_MOD, ERR_PAM_NON_EXIST,__FILE__, __LINE__); // Parameter does not exist not Found
    }
    return ret;
}

bool parameter_class::setParameter(CMD_STRUCT* cmd, PARAM_DEV* module_parameter)
{
    bool ret = false;

    //send_string_DMA("SET PARAMEER of PARAMETER UNIT",true);
    //send_string_USB("SET PARAMEER of PARAMETER UNIT",true);

    uint16_t len = module_parameter[0].value.ui16; // table length

    for(uint16_t i = 0; i < len; i++)
    {
        if(cmd->i_address == module_parameter[i].adress)
        {
        	if(module_parameter[i].read_only)
        	{
        		this->myError->add(PARAMETER_MOD, ERR_PAM_READ_ONLY, __FILE__, __LINE__); // Parameter does not exist not Found
        		ret = true;
        	}
        	else
        	{
        		ret = parameter_class::convert_value_from_string(&cmd->value,  module_parameter[i].type, &cmd->argument[3][0], module_parameter[i].minimum, module_parameter[i].maximum);
				//module_parameter[i].numerator 	= cmd->i_numerator;
				//module_parameter[i].denominator = cmd->i_denominator;
        		if(ret == true)
        		{
        			module_parameter[i].value.ui64 	= cmd->value.ui64;
        		}
        		else
        		{
        			ret = true;
        		}
				i = len; // end for loop
        	}
        }
    }
	
    if(ret == false)
    {
        this->myError->add(PARAMETER_MOD, ERR_PAM_NON_EXIST, __FILE__, __LINE__); // Parameter does not exist not Found
        ret = true;
    }
    return ret;
}

bool parameter_class::convert_value_from_string(UNION_VALUE* value, PARAM_TYPE type, char* str, int32_t min, int32_t max)
{
	char * p;
	long int tmp = 0;
	bool ret = true;

	switch(type)
	{
		case _BOOL:
			tmp = strtol(str,& p, 10);
			switch(tmp)
			{
				case 0:
					value->_bool = true;
				case 1:
					value->_bool = false;
				default:
					 ret = false;
			}
			break;
		case _UINT64:	// min and max only 32 bit!!!!
			value->ui64 = strtol(str,& p, 10);
			if((value->ui64 < min) || (value->ui64 > max))
			{
				 ret = false;
			}
			break;
		case _UINT16:
			value->ui16 = strtol(str,& p, 10);
			if((value->ui16 < min) || (value->ui16 > max))
			{
				ret = false;
			}
			break;
		case _UINT32:
			value->ui32 = strtol(str,& p, 10);
			if((value->ui32 < min) || (value->ui32 > max))
			{
				ret = false;
			}
			break;
		case _INT64:	// min and max only 32 bit!!!!
			value->i64 = strtol(str,& p, 10);
			if((value->i64 < min) || (value->i64 > max))
			{
				ret = false;
			}
			break;
		case _INT16:
			value->i16 = strtol(str,& p, 10);
			if((value->i16 < min) || (value->i16 > max))
			{
				ret = false;
			}

			break;
		case _INT32:
			value->i32 = strtol(str,& p, 10);
			if((value->i32 < min) || (value->i32 > max))
			{
				ret = false;
			}
			break;
		case _DOUBLE:
			value->d64 = strtod(str,& p);
			if((value->d64 < min) || (value->d64 > max))
			{
				ret = false;
			}
			break;
		case _FLOAT:
			value->f32 = strtod(str,& p);
			if((value->f32 < min) || (value->f32 > max))
			{
				ret = false;
			}
			value->f32 = strtod(str,& p);
			break;
		case POS_INT:
			value->i64 = strtol(str,& p, 10);
			if((value->i64 < min) || (value->i64 > max))
			{
				ret = false;
			}
			break;
		case _STRING:
			strcpy(value->str8,str);
		default:
			ret = false;
	}
	if(ret == false)
	{
		 this->myError->add(PARAMETER_MOD, ERR_PAM_OUT_OF_RANGE, __FILE__, __LINE__); // Parameter out of range
	}
	return ret;
}

bool parameter_class::sendParameterHelp(uint16_t* param_id, PARAM_DEV* module_parameter)
{
    bool ret = false;

    char str[80]= {0};

    // copy module name from first parameter description
    uint16_t len = sizeof(module_parameter[0].description);
    str[0] = 0x23;
    str[1] = 0x20;
    for(uint8_t i=10;i<len;i++)
    {
    	char tmp = module_parameter[0].description[i];
    	if(tmp == 0)
    	{
    		i = len;	//end
    	}
    	str[i-8] = tmp;
    }

    //send_string_DMA(str,true);
    send_string_USB(str,true);



    len = module_parameter[0].value.ui16; // table length

    // run table an get information to send
    for(uint16_t i=0;i<len;i++)
    {
    	// check for ID, if ID is 0x00, print whole table
        if((module_parameter[i].adress == *param_id)||(*param_id == 0))
        {
        	sprintf(str,"0x%02x Type: %s Min: %d, Max: %d - %s",module_parameter[i].adress,  this->param_type_str[module_parameter[i].type],module_parameter[i].minimum, module_parameter[i].maximum, module_parameter[i].description);
        	//send_string_DMA(str,true);  // send parameter info...
        	send_string_USB(str,true);  // send parameter info...
            ret = true;
        }
    }
    //send_string_DMA("#END LIST",true); // List end...
    send_string_USB("#END LIST",true); // List end...

    return ret;
}

