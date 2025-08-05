#include "commando_interpreter.h"

/*
--------------------------------------------------------------
Constructor
--------------------------------------------------------------
*/

ComInt_class::ComInt_class(error_class* myError_ptr)
{
    ComInt_class::myError 		= myError_ptr;	//Pointer Error Class
    ComInt_class::myParameter 	= NULL;			//Pointer Parameter Class
    ComInt_class::myDR 			= NULL;			//Pointer Data Recorder Class
}

/*
--------------------------------------------------------------
interpreter --> input: device string
--------------------------------------------------------------
*/
bool ComInt_class::interpreter(char *str)
{
    bool ret = false;

    ComInt_class::seperate_cmd(str);

    for(int n=0;n<MAX_NUM_COMANDS;n++)
    {
        if(strcmp(&this->myCmd.cmd_str[0],ComInt_class::cmd_tab[n].cmd)==0) // command found...
        {
            (this->*cmd_tab[n].meth_fnc)(); //calls function from table
            n = MAX_NUM_COMANDS;
            ret = true;
        }
    }
    // Error Command not found
    if(ret == false)
    {
        ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_CMD, __FILE__, __LINE__); // Unknown CMD
    }
    return ret; 
}

/*
--------------------------------------------------------------
Function to split the CMD into its arguments list 
Additionally, this function gets rid of all characters which are not supported by the device...
--------------------------------------------------------------
*/
bool ComInt_class::seperate_cmd(char *str)
{
    bool ret = true;
    char sign = 0;
    uint16_t n = 0;
    uint16_t m = 0;
    uint16_t cmd_args_cnt = 0;
    uint16_t s_cnt =0;

    for(uint16_t i=0;i<CMD_MAX_ARGS+2;i++)
    {
        this->myCmd.argument[i][0] = 0;
        this->myCmd.argument[i][1] = 0;
    }

    for(uint16_t i=0;i<256;i++)
    {
        if((str[i]>0x27) && (str[i]< 0x7B))    // not all signs allowed
        {
            if(str[i]>0x60) // transform all small letters to big letters
            {
                sign = str[i] - 32;
            }
            else
            {
                sign = str[i];
            }
            switch(sign)    // split string to argument list
            {
                case 0x3A:  // Spacer ":" --> new value pair
                    this->myCmd.argument[n][m] = 0; // end string array
                    if(m!=0)
                    {
                        n++;
                        if(n >= CMD_MAX_ARGS+2) // too many arguments
                        {
                            i = 256;
                        }
                        m=0;
                    }
                    break;

                case 0x3B: // semicolon --> end
                    this->myCmd.argument[n][m] = 0; // end string array
                    cmd_args_cnt = n; //set number of command arguments
                    i=256;
                    ret = true;
                    break;

                default:
                    this->myCmd.argument[n][m] = sign;
                    m++;
            }
            // store input string for error handling
            if((sign != 0x0A) && (sign != 0x0D)) // throw out LF and CR
            {
            	ComInt_class::myError->last_cmd[s_cnt] = sign;
            	s_cnt++;
            }
        }
    }
    //Terminate CMD-string for error handling
    ComInt_class::myError->last_cmd[s_cnt] = 0;

    this->myCmd.dv_module = NON_MOD;
    this->myCmd.i_instance = 100;
    // prepare for Parameter (not used in all CMDs, but always prepared)
    this->myCmd.i_address = 0;       // Address number
    this->myCmd.i_numerator = 0;
    this->myCmd.i_denominator = 0;

    // copy cmd to myCmd
    strcpy(&this->myCmd.cmd_str[0],&this->myCmd.argument[0][0]);
    if(cmd_args_cnt >= 1)
    {
        // get module and instance if available
        for(uint16_t i=0;i<8;i++)
        {
            if(this->myCmd.argument[1][i] == 0x5F)                              //underscore
            {
                this->myCmd.module_str[i] = 0;                      // module string end
                // poof for valid instance number
                if((this->myCmd.argument[1][i+1]>0x2F)&&(this->myCmd.argument[1][i+1]<0x3A))//  only 0 to 9
                {
                    this->myCmd.i_instance = (uint16_t)(this->myCmd.argument[1][i+1] - 0x30);
                }
                i=8;
            }
            else
            {
                this->myCmd.module_str[i] = this->myCmd.argument[1][i];
                if(this->myCmd.argument[1][i] == 0)                                     //string end
                {
                    i=8;
                }
            }
        }
        // get module number
        this->myCmd.dv_module = ComInt_class::arg2module(&this->myCmd.module_str[0]);
    }
    return ret; // negative return, no string end
}

