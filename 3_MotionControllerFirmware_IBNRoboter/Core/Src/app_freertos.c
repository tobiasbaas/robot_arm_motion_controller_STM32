/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_freertos.h"

#include "CO_app_STM32.h"
#include "fdcan.h"
#include "tim.h"

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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for canopen */
osThreadId_t canopenHandle;
const osThreadAttr_t canopen_attributes = {
  .name = "canopen",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 128 * 4
};
/* Definitions for IBNusbTaskHandl */
osThreadId_t IBNusbTaskHandlHandle;
const osThreadAttr_t IBNusbTaskHandl_attributes = {
  .name = "IBNusbTaskHandl",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void canopen_task(void *argument);
void startIBNusbTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of canopen */
  canopenHandle = osThreadNew(canopen_task, NULL, &canopen_attributes);

  /* creation of IBNusbTaskHandl */
  IBNusbTaskHandlHandle = osThreadNew(startIBNusbTask, NULL, &IBNusbTaskHandl_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */

  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_Device */
  MX_USB_Device_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {

	  //test_monitor();
	  osDelay(1);

  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_canopen_task */
/**
* @brief Function implementing the canopen thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_canopen_task */
void canopen_task(void *argument)
{
  /* USER CODE BEGIN canopen_task */
	CANopenNodeSTM32 canopenNodeSTM32;
	canopenNodeSTM32.CANHandle = &hfdcan1;
	canopenNodeSTM32.HWInitFunction = MX_FDCAN1_Init;
	canopenNodeSTM32.timerHandle = &htim17;
	canopenNodeSTM32.desiredNodeID = 4;
	canopenNodeSTM32.baudrate = 125;
	canopen_app_init(&canopenNodeSTM32);
	/* Infinite loop */
	for(;;)
	{
		//Reflect CANopenStatus on LEDs
	  //HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, !canOpenNodeSTM32.outStatusLEDGreen);
	  //HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, !canOpenNodeSTM32.outStatusLEDRed);
	  HAL_GPIO_TogglePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin);
	  osDelay(300);
	  canopen_app_process();
	  // Sleep for 1ms, you can decrease it if required, in the canopen_app_process we will double check to make sure 1ms passed
	  vTaskDelay(pdMS_TO_TICKS(1));
	}
  /* USER CODE END canopen_task */
}

/* USER CODE BEGIN Header_startIBNusbTask */
/**
* @brief Function implementing the IBNusbTaskHandl thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startIBNusbTask */
void startIBNusbTask(void *argument)
{
  /* USER CODE BEGIN startIBNusbTask */
  /* Infinite loop */
  for(;;)
  {
	  run_monitor();
    osDelay(100);
  }
  /* USER CODE END startIBNusbTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

