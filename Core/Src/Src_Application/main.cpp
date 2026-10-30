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
#include "stm32g4xx_hal.h"
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

volatile uint8_t axis_to_run = 0; // volatile bc the variable is used in two different interrupts
uint64_t servo_cnt = 0;

int32_t joint1_old = 0;
int32_t joint2_old = 0;

// TODO conVal needed??? -> delete??
//volatile float convVal1 = 0; 
//volatile float convVal2 = 0;
//volatile float convVal3 = 0;

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
void adc_selftest();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

axis_class 				myAxis[MAX_NUM_AXIS];
set_pwm_class			myPWM[MAX_NUM_AXIS];

error_class 			myError;
parameter_class 	myParameter;
system_class 		  mySystem(&myParameter);

ComInt_class 			com_inter(&myError);

// Classes belonging to axis
trajectory_class 		  trajectory[MAX_NUM_AXIS];
pos_sensor_class		  pos_sensor[MAX_NUM_AXIS];
PID_class				      pid_pos[MAX_NUM_AXIS];
current_control_class current_control[MAX_NUM_AXIS];

// Data recorder
data_recorder_class 		myDR(&myParameter, &myError);

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
  //MX_I2C2_Init();
  MX_TIM6_Init();
  MX_LPUART1_UART_Init();

  /* USER CODE BEGIN 2 */

  #ifndef SEND_OVER_USB
    HAL_UART_Receive_DMA(&hlpuart1, rx_buff, 1); // Start DMA receive
  #endif

  // Stoppuhr; nur für Debug -> löschen
  //CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;   // Debug-Block einschalten
  //DWT->CYCCNT = 0;                                  // Zähler auf 0
  //DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;             // Zähler starten


	myParameter.init_param(&myError);

	myDR_ptr = &myDR;	// TODO -> delete?

  // Init commando interpreter
	com_inter.myDR = &myDR;
	com_inter.myParameter	= &myParameter;
  com_inter.mySystem = &mySystem;

   // Initialization of axis modules
   for(uint8_t i=0;i<MAX_NUM_AXIS;i++)
   {
 	  pid_pos[i].init(
      &myAxis[i],
      &myParameter,
      &myError
    );

 	  trajectory[i].init(
      &myAxis[i],
      &myParameter,
      &myError
    );

    pos_sensor[i].init(
      &myAxis[i],
      &myParameter,
      &myError,
      (i==0)?&hspi1:&hspi2,
      (i==0)?GPIOG:GPIOD,
      (i==0)?GPIO_PIN_5:GPIO_PIN_15
    );

 	  myAxis[i].init(
      i,
      &myError,
      &trajectory[i],
      &pos_sensor[i],
      &pid_pos[i],
      &current_control[i],
      &myDR,
      &myParameter
    );

    // Re-initialize current control with the known Axis
    current_control[i].init(
      &myAxis[i],
      &myParameter,
      &myError,
      &myPWM[i],
      &trajectory[i]
    );

    myAxis_ptr[i] 			= &myAxis[i];
    com_inter.myAxis[i] = myAxis_ptr[i];
    myDR.myAxis[i] 			= myAxis_ptr[i];

    myPID_pos_ptr[i]     = &pid_pos[i];
    myCurrent_Control[i] = &current_control[i];
  }

  mySystem.init();
  myDR.init();

  // enable Axis 
  myAxis[0].enabled = true;
  myPID_pos_ptr[0]->servo_on_off(true);

  #if AXIS1_ENABLED
  myAxis[1].enabled = true;
  myPID_pos_ptr[1]->servo_on_off(true);
  #endif

  
  
  // 1. Slave (TIM1, Achse A) vorbereiten: Kanäle freigeben, Zähler läuft noch nicht
  myCurrent_Control[0]->myPWM->startPWM();

  // 2. Zähler um die halbe Periode vorstellen
  __HAL_TIM_SET_COUNTER(&htim1, __HAL_TIM_GET_AUTORELOAD(&htim1) - 1);

  // 3. Master (TIM20, Achse B) zuletzt starten, das startet TIM1 gleich mit
  myCurrent_Control[1]->myPWM->startPWM();




  // Start ADC
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_InjectedStart(&hadc1);

  HAL_ADCEx_Calibration_Start(&hadc4, ADC_SINGLE_ENDED);   
  HAL_ADCEx_InjectedStart(&hadc4);     

  // ADC Offset ermitteln
  for(uint8_t i=0;i<MAX_NUM_AXIS;i++)
  {
    myAxis_ptr[i]->myCurrent_Control->setAllCurrentsToZero(); // PWM auf LOW
    HAL_Delay(20);
    if(i == 0)
    {
      myAxis_ptr[i]->myCurrent_Control->calibrateADC_Offsets(&hadc1);
    }
    else
    {
      myAxis_ptr[i]->myCurrent_Control->calibrateADC_Offsets(&hadc4);
    }
  } 

  // start encoder
  for(uint8_t i=0;i<MAX_NUM_AXIS;i++)
  {
    myAxis_ptr[i]->myCurrent_Control->setElectricAngleToZero(); // PWM auf LOW
    HAL_Delay(500); // wait for motor to move
    if (!myAxis_ptr[i]->myPos->mt6835_init())  // get start value and offset electrical angle
    {
      myError.add(ENCODER_MOD, ERR_ENC_INIT_FAILED, __FILE__, __LINE__);
    }
  }

  // for Encoder Test
  myAxis_ptr[0]->myCurrent_Control->setAllCurrentsToZero();
  myAxis_ptr[1]->myCurrent_Control->setAllCurrentsToZero();




  #ifdef USE_FOC
    // Start current control loop (runs in ADC ISR)
    HAL_ADCEx_InjectedStop(&hadc1); // axis 1/A
    HAL_ADCEx_InjectedStart_IT(&hadc1); // axis 2/B

    HAL_ADCEx_InjectedStop(&hadc4); // axis 1/A
    HAL_ADCEx_InjectedStart_IT(&hadc4); // axis 2/B
  #endif
  
  HAL_TIM_Base_Start_IT(&htim3); // Start Servo-Loop
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
    /*
    	Sage mir, Muse, die Taten des vielgewanderten Mannes,
      Welcher so weit geirrt, nach der heiligen Troja Zerstörung,
      Vieler Menschen Städte gesehn, und Sitte gelernt hat,
      Und auf dem Meere so viel' unnennbare Leiden erduldet ...
    */
	}

  /* USER CODE END WHILE */

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
   */
}
  
