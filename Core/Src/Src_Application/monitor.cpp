/*
 * monitor.c
 *
 *  Created on: Mar 3, 2025
 *      Author: manue
 */

#include "main.h"
#include "monitor.h"
#include "usbd_cdc_if.h"  // Enthält CDC_Transmit_FS()
#include "usb_device.h"
#include "commando_interpreter.h"  // Falls `com_inter` benutzt wird

extern "C" ComInt_class com_inter;


uint8_t rx_buff[RX_BUFFER_LENGTH];
uint8_t tx_buff[TX_BUFFER_LENGTH]={0}; //ABCDEFGHIJ in ASCII code
//char cmd_str[CMD_STR_LENGTH] = {0};
volatile uint16_t rx_cmd_str_cnt = 0;
//bool new_cmd = 0;
volatile bool TX_DMA_buff_free = 1;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// Zeilenende bei LF (0x0A) und CR (0x0D) erkennen
	if((rx_buff[0] == 0x0A) || (rx_buff[0] == 0x0D))
	{
		// Leerzeile (nur CR/LF) nicht als Befehl verarbeiten
		if(rx_cmd_str_cnt > 0)
		{
			cmd_str[rx_cmd_str_cnt]=0;
			rx_cmd_str_cnt = 0;
			new_cmd = true;
		}
		else
		{
			rx_cmd_str_cnt = 0;
		}
	}
	else
	{
		// Nur druckbare Zeichen in den Puffer uebernehmen
		if((rx_buff[0] >= 0x20) && (rx_buff[0] < 0x7F))
		{
			// Puffer-Overflow verhindern (Platz fuer Terminator lassen)
			if(rx_cmd_str_cnt < (CMD_STR_LENGTH - 1))
			{
				cmd_str[rx_cmd_str_cnt] = rx_buff[0];
				rx_cmd_str_cnt++;
			}
			// else: Puffer voll, restliche Zeichen der Zeile verwerfen
		}
	}

	#ifndef SEND_OVER_USB
		HAL_UART_Receive_DMA(&hlpuart1, rx_buff, 1); //You need to toggle a breakpoint on this line!
	#endif
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	TX_DMA_buff_free = true;
}

void send_string(const char* str, bool new_line)
{
	#ifdef SEND_OVER_USB
		// send_string_USB(): 
    	// use global buffer from monitor.h; else uint8_t tx_buff[TX_BUFFER_LENGTH] = {0};
		uint16_t len = 0;

		for (uint16_t n = 0; n < TX_BUFFER_LENGTH - 2; n++)
		{
			if (str[n] != '\0')
			{
				tx_buff[n] = str[n];
			}
			else
			{
				if (new_line)
				{
					tx_buff[n++] = '\n';  // Zeilenumbruch hinzufügen
				}
				len = n;
				break;
			}
		}

		if (len > 0)
		{
			CDC_Transmit_FS(tx_buff, len);
		}

	#else
		// send_string_DMA():
		/**
		* @brief string send command (DMA) with CR and LF
		* new_line true = true --> CR and LF is added
		*/
    	uint16_t len = TX_BUFFER_LENGTH;
		while(TX_DMA_buff_free == false){}	// wait for last transmission is ended
		for(uint16_t n= 0;n<TX_BUFFER_LENGTH-2;n++ )
		{
			if(str[n]  != 0)
			{
				tx_buff[n] = str[n];
			}
			else
			{
				if(new_line == true)
				{
					//tx_buff[n++] = '\r';	//add a line feed
					tx_buff[n++] = '\n';	//add a line feed
				}
				len = n;
				n = TX_BUFFER_LENGTH;
			}
		}
		if(len >0)
		{
			TX_DMA_buff_free = false; // new transmission started
			HAL_UART_Transmit_DMA(&hlpuart1, tx_buff,len);
		}
	#endif
}

void send_raw_array_DMA(const uint8_t *char_array_ptr, uint16_t len)
{
	while(TX_DMA_buff_free == false){}	// wait for last transmission is ended
	TX_DMA_buff_free = false; // new transmission started
	HAL_UART_Transmit_DMA(&hlpuart1, char_array_ptr,len);
}

// TODO send raw array USB

void test_monitor()
{
	send_string("Hello PDOG!", 1);  // Dummy-Nachricht
}

void run_monitor(){

	//parse serial buffer
	if(new_cmd == 1)
	{
	  // execute CMD
	  com_inter.interpreter(&cmd_str[0]);

	  new_cmd = 0;
	}
}
