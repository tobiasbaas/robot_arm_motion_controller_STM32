/***************************************************************************
 *  Position Sensor-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#ifndef _POS_SENSOR_H
#define _POS_SENSOR_H

#include "parameter.h"
#include "axis.h"
#include "spi.h"
#include "stm32g4xx_hal_spi.h"
#include "tim.h"

// forward declaration
class axis_class;
class parameter_class;
class error_class;

// mt6835
//extern SPI_HandleTypeDef hspi1;          // von CubeMX generiertes SPI-Handle 
//#define MT6835_SPI_HANDLE      (&hspi1)

//#define MT6835_CS_GPIO_Port    GPIOG     
//#define MT6835_CS_Pin          GPIO_PIN_5

#define MT6835_FRAME_LEN       6u

typedef struct {
    int32_t   angle_raw;   // 21-Bit Rohwert, verschoben um den Startwert (genullt, kann negativ werden)
    float     angle_deg;   // umgerechnet auf 0..360 Grad
	float 	  angle_rad;	  // umgerechnet auf 0..2Pi 		
    uint8_t   status;      // STATUS[2:0]: Bit0=Overspeed, Bit1=Weak Field, Bit2=Undervoltage 
    uint8_t   crc;         // CRC[7:0], hier noch ungeprüft 
} mt6835_result_t;

class pos_sensor_class
{
	private:
		axis_class* 		myAxis;
		parameter_class* 	myParameter;
		error_class* 		myError;

		// Rohwert aus einem 6-Byte-MT6835-Frame extrahieren (ANGLE[20:0])
		uint32_t mt6835_parse_angle(const uint8_t d[MT6835_FRAME_LEN]) const;

	public:
		pos_sensor_class();

		void init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr, SPI_HandleTypeDef* spi_ptr, GPIO_TypeDef* cs_port, uint16_t cs_pin);
		bool setParameter(CMD_STRUCT* cmd);
		bool refresh_parameter(void);

		bool get_stepper_position(void);

		bool get_position_AB(void);
		bool get_value_AB(void);

		bool get_position_SPI(void);
		void mt6835_init(void);
		void mt6835_spi_txrx_cplt_callback(SPI_HandleTypeDef *hspi);
		void mt6835_start_read(void);
		void mt6835_update_unwrapped_position(uint32_t raw_ticks);
		mt6835_result_t mt6835_get_last_result(void);

		//bool get_value_SinCos(void);
		

		#define MAX_NUM_PAM_SENSOR 3
		// parameter TYPES: _UINT64, _UINT32, _UINT16, _INT64, _INT32, _INT16, _FLOAT, _DOUBLE, _STRING, _BOOL, _INIT

		PARAM_DEV module_parameter[MAX_NUM_PAM_SENSOR] =
		{
			{0x01, _UINT16, 	MAX_NUM_PAM_SENSOR, 0, 0,			true,	 "Number of Axis Module Parameters"}, // every Module needs this parameter with number of
			{0x10, _UINT32, 	1, 					0, UINT_32_MAX,	false,	 "Numerator"},
			{0x11, _UINT32, 	8192, 		 		0, UINT_32_MAX,	false,	 "Denominator"},
		};


		float		position; 			// position in [?]
		float		position_1;
		float		position_offset;	// offset in [?]

		float		velocity;			 	// in [?]
		float		velocity_filtered;		
		float		velocity_filtered_1;	
		
		int32_t 	sensor_raw;	
		int32_t 	sensor_raw_1;

		uint32_t	numerator;
		uint32_t 	denominator;

		//uint32_t	mt6835_raw_offset;	// Startwert des MT6835 (in mt6835_init gesetzt)

		//mt6835 -- Puffer pro Achse (jede Achse hat eigenen Encoder/SPI)
		uint8_t				tx_buf[MT6835_FRAME_LEN];
		uint8_t				rx_buf[2][MT6835_FRAME_LEN]; // Ping-Pong RX-Puffer: einer wird vom DMA befüllt, andere kann gelesen werden.

		volatile uint8_t 	next_idx;   // Puffer, in den der laufende/naechste Transfer schreibt 
		volatile uint8_t 	ready_idx;  // Puffer mit dem zuletzt fertigen Datensatz 
		volatile bool 		data_ready;

		// Pro Achse: welcher SPI und welche CS-Leitung für mt68j35 (wird in init() gesetzt)
		SPI_HandleTypeDef*	mt6835_spi;
		GPIO_TypeDef*		mt6835_cs_port;
		uint16_t			mt6835_cs_pin;


		uint32_t mt6835_last_raw_ticks = 0;
		int64_t  mt6835_position_ticks = 0;   // kontinuierlich, mehrere Umdrehungen möglich
		bool     mt6835_first_sample   = true;
		mt6835_result_t mt6835_result;	
};

#endif
