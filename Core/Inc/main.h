/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32g4xx_hal_def.h"
#include "stm32g4xx_it.h"
#include "tim.h"
#include "gpio.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MOT_A_RESET_Pin GPIO_PIN_13
#define MOT_A_RESET_GPIO_Port GPIOC
#define MOT_A_TEMP_FAULT_Pin GPIO_PIN_14
#define MOT_A_TEMP_FAULT_GPIO_Port GPIOC
#define MOT_A_FAULT_Pin GPIO_PIN_15
#define MOT_A_FAULT_GPIO_Port GPIOC
#define MOT_A_M1_Pin GPIO_PIN_3
#define MOT_A_M1_GPIO_Port GPIOF
#define MOT_A_M2_Pin GPIO_PIN_4
#define MOT_A_M2_GPIO_Port GPIOF
#define MOT_A_M3_Pin GPIO_PIN_5
#define MOT_A_M3_GPIO_Port GPIOF
#define T5_CH_2_ENCODER_B_B_NEU_Pin GPIO_PIN_7
#define T5_CH_2_ENCODER_B_B_NEU_GPIO_Port GPIOF
#define T15_CH1_BRAKE_A_NEU_Pin GPIO_PIN_9
#define T15_CH1_BRAKE_A_NEU_GPIO_Port GPIOF
#define T15_CH2_BRAKE_B_NEU_Pin GPIO_PIN_10
#define T15_CH2_BRAKE_B_NEU_GPIO_Port GPIOF
#define HSE_IN_Pin GPIO_PIN_0
#define HSE_IN_GPIO_Port GPIOF
#define HSE_OUT_Pin GPIO_PIN_1
#define HSE_OUT_GPIO_Port GPIOF
#define nReset_Pin GPIO_PIN_10
#define nReset_GPIO_Port GPIOG
#define T1_CH1_PWM_Mot_A_A_Pin GPIO_PIN_0
#define T1_CH1_PWM_Mot_A_A_GPIO_Port GPIOC
#define T1_CH2_PWM_Mot_A_B_Pin GPIO_PIN_1
#define T1_CH2_PWM_Mot_A_B_GPIO_Port GPIOC
#define T1_CH3_PWM_Mot_A_C_Pin GPIO_PIN_2
#define T1_CH3_PWM_Mot_A_C_GPIO_Port GPIOC
#define T1_CH4_PWM_Mot_A_D_Pin GPIO_PIN_3
#define T1_CH4_PWM_Mot_A_D_GPIO_Port GPIOC
#define T2_CH1_ENCODER_A_A_NEU_Pin GPIO_PIN_0
#define T2_CH1_ENCODER_A_A_NEU_GPIO_Port GPIOA
#define ADC1_CH2_Cur_A_B_Pin GPIO_PIN_1
#define ADC1_CH2_Cur_A_B_GPIO_Port GPIOA
#define ADC1_CH3_Cur_A_C_Pin GPIO_PIN_2
#define ADC1_CH3_Cur_A_C_GPIO_Port GPIOA
#define ADC1_CH4_Cur_A_D_Pin GPIO_PIN_3
#define ADC1_CH4_Cur_A_D_GPIO_Port GPIOA
#define SPI3_SS_EXT_NEU_Pin GPIO_PIN_4
#define SPI3_SS_EXT_NEU_GPIO_Port GPIOA
#define SPI1_SCK_TW29_A_Pin GPIO_PIN_5
#define SPI1_SCK_TW29_A_GPIO_Port GPIOA
#define SPI1_MISO_TW29_A_Pin GPIO_PIN_6
#define SPI1_MISO_TW29_A_GPIO_Port GPIOA
#define SPI1_MOSI_TW29_A_Pin GPIO_PIN_7
#define SPI1_MOSI_TW29_A_GPIO_Port GPIOA
#define LED1_GREEN_Pin GPIO_PIN_4
#define LED1_GREEN_GPIO_Port GPIOC
#define LED2_GREEN_Pin GPIO_PIN_5
#define LED2_GREEN_GPIO_Port GPIOC
#define LED3_YELLOW_Pin GPIO_PIN_0
#define LED3_YELLOW_GPIO_Port GPIOB
#define LED4_RED_Pin GPIO_PIN_1
#define LED4_RED_GPIO_Port GPIOB
#define T5_CH2_ENCODER_B_A_NEU_Pin GPIO_PIN_2
#define T5_CH2_ENCODER_B_A_NEU_GPIO_Port GPIOB
#define T20_CH1_PWM_Mot_B_A_Pin GPIO_PIN_12
#define T20_CH1_PWM_Mot_B_A_GPIO_Port GPIOF
#define T20_CH2_PWM_Mot_B_B_Pin GPIO_PIN_13
#define T20_CH2_PWM_Mot_B_B_GPIO_Port GPIOF
#define T20_CH3_PWM_Mot_B_C_Pin GPIO_PIN_14
#define T20_CH3_PWM_Mot_B_C_GPIO_Port GPIOF
#define T20_CH4_PWM_Mot_B_D_Pin GPIO_PIN_15
#define T20_CH4_PWM_Mot_B_D_GPIO_Port GPIOF
#define SIN_COS_A_RESET_NEU_Pin GPIO_PIN_9
#define SIN_COS_A_RESET_NEU_GPIO_Port GPIOE
#define SIN_COS_A_FAULT_NEU_Pin GPIO_PIN_10
#define SIN_COS_A_FAULT_NEU_GPIO_Port GPIOE
#define SIN_COS_B_RESET_NEU_Pin GPIO_PIN_11
#define SIN_COS_B_RESET_NEU_GPIO_Port GPIOE
#define SIN_COS_B_FAULT_NEU_Pin GPIO_PIN_12
#define SIN_COS_B_FAULT_NEU_GPIO_Port GPIOE
#define ADC1_CH11_Cur_A_A_NEU_Pin GPIO_PIN_12
#define ADC1_CH11_Cur_A_A_NEU_GPIO_Port GPIOB
#define SPI2_SCK_TW29_B_Pin GPIO_PIN_13
#define SPI2_SCK_TW29_B_GPIO_Port GPIOB
#define SPI2_MISO_TW29_B_Pin GPIO_PIN_14
#define SPI2_MISO_TW29_B_GPIO_Port GPIOB
#define SPI2_MOSI_TW29_B_Pin GPIO_PIN_15
#define SPI2_MOSI_TW29_B_GPIO_Port GPIOB
#define ADC4_CH12_Cur_B_D_Pin GPIO_PIN_8
#define ADC4_CH12_Cur_B_D_GPIO_Port GPIOD
#define ADC4_CH13_Cur_B_C_Pin GPIO_PIN_9
#define ADC4_CH13_Cur_B_C_GPIO_Port GPIOD
#define ADC4_CH7_Cur_B_B_Pin GPIO_PIN_10
#define ADC4_CH7_Cur_B_B_GPIO_Port GPIOD
#define ADC4_CH8_Cur_B_A_Pin GPIO_PIN_11
#define ADC4_CH8_Cur_B_A_GPIO_Port GPIOD
#define T4_CH1_MOT_B_HALL_A_NEU_Pin GPIO_PIN_12
#define T4_CH1_MOT_B_HALL_A_NEU_GPIO_Port GPIOD
#define T4_CH2_MOT_B_HALL_B_NEU_Pin GPIO_PIN_13
#define T4_CH2_MOT_B_HALL_B_NEU_GPIO_Port GPIOD
#define T4_CH3_MOT_B_HALL_C_NEU_Pin GPIO_PIN_14
#define T4_CH3_MOT_B_HALL_C_NEU_GPIO_Port GPIOD
#define SPI2_SS_TW29_B_Pin GPIO_PIN_15
#define SPI2_SS_TW29_B_GPIO_Port GPIOD
#define T8_CH1_MOT_A_HALL_A_NEU_Pin GPIO_PIN_6
#define T8_CH1_MOT_A_HALL_A_NEU_GPIO_Port GPIOC
#define T8_CH2_MOT_A_HALL_B_NEU_Pin GPIO_PIN_7
#define T8_CH2_MOT_A_HALL_B_NEU_GPIO_Port GPIOC
#define MOT_B_M3_Pin GPIO_PIN_1
#define MOT_B_M3_GPIO_Port GPIOG
#define MOT_B_M2_Pin GPIO_PIN_2
#define MOT_B_M2_GPIO_Port GPIOG
#define MOT_B_M1_Pin GPIO_PIN_3
#define MOT_B_M1_GPIO_Port GPIOG
#define MOT_B_FAULT_Pin GPIO_PIN_4
#define MOT_B_FAULT_GPIO_Port GPIOG
#define MOT_B_TEMP_FAULT_Pin GPIO_PIN_8
#define MOT_B_TEMP_FAULT_GPIO_Port GPIOC
#define MOT_B_RESET_Pin GPIO_PIN_9
#define MOT_B_RESET_GPIO_Port GPIOC
#define USB_D__Pin GPIO_PIN_11
#define USB_D__GPIO_Port GPIOA
#define USB_D_A12_Pin GPIO_PIN_12
#define USB_D_A12_GPIO_Port GPIOA
#define SWD_SWDIO_Pin GPIO_PIN_13
#define SWD_SWDIO_GPIO_Port GPIOA
#define T5_ETR_ENCODER_B_INDEX_NEU_Pin GPIO_PIN_6
#define T5_ETR_ENCODER_B_INDEX_NEU_GPIO_Port GPIOF
#define SWD_SWCLK_Pin GPIO_PIN_14
#define SWD_SWCLK_GPIO_Port GPIOA
#define UART4_TX_Pin GPIO_PIN_10
#define UART4_TX_GPIO_Port GPIOC
#define UART4_RX_Pin GPIO_PIN_11
#define UART4_RX_GPIO_Port GPIOC
#define UART_5_TX_NEU_Pin GPIO_PIN_12
#define UART_5_TX_NEU_GPIO_Port GPIOC
#define SPI3_SCK_EXT_Pin GPIO_PIN_9
#define SPI3_SCK_EXT_GPIO_Port GPIOG
#define UART_5_RX_Pin GPIO_PIN_2
#define UART_5_RX_GPIO_Port GPIOD
#define T2_CH2_ENCODER_A_A_NEU_Pin GPIO_PIN_4
#define T2_CH2_ENCODER_A_A_NEU_GPIO_Port GPIOD
#define SPI3_MISO_EXT_Pin GPIO_PIN_4
#define SPI3_MISO_EXT_GPIO_Port GPIOB
#define SPI3_MOSI_EXT_Pin GPIO_PIN_5
#define SPI3_MOSI_EXT_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_7
#define I2C1_SDA_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_8
#define I2C1_SCL_GPIO_Port GPIOB
#define T8_CH3_MOT_A_HALL_C_NEU_Pin GPIO_PIN_9
#define T8_CH3_MOT_A_HALL_C_NEU_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

void SystemClock_Config(void);



/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
