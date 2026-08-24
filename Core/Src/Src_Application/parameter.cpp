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

bool parameter_class::getParameter(uint16_t* param_id, PARAM_DEV* module_parameter)
{
    bool ret = false;

    uint16_t len = module_parameter[0].value.ui16; // table length

    //send parameter value with corresponding address, case address is zero send all module parameter values
	for(uint16_t i = 0; i < len; i++)
	{
		if((*param_id == module_parameter[i].adress)||(*param_id == 0))
		{
			//call_param->adress 		= module_parameter[i].adress;
			//call_param->numerator 	= module_parameter[i].numerator;
			//call_param->denominator = module_parameter[i].denominator;
			//call_param->type 		= module_parameter[i].type;
			//call_param->value 		= module_parameter[i].value;
			ret = true;
			this->print_param_value_raw(module_parameter[i].type,module_parameter[i].value);
			if(*param_id != 0) // end after one value
			{
				i = len; // end for loop
			}
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
    bool found = false;

    uint16_t len = module_parameter[0].value.ui16; // table length

    for(uint16_t i = 0;i < len; i++)
    {
        if(cmd->i_address == module_parameter[i].adress)
        {
        	found = true;
        	if(module_parameter[i].read_only)
        	{
        		this->myError->add(PARAMETER_MOD, ERR_PAM_READ_ONLY, __FILE__, __LINE__); // Parameter does not exist not Found
        	}
        	else
        	{
        		// bei Fehler (z.B. out of range) bleibt ret == false, der Fehler ist bereits in der Error-Liste
				if(parameter_class::convert_value_from_string(&cmd->value,  module_parameter[i].type, &cmd->argument[3][0], module_parameter[i].minimum, module_parameter[i].maximum))
				{
					module_parameter[i].value.ui64 	= cmd->value.ui64;
					ret = true;
				}
        	}
			i = len; // end for loop
        }
    }

    if(found == false)
    {
        this->myError->add(PARAMETER_MOD, ERR_PAM_NON_EXIST, __FILE__, __LINE__); // Parameter does not exist not Found
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
					value->_bool = false;	// 0 = aus
					break;
				case 1:
					value->_bool = true;	// 1 = an
					break;
				default:
					 ret = false;			// nur 0 oder 1 erlaubt
			}
			break;

		case _UINT64:	// min and max only 32 bit!!!!
			// HINWEIS: strtol liefert nur 32 Bit (long). Fuer echte 64-Bit-Werte strtoll() verwenden.
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
			// HINWEIS: strtol liefert nur 32 Bit (long). Fuer echte 64-Bit-Werte strtoll() verwenden.
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
			break;

		case POS_INT:
			// HINWEIS: strtol liefert nur 32 Bit (long). Fuer echte 64-Bit-Werte strtoll() verwenden.
			value->i64 = strtol(str,& p, 10);
			if((value->i64 < min) || (value->i64 > max))
			{
				ret = false;
			}
			break;

		case _STRING:
			// HINWEIS: str8 ist nur 8 Bytes gross! strcpy kann den Puffer ueberlaufen
			// (Argument darf bis zu 19 Zeichen haben) und benachbarte Struct-Member
			// (minimum/maximum/description) zerstoeren. Besser strncpy(str8, str, 7) + Terminator.
			strcpy(value->str8,str);
			break;

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

    char str[100]		= {0};
    char tmp_str[80] 	= {0};

    uint16_t len = module_parameter[0].value.ui16; // table length

    //check if Module has Parameter
    if(len >1 && len <= 100)
    {
    	send_string("#PARAMETER LIST START - ",false);
    	//strcpy(tmp_str,module_parameter[0].description);
		// copy module name from first parameter description
		uint16_t len2 = 80;//sizeof(tmp_str);
		str[0] = 0x23;
		str[1] = 0x20;
		for(uint8_t j=10;j<len2;j++)
		{
			char tmp = module_parameter[0].description[j];
			if(tmp == 0)
			{
				j = len2;	//end
			}
			str[j-10] = tmp;
		}
		// Send Headline
		send_string(str,true);

    	// run table an get information to send
		for(uint16_t i=0;i<len;i++)
		{
			// check for ID, if ID is 0x00, print whole table
			if((module_parameter[i].adress == *param_id)||(*param_id == 0))
			{
				if(module_parameter[i].read_only == true)
				{
					sprintf(str,"0x%02x (R) Type: %s Min: %d, Max: %d # %s",module_parameter[i].adress,  this->param_type_str[module_parameter[i].type],module_parameter[i].minimum, module_parameter[i].maximum, module_parameter[i].description);
				}
				else
				{
					sprintf(str,"0x%02x (R/W) Type: %s Min: %d, Max: %d # %s",module_parameter[i].adress,  this->param_type_str[module_parameter[i].type],module_parameter[i].minimum, module_parameter[i].maximum, module_parameter[i].description);
				}
				send_string(str,true);  // send parameter info...
				ret = true;
			}
		}
    }
    else
    {
    	send_string("#PARAMETER LIST START - MODULE HAS NO PARAMETER",true); // List end...
    }
    send_string("#END LIST",true); // List end...
    return ret;
}

/*
--------------------------------------------------------------
Read Module Parameter from FLASH
--------------------------------------------------------------

module_parameter 	- 	Pointer to module Parameter
address				-  	Address in FLASH where the data is stored
print				- 	0 for reload parameter from EEPROM, else send EEPROM values to port
*/

bool parameter_class::read_eeprom_parameter(PARAM_DEV *module_parameter, uint32_t address, int print)
{
    bool ret = true;
    // read table size (first element of module parameter
	uint16_t size = module_parameter[0].value.ui16;
	UNION_VALUE p;

	// test if FLASH values have been written before
	p.ui64 = *(uint64_t *)address;		// read FLASH data
	if(p.ui64 == 0xFFFFFFFFFFFFFFFF)
	{
		if(print == 0)
		{
			return false;
		}
		else
		{
			// get all parameter values as ui64
			for(uint8_t n=0;n<size;n++)
			{
				UNION_VALUE tmp;
				tmp.ui16 = 0;
				PARAM_TYPE type = _UINT16;
				this->print_param_value_raw(type,tmp);	// print out zero to avoid PC-Programm get no values...
			}
			return true;
		}

	}

	if(print == 0) // reload Parameter from EEPROM
	{
		// get all parameter values as ui64
		for(uint8_t n=0;n<size;n++)
		{
			p.ui64 = *(uint64_t *)address;		// read FLASH data
			module_parameter[n].value.ui64 = p.ui64;	// copy to parameter list
			address += 8;							// increase Flash address by 8 byte (ui64)
		}
	}
	else // write value to terminal
	{

		// get all parameter values as ui64
	   	for(uint8_t n=0;n<size;n++)
	   	{
	   		p.ui64 = *(uint64_t *)address;								// read FLASH data
	   		this->print_param_value_raw(module_parameter[n].type,p);	// print out data, get information on type from Parameter List
	   		address += 8;												// increase Flash address by 8 byte (ui64)
	   	}
	}

    return ret;
}

/*
--------------------------------------------------------------
Write Parameter of a Module to FLASH
--------------------------------------------------------------
module_parameter 	- 	Pointer to module Parameter
address				-  	Address in FLASH where the data is stored

*/
bool parameter_class::write_eeprom_parameter(PARAM_DEV *module_parameter, uint32_t address)
{
    bool ret = true;
    // read table size (first element of module parameter
	uint16_t size = module_parameter[0].value.ui16;

	// generate data vector to write to flash
	uint64_t data[size] = {0};

    // get all parameter values as ui64
	for(uint8_t n=0;n<size;n++)
	{
		data[n] = module_parameter[n].value.ui64;
	}
	// write data to flash (one page per module)
	this->write_data_to_flash(data,size,address);
    return ret;
}

/*
--------------------------------------------------------------
pront parameter value
--------------------------------------------------------------
 _UINT64, , _UINT16, _INT64, _INT32, _INT16, _FLOAT, _DOUBLE, _STRING, _BOOL, _INIT, POS_INT

 */
bool parameter_class::print_param_value_raw(PARAM_TYPE type, UNION_VALUE value)
{
	char str[100];
	long int tmp = 0;
	switch(type)
	{
		case _BOOL:
			sprintf(str,"%u",(uint16_t)value._bool);
			break;

		case _UINT64:
			// HINWEIS: "%u" erwartet 32 Bit; value.ui64 ist 64 Bit -> falsche Ausgabe.
			// nano.printf unterstuetzt kein %llu -> ggf. als zwei 32-Bit-Haelften ausgeben.
			sprintf(str,"%u",value.ui64);
			break;

		case _UINT16:
			sprintf(str,"%u",value.ui16);
			break;

		case _UINT32:
			sprintf(str,"%u",value.ui32);
			break;

		case _INT64:
			// HINWEIS: "%ld" erwartet 32 Bit (long); value.i64 ist 64 Bit -> falsche Ausgabe.
			sprintf(str,"%ld",value.i64);
			break;

		case _INT16:
			sprintf(str,"%d",value.i16);
			break;

		case _INT32:
			sprintf(str,"%d",value.i32);
			break;

		case _DOUBLE:
			sprintf(str,"%5.10f",value.d64);
			break;

		case _FLOAT:
			sprintf(str,"%4.5f",value.f32);
			break;

		case POS_INT:
			//tmp = (param->value.i64 * param->numerator)/param->denominator;
			tmp = value.i64;
			sprintf(str,"%ld",tmp);
			break;

		case _STRING:
			strcpy(str,value.str8);
			break;

		default:
			this->myError->add(COMMAND_MOD, ERR_VARIABLE_STR_CONV, __FILE__, __LINE__); // ??? Fehler anlegen!
	}
	send_string(str,true);
	return true;
}

bool parameter_class::write_data_to_flash(uint64_t *flash_buffer_w,uint16_t size,uint32_t Address)
{
	uint32_t  PAGEError = 0;
	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();


	/* Clear OPTVERR bit set on virgin samples */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

	/* Unlock the Options Bytes *************************************************/
	HAL_FLASH_OB_Unlock();

	FLASH_EraseInitTypeDef EraseInitStruct = {0};

	/* Erase the user Flash area
		(area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase   	= FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks         	= FLASH_BANK_2;
	EraseInitStruct.Page 			= GetPage(Address);
	EraseInitStruct.NbPages      	= 1;

	/* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
		 you have to make sure that these data are rewritten before they are accessed during code
		 execution. If this cannot be done safely, it is recommended to flush the caches by setting the
		 DCRST and ICRST bits in the FLASH_CR register. */
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	{
		/*
		  Error occurred while page erase.
		  User can add here some code to deal with this error.
		  PAGEError will contain the faulty page and then to know the code error on this page,
		  user can call function 'HAL_FLASH_GetError()'
		 */
	}
	/* Program the user Flash area word by word
		(area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
	for(uint32_t i=0;i<size;i++)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, flash_buffer_w[i]) == HAL_OK)
		{
			Address = Address + 8;
		}
		else
		{
			/* Error occurred while writing data in Flash memory.
			 User can add here some code to deal with this error */
		}
	}
	/* Lock the Flash to disable the flash control register access (recommended
		 to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock();
	return 0;
}

bool parameter_class::clear_parameter_flash(void)
{
	uint32_t  PAGEError = 0;
	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();


	/* Clear OPTVERR bit set on virgin samples */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

	/* Unlock the Options Bytes *************************************************/
	HAL_FLASH_OB_Unlock();

	FLASH_EraseInitTypeDef EraseInitStruct = {0};

	/* Erase the user Flash area
		(area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase   	= FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks         	= FLASH_BANK_2;
	EraseInitStruct.Page 			= GetPage(0x08073000);
	EraseInitStruct.NbPages      	= 25;

	/* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
		 you have to make sure that these data are rewritten before they are accessed during code
		 execution. If this cannot be done safely, it is recommended to flush the caches by setting the
		 DCRST and ICRST bits in the FLASH_CR register. */
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	{
		/*
		  Error occurred while page erase.
		  User can add here some code to deal with this error.
		  PAGEError will contain the faulty page and then to know the code error on this page,
		  user can call function 'HAL_FLASH_GetError()'
		 */
	}

	/* Lock the Flash to disable the flash control register access (recommended
		 to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock();
	return 0;
}

uint32_t parameter_class::GetPage(uint32_t Addr)
{
  uint32_t page = 0;

  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }
  return page;
}