/*
--------------------------------------------------------------
get module number from command argument
--------------------------------------------------------------
*/
DEV_CLASS ComInt_class::arg2module(char* str)
{
    DEV_CLASS module = NON_MOD;
    // find Module 
    for(uint16_t i=0; i<NUM_OF_DEV_CLASSES; i++)
    {
        if( strcmp(str,&ComInt_class::myError->dev_class_str[i].cmd[0]) == 0 )
        {
            module = (DEV_CLASS)i;
            i = NUM_OF_DEV_CLASSES;
        }
    }
    if( module == NON_MOD) // Module not found
    {
        ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NAME_UNKNOWN, __FILE__, __LINE__); // Module Name not Found
    }
    return module;
}



bool ComInt_class::exit(void)
{

	//send_string_DMA("Function Call: exit",true);
	send_string_USB("Function Call: exit",true);

    return false;
}


void ComInt_class::cpy_CMD_arguments_4_parameters(void)
{
	char * p;
	if(this->myCmd.argument[2][1] == 0x58){
		this->myCmd.i_address 		= (uint16_t)strtol(&this->myCmd.argument[2][2], & p, 16);	// first argument in Parameter commands is adress
	}
	else{
		this->myCmd.i_address 		= (uint16_t)strtol(&this->myCmd.argument[2][0], & p, 10);	// first argument in Parameter commands is adress
	}
	if ( * p != 0 ) { // Address conversion failed
		ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // 	{8, "Unknown Parameter ID"},
	}

	//	= (int64_t)atoi(&this->myCmd.argument[3][0]); 	// second argument in Parameter commands is value


	this->myCmd.i_numerator		= atoi(&this->myCmd.argument[4][0]);	// third (optional) argument in Parameter commands is numerator
	this->myCmd.i_denominator 	= atoi(&this->myCmd.argument[5][0]); 	// 4th (optional) argument in Parameter commands is denominator
}

/*
--------------------------------------------------------------
print parameter value
--------------------------------------------------------------
 _UINT64, , _UINT16, _INT64, _INT32, _INT16, _FLOAT, _DOUBLE, _STRING, _BOOL, _INIT, POS_INT

 */
bool ComInt_class::convert_value_from_string(PARAM_DEV* param)
{
	char str[25];
	long int tmp = 0;
	switch(param->type)
	{
		case _BOOL:
			sprintf(str,"%u",(uint16_t)param->value._bool);
			break;
		case _UINT16:
			sprintf(str,"%u",param->value.ui16);
			break;
		case _UINT32:
			sprintf(str,"%u",param->value.ui32);
			break;
		case _INT64:
			sprintf(str,"%ld",param->value.i64);
			break;
		case _INT16:
			sprintf(str,"%d",param->value.i16);
			break;
		case _INT32:
			sprintf(str,"%d",param->value.i32);
			break;
		case _DOUBLE:
			sprintf(str,"%5.10f",param->value.d64);
			break;
		case _FLOAT:
			sprintf(str,"%4.5f",param->value.f32);
			break;
		case POS_INT:
			//tmp = (param->value.i64 * param->numerator)/param->denominator;
			tmp = param->value.i64;
			sprintf(str,"%ld",tmp);
			break;
		default:
			ComInt_class::myError->add(COMMAND_MOD, ERR_VALUE_ERR, __FILE__, __LINE__); // ??? Fehler anlegen!
	}
	//send_string_DMA(str,true);
	send_string_USB(str,true);
	return true;
}


/*
--------------------------------------------------------------
COMMAND SET PARAMETERS --> "SPA"
--------------------------------------------------------------
*/
bool ComInt_class::set_eeprom_parameter(void)
{
    bool ret = true;
    //send_string_DMA("Function Call: set_eeprom_parameter",true);
    send_string_USB("Function Call: set_eeprom_parameter",true);


    // get parameter values from cmd arguments
    ComInt_class::cpy_CMD_arguments_4_parameters();

    switch(this->myCmd.dv_module)
    {
		case NON_MOD:

			break;
        case SYSTEM_MOD:

            break;
        case DATARECORDER_MOD:

            break;
        case AXIS_MOD:

        	//ret = this->myParameter->setParameter(&this->return_parameter, this->myCmd.address_ptr, this->myAxis[this->myCmd.i_instance]->module_parameter);
            break;
        case MOTION_PID_MOD:

            break;
        case C_PID_MOD:

            break;
        case ENCODER_MOD:

            break;
        case TRAJECTORY_MOD:

            break;
        case PWM_MOD:

            break;
        case PARAMETER_MOD:
            //ret = this->myParameter->setParameter(&this->return_parameter, this->myCmd.i_address, this->myParameter->module_parameter);
            break;
        case COMMAND_MOD:

            break;
        default:
            ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // Unknown Module Identifier
            ret = false;
    }
    return ret;
}