/* USER CODE BEGIN 3 */
/* USER CODE END 3 */

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
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  #ifdef USE_FOC
    if (hadc->Instance == ADC1)
    {
      //GPIOA->ODR ^= GPIO_ODR_OD4; // Toggle GPIO PA4 for debug reasons
      GPIOA->BSRR = GPIO_BSRR_BS4;  // set GPIO PA4 for debug reasons
      //myAxis_ptr[0]->myCurrent_Control->testRead();
      myAxis_ptr[0]->myCurrent_Control->currentControlLoop();
      GPIOA->BSRR = GPIO_BSRR_BR4;  // reset GPIO PA4

    }     
    else if (hadc->Instance == ADC4)
    {
      //GPIOA->ODR ^= GPIO_ODR_OD9; // Toggle GPIO PA9 for debug reasons
      //GPIOA->BSRR = GPIO_BSRR_BS9;  // set GPIO PA9 for debug reasons
      myAxis_ptr[1]->myCurrent_Control->currentControlLoop();
      //GPIOA->BSRR = GPIO_BSRR_BR9;  // set GPIO PA9 for debug reasons
    }
    
  #endif
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  // TODO used? -> remove?
	if (htim->Instance == TIM7) 
  {
	  HAL_IncTick();
	}

	// Handle CANOpen app interrupts
	if (htim == canopenNodeSTM32->timerHandle) 
  {
		canopen_app_interrupt();
	}

  
  //--------------------------------------------------------------
  //                    Servo-Loop
  //--------------------------------------------------------------
	if (htim->Instance == TIM3) 
  {
    if(axis_to_run == 0) myAxis_ptr[axis_to_run]->myPos->mt6835_start_read(); // Initiates the request for the next encoder value via SPI and DMA

    servo_cnt++;  // counter (Servo Loop)
		


		// run axis if pointer is valid
		if (axis_to_run < MAX_NUM_AXIS && myAxis_ptr[axis_to_run]) 
    {
			// CANopen Daten
			if(axis_to_run == 0 && (joint1_old != OD_PERSIST_COMM.x6001_joint1))
			{
				myAxis_ptr[axis_to_run]->myTrajectory->setpoint = OD_PERSIST_COMM.x6001_joint1 * JOINT0_SP_SCALE; // Skalierung je Board (typedefs.h)
				joint1_old = OD_PERSIST_COMM.x6001_joint1;
			} 
      else if(axis_to_run == 1 && (joint2_old != OD_PERSIST_COMM.x6002_joint2))
			{
				myAxis_ptr[axis_to_run]->myTrajectory->setpoint = OD_PERSIST_COMM.x6002_joint2 * JOINT1_SP_SCALE; // Skalierung je Platine (typedefs.h)
				joint2_old = OD_PERSIST_COMM.x6002_joint2;
			}

      // run axis
      if(axis_to_run == 0)
      {
        myAxis_ptr[axis_to_run]->run();
      }
      else 
      {
        #ifdef AXIS1_ENABLED
        myAxis_ptr[axis_to_run]->run();
        #endif
      }
		}

    axis_to_run = (axis_to_run + 1) % MAX_NUM_AXIS; // jump between axes

		// run data recorder if pointer is valid
		if (myDR_ptr) {
			myDR_ptr->run();
		}
	}
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    for (uint8_t i = 0; i < MAX_NUM_AXIS; i++)
    {
        if (pos_sensor[i].mt6835_spi == hspi)
        {
            pos_sensor[i].mt6835_spi_txrx_cplt_callback(hspi);
            break;
        }
    }
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