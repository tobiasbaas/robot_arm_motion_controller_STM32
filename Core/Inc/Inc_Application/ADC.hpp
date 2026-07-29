/*
 * ADC.hpp
 *
 *  Created on: Jan 12, 2024
 *      Author: nirut
 */

#ifndef INC_ADC_HPP_
#define INC_ADC_HPP_

//#include "main.h
#include <cstdint>
#include "stm32g4xx_hal.h"
#include "typedefs.h"

const float offset_m = 0.811;
const float offset_c = -0.039;

const float resistance = 0.33;
const float conv = offset_m * (3.3f/4095.0) / resistance;	// I=ADC_Val*Umax/2^12/0.33

const float ADC_OFFSET_CH1 = 1940;
const float ADC_OFFSET_CH2 = 1960; // 1.58V*(4095/3.3V)
const float ADC_OFFSET_CH3 = 1940;


#ifdef __cplusplus
  class ADC_class
  {
    public:
    ADC_class();
    void getADC_Val(float &pADC_Value, float pPI_Y);
    void getADC_Val_BLDC_BC(float &pADCValue,int8_t &pPhaseA,int8_t &pPhaseB,int8_t &pPhaseC, int &pdirection);
    void getADC_Val_BLDC_FOC(float &pADCValueA,float &pADCValueB,float &pADCValueC);
  };
#endif

#endif /* INC_ADC_HPP_ */