/*
--------------------------------------------------------------
COMMAND SET PARAMETER --> "SPA"
--------------------------------------------------------------
*/
bool ComInt_class::set_parameter(void)
{
    bool ret = true;

    // get parameter values from cmd arguments
    ComInt_class::cpy_CMD_arguments_4_parameters();

    switch(this->myCmd.dv_module)
    {
		case NON_MOD:
			ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
			break;
        case SYSTEM_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
            break;
        case DATARECORDER_MOD:
        	ret = this->myDR->setParameter(&this->myCmd);
            break;
        case AXIS_MOD:
        	ret = this->myAxis[this->myCmd.i_instance]->setParameter(&this->myCmd);
            break;
        case MOTION_PID_MOD:
			ret = this->myAxis[this->myCmd.i_instance]->myPid_Pos->setParameter(&this->myCmd);
            break;
        case C_PID_MOD:
			ret = this->myAxis[this->myCmd.i_instance]->myCurrent_Control->setParameter(&this->myCmd);
            break;
        case ENCODER_MOD:
        	ret = this->myAxis[this->myCmd.i_instance]->myPos->setParameter(&this->myCmd);
            break;
        case TRAJECTORY_MOD:
        	ret = this->myAxis[this->myCmd.i_instance]->myTrajectory->setParameter(&this->myCmd);
            break;
        case PWM_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
            break;
        case PARAMETER_MOD:
        	ret = this->myParameter->setParameter(&this->myCmd,this->myParameter->module_parameter);
            break;
        case COMMAND_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
            break;
        default:
            ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // Unknown Module Identifier
            ret = false;
    }
    return ret;
}

/*
--------------------------------------------------------------
COMMAND SET PARAMETER --> "SPA"
--------------------------------------------------------------
*/
bool ComInt_class::get_parameter_hlp(void)
{
    bool ret 		= true;
    bool no_param 	= false;

    PARAM_DEV*		param_ptr;
    // get parameter values from cmd arguments
    ComInt_class::cpy_CMD_arguments_4_parameters();

    switch(this->myCmd.dv_module)
    {
		case NON_MOD:
			no_param = true;
			break;
        case SYSTEM_MOD:
        	no_param = true;
            break;
        case DATARECORDER_MOD:
        	param_ptr = this->myDR->module_parameter;
            break;
        case AXIS_MOD:
        	param_ptr = this->myAxis[0]->module_parameter;
            break;
        case MOTION_PID_MOD:
        	param_ptr = this->myAxis[0]->myPid_Pos->module_parameter;
            break;
        case C_PID_MOD:
        	param_ptr = this->myAxis[0]->myCurrent_Control->module_parameter;
            break;
        case ENCODER_MOD:
        	param_ptr = this->myAxis[0]->myPos->module_parameter;
            break;
        case TRAJECTORY_MOD:
        	param_ptr = this->myAxis[0]->myTrajectory->module_parameter;
            break;
        case PWM_MOD:
        	no_param = true;
            break;
        case PARAMETER_MOD:
        	param_ptr = this->myParameter->module_parameter;
            break;
        case COMMAND_MOD:
        	no_param = true;
            break;
        default:
            ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // Unknown Module Identifier
            ret = false;
    }
    if(no_param == false)
    {
    	ret = this->myParameter->sendParameterHelp(&this->myCmd.i_address,param_ptr);
    }
    else
    {
    	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
    }

    return ret;
}


