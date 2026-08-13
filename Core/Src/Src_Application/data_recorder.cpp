/***************************************************************************
 *  Data Recorder-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#include "data_recorder.h"


data_recorder_class::data_recorder_class(parameter_class* myParameter_ptr, error_class* myError_ptr)
{
	data_recorder_class::myParameter		= myParameter_ptr;
	data_recorder_class::myError			= myError_ptr;
	data_recorder_class::null 				= 0.0;
	data_recorder_class::record_rate_cnt 	= 0;
	data_recorder_class::rec_cnt 			= 0;
	data_recorder_class::record_rate_max	= 0;
	data_recorder_class::rec_on 			= false;
	data_recorder_class::record_tables_max 	= 4;
	data_recorder_class::trigger_type		= DR_TGR_OFF;
	data_recorder_class::rec_table_ptr[0] 	= NULL;
	data_recorder_class::rec_table_ptr[1] 	= NULL;
	data_recorder_class::rec_table_ptr[2] 	= NULL;
	data_recorder_class::rec_table_ptr[3] 	= NULL;
	//data_recorder_class::rec_table_ptr[4] 	= NULL;//NEW
	//data_recorder_class::rec_table_ptr[5] 	= NULL;//NEW
	//data_recorder_class::rec_table_ptr[6] 	= NULL;//NEW
	//data_recorder_class::rec_table_ptr[7] 	= NULL;//NEW
}

void data_recorder_class::init(void)
{
	// load parameter from FLASH
	this->myParameter->read_eeprom_parameter(&this->module_parameter[0], ADDR_FLASH_DRC, 0);
	data_recorder_class::refresh_parameter();
}

bool data_recorder_class::trigger_start(TRIGGER_TYPE type)
{
	bool ret = true;

	if(type == this->trigger_type) // correct trigger source
	{
		if(this->rec_on == false)	// if no recording is actually processed --> start a new
		{
			this->record_rate_cnt = 0;
			this->rec_cnt = 0;
			this->rec_on = true;
		}
		else	// do nothing and return false
		{
			ret = false;
		}
	}
	
	return ret; // TODO why return true if trigger_type is wrong? 
}

void data_recorder_class::run(void)
{
	// Start recording
	if(this->rec_on == true)
	{
		// Record rate
		if(this->record_rate_cnt%this->record_rate_max == 0)
		{
			this->record_table_1[this->rec_cnt] = *this->rec_table_ptr[0];
			this->record_table_2[this->rec_cnt] = *this->rec_table_ptr[1];
			this->record_table_3[this->rec_cnt] = *this->rec_table_ptr[2];
			this->record_table_4[this->rec_cnt] = *this->rec_table_ptr[3];
			//this->record_table_5[this->rec_cnt] = *this->rec_table_ptr[4];//NEW
			//this->record_table_6[this->rec_cnt] = *this->rec_table_ptr[5];//NEW
			//this->record_table_7[this->rec_cnt] = *this->rec_table_ptr[6];//NEW
			//this->record_table_8[this->rec_cnt] = *this->rec_table_ptr[7];//NEW
			this->rec_cnt++;

			// record end
			if(this->rec_cnt == DR_TABLE_LENGTH)
			{
				this->rec_on = false;
			}
		}
		this->record_rate_cnt++;
	}
}

bool data_recorder_class::setParameter(CMD_STRUCT* cmd)
{
	bool ret;
	ret = this->myParameter->setParameter(cmd, &this->module_parameter[0]);
	this->refresh_parameter();
	return ret;
}

bool data_recorder_class::refresh_parameter(void)
{
	bool ret = true;
	this->record_rate_max 	= this->module_parameter[1].value.ui16;
	this->trigger_type 		= (TRIGGER_TYPE)this->module_parameter[2].value.ui16;
	ret = this->table_ptr_update(this->module_parameter[4].value.ui16,0);
	ret = this->table_ptr_update(this->module_parameter[5].value.ui16,1);
	ret = this->table_ptr_update(this->module_parameter[6].value.ui16,2);
	ret = this->table_ptr_update(this->module_parameter[7].value.ui16,3);
	//ret = this->table_ptr_update(this->module_parameter[8].value.ui16,4);//NEW
	//ret = this->table_ptr_update(this->module_parameter[9].value.ui16,5);//NEW
	//ret = this->table_ptr_update(this->module_parameter[10].value.ui16,6);//NEW
	//ret = this->table_ptr_update(this->module_parameter[11].value.ui16,7);//NEW
	return ret;
}

bool data_recorder_class::table_ptr_update(uint16_t id,uint8_t trace)
{

	bool ret = true;
	uint8_t axis;

	// check for valid trace
	if((trace >= 0) && (trace < 4))
	{
		ret = true;
	}
	else
	{
		trace = 0;
		ret = false;
		// error handler (Trace error)
		this->myError->add(DATARECORDER_MOD, 1000, __FILE__, __LINE__); // TODO Fehler anlegen!
	}

	// get axis from id (case grater 100 for axis 1 else axis 0)
	if(id >= 100)
	{
		axis = 1;
		id = id-100;
	}
	else
	{
		axis = 0;
	}

	// set pointer to variable
	switch(id)
	{
		case 0:		// no data (0.0)
					this->rec_table_ptr[trace] = &this->null;
					sprintf(&this->table_name[trace][0],"NON - Axis: %d",axis);
					break;
		case 1:		// Actual Position
					this->rec_table_ptr[trace] = &this->myAxis[axis]->myPos->position;
					sprintf(&this->table_name[trace][0],"Position - Axis: %d",axis);
					break;
		case 2:		// slowed Target
					this->rec_table_ptr[trace] = &this->myAxis[axis]->myTrajectory->slowed_target;
					sprintf(&this->table_name[trace][0],"Slowed Target - Axis: %d",axis);
					break;
		case 3:		// Position Error
					this->rec_table_ptr[trace] = &this->myAxis[axis]->myPid_Pos->error;
					sprintf(&this->table_name[trace][0],"Position Error - Axis: %d",axis);
					break;
		case 4:		// Setpoint
					this->rec_table_ptr[trace] = &this->myAxis[axis]->myTrajectory->setpoint;
					sprintf(&this->table_name[trace][0],"Setpoint - Axis: %d",axis);
					break;
		case 5:		// Control out
					this->rec_table_ptr[trace] = &this->myAxis[axis]->myPid_Pos->pid_y;
					sprintf(&this->table_name[trace][0],"Control Out - Axis: %d",axis);
					break;
		case 6:		// Velocity
					this->rec_table_ptr[trace] = &this->myAxis[axis]->myTrajectory->vel;
					sprintf(&this->table_name[trace][0],"Velocity - Axis: %d",axis);
					break;
		case 7:		// Acceleration
					this->rec_table_ptr[trace] = &this->myAxis[axis]->myTrajectory->acc;
					sprintf(&this->table_name[trace][0],"Acceleration - Axis: %d",axis);
					break;
		// case 8:		// PID out Id
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->PI[FOC_D].pid_y;
		// 			sprintf(&this->table_name[trace][0],"FOC (d) PI Control Out - Axis: %d",axis);
		// 			break;
		// case 9:		// PID out Iq
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->PI[FOC_Q].pid_y;
		// 			sprintf(&this->table_name[trace][0],"FOC (q) PI Control Out - Axis: %d",axis);
		// 			break;
		// case 10:	// PID ERROR out Id
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->PI[FOC_D].error;
		// 			sprintf(&this->table_name[trace][0],"FOC (d) PI error - Axis: %d",axis);
		// 			break;
		// case 11:	// PID ERROR out Iq
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->PI[FOC_Q].error;
		// 			sprintf(&this->table_name[trace][0],"FOC (q) PI error - Axis: %d",axis);
		// 			break;
		// case 12:	// Current in (Motor current)
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->current_in;
		// 			sprintf(&this->table_name[trace][0],"FOC Current In - Axis: %d",axis);
		// 			break;
		// case 13:	//Phi Mechanic
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->phi_mech;
		// 			sprintf(&this->table_name[trace][0],"FOC Mechanical Angle - Axis: %d",axis);
		// 			break;
		// case 14:	//Phi Elektric
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->phi_el;
		// 			sprintf(&this->table_name[trace][0],"FOC Electrical Angle - Axis: %d",axis);
		// 			break;
		// case 15:	// Delta PHI
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->fph_delta_counts_fl;
		// 			sprintf(&this->table_name[trace][0],"FPH Delta Counts - Axis: %d",axis);
		// 			break;
		// case 16:	// Uref
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->U_ref;
		// 			sprintf(&this->table_name[trace][0],"FOC (SVM) U-Vector  - Axis: %d",axis);
		// 			break;
		// case 17:	// Current U
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->Ia;
		// 			sprintf(&this->table_name[trace][0],"FOC Phase Current U - Axis: %d",axis);
		// 			break;
		// case 18:	// Current V
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->Ib;
		// 			sprintf(&this->table_name[trace][0],"FOC Phase Current V - Axis: %d",axis);
		// 			break;
		// case 19:	// Current W
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->Ic;
		// 			sprintf(&this->table_name[trace][0],"FOC Phase Current W - Axis: %d",axis);
		// 			break;
		// case 20:	// PWM Time A
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->OnTimeA;
		// 			sprintf(&this->table_name[trace][0],"FOC Phase Voltage U - Axis: %d",axis);
		// 			break;
		// case 21:	// PWM Time B
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->OnTimeB;
		// 			sprintf(&this->table_name[trace][0],"FOC Phase Voltage V - Axis: %d",axis);
		// 			break;
		// case 22:	// PWM Time C
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->OnTimeC;
		// 			sprintf(&this->table_name[trace][0],"FOC Phase Voltage W - Axis: %d",axis);
		// 			break;
		// case 23:	// FOC Id
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->Id;
		// 			sprintf(&this->table_name[trace][0],"FOC Id - Axis: %d",axis);
		// 			break;
		// case 24:	// FOC Iq
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->Iq;
		// 			sprintf(&this->table_name[trace][0],"FOC Iq - Axis: %d",axis);
		// 			break;
		// case 25:	// FOC I_alpha
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->I_alpha;
		// 			sprintf(&this->table_name[trace][0],"FOC I_Alpha - Axis: %d",axis);
		// 			break;
		// case 26:	// FOC I_beta
		// 			this->rec_table_ptr[trace] = &this->myAxis[axis]->myCurrent_Control->I_beta;
		// 			sprintf(&this->table_name[trace][0],"FOC I_Beta - Axis: %d",axis);
		// 			break;
		default:	// no data (0.0)
					this->rec_table_ptr[trace] = &this->null;
					sprintf(&this->table_name[trace][0],"NON - Axis: %d",axis);
					ret = false;
	};
	return ret;
}

bool data_recorder_class::send_DR_hlp(void)
{
	bool ret = true;
	char str_mem[40] = {0};
	char str_send[50] = {0};
	char str_tmp[50] = {0};

	uint8_t trace 	= 0;
	float* 	ptr_mem;

	// store actual table name
	sprintf(str_mem, &this->table_name[trace][0]);
	ptr_mem = this->rec_table_ptr[trace];

	send_string_DMA("#DATA RECORDER HELP",true);
	send_string_DMA("* ID: SIGNAL",true);
	// get all table names...
	for(uint8_t id=0;id<=MAX_NUM_DR_TRACES_TO_CHOOSE;id++)
	{
		ret = data_recorder_class::table_ptr_update(id,trace);
		// send as string...
		if(ret == true)
		{
			// Copy name
			for(uint8_t n =0; n<50;n++)
			{
				if(this->table_name[trace][n] != 0x2D)	// find Sign "-" --> cut "- Axis: X"
				{
					str_tmp[n] = this->table_name[trace][n];
				}
				else
				{
					str_tmp[n] = 0;
					n = 50;
				}
			}
			sprintf(str_send,"%d: %s",id,str_tmp);
			send_string_DMA(str_send,true);
		}

	}
	send_string_DMA("#END",true);

	sprintf(&this->table_name[trace][0], str_mem);
	this->rec_table_ptr[trace] = ptr_mem;
	return true;
}

/*
--------------------------------------------------------------
Send recorder data as ASCII
--------------------------------------------------------------
*/
bool data_recorder_class::send_data(void)
{
	char str[200];
	sprintf(str,"#REC HEADER START: \n* RECORD RATE: %d \n* NUMBER Tables: %d \n* TABLE LENGTH: %d",this->record_rate_max, this->record_tables_max , DR_TABLE_LENGTH );
	send_string_DMA(str,true);
	send_string_DMA(&this->table_name[0][0],true);
	send_string_DMA(&this->table_name[1][0],true);
	send_string_DMA(&this->table_name[2][0],true);
	send_string_DMA(&this->table_name[3][0],true);
	send_string_DMA("#HEADER END",true);
	send_string_DMA("#TABLE START:",true);
	for(uint16_t i=0;i<DR_TABLE_LENGTH;i++)
	//for(uint16_t i=0;i<100;i++)
	{
		sprintf(str,"%6.6f;%6.6f;%6.6f;%6.6f;",this->record_table_1[i],this->record_table_2[i],this->record_table_3[i],this->record_table_4[i]);
		send_string_DMA(str,true);
	}
	send_string_DMA("#TABLE END",true);

	/*
	send_string_DMA("#TABLE 1 START:",true);
	//send_raw_array_DMA((uint8_t*)&this->record_table_1[0],DR_TABLE_LENGTH * 4);
	send_string_DMA("#TABLE 1 END",true);
	send_string_DMA("#TABLE 2 START:",true);
	send_raw_array_DMA((uint8_t*)&this->record_table_2[0],DR_TABLE_LENGTH * 4);
	send_string_DMA("#TABLE 2 END",true);
	send_string_DMA("#TABLE 3 START:",true);
	send_raw_array_DMA((uint8_t*)&this->record_table_3[0],DR_TABLE_LENGTH * 4);
	send_string_DMA("#TABLE 3 END",true);
	send_string_DMA("#TABLE 4 START:",true);
	send_raw_array_DMA((uint8_t*)&this->record_table_4[0],DR_TABLE_LENGTH * 4);
	send_string_DMA("#TABLE 4 END",true);*/
	return true;
}

