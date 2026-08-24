/*
 * System.cpp
 *
 *  Created on: Mar 29, 2025
 *      Author: tobiasbaas
 */
#include <system.h>
#include <stdlib.h>

system_class::system_class(parameter_class * myParameter_ptr)
{
	// initialisierung mit default values
	system_class::myParameter = myParameter_ptr;
	strcpy(system_class::device_name,"NEWinit");
	system_class::device_serial_number = 0;
}


bool system_class::init(void)
{
	this->myParameter->read_eeprom_parameter(&this->module_parameter[0], ADDR_FLASH_PAGE_SYSTEM,0);
	this->refresh_parameter();
	return true;
}


bool system_class::refresh_parameter(void)
{
	bool ret = true;
	strcpy(this->device_name,this->module_parameter[1].value.str8);
	this->device_serial_number			= this->module_parameter[2].value.ui16;
	return ret;
}

bool system_class::setParameter(CMD_STRUCT* cmd)
{
	bool ret;

	ret = this->myParameter->setParameter(cmd,&this->module_parameter[0]);
	this->refresh_parameter();

	return ret;
}