/*
--------------------------------------------------------------
COMMAND GET PARAMETERS --> "GPA"
--------------------------------------------------------------
*/
bool ComInt_class::get_parameter(void)
{
    bool ret 						= true;
    bool no_param 					= false;
    PARAM_DEV* modul_parameter_ptr 	= 0;

    ComInt_class::cpy_CMD_arguments_4_parameters();

    switch(this->myCmd.dv_module)
    {
		case NON_MOD:
			no_param = true;
			break;
        case SYSTEM_MOD:
        	no_param = true;
            break;
        case DATARECORDER_MOD:
        	modul_parameter_ptr = this->myDR->module_parameter;
            break;
        case AXIS_MOD:
        	modul_parameter_ptr = this->myAxis[this->myCmd.i_instance]->module_parameter;
            break;
        case MOTION_PID_MOD:
        	modul_parameter_ptr = this->myAxis[this->myCmd.i_instance]->myPid_Pos->module_parameter;
            break;
        case C_PID_MOD:
        	modul_parameter_ptr = this->myAxis[this->myCmd.i_instance]->myCurrent_Control->module_parameter;
            break;
        case ENCODER_MOD:
        	modul_parameter_ptr = this->myAxis[this->myCmd.i_instance]->myPos->module_parameter;
            break;
        case TRAJECTORY_MOD:
        	modul_parameter_ptr = this->myAxis[this->myCmd.i_instance]->myTrajectory->module_parameter_ptr;
            break;
        case PWM_MOD:
        	no_param = true;
            break;
        case PARAMETER_MOD:
        	modul_parameter_ptr = this->myParameter->module_parameter;
            break;
        case COMMAND_MOD:
        	no_param = true;
            break;
        default:
            ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // Unknown Module Identifier
            ret = false;
    }
    if(no_param == true)
    {
    	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // no Parameter in Module
    }
    else
    {
    	ret = this->myParameter->getParameter(&this->return_parameter, &this->myCmd.i_address, modul_parameter_ptr);
    	if(ret == true)
    	{
    		ComInt_class::print_param_value(&this->return_parameter);
    	}
    }
    return ret;
}

/*
--------------------------------------------------------------
pront parameter value
--------------------------------------------------------------
 _UINT64, , _UINT16, _INT64, _INT32, _INT16, _FLOAT, _DOUBLE, _STRING, _BOOL, _INIT, POS_INT

 */
bool ComInt_class::print_param_value(PARAM_DEV* param)
{
	char str[100];
	long int tmp = 0;
	switch(param->type)
	{
		case _BOOL:
			sprintf(str,"%u",(uint16_t)param->value._bool);
			break;
		case _UINT64:
			sprintf(str,"%u",param->value.ui64);
			break;
		case _UINT16:
			sprintf(str,"%u",param->value.ui16);
			break;
		case _UINT32:
			sprintf(str,"%u",param->value.ui32);
			break;
		case _INT64:
			sprintf(str,"%ld",param->value.i64);
			break;
		case _INT16:
			sprintf(str,"%d",param->value.i16);
			break;
		case _INT32:
			sprintf(str,"%d",param->value.i32);
			break;
		case _DOUBLE:
			sprintf(str,"%5.10f",param->value.d64);
			break;
		case _FLOAT:
			sprintf(str,"%4.5f",param->value.f32);
			break;
		case POS_INT:
			//tmp = (param->value.i64 * param->numerator)/param->denominator;
			tmp = param->value.i64;
			sprintf(str,"%ld",tmp);
			break;
		default:
			ComInt_class::myError->add(COMMAND_MOD, ERR_VARIABLE_STR_CONV, __FILE__, __LINE__); // ??? Fehler anlegen!
	}
	//send_string_DMA(str,true);
	send_string_USB(str,true);

	return true;
}

/*
--------------------------------------------------------------
COMMAND HELP --> "HLP"
--------------------------------------------------------------
*/
bool ComInt_class::help(void)
{
    bool ret = true;

    switch(this->myCmd.dv_module)
    {
    	case NON_MOD:
    		this->print_cmd_list();
    		break;
        case SYSTEM_MOD:
        	this->print_module_list();
            break;
        case DATARECORDER_MOD:
        	ret = this->myDR->send_DR_hlp();
            break;
        case AXIS_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_HLP, __FILE__, __LINE__); // Module has no help entries
            break;
        case MOTION_PID_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_HLP, __FILE__, __LINE__); // Module has no help entries
            break;
        case C_PID_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_HLP, __FILE__, __LINE__); // Module has no help entries
            break;
        case ENCODER_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_HLP, __FILE__, __LINE__); // Module has no help entries
            break;
        case TRAJECTORY_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_HLP, __FILE__, __LINE__); // Module has no help entries
            break;
        case PWM_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_HLP, __FILE__, __LINE__); // Module has no help entries
            break;
        case PARAMETER_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_HLP, __FILE__, __LINE__); // Module has no help entries
            break;
        case COMMAND_MOD:
        	this->print_cmd_list();
            break;
        case ERROR_MOD:
        	this->myError->print_error_ids();
            break;
        default:
            this->myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // Unknown Module Identifier
            ret = false;
    }

    return ret;
}

/*
--------------------------------------------------------------
Print Command List
--------------------------------------------------------------
*/
void ComInt_class::print_module_list(void)
{
	char str[40] = {0};

	//send_string_DMA("#MODULE LIST:",true);
	send_string_USB("#MODULE LIST:",true);

	for(uint16_t i = 0;i<NUM_OF_DEV_CLASSES;i++)
	{
		sprintf(str,"%s: %s",this->myError->dev_class_str[i].cmd, this->myError->dev_class_str[i].description);
		//send_string_DMA(str,true);
		send_string_USB(str,true);

	}
	//send_string_DMA("#END LIST",true);
	send_string_USB("#END LIST",true);

}

/*
--------------------------------------------------------------
Print Command List
--------------------------------------------------------------
*/
void ComInt_class::print_cmd_list(void)
{
	//send_string_DMA("#COMMAND LIST:",true);
	send_string_USB("#COMMAND LIST:",true);

	for(uint16_t i = 0;i<MAX_NUM_COMANDS;i++)
	{
		//send_string_DMA(this->cmd_tab[i].cmd,false);
		//send_string_DMA(": ",false);
		//send_string_DMA(this->cmd_tab[i].description,true);
		send_string_USB(this->cmd_tab[i].cmd,false);
		send_string_USB(": ",false);
		send_string_USB(this->cmd_tab[i].description,true);

	}
	//send_string_DMA("#END LIST",true);
	send_string_USB("#END LIST",true);

}
/*
--------------------------------------------------------------
COMMAND GET DEVICE ID --> "IDN"
--------------------------------------------------------------
*/
bool ComInt_class::get_dev_id(void)
{

	//send_string_DMA("HKA - LIS Motion Control Firmware",true);
	//send_string_DMA("Motion Controller: ",false);
	//send_string_DMA(CONTROLLER_NAME_STR,true);
	//send_string_DMA("Version: ",false);
	//send_string_DMA(VERSION_STR,true);
	//send_string_DMA(CPY_STR,true);
	send_string_USB("HKA - LIS Motion Control Firmware",true);
	send_string_USB("Motion Controller: ",false);
	send_string_USB(CONTROLLER_NAME_STR,true);
	send_string_USB("Version: ",false);
	send_string_USB(VERSION_STR,true);
	send_string_USB(CPY_STR,true);
    return true;
}
/*
--------------------------------------------------------------
COMMAND MOV AXIS --> "MOV"
--------------------------------------------------------------
*/
bool ComInt_class::mov(void)
{
	bool ret = true;
	char * p;

	float target = strtof(&this->myCmd.argument[2][0], & p);
	if ( * p != 0 ) { // Address conversion failed
		ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // 	{8, "Unknown Parameter ID"
		ret = false;
	}
	else
	{
		this->myAxis[this->myCmd.i_instance]->myTrajectory->set_target(target);
		this->myDR->trigger_start(DR_TGR_MOV);
	}
    return ret;
}


/*
--------------------------------------------------------------
COMMAND MOV AXIS --> "MOV"
--------------------------------------------------------------
*/
bool ComInt_class::qPOS(void)
{
	bool ret = true;
	char str[20];

	sprintf(str,"%4.5f",this->myAxis[this->myCmd.i_instance]->myPos->position);

	//send_string_DMA(str,true);
	send_string_USB(str,true);


    return ret;
}

/*
--------------------------------------------------------------
COMMAND MOV AXIS --> "MOV"
--------------------------------------------------------------
*/
bool ComInt_class::enable_disable_axis(void)
{
	bool ret = true;
	char * p;

	bool cmd = (bool)strtol(&this->myCmd.argument[2][0], & p, 10);
	if ( * p != 0 ) { // Address conversion failed
		ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // 	{8, "Unknown Parameter ID"
		ret = false;
	}
	else
	{
		if(cmd == true)
		{
			this->myAxis[this->myCmd.i_instance]->enable();
		}
		else
		{
			this->myAxis[this->myCmd.i_instance]->disable();
		}
	}
    return ret;
}



