#ifndef __JDY_H
#define __JDY_H

#include "stm32f1xx_hal.h"   
#include <string.h>


extern UART_HandleTypeDef huart1;


void JDY_SendString(const char *str);

void JDY_SendSensorData(uint8_t temp_int, uint8_t temp_dec,
                        uint8_t humi_int, uint8_t humi_dec);

#endif
