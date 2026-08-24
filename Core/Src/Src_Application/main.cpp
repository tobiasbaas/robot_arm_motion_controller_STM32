/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "fdcan.h"
#include "i2c.h"
#include "usart.h"
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <stdint.h>
#include "typedefs.h"
#include "commando_interpreter.h"
#include "app_freertos.h"
#include "monitor.h"
#include <stdbool.h>
#include "CO_app_STM32.h"
#include "OD.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TIM_OC_InitTypeDef sEmulConfigOC;

char cmd_str[CMD_STR_LENGTH] = {0};
volatile bool new_cmd = false;

uint8_t axis_to_run = 0;
uint64_t servo_cnt = 0;

int32_t joint1_old = 0;
int32_t joint2_old = 0;


volatile float convVal1 = 0;
volatile float convVal2 = 0;
volatile float convVal3 = 0;

// Global class pointer for timer interrupt handler
axis_class*             myAxis_ptr[MAX_NUM_AXIS];
data_recorder_class*    myDR_ptr;
PID_class*              myPID_pos_ptr[MAX_NUM_AXIS];
current_control_class*  myCurrent_Control[MAX_NUM_AXIS];
set_pwm_class*          myPWM_ptr[MAX_NUM_AXIS];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void send_string(const char* str, bool new_line);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

axis_class 				myAxis[MAX_NUM_AXIS];
set_pwm_class			myPWM[MAX_NUM_AXIS];

error_class 			myError;

parameter_class 	myParameter;

system_class 		mySystem(&myParameter);

ComInt_class 			com_inter(&myError);

// Classes belonging to axis
trajectory_class 		  trajectory[MAX_NUM_AXIS];
pos_sensor_class		  pos_sensor[MAX_NUM_AXIS];
PID_class				      pid_pos[MAX_NUM_AXIS];
current_control_class current_control[MAX_NUM_AXIS];

// Data recorder
data_recorder_class 		myDR(&myParameter,&myError);

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  // tells the CPU that the interrupt vector table starts at the default flash address 0x08000000
	SCB->VTOR = 0x08000000;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  MX_USB_Device_Init();

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC4_Init();
  MX_FDCAN1_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
  MX_TIM20_Init();
  MX_UART4_Init();
  MX_TIM4_Init();
  MX_TIM8_Init();
  MX_TIM15_Init();
  MX_UART5_Init();
  MX_TIM3_Init();
  MX_TIM17_Init();
  MX_TIM16_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_I2C2_Init();
  MX_TIM6_Init();
  MX_LPUART1_UART_Init();

  /* USER CODE BEGIN 2 */

  #ifndef SEND_OVER_USB
    HAL_StatusTypeDef uart_status;
    uart_status = HAL_UART_Receive_DMA(&hlpuart1, rx_buff, 1);
  #endif

	myParameter.myError = &myError;

	myDR_ptr = &myDR;	// pointer only for debug reason


	com_inter.myDR = &myDR;
	com_inter.myParameter	= &myParameter;
  com_inter.mySystem = &mySystem;


   // Initialization of axis modules
   for(uint8_t i=0;i<MAX_NUM_AXIS;i++)
   {
 	  pid_pos[i].init(&myAxis[i],&myParameter,&myError);
 	  trajectory[i].init(&myAxis[i],&myParameter,&myError);
 	  pos_sensor[i].init(&myAxis[i],&myParameter,&myError);
 	  current_control[i].init(&myAxis[i],&myParameter,&myError, &myPWM[i],&trajectory[i]);
 	  // pointer to classes belonging to an Axis:
 	  myAxis[i].init(i,&myError,&trajectory[i],&pos_sensor[i],&pid_pos[i],&current_control[i],&myDR,&myParameter);

 	  current_control[i].init(&myAxis[i],&myParameter,&myError, &myPWM[i], &trajectory[i]); // second init with konown axes


 	  myAxis_ptr[i] 			= &myAxis[i];
 	  com_inter.myAxis[i] = myAxis_ptr[i];
 	  myDR.myAxis[i] 			= myAxis_ptr[i];

 	  myPID_pos_ptr[i]     = &pid_pos[i];
 	  myCurrent_Control[i] = &current_control[i];
   }


   mySystem.init();
   myDR.init();

  //send_string("Startup Controller...", true);

   myAxis[1].enabled = true;
   myPID_pos_ptr[1]->servo_on_off(true);

   myAxis[0].enabled = true;
   myPID_pos_ptr[0]->servo_on_off(true);

   HAL_TIM_Base_Start_IT(&htim3);


  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while(1){

	  if(new_cmd == 1)
		// run data recorder if pointer is valid
		if (myDR_ptr) {
			myDR_ptr->run();
		}
	}

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
   */
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO_PG10, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim TIM handle
  * @retval None
  */
__weak void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */

	if (htim->Instance == TIM7) {
			HAL_IncTick();
		}

	// Handle CANOpen app interrupts
	if (htim == canopenNodeSTM32->timerHandle) {
		canopen_app_interrupt();
	}

	if (htim->Instance == TIM3) {
		// counter (Servo Loop)
		servo_cnt++;

		// jump between axes
		axis_to_run = (axis_to_run + 1) % MAX_NUM_AXIS;

		// run axis if pointer is valid
		if (axis_to_run < MAX_NUM_AXIS && myAxis_ptr[axis_to_run]) {
			// CANopen Daten
			if(axis_to_run == 0 && (joint1_old != OD_PERSIST_COMM.x6001_joint1))
			{
				myAxis_ptr[axis_to_run]->myTrajectory->setpoint = (OD_PERSIST_COMM.x6001_joint1/1000000.0)*5.0;//*10.0;//*5.0; // M1 Faktor 5.0
				joint1_old = OD_PERSIST_COMM.x6001_joint1;
			}

			if(axis_to_run == 1 && (joint2_old != OD_PERSIST_COMM.x6002_joint2))
			{
				myAxis_ptr[axis_to_run]->myTrajectory->setpoint = (OD_PERSIST_COMM.x6002_joint2/1000000.0)*-5.0;//*10.0;//*5.0; // M1 Faktor 5.0
				joint2_old = OD_PERSIST_COMM.x6002_joint2;
			}

			myAxis_ptr[axis_to_run]->run();

			// calculate servoload if TIM4 is initialized and constant is valid
			#ifdef TIMER_COUTER_MAX
			if (TIMER_COUTER_MAX > 0) {
				myAxis_ptr[axis_to_run]->servoload = (TIM4->CNT * 100) / TIMER_COUTER_MAX;
			}
			#endif
		}

		// run data recorder if pointer is valid
		if (myDR_ptr) {
			myDR_ptr->run();
		}
	}

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
   */
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	  /*
	  // Überprüfe auf Überhitzung oder Fehler
	  if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14) == GPIO_PIN_RESET) {
		  // Überhitzung
	  }
	  if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_RESET) {
		  // Fehler
	  }

	//Driver B
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == GPIO_PIN_RESET) {
			// Überhitzung
		}
		if (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_4) == GPIO_PIN_RESET) {
			// Fehler
		}
	  }
	*/
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */