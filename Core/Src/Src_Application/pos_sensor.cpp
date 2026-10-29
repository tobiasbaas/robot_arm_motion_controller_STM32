#include "pos_sensor.h"
#include "typedefs.h"
#include <cstdint>
#include <cstring>

pos_sensor_class::pos_sensor_class(void)
{

}

void pos_sensor_class::init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr, SPI_HandleTypeDef* spi_ptr, GPIO_TypeDef* cs_port, uint16_t cs_pin)
{
	this->myParameter	= myParameter_ptr;
	this->myError		= myError_ptr;
	this->myAxis 		= Axis_ptr;

	this->mt6835_spi	= spi_ptr;		// SPI je Achse (A: SPI1, B: SPI2)
	this->mt6835_cs_port = cs_port;	// CS-Leitung je Achse
	this->mt6835_cs_pin	= cs_pin;

    // MT6835 Lese-Kommando in den (pro Achse eigenen) TX-Puffer legen
	const uint8_t cmd[MT6835_FRAME_LEN] = {0xA0, 0x03, 0x00, 0x00, 0x00, 0x00};
	memcpy(this->tx_buf, cmd, MT6835_FRAME_LEN);

	this->refresh_parameter();

    // calculate the step size for the electrical angle
    if(myAxis->my_instance == 0)    // Axis 1/A
    {
        this->pole_pairs = AXA_MOTORSTEP_COUNT / 4;
    }
    else                            // Axis 2/B
    {
        this->pole_pairs = AXB_MOTORSTEP_COUNT / 4;
    }

    this->pos_scale = (float)(M_2PI * (float)numerator / (float)denominator);   // scale for mech angle

    this->mech_position = 0.0f;
    this->mech_position_1 = 0.0f;

    this->velocity = 0.0f;
    this->velocity_filtered = 0.0f;
    this->velocity_filtered_1 = 0.0f;

    this->data_ready = false;
    this->mt6835_init_sample = true;

	//mt6835_init();
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
		//this->abs_offset		= this->module_parameter[3].value.ui32;

        this->pos_scale = (float)(M_2PI * (float)numerator / (float)denominator); // scale for mech angle
	}
	else
	{
		this->myError->add(PARAMETER_MOD, ERR_PAM_LOCKED_AXS_ON, __FILE__, __LINE__); // Parameter update not allowed, motion in progress
	}
	return ret;
}

bool pos_sensor_class::get_stepper_position(void)
{
	this->mech_position = myAxis->myCurrent_Control->getMechAngle(); //in Rad
	this->velocity = (this->mech_position - this->mech_position_1) * SERVO_FRQ;
	this->velocity_filtered_1 = this->velocity_filtered;
	this->velocity_filtered = 0.98f *this->velocity_filtered_1 + 0.02f * this->velocity;
	this->mech_position_1 = this->mech_position;

	return 0;
}

float pos_sensor_class::get_el_position(void)
{
    if (data_ready) { mt6835_get_last_result(); stale_cnt = 0; }
    else if (stale_cnt < 0xFFFFu) stale_cnt++;
    return mt6835_result.angle_el;       // letzter gültiger Winkel
}

bool pos_sensor_class::mt6835_init(void)
{
    // CSN idle-high, vom Encoder erwartet
    HAL_GPIO_WritePin(this->mt6835_cs_port, this->mt6835_cs_pin, GPIO_PIN_SET);

    // Ping-Pong-Zaehler für den asynchronen Betrieb zurücksetzen
    this->next_idx   = 0;
    this->ready_idx  = 0;
    this->data_ready = false;
    memset(&mt6835_result, 0, sizeof(mt6835_result));   // reset all values
    el_offset = 0;
    stale_cnt = 0;
    this->mt6835_init_sample = true;

    
    //myAxis->myCurrent_Control->setElectricAngleToZero();    // Set motor to know electrical angle

    mt6835_start_read();
    const uint32_t t0 = HAL_GetTick();
    while (!data_ready) { if (HAL_GetTick() - t0 > 100u) return false; }   // Fehler melden

    mt6835_get_last_result();
    this->mt6835_init_sample = false;  // from here, electrical and absolute angles will be counted in the usual way

    return true;
}

