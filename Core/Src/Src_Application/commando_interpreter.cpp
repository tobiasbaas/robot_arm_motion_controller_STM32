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

    ret = ComInt_class::seperate_cmd(str);

    if(ret == true)
    {
    	ret = false;	// reset to detect unknown CMD
    	for(int n=0; n<MAX_NUM_COMANDS; n++)
		{
			if(strcmp(&this->myCmd.cmd_str[0], ComInt_class::cmd_tab[n].cmd) == 0) // command found...
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
    bool ret = false;
    char sign = 0;
    uint16_t number_args = 0;
    uint16_t m = 0;
    uint16_t s_cnt =0;
    uint16_t str_len = strlen(str);

    for(uint16_t i=0;i<CMD_MAX_ARGS+2;i++)
    {
        this->myCmd.argument[i][0] = 0;
        this->myCmd.argument[i][1] = 0;
    }

    // Lese nur innerhalb des Puffers (CMD_STR_LENGTH), nicht fix 256 Bytes
    for(uint16_t i=0;(i<str_len) && (i<CMD_STR_LENGTH);i++)
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
                    this->myCmd.argument[number_args][m] = 0; // end string array
                    if(m!=0)
                    {
                        number_args++;
                        if(number_args >= CMD_MAX_ARGS+2) // too many arguments
                        {
                            i = CMD_STR_LENGTH;
                        }
                        m=0;
                    }
                    break;

                case 0x3B: // semicolon --> end
                    this->myCmd.argument[number_args][m] = 0; // end string array
                    i=CMD_STR_LENGTH;
                    ret = true;
                    break;

                default:
                    // max. 19 Zeichen pro Argument (20. Position bleibt Terminator)
                    if(m < 19)
                    {
                        this->myCmd.argument[number_args][m] = sign;
                        m++;
                    }
            }
            // store input string for error handling
            if((sign != 0x0A) && (sign != 0x0D)) // throw out LF and CR
            {
                // last_cmd-Puffer (CMD_STR_LENGTH) nicht ueberlaufen
                if(s_cnt < CMD_STR_LENGTH)
                {
                	ComInt_class::myError->last_cmd[s_cnt] = sign;
                	s_cnt++;
                }
            }
        }
    }
    if(ret == false)
    {
    	ComInt_class::myError->add(COMMAND_MOD, ERR_NO_CMD_END, __FILE__, __LINE__); // Unknown CMD
    	return ret; // negative return, no string end
    }
    //Terminate CMD-string for error handling
    if(s_cnt < CMD_STR_LENGTH)
    {
    	ComInt_class::myError->last_cmd[s_cnt] = 0;
    }
    else
    {
    	ComInt_class::myError->last_cmd[CMD_STR_LENGTH-1] = 0;
    }

    this->myCmd.dv_module = NON_MOD;	// no module
    this->myCmd.i_instance = 100; 		// no instance
    // prepare for Parameter (not used in all CMDs, but always prepared)
    this->myCmd.i_address = 0;       // Address number
    this->myCmd.i_numerator = 0;
    this->myCmd.i_denominator = 0;


    // copy cmd to myCmd (cmd_str hat nur 4 Zeichen --> begrenzt kopieren)
    strncpy(&this->myCmd.cmd_str[0],&this->myCmd.argument[0][0],3);
    this->myCmd.cmd_str[3] = 0;

    // get module and instance
    if(number_args >= 1)
    {
    	// set ret to false to proof for valid entries...
    	ret = false;

        for(uint16_t i=0;i<8;i++)
        {
            if(this->myCmd.argument[1][i] == 0x5F)  //underscore
            {
                // Modulname immer null-terminieren (module_str hat nur 4 Zeichen)
                if(i < 4)
                {
                	this->myCmd.module_str[i] = 0;                      // module string end
                }
                else
                {
                	this->myCmd.module_str[3] = 0;   // Name > 3 Zeichen --> kuerzen
                }
                // poof for valid instance number
                if((i+1 < 20) && (this->myCmd.argument[1][i+1]>0x2F)&&(this->myCmd.argument[1][i+1]<0x3A))//  only 0 to 9
                {
                    this->myCmd.i_instance = (uint16_t)(this->myCmd.argument[1][i+1] - 0x30);
					if(this->myCmd.i_instance < MAX_NUM_AXIS)	// NEW: check instance range
					{
						ret = true; // module and instance found...
					}
					else
					{
						ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_INSTANCE, __FILE__, __LINE__); // instance out of range
						ret = false;
					}
                }
                i=8;
            }
            else
            {
                if(i < 4)
                {
                	this->myCmd.module_str[i] = this->myCmd.argument[1][i];
                }
                if(this->myCmd.argument[1][i] == 0)                                     //string end
                {
                    i=8;
                    ret = false;
                }
            }
        }
        if(ret == true)
        {
        	// get module number
        	this->myCmd.dv_module = ComInt_class::arg2module(&this->myCmd.module_str[0]);
        }
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

