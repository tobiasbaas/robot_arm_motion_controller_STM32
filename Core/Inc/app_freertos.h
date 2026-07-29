/*
 * app_freertos.h
 *
 *  Created on: Mar 3, 2025
 *      Author: manue
 */

#ifndef INC_APP_FREERTOS_H_
#define INC_APP_FREERTOS_H_

#ifdef __cplusplus

extern "C" {
#endif

void run_monitor();
void test_monitor();
void MX_FREERTOS_Init();
void MX_USB_Device_Init();

#ifdef __cplusplus
}
#endif



#endif /* INC_APP_FREERTOS_H_ */
