#include "jdy.h"
#include <stdio.h>
void JDY_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 25);
}


void JDY_SendSensorData(uint8_t temp_int, uint8_t temp_dec,
                        uint8_t humi_int, uint8_t humi_dec)
{
    char buf[64];
   snprintf(buf, sizeof(buf), "Temp:%d.%d  and  Humi:%d.%d %%\r\n",
        temp_int, temp_dec, humi_int, humi_dec);
   JDY_SendString(buf);
}

