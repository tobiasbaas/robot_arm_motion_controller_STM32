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
	data_recorder_class::recInstance		= 0;
	data_recorder_class::recDefaultInstance	= 0;
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

bool data_recorder_class::trigger_start_internal(uint8_t inst, TRIGGER_TYPE type)
{
	bool ret = true;

	if(type == this->trigger_type) // correct trigger source
	{
		if(this->rec_on == false)	// if no recording is actually processed --> start a new
		{
			if(inst >= MAX_NUM_AXIS)	// kein/ungueltiger Achsbezug (z.B. REC; --> 100) --> Default-Achse
			{
				inst = this->recDefaultInstance;
			}
			if(inst >= MAX_NUM_AXIS)	// Sicherheitsnetz
			{
				inst = 0;
			}
			this->recInstance = inst;
			this->update_table_pointers();	// Pointer auf die gueltige Achse umbauen
			this->record_rate_cnt = 0;
			this->rec_cnt = 0;
			this->rec_on = true;
		}
		else	// do nothing and return false
		{
			ret = false;
		}
	}
	else
	{
		ret = false; // trigger source does not match
	}
	
	return ret;
}

bool data_recorder_class::trigger_start(TRIGGER_TYPE type)
{
	return this->trigger_start_internal(this->recDefaultInstance, type);
}

bool data_recorder_class::trigger_start(uint8_t instance, TRIGGER_TYPE type)
{
	return this->trigger_start_internal(instance, type);
}