/*
--------------------------------------------------------------
Select FLASH address depending on axis instance
--------------------------------------------------------------
*/
uint32_t ComInt_class::flash_addr_axis(uint8_t instance, uint32_t addr_a1, uint32_t addr_a2)
{
	return (instance == 0) ? addr_a1 : addr_a2;
}

bool ComInt_class::exit(void)
{
	send_string("Function Call: exit", true);
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
	send_string(str,true);
	return true;
}


/*
--------------------------------------------------------------
COMMAND SET EEPROM PARAMETERS --> "SPE"
--------------------------------------------------------------
*/
bool ComInt_class::set_eeprom_parameter(void)
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
        	this->myParameter->write_eeprom_parameter(this->mySystem->module_parameter , ADDR_FLASH_PAGE_SYSTEM);
            break;

        case DATARECORDER_MOD:
        	this->myParameter->write_eeprom_parameter(this->myDR->module_parameter , ADDR_FLASH_DRC);
            break;

        case AXIS_MOD:
        	this->myParameter->write_eeprom_parameter(this->myAxis[this->myCmd.i_instance]->module_parameter , this->flash_addr_axis(this->myCmd.i_instance, ADDR_FLASH_A1, ADDR_FLASH_A2));
            break;

        case MOTION_PID_MOD:
        	this->myParameter->write_eeprom_parameter(this->myAxis[this->myCmd.i_instance]->myPid_Pos->module_parameter , this->flash_addr_axis(this->myCmd.i_instance, ADDR_FLASH_A1_PID_POS, ADDR_FLASH_A2_PID_POS));
            break;

        case C_PID_MOD:
        	this->myParameter->write_eeprom_parameter(this->myAxis[this->myCmd.i_instance]->myCurrent_Control->module_parameter , this->flash_addr_axis(this->myCmd.i_instance, ADDR_FLASH_A1_CUR_CTL, ADDR_FLASH_A2_CUR_CTL));
            break;

        case ENCODER_MOD:
        	this->myParameter->write_eeprom_parameter(this->myAxis[this->myCmd.i_instance]->myPos->module_parameter , this->flash_addr_axis(this->myCmd.i_instance, ADDR_FLASH_A1_POS, ADDR_FLASH_A2_POS));
            break;

        case TRAJECTORY_MOD:
        	this->myParameter->write_eeprom_parameter(this->myAxis[this->myCmd.i_instance]->myTrajectory->module_parameter , this->flash_addr_axis(this->myCmd.i_instance, ADDR_FLASH_A1_TRJ, ADDR_FLASH_A2_TRJ));
            break;

        case ERROR_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
        	break;

        case PARAMETER_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
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
COMMAND RPE and GPE --> "Reload or Read EEPROM Parameter "
--------------------------------------------------------------
*/
bool ComInt_class::get_eeprom_parameter(void)
{
    bool ret = true;

    int print = strcmp(this->myCmd.cmd_str,"RPE");

	// get parameter values from cmd arguments
    ComInt_class::cpy_CMD_arguments_4_parameters();

    switch(this->myCmd.dv_module)
    {
		case NON_MOD:
			ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
			break;

        case SYSTEM_MOD:
        	this->myParameter->read_eeprom_parameter(this->mySystem->module_parameter , ADDR_FLASH_PAGE_SYSTEM, print);
        	this->mySystem->refresh_parameter();
            break;

        case DATARECORDER_MOD:
        	this->myParameter->read_eeprom_parameter(this->myDR->module_parameter , ADDR_FLASH_DRC, print);
        	this->myDR->refresh_parameter();
            break;

        case AXIS_MOD:
        	this->myParameter->read_eeprom_parameter(this->myAxis[this->myCmd.i_instance]->module_parameter , this->flash_addr_axis(this->myCmd.i_instance, ADDR_FLASH_A1, ADDR_FLASH_A2), print);
        	this->myAxis[this->myCmd.i_instance]->refresh_parameter();
            break;

        case MOTION_PID_MOD:
        	this->myParameter->read_eeprom_parameter(this->myAxis[this->myCmd.i_instance]->myPid_Pos->module_parameter, this->flash_addr_axis(this->myCmd.i_instance, ADDR_FLASH_A1_PID_POS, ADDR_FLASH_A2_PID_POS), print);
        	this->myAxis[this->myCmd.i_instance]->myPid_Pos->refresh_parameter();
        	break;

        case C_PID_MOD:
        	this->myParameter->read_eeprom_parameter(this->myAxis[this->myCmd.i_instance]->myCurrent_Control->module_parameter, this->flash_addr_axis(this->myCmd.i_instance, ADDR_FLASH_A1_CUR_CTL, ADDR_FLASH_A2_CUR_CTL), print);
        	this->myAxis[this->myCmd.i_instance]->myCurrent_Control->refresh_parameter();
            break;

        case ENCODER_MOD:
        	this->myParameter->read_eeprom_parameter(this->myAxis[this->myCmd.i_instance]->myPos->module_parameter, this->flash_addr_axis(this->myCmd.i_instance, ADDR_FLASH_A1_POS, ADDR_FLASH_A2_POS), print);
        	this->myAxis[this->myCmd.i_instance]->myPos->refresh_parameter();
            break;

        case TRAJECTORY_MOD:
        	this->myParameter->read_eeprom_parameter(this->myAxis[this->myCmd.i_instance]->myTrajectory->module_parameter, this->flash_addr_axis(this->myCmd.i_instance, ADDR_FLASH_A1_TRJ, ADDR_FLASH_A2_TRJ), print);
        	this->myAxis[this->myCmd.i_instance]->myTrajectory->refresh_parameter();
            break;

        case PARAMETER_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
            break;

        case COMMAND_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
            break;

        case ERROR_MOD:
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
COMMAND SET PARAMETER --> "SPV"
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
        	ret = this->mySystem->setParameter(&this->myCmd);
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
        case PARAMETER_MOD:
        	//ret = this->myParameter->setParameter(&this->myCmd,this->myParameter->module_parameter);
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
            break;
        case COMMAND_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
            break;
        case ERROR_MOD:
        	ComInt_class::myError->add(COMMAND_MOD, ERR_MOD_NO_PAM, __FILE__, __LINE__); // Module has No Parameter
			break;
        default:
            ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // Unknown Module Identifier
            ret = false;
    }

    // Fehler sichtbar machen, da der Interpreter den Rueckgabewert sonst ignoriert
    if(ret == false)
    {
    	send_string("#PARAMETER SET FAILED (see error list)",true);
    }
    return ret;
}

