/***************************************************************************
 *  Position Sensor-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#include "pos_sensor.h"
#include "typedefs.h"
#include <cstring>

pos_sensor_class::pos_sensor_class(void)
{
	this->position 				= 0.0;
	this->velocity 				= 0.0;
	this->velocity_filtered 	= 0.0;
	this->velocity_filtered_1	= 0.0;
	this->position_1 			= 0.0;
	this->sensor_raw			= 0;
	this->sensor_raw_1			= 0;
	this->numerator				= 0;
	this->denominator			= 0;
	this->position_offset		= 0;
	this->next_idx   = 0;
    this->ready_idx  = 0;
    this->data_ready = false;
	//this->mt6835_raw_offset		= 0;

	// MT6835 Lese-Kommando in den (pro Achse eigenen) TX-Puffer legen
	const uint8_t cmd[MT6835_FRAME_LEN] = {0xA0, 0x03, 0x00, 0x00, 0x00, 0x00};
	memcpy(this->tx_buf, cmd, MT6835_FRAME_LEN);

}

void pos_sensor_class::init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr, SPI_HandleTypeDef* spi_ptr, GPIO_TypeDef* cs_port, uint16_t cs_pin)
{
	this->myParameter	= myParameter_ptr;
	this->myError		= myError_ptr;
	this->myAxis 		= Axis_ptr;
	this->mt6835_spi	= spi_ptr;		// SPI je Achse (A: SPI1, B: SPI2)
	this->mt6835_cs_port = cs_port;	// CS-Leitung je Achse
	this->mt6835_cs_pin	= cs_pin;
	this->refresh_parameter();

	//mt6835
	mt6835_init();
}

bool pos_sensor_class::setParameter(CMD_STRUCT* cmd)
{
	bool ret;

	if(this->myAxis->enabled == false)
	{
		ret = this->myParameter->setParameter(cmd, &this->module_parameter[0]);
		this->refresh_parameter();
		ret = true;
	}
	else
	{
		this->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_AXS_ON, __FILE__, __LINE__); // Module Parameter Locked - Servo is Running
		ret = false;
	}

	return ret;
}

bool pos_sensor_class::refresh_parameter(void)
{
	bool ret = true;

	// refresh parameter, only if no trajectory_class is performed
	if(this->myAxis->enabled == false)
	{
		this->numerator			= this->module_parameter[1].value.ui32;
		this->denominator		= this->module_parameter[2].value.ui32;
	}
	else
	{
		this->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_AXS_ON, __FILE__, __LINE__); // Parameter update not allowed, motion in progress
	}
	return ret;
}

// if no position sensor available
bool pos_sensor_class::get_stepper_position(void)
{
	this->position = myAxis->myCurrent_Control->getPosition(); // z. B. in Rad
	this->velocity = (this->position - this->position_1) * SERVO_FRQ;
	this->velocity_filtered_1 = this->velocity_filtered;
	this->velocity_filtered = 0.98 *this->velocity_filtered_1 + 0.02 * this->velocity;
	this->position_1 = this->position;

	return 1;
}

bool pos_sensor_class::get_position_AB(void)
{
	bool ret = true;
	ret = pos_sensor_class::get_value_AB(); // get raw sensor value in "sensor_raw"


	this->position = (float)this->sensor_raw * (float)this->numerator / (float)this->denominator + this->position_offset;
	this->velocity = (this->position - this->position_1) * SERVO_FRQ;
	this->velocity_filtered_1 = this->velocity_filtered;
	this->velocity_filtered = 0.98 *this->velocity_filtered_1 + 0.02 * this->velocity;
	this->position_1 = this->position;
	
	return ret;
}

bool pos_sensor_class::get_value_AB(void)
{
	bool ret = true;
	// get value from Hardware ... need to be adapted to HARDWARE Register TIMER?
	this->sensor_raw_1 = this->sensor_raw;
	this->sensor_raw =  __HAL_TIM_GetCounter(&htim2);
	return ret;
}

bool pos_sensor_class::get_position_SPI()
{
	// Check CRC
	// Check Status

	this->mt6835_result = mt6835_get_last_result();
	//this->position = mt6835_result.angle_raw; //TODO

	return true;
}

void pos_sensor_class::mt6835_init(void)
{
    // CSN idle-high, vom Encoder erwartet
    HAL_GPIO_WritePin(this->mt6835_cs_port, this->mt6835_cs_pin, GPIO_PIN_SET);

    // Ping-Pong-Zaehler für den asynchronen Betrieb zurücksetzen
    this->next_idx   = 0;
    this->ready_idx  = 0;
    this->data_ready = false;
}

// Aufruf aus der ISR herraus
void pos_sensor_class::mt6835_spi_txrx_cplt_callback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance != this->mt6835_spi->Instance) {
        return; /* falls mehrere SPI-Peripherien im Projekt genutzt werden */
    }

    // Transfer fertig -> CS wieder high
    HAL_GPIO_WritePin(this->mt6835_cs_port, this->mt6835_cs_pin, GPIO_PIN_SET);

    // gerade fertig gewordenen Puffer als "ready" markieren,
    // den anderen für den nächsten Aufruf von mt6835_start_read() vormerken
    this->ready_idx = this->next_idx;
    this->next_idx ^= 1u;	// wechsel zwischen 0 und 1 (hin und her und rund herum)
    this->data_ready = true;
}

void pos_sensor_class::mt6835_start_read(void)
{
    //CS low, danach DMA-Transfer anstoßen und sofort zurückkehren.
    //Ab hier läuft die Übertragung komplett autonom in Hardware.

    HAL_GPIO_WritePin(this->mt6835_cs_port, this->mt6835_cs_pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive_DMA(this->mt6835_spi, this->tx_buf, this->rx_buf[this->next_idx], MT6835_FRAME_LEN);
}

uint32_t pos_sensor_class::mt6835_parse_angle(const uint8_t d[MT6835_FRAME_LEN]) const
{
    // d[0], d[1]: Antwort waehrend der Kommandophase, Encoder liefert hier Hi-Z -> irrelevant
    return ((int32_t)d[2] << 13)     // ANGLE[20:13]
         | ((int32_t)d[3] <<  5)     // ANGLE[12:5]
         | ((int32_t)(d[4] >> 3));   // ANGLE[4:0] (STATUS[2:0] in Bit2..0)
}

mt6835_result_t pos_sensor_class::mt6835_get_last_result(void)
{
	// TODO: Call by reference??? 

    mt6835_result_t r;
    const uint8_t *d = rx_buf[this->ready_idx];

    // Roher, unverschobener Winkelwert direkt vom Chip (0..2^21-1)
    uint32_t angle_raw = (uint32_t)this->mt6835_parse_angle(d);

    r.angle_raw = angle_raw;
    r.angle_deg = (float)angle_raw * 360.0f / (float)(1UL << 21);
    r.angle_rad = (float)angle_raw * M_2PI / (float)(1UL << 21);
    r.status    = d[4] & 0x07u;
    r.crc       = d[5];

    return r;
}