void pos_sensor_class::mt6835_start_read(void)
{
    //CS low, danach DMA-Transfer anstoßen und sofort zurückkehren.
    //Ab hier läuft die Übertragung komplett autonom in Hardware.

    HAL_GPIO_WritePin(this->mt6835_cs_port, this->mt6835_cs_pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive_DMA(this->mt6835_spi, this->tx_buf, this->rx_buf[this->next_idx], MT6835_FRAME_LEN);
}

void pos_sensor_class::mt6835_spi_txrx_cplt_callback(SPI_HandleTypeDef *hspi)
{
    //if (hspi->Instance != this->mt6835_spi->Instance) {
    //    return; /* falls mehrere SPI-Peripherien im Projekt genutzt werden */
    //}

    // Transfer fertig -> CS wieder high
    HAL_GPIO_WritePin(this->mt6835_cs_port, this->mt6835_cs_pin, GPIO_PIN_SET);

    // gerade fertig gewordenen Puffer als "ready" markieren,
    // den anderen für den nächsten Aufruf von mt6835_start_read() vormerken
    this->ready_idx = this->next_idx;
    this->next_idx ^= 1u;	// wechsel zwischen 0 und 1 (hin und her und rund herum)
    this->data_ready = true;
}

uint32_t pos_sensor_class::mt6835_parse_angle(const uint8_t d[MT6835_FRAME_LEN]) const
{
    // d[0], d[1]: Antwort während der Kommandophase, Encoder liefert hier Hi-Z -> irrelevant
    return ((int32_t)d[2] << 13)     // ANGLE[20:13]
         | ((int32_t)d[3] <<  5)     // ANGLE[12:5]
         | ((int32_t)(d[4] >> 3));   // ANGLE[4:0] (STATUS[2:0] in Bit2..0)
}

void pos_sensor_class::mt6835_get_last_result(void)
{

    const uint8_t *d = rx_buf[this->ready_idx]; // get SPI buffer
    uint32_t raw = this->mt6835_parse_angle(d);   // extract raw angle from SPI buffer

    // initial measurement
    if(this->mt6835_init_sample)
    { 
        this->mt6835_result.angle_raw = (int32_t)raw;
        abs_offset = mt6835_result.angle_raw;   // is used to set the fist position as the zero position on start-up; 
                                                // later, abs_offset should be set via the parameter to get the same zero position every time
        this->mt6835_result.angle_rad = (float)(mt6835_result.angle_raw - abs_offset) * pos_scale;
        el_offset = (mt6835_result.angle_raw * pole_pairs) & pos_sensor_const::MT6835_MASK;
        //this->mt6835_result.angle_el = 0.0f;
        this->mt6835_last_raw_ticks = raw;
        return;
    }


    // kürzeste Distanz zum letzten Sample ermitteln (signed, mit 21-Bit-Wrap)
    int32_t delta = (int32_t)((raw - this->mt6835_last_raw_ticks) & pos_sensor_const::MT6835_MASK);
    if (delta >= (1L << 20)) delta -= (1L << 21);   // in [-2^20, 2^20) zwingen -> kürzester Weg, auch über den Wrap hinweg


    this->mt6835_last_raw_ticks  = raw;

    // raw angle
    mt6835_result.angle_raw += delta;

    // absolute angle [rad]
    mt6835_result.angle_rad = (float)(mt6835_result.angle_raw - abs_offset) * pos_scale;


    // electircal angle [rad]; 0 <= angle_el <= 2*Pi
    const uint32_t e  = (raw * pole_pairs) & pos_sensor_const::MT6835_MASK;      // Lage innerhalb einer elektrischen Periode
    const uint32_t el = (e - el_offset) & pos_sensor_const::MT6835_MASK;
    mt6835_result.angle_el = (float)el * pos_sensor_const::RAD_PER_TICK_EL;      // 0 .. 2π

    /*mt6835_result.angle_el  = (mt6835_result.angle_raw * rad_per_tick);  
    if (mt6835_result.angle_el >= M_2PI) {
        mt6835_result.angle_el -= M_2PI;
    } else if (mt6835_result.angle_el < 0.0f) {
        mt6835_result.angle_el += M_2PI;
    }*/

    // status
    mt6835_result.status    = d[4] & 0x07u;
    mt6835_result.crc       = d[5];


    data_ready = false;

    return;
}