/*
--------------------------------------------------------------
COMMAND GET PARAMETER LIST --> "GPL"
--------------------------------------------------------------
*/
bool ComInt_class::get_parameter_hlp(void)
{
    bool ret 		= true;

    PARAM_DEV*		param_ptr = NULL;
    // get parameter values from cmd arguments
    ComInt_class::cpy_CMD_arguments_4_parameters();

    switch(this->myCmd.dv_module)
    {
		case NON_MOD:
			// Module has No Parameter
	    	send_string("#MODULE HAS NO Parameter",true);
	    	send_string("#END LIST",true);

			break;
        case SYSTEM_MOD:
        	param_ptr = this->mySystem->module_parameter;
            break;

        case DATARECORDER_MOD:
        	param_ptr = this->myDR->module_parameter;
            break;

        case AXIS_MOD:
        	param_ptr = this->myAxis[this->myCmd.i_instance]->module_parameter;
            break;

        case MOTION_PID_MOD:
        	param_ptr = this->myAxis[this->myCmd.i_instance]->myPid_Pos->module_parameter;
            break;

        case C_PID_MOD:
        	param_ptr = this->myAxis[this->myCmd.i_instance]->myCurrent_Control->module_parameter;
            break;

        case ENCODER_MOD:
        	param_ptr = this->myAxis[this->myCmd.i_instance]->myPos->module_parameter;
            break;

        case TRAJECTORY_MOD:
        	param_ptr = this->myAxis[this->myCmd.i_instance]->myTrajectory->module_parameter;
            break;

        case PARAMETER_MOD:
        	param_ptr = this->myParameter->module_parameter;
            break;

        case COMMAND_MOD:
        	param_ptr = this->module_parameter;
            break;

        case ERROR_MOD:
        	param_ptr = this->myError->module_parameter;
        	break;

        default:
            ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // Unknown Module Identifier
            ret = false;
    }
    if(param_ptr != NULL)
    {
    	ret = this->myParameter->sendParameterHelp(&this->myCmd.i_address,param_ptr);
    }
    return ret;
}