/*
--------------------------------------------------------------
Send recorder data as Binary
--------------------------------------------------------------
*/
bool data_recorder_class::send_data_binary(void)
{
	char str[200];
	float data[4];
	sprintf(str,"#REC HEADER START: \n* SERVO FREQUENCY: %d \n* RECORD RATE: %d \n* NUMBER Tables: %d \n* TABLE LENGTH: %d", SERVO_FRQ,this->record_rate_max, this->record_tables_max, DR_TABLE_LENGTH );
	send_string_DMA(str,true);
	send_string_DMA(&this->table_name[0][0],true);
	send_string_DMA(&this->table_name[1][0],true);
	send_string_DMA(&this->table_name[2][0],true);
	send_string_DMA(&this->table_name[3][0],true);
	//send_string_DMA(&this->table_name[4][0],true);
	//send_string_DMA(&this->table_name[5][0],true);
	//send_string_DMA(&this->table_name[6][0],true);
	//send_string_DMA(&this->table_name[7][0],true);
	send_string_DMA("#HEADER END",true);
	send_string_DMA("#TABLE BINARY START:",true);
	for (uint16_t i = 0; i < DR_TABLE_LENGTH; ++i) {
		data[0] = this->record_table_1[i];
		data[1] = this->record_table_2[i];
		data[2] = this->record_table_3[i];
		data[3] = this->record_table_4[i];
		//data[4] = this->record_table_5[i];
		//data[5] = this->record_table_6[i];
		//data[6] = this->record_table_7[i];
		//data[7] = this->record_table_8[i];
		// Achtung: Pointer-Cast zu Bytes!
		send_raw_array_DMA((uint8_t*)data, sizeof(data));
	    }
	send_string_DMA("#TABLE BINARY END",true);

	return true;
}
