#pragma once

#include "axis.h"
#include "parameter.h"
#include "controller_error.h"
#include "cmsis_gcc.h"
#include "typedefs.h"
#include <cstdint>
#include <cstring>

// forward declaration
class axis_class;
class parameter_class;
class error_class;

#define MT6835_FRAME_LEN 6u

namespace pos_sensor_const{
    constexpr uint32_t MT6835_MAX_VAL    = 2097152u;               // 2^21 Ticks pro Umdrehung
    constexpr uint32_t MT6835_MASK = MT6835_MAX_VAL - 1u;
    constexpr float    RAD_PER_TICK_EL = 6.2831853f / 2097152.0f;   // eine elektrische Periode = 2^21 "Tick-Einheiten"

    // Plausibilitaetsgrenze fuer den Sprung zwischen zwei Samples [Ticks].
    // 2^17 Ticks = 1/16 Umdrehung pro Sample; bei 10 kHz entspricht das
    // 625 U/s und liegt damit deutlich ueber jeder realen Motordrehzahl.
    // Groessere Spruenge werden als Fehlerframe verworfen.
    constexpr int32_t  MT6835_MAX_DELTA = 131072;                    // 2^17
};

typedef struct {
    int32_t   angle_raw;   	// absoluter mechanischer Winkel in [ticks]
    float     angle_rad;   	// absoluter mechanischer Winkel in Bezug auf ein Festgelegten 0 Punkt; -lim bis + lim in [rad]
	float 	  angle_el;	    // elektrischer Winkel auf 0..2Pi normiert; [rad]		
    uint8_t   status;      	// STATUS[2:0]: Bit0=Overspeed, Bit1=Weak Field, Bit2=Undervoltage 
    uint8_t   crc;         	// empfangene CRC[7:0]; wird gegen mt6835_crc8() geprueft
} mt6835_result_t;

class pos_sensor_class
{
    public:
        pos_sensor_class();
		void init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr, SPI_HandleTypeDef* spi_ptr, GPIO_TypeDef* cs_port, uint16_t cs_pin);
		bool setParameter(CMD_STRUCT* cmd);
		bool refresh_parameter(void);

        bool get_stepper_position(void);    // returns assumed pos value based on the trajectory calculation

        // get_mech_position(); // calculates the absolute mechanical angle based on a specified start point
        float get_el_position(void);        // provides the electrical angle within one period

        bool mt6835_init(void);             // initialize encoder
        // calibrate
        bool mt6835_check_crc(const uint8_t d[MT6835_FRAME_LEN]) const;   // true = Prüfsumme stimmt
        static uint8_t mt6835_crc8(const uint8_t *data, uint32_t len);

        void mt6835_start_read(void);                                           // starts reading the current encoder value; DMA takes over
		void mt6835_spi_txrx_cplt_callback(SPI_HandleTypeDef *hspi);            // runs every SPI ISR
        void mt6835_spi_error_callback(SPI_HandleTypeDef *hspi);                 // runs on SPI/DMA error; releases CS again
        void mt6835_get_last_result(void);                           // updates the position values based on the latest SPI buffer; false = frame verworfen
        uint8_t mt6835_crc8(const uint8_t d[MT6835_FRAME_LEN]) const;            // CRC8 ueber ANGLE[20:0] + STATUS[2:0]
        uint32_t mt6835_parse_angle(const uint8_t d[MT6835_FRAME_LEN]) const;   // extract raw data from a 6-byte MT6835 frame (ANGLE[20:0])


        #define MAX_NUM_PAM_SENSOR 4
		// parameter TYPES: _UINT64, _UINT32, _UINT16, _INT64, _INT32, _INT16, _FLOAT, _DOUBLE, _STRING, _BOOL, _INIT

		PARAM_DEV module_parameter[MAX_NUM_PAM_SENSOR] =
		{
			{0x01, _UINT16, 	MAX_NUM_PAM_SENSOR, 0, 0,			true,	 "Number of Axis Module Parameters"}, // every Module needs this parameter with number of
			{0x10, _UINT32, 	1, 					0, UINT_32_MAX,	false,	 "Numerator"},
			{0x11, _UINT32, 	pos_sensor_const::MT6835_MAX_VAL, 		 		1, UINT_32_MAX,	false,	 "Denominator"},
            {0x12, _UINT32, 0, 0, UINT_32_MAX, false, "Offset for absolute zero position; [raw ticks]"}
		};


        // Public variables
        SPI_HandleTypeDef*	mt6835_spi;

        float		mech_position; 			// mechanical position in [rad]
        float		mech_position_1;        // mechanical position in [rad]; n-1

        float		velocity;			 	// velocity in [rad/s]
        float		velocity_filtered;		// velocity filtered; [rad/s]
        float		velocity_filtered_1;	// velocity filtered; n-1

        mt6835_result_t mt6835_result;	    // latest encoder readings
        float angle_raw_float;  // to be able to record raw angle with data recorder

        volatile uint32_t crc_err_cnt;     // Anzahl wegen CRC-Fehler verworfener Frames
        //volatile uint32_t spi_err_cnt;     // Anzahl fehlgeschlagener SPI-Starts / SPI-Fehler

        uint8_t dbg_init_frame[6], dbg_first_frame[6]; bool dbg_first_taken;

    private:
        axis_class* 		myAxis;
		parameter_class* 	myParameter;
		error_class* 		myError;

        // per axis: which SPI and which CS line for the mt68j35 (set in init())    
        GPIO_TypeDef*		mt6835_cs_port;
        uint16_t			mt6835_cs_pin;

        //mt6835 -- Buffer per axis (each axis has its own encoder/SPI)
        uint8_t				tx_buf[MT6835_FRAME_LEN];   // contains the SPI command for reading the absolute value
        uint8_t				rx_buf[2][MT6835_FRAME_LEN]; // Ping-pong RX buffer: one is filled by the DMA, whilst the other can be read

        volatile uint8_t 	next_idx;   // Puffer, in den der laufende/naechste  Transfer schreibt 
        volatile uint8_t 	ready_idx;  // Puffer mit dem zuletzt fertigen Datensatz 
        volatile bool 		data_ready;
        uint16_t stale_cnt;

        // module parameters
        uint32_t	numerator;              // * 2Pi / denominator = scaling factor that indicates how many radians the axis (not the motor) rotates per encoder step
        uint32_t 	denominator;            // 
        volatile int32_t abs_offset;             // Offset for absolute zero position in [raw ticks]

        uint32_t    init_el_angle;              // value for which the electrical angle is 0

        uint32_t pole_pairs;
        volatile float  pos_scale;              // mech winkel [rad Abtrieb pro Tick]

        bool mt6835_init_sample;            // important for the (first) initial measurement
        uint32_t mt6835_last_raw_ticks;      // saves the last raw value to handle wrap-around
};