/*
--------------------------------------------------------------
COMMAND GET PARAMETERS --> "GPV"
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
        	modul_parameter_ptr = this->mySystem->module_parameter;
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
        case PARAMETER_MOD:
        	//modul_parameter_ptr = this->myParameter->module_parameter;
        	no_param = true; 		// actually no parameters in moule
            break;
        case COMMAND_MOD:
        	//modul_parameter_ptr = this->module_parameter;
        	no_param = true; 		// actually no parameters in moule
            break;
        case ERROR_MOD:
        	//modul_parameter_ptr = this->myError->module_parameter;
        	no_param = true;		// actually no parameters in moule
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
    	ret = this->myParameter->getParameter(&this->myCmd.i_address, modul_parameter_ptr);
    }
    return ret;
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

	send_string("#MODULE LIST:",true);
	for(uint16_t i = 0;i<NUM_OF_DEV_CLASSES;i++)
	{
		sprintf(str,"%s: %s",this->myError->dev_class_str[i].cmd, this->myError->dev_class_str[i].description);
		send_string(str,true);
	}
	send_string("#END LIST",true);
}

/*
--------------------------------------------------------------
Reboot CMD
--------------------------------------------------------------
*/
bool ComInt_class::reboot(void)
{
	send_string("#USER SYSTEM REBOOT",true);
	HAL_Delay(500);
	NVIC_SystemReset();
	return true;
}

/*
--------------------------------------------------------------
Reboot CMD
--------------------------------------------------------------
*/
bool ComInt_class::clear_FLASH(void)
{
	// get parameter values from cmd arguments
    ComInt_class::cpy_CMD_arguments_4_parameters();

	send_string("#USER FLASH EASE !!!!",true);
	if((this->myCmd.dv_module == SYSTEM_MOD) && (this->myCmd.i_address  == 1234 ))
	{
		this->myParameter->clear_parameter_flash();
	}
	else
	{
		send_string("#FLASH EASE FAILD",true);
	}
	return true;
}


