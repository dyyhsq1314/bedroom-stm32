#ifndef DHT11_DHT11_H
#define DHT11_DHT11_H
#include "stm32f1xx.h"


void DHT11_OUT(void );
void DHT11_IN(void );
void DHT11_Reset(void );
uint8_t DHT11_Check(void );
uint8_t DHT11_Read_Bit(void );
uint8_t DHT11_Read_Byte(void );
uint8_t DHT11_Read_Data(uint8_t* temp,uint8_t* temp_,uint8_t* humi,uint8_t* humi_);
void OLED_ShowTempHumidity(uint8_t temp, uint8_t temp_, uint8_t humi, uint8_t humi_);
void DHT11_Read_And_Display(void);
#endif