/*
--------------------------------------------------------------
COMMAND MOV AXIS --> "MOV"
--------------------------------------------------------------
*/
bool ComInt_class::servo_on_off(void)
{
	bool ret = true;
	char * p;

	bool cmd = (bool)strtol(&this->myCmd.argument[2][0], & p, 10);
	if ( * p != 0 ) { // Address conversion failed
		ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // 	{8, "Unknown Parameter ID"
		ret = false;
	}
	else
	{
			ret = this->myAxis[this->myCmd.i_instance]->myPid_Pos->servo_on_off(cmd);
	}
	return ret;
}



/*
--------------------------------------------------------------
COMMAND START RECORDING --> "REC"
--------------------------------------------------------------
*/
bool ComInt_class::rec(void)
{
	this->myDR->trigger_start(DR_TGR_MOV);
    return true;
}


/*
--------------------------------------------------------------
COMMAND START RECORDING --> "REC"
--------------------------------------------------------------
*/
bool ComInt_class::run_current_loop(void)
{
	bool ret = true;

	return ret;
}




/*
--------------------------------------------------------------
COMMAND Find Phase Algorithm start
--------------------------------------------------------------
*/
bool ComInt_class::find_phase(void)
{

    return true;
}



/*
--------------------------------------------------------------
COMMAND GET STATUS --> "STA"
--------------------------------------------------------------
*/
bool ComInt_class::status(void)
{
	char str[24];

	this->myError->status.res0 	= 0;
	this->myError->status.a0en 	= this->myAxis[0]->enabled;
	this->myError->status.a0so 	= this->myAxis[0]->myPid_Pos->servo_on;
	this->myError->status.a0ont = this->myAxis[0]->myTrajectory->ont;
	this->myError->status.res1 	= 0;

#if MAX_NUM_AXIS == 2
	this->myError->status.a1en 	= this->myAxis[1]->enabled;
	this->myError->status.a1so 	= this->myAxis[1]->myPid_Pos->servo_on;
	this->myError->status.a1ont = this->myAxis[1]->myTrajectory->ont;
	this->myError->status.res2 	= 0;
#endif
	if(this->myError->status.all >= 4096)
	{
		sprintf(str,"*STA: 0x%x",this->myError->status.all);
	}
	else if(this->myError->status.all >= 256)
	{
		sprintf(str,"*STA: 0x0%x",this->myError->status.all);
	}
	else if(this->myError->status.all >= 16)
	{
		sprintf(str,"*STA: 0x00%x",this->myError->status.all);
	}
	else
	{
		sprintf(str,"*STA: 0x000%x",this->myError->status.all);
	}
	//send_string_DMA(str,true);
	send_string_USB(str,true);

    return true;
}



/*
--------------------------------------------------------------
COMMAND SAVE PARAMETERS TO EPROM --> "SEP"
--------------------------------------------------------------
*/
bool ComInt_class::get_error_list(void)
{
	//send_string_DMA("Function Call: get_error_list",true);
	send_string_USB("Function Call: get_error_list",true);
    ComInt_class::myError->print_error_list();
    return true;
}

/*
--------------------------------------------------------------
COMMAND Start Current Loop Test FOC --> "CLT"
--------------------------------------------------------------
*/
bool ComInt_class::start_current_test(void)
{

    return true;
}


/*
--------------------------------------------------------------
COMMAND Start SVM Test --> "SVT"
--------------------------------------------------------------
*/
bool ComInt_class::start_svm_test(void)
{

    return true;
}


/*
--------------------------------------------------------------
COMMAND Start SVM Test --> "SVT"
--------------------------------------------------------------
*/
bool ComInt_class::stop(void)
{
	if( (this->myCmd.i_instance >= 0) && (this->myCmd.i_instance < MAX_NUM_AXIS) )
	{
		this->myAxis[this->myCmd.i_instance]->disable();
	}
	else
	{
		for(uint8_t i =0;i<MAX_NUM_AXIS;i++)
		{
			this->myAxis[i]->disable();
		}
	}

    return true;
}



/*
--------------------------------------------------------------
COMMAND READ RECORD TABLE --> "RRT"
--------------------------------------------------------------
*/
bool ComInt_class::read_record_table(void)
{
	this->myDR->send_data();
    return true;
}

