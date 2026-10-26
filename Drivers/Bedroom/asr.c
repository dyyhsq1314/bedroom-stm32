#include "freertos.h"
#include "asr.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"
#include "gpio.h"
#include "led.h"
#include "jdy.h"
#include "main.h"
#include <string.h>
extern uint8_t rxLine[64];

void asr_exec(const char *cmd)
{
    if(strstr(cmd,"LED_ON"))
    {
		
		HAL_GPIO_WritePin(LED_Bule_GPIO_Port, LED_Bule_Pin, GPIO_PIN_SET);
        JDY_SendString("LED_ON\r\n");
    }
    else if(strstr(cmd,"LED_OFF"))
    {
        HAL_GPIO_WritePin(LED_Bule_GPIO_Port, LED_Bule_Pin, GPIO_PIN_RESET);
        JDY_SendString("LED_OFF\r\n");
    }
    else if(strstr(cmd,"READ_TEMP"))
    {
        extern SensorData_t current_data;   
        char msg[32];
        sprintf(msg,"Temp=%d.%d and Humi=%d.%d\r\n",current_data.temp_int,current_data.temp_decimal,current_data.humi_int,current_data.humi_decimal);
		
        HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),10);
    }
   
}