/*
--------------------------------------------------------------
Print Command List
--------------------------------------------------------------
*/
void ComInt_class::print_cmd_list(void)
{
	send_string("#COMMAND LIST:",true);
	for(uint16_t i = 0;i<MAX_NUM_COMANDS;i++)
	{
		send_string(this->cmd_tab[i].cmd,false);
		send_string(": ",false);
		send_string(this->cmd_tab[i].description,true);
	}
	send_string("#END LIST",true);
}
/*
--------------------------------------------------------------
COMMAND GET DEVICE ID --> "IDN"
--------------------------------------------------------------
*/
bool ComInt_class::get_dev_id(void)
{
	char str[20] = {0};
	send_string("HKA - LIS Motion Control Firmware",true);
	send_string("# Motion Controller Type: ",false);
	send_string(CONTROLLER_NAME_STR,true);
	send_string("# Name: ",false);
	send_string(this->mySystem->device_name,true);
	// Serial Number
	sprintf(str,"Serial Number: %d",this->mySystem->device_serial_number);
	send_string(str,true);
	send_string("Firmware Version: ",false);
	send_string(VERSION_STR,true);
	send_string(CPY_STR,true);
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
COMMAND MOV AXIS --> "MVR"
--------------------------------------------------------------
*/
bool ComInt_class::mvr(void)
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
		float new_target = target + this->myAxis[this->myCmd.i_instance]->myTrajectory->setpoint;
		this->myAxis[this->myCmd.i_instance]->myTrajectory->set_target(new_target);
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

	if(this->myCmd.i_instance == 100) // instance missing
	{
		ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_INSTANCE, __FILE__, __LINE__); // 	{8, "Module instance"
	}
	else
	{
		sprintf(str,"%4.5f",this->myAxis[this->myCmd.i_instance]->myPos->position);
		send_string(str,true);
	}

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
	char * p;

	float current = strtof(&this->myCmd.argument[2][0], & p);
	if ( * p != 0 ) { // Address conversion failed
		ComInt_class::myError->add(COMMAND_MOD, ERR_UNKNOWN_MOD_ID, __FILE__, __LINE__); // 	{8, "Unknown Parameter ID"
		ret = false;
	}
	else
	{
		//this->myAxis[this->myCmd.i_instance]->myCurrent_Control->start_current_run(current);
		this->myDR->trigger_start(DR_TGR_MOV);
	}
	return ret;
}




/*
--------------------------------------------------------------
COMMAND Find Phase Algorithm start
--------------------------------------------------------------
*/
bool ComInt_class::find_phase(void)
{
	if(this->myAxis[this->myCmd.i_instance]->enabled == true)
	{
		//this->myAxis[this->myCmd.i_instance]->myCurrent_Control->start_phase_finding();
	}
	else
	{
		ComInt_class::myError->add(COMMAND_MOD, ERR_AXS_DISABLED, __FILE__, __LINE__); // 	AXIS Disabled"
	}
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
	send_string(str,true);
    return true;
}



/*
--------------------------------------------------------------
COMMAND SAVE PARAMETERS TO EPROM --> "SEP"
--------------------------------------------------------------
*/
bool ComInt_class::get_error_list(void)
{
	send_string("Function Call: get_error_list",true);
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
	//this->myAxis[this->myCmd.i_instance]->myCurrent_Control->start_current_loop_test();
    return true;
}


/*
--------------------------------------------------------------
COMMAND Start SVM Test --> "SVT"
--------------------------------------------------------------
*/
bool ComInt_class::start_svm_test(void)
{
	//this->myAxis[this->myCmd.i_instance]->myCurrent_Control->start_svm_test();
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

/*NEW
--------------------------------------------------------------
COMMAND READ RECORD TABLE BINARY --> "RTB"
--------------------------------------------------------------
*/
bool ComInt_class::read_record_table_binary(void)
{
	this->myDR->send_data_binary();
    return true;
}

