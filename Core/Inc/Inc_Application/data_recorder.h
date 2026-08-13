/***************************************************************************
 *  Data Recorder-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#ifndef _DATA_RECORDER_H
#define _DATA_RECORDER_H

#include "axis.h"
#include "typedefs.h"
#include "parameter.h"


// forward declaration
class axis_class;

extern "C" void send_string_DMA(const char* str, bool new_line);
extern "C" void send_raw_array_DMA(const uint8_t *char_array_ptr, uint16_t len);

/*
// TODO delete?
typedef struct
{
	const char 	table_name[40];
}DR_TABELS;
*/

#define DR_TABLE_LENGTH 1500
#define	MAX_NUM_DR_TRACES_TO_CHOOSE 26	//Number DR Traces; max. number of variables from which to choose for recording
typedef enum{DR_TGR_OFF=0, DR_TGR_MOV=1, DR_TGR_CMD=3}TRIGGER_TYPE;

class data_recorder_class
{
	private:
		//bool table_ptr_update(float* table_ptr, uint16_t id);
		parameter_class* 	myParameter;
		error_class* 		myError;

		/*
		// TODO delete?
		DR_TABELS dr_table[3] =
		{
			{ "Position - Axis:"},
			{ "Position - Axis:"},
			{ "Position - Axis:"},
		};
		*/

	public:
		data_recorder_class(parameter_class* myParameter_ptr, error_class* myError_ptr);
		
		void init(void);
		bool trigger_start(TRIGGER_TYPE type);
		void run(void);
		bool setParameter(CMD_STRUCT* cmd);
		bool refresh_parameter(void);
		bool table_ptr_update(uint16_t id, uint8_t trace);
		bool send_DR_hlp(void);
		bool send_data(void);
		bool send_data_binary(void);

		//bool setup(void);
		//bool read_record(void);

		#define MAX_NUM_PARAMETER_DR 8
		PARAM_DEV module_parameter[MAX_NUM_PARAMETER_DR] =
		{
			{0x01, _UINT16, 	MAX_NUM_PARAMETER_DR,MAX_NUM_PARAMETER_DR, MAX_NUM_PARAMETER_DR, true, "Number of Parameters in Data Recorder Module"}, // every Module needs this parameter with number of
			{0x10, _UINT16, 	10, 						0, 5000,							false, "Record Rate"},
			{0x11, _UINT16, 	1, 						0, 2,							false, "Data Recorder Trigger Type"},				// 0 = OFF, 1 = MOV
			{0x12, _UINT16, 	4, 						0, 0,							false, "Number of Tables (1 to 4)"},
			{0x21, _UINT16, 	1, 						0, MAX_NUM_DR_TRACES_TO_CHOOSE,	false, "Table 1 ID"},		// [4]
			{0x22, _UINT16, 	2, 						0, MAX_NUM_DR_TRACES_TO_CHOOSE,	false, "Table 2 ID"},
			{0x23, _UINT16, 	6, 						0, MAX_NUM_DR_TRACES_TO_CHOOSE,	false, "Table 3 ID"},
			{0x24, _UINT16, 	7, 						0, MAX_NUM_DR_TRACES_TO_CHOOSE,	false, "Table 4 ID"},
			//{0x25, _UINT16, 	0, 						0, MAX_DR_TRACES,	false, "Table 5 ID"},//NEW
			//{0x26, _UINT16, 	0, 						0, MAX_DR_TRACES,	false, "Table 6 ID"},//NEW
			//{0x27, _UINT16, 	0, 						0, MAX_DR_TRACES,	false, "Table 7 ID"},//NEW
			//{0x28, _UINT16, 	0, 						0, MAX_DR_TRACES,	false, "Table 8 ID"},//NEW
		};

		uint16_t 		rec_cnt;										// record table value counter for record
		float 			record_table_1[DR_TABLE_LENGTH] = {0};		// record table 1
		float 			record_table_2[DR_TABLE_LENGTH] = {0};		// record table 2
		float 			record_table_3[DR_TABLE_LENGTH] = {0};		// record table 3
		float 			record_table_4[DR_TABLE_LENGTH] = {0};		// record table 4
		//float 			record_table_5[DR_TABLE_LENGTH] = {0};		// record table 5//NEW
		//float 			record_table_6[DR_TABLE_LENGTH] = {0};		// record table 6//NEW
		//float 			record_table_7[DR_TABLE_LENGTH] = {0};		// record table 7//NEW
		//float 			record_table_8[DR_TABLE_LENGTH] = {0};		// record table 8//NEW
		uint16_t 		record_rate_cnt;								// Counter Record Rate
		uint16_t 		record_rate_max;								// Record Rate Max; corresponds to the Record Rate parameter
		uint8_t			record_tables_max;
		bool			rec_on;											// Record running / trigger flag...
		float 			null;
		float* 			rec_table_ptr[4];								// Pointer to record variables
		TRIGGER_TYPE  	trigger_type;
		axis_class* 	myAxis[MAX_NUM_AXIS];
		char 			table_name[4][40];
};

#endif