void data_recorder_class::run(void)
{
	timeStamp = (float)HAL_GetTick();

	// Start recording
	if(this->rec_on == true)
	{
		// Record rate
		if((this->record_rate_max > 0) && (this->record_rate_cnt%this->record_rate_max == 0))
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
	this->recDefaultInstance = this->module_parameter[3].value.ui16;
	if(this->recDefaultInstance >= MAX_NUM_AXIS)	// Sicherheitsnetz (z.B. verschobener Alt-Wert)
	{
		this->recDefaultInstance = 0;
	}

	if(this->rec_on == false)
	{
		this->recInstance = this->recDefaultInstance;
		this->update_table_pointers();
	}
	return ret;
}

void data_recorder_class::update_table_pointers(void)
{
	this->table_ptr_update(this->module_parameter[5].value.ui16,0);
	this->table_ptr_update(this->module_parameter[6].value.ui16,1);
	this->table_ptr_update(this->module_parameter[7].value.ui16,2);
	this->table_ptr_update(this->module_parameter[8].value.ui16,3);
}

bool data_recorder_class::table_ptr_update(uint16_t id,uint8_t trace)
{

	bool ret = true;

	if(this->recInstance >= MAX_NUM_AXIS)	// Sicherheitsnetz
	{
		this->recInstance = 0;
	}

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


	// set pointer to variable
	switch(id)
	{
		case 0:		// no data (0.0)
					this->rec_table_ptr[trace] = &this->null;
					sprintf(&this->table_name[trace][0],"NON - Axis: %d",recInstance);
					break;
		case 1:		// Actual Position
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myPos->mech_position;
					sprintf(&this->table_name[trace][0],"Position - Axis: %d",recInstance);
					break;
		case 2:		// slowed Target
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myTrajectory->slowed_target;
					sprintf(&this->table_name[trace][0],"Slowed Target - Axis: %d",recInstance);
					break;
		case 3:		// Position Error
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myPid_Pos->error;
					sprintf(&this->table_name[trace][0],"Position Error - Axis: %d",recInstance);
					break;
		case 4:		// Setpoint
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myTrajectory->setpoint;
					sprintf(&this->table_name[trace][0],"Setpoint - Axis: %d",recInstance);
					break;
		case 5:		// Control out
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myPid_Pos->pid_y;
					sprintf(&this->table_name[trace][0],"Control Out - Axis: %d",recInstance);
					break;
		case 6:		// Velocity
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myTrajectory->vel;
					sprintf(&this->table_name[trace][0],"Velocity - Axis: %d",recInstance);
					break;
		case 7:		// Acceleration
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myTrajectory->acc;
					sprintf(&this->table_name[trace][0],"Acceleration - Axis: %d",recInstance);
					break;

		case 8:		// Encoder raw
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myPos->angle_raw_float;
					sprintf(&this->table_name[trace][0],"MT6835 Enc Pos: raw - Axis: %d",recInstance);
					break;

		case 9:		// MT6835 Encoder Pos abs
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myPos->mt6835_result.angle_rad;
					sprintf(&this->table_name[trace][0],"MT6835 Enc Pos: abs - Axis: %d",recInstance);
					break;
		case 10:	// MT6835 Encoder Pos el
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myPos->mt6835_result.angle_el;
					sprintf(&this->table_name[trace][0],"MT6835 Enc Pos: el - Axis: %d",recInstance);
					break;
				
		case 11:	// Motor current
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myCurrent_Control->i_alpha;
					sprintf(&this->table_name[trace][0],"Current Phase1 - Axis: %d",recInstance);
					break;
		case 12:	// Motor current
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myCurrent_Control->i_beta;
					sprintf(&this->table_name[trace][0],"Current Phase2 - Axis: %d",recInstance);
					break;
		/*
		case 11:		// Motor current
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myCurrent_Control->myADC_Values[2];
					sprintf(&this->table_name[trace][0],"Current Phase2-ShuntC - Axis: %d",recInstance);
					break;
		case 12:	// Motor current
					this->rec_table_ptr[trace] = &this->myAxis[recInstance]->myCurrent_Control->myADC_Values[3];
					sprintf(&this->table_name[trace][0],"Current Phase2-ShuntD - Axis: %d",recInstance);
					break;
		*/
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
					sprintf(&this->table_name[trace][0],"NON - Axis: %d",recInstance);
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

	send_string("#DATA RECORDER HELP",true);
	send_string("* ID: SIGNAL",true);
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
			send_string(str_send,true);
		}

	}
	send_string("#END",true);

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
	send_string(str,true);
	send_string(&this->table_name[0][0],true);
	send_string(&this->table_name[1][0],true);
	send_string(&this->table_name[2][0],true);
	send_string(&this->table_name[3][0],true);
	send_string("#HEADER END",true);
	send_string("#TABLE START:",true);
	for(uint16_t i=0;i<DR_TABLE_LENGTH;i++)
	//for(uint16_t i=0;i<100;i++)
	{
		sprintf(str,"%6.6f;%6.6f;%6.6f;%6.6f;",this->record_table_1[i],this->record_table_2[i],this->record_table_3[i],this->record_table_4[i]);
		send_string(str,true);
	}
	send_string("#TABLE END",true);

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
	// static: Buffer darf den Funktionsrahmen ueberleben, da HAL_UART_Transmit_DMA
	// nicht blockierend arbeitet und nach Rueckkehr weiter auf diesen Speicher zugreift.
	static float data[4];
	sprintf(str,"#REC HEADER START: \n* SERVO FREQUENCY: %d \n* RECORD RATE: %d \n* NUMBER Tables: %d \n* TABLE LENGTH: %d", (int)SERVO_FRQ,this->record_rate_max, this->record_tables_max, DR_TABLE_LENGTH );
	send_string(str,true);
	send_string(&this->table_name[0][0],true);
	send_string(&this->table_name[1][0],true);
	send_string(&this->table_name[2][0],true);
	send_string(&this->table_name[3][0],true);
	//send_string_DMA(&this->table_name[4][0],true);
	//send_string_DMA(&this->table_name[5][0],true);
	//send_string_DMA(&this->table_name[6][0],true);
	//send_string_DMA(&this->table_name[7][0],true);
	send_string("#HEADER END",true);
	send_string("#TABLE BINARY START:",true);
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
	send_string("#TABLE BINARY END",true);

	return true;
}
