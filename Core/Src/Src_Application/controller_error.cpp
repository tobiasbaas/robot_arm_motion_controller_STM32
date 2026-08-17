/***************************************************************************
 *  Error-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#include "controller_error.h"


error_class::error_class()
{
    error_class::cnt = 0;
    error_class::debug_out = true; // enable/disable file and code line output
    error_class::status.all = 0x00;
    error_class::last_cmd_ptr = &error_class::last_cmd[0];
    // reset error list to zero
    for(uint8_t i=0;i<MAX_NUM_OF_ERRORS_IN_LIST;i++)
    {
    	error_class::myErrors[i].cmd_str[0] = 0;
    	error_class::myErrors[i].file[0] = 0;
    	error_class::myErrors[i].line = 0;
    	error_class::myErrors[i].error_value = 0;
    	error_class::myErrors[i].dev_class = NON_MOD;
    }
}

bool error_class::add(DEV_CLASS dev_class, int16_t error, const char *file, uint16_t line)
{
	if(error >= 0)	// Error -> error number equal or above zero
	{
		error_class::status.err = 0x01;		// set error flag
	}
	else	// waning -> negative error number
	{
		error_class::status.warn = 0x01;	//set warning flag
	}

    error_class::cnt++;
    uint16_t error_no = (error_class::cnt -1)%MAX_NUM_OF_ERRORS_IN_LIST; // get number in list

    // copy name of file..
    for(int i=0; i<30; i++)
    {
        error_class::myErrors[error_no].file[i] = file[i];
        if(file[i] == 0)
        {
            i=30;
        }
    }

    error_class::myErrors[error_no].line = line;
    error_class::myErrors[error_no].error_value = error;
    error_class::myErrors[error_no].dev_class = dev_class;
    strcpy(error_class::myErrors[error_no].cmd_str, error_class::last_cmd);
    return true;
}

bool error_class::print_error_ids(void)
{
	bool ret = true;
	char str[65] = {0};

	send_string("#ERROR-IDs:",true);
	for(uint8_t i=0; i< MAX_NUM_ERRORS; i++)
	{
		sprintf(str, "%d: %s", this->my_sys_error_list[i].error_value, this->my_sys_error_list[i].description);
		send_string(str,true);
	}

	send_string("#END",true);
    return  ret;
}


bool error_class::print_error_list()
{
    bool ret = false;
    char str[180];
    uint16_t m = 20;

    error_class::status.err = 0x00;	//clear error flag
    if(error_class::cnt < MAX_NUM_OF_ERRORS_IN_LIST)
    {
        m = error_class::cnt;
    }

    send_string("#ERROR-List:",true);
    uint16_t descript_index = MAX_NUM_ERRORS;
    for(uint16_t n=0; n<m; n++)
    {
        for(uint16_t m=0; m<MAX_NUM_ERRORS; m++)
        {
            if(my_sys_error_list[m].error_value == error_class::myErrors[n].error_value)
            {
                descript_index = m;
                m = MAX_NUM_ERRORS; //end for loop
            }
        }

        sprintf(str, "CMD: %s, ERROR %d:", error_class::myErrors[n].cmd_str, (int)error_class::myErrors[n].error_value);
        send_string(str,false);

        sprintf(str, " %s - Module: %s",
            my_sys_error_list[descript_index].description,
			error_class::dev_class_str[error_class::myErrors[n].dev_class].description
            );

        error_class::myErrors[n].file[29] = 0;

        if(error_class::debug_out == true){
        	send_string(str,false);
        	sprintf(str, " File: %s l: %d",
            error_class::myErrors[n].file,
            (int)error_class::myErrors[n].line
            );
        send_string(str,true);
        }
        else{
        	send_string(str,true);
        }

        ret = true;
    }
    
    if(ret == false)
    {
    	send_string("#EMPTY-List",true);
    }
    return ret;
}



