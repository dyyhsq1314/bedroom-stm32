#include "DTH11.h"
#include "tim.h"
#include <stdio.h>
#include "OLED.h"
#include "freertos.h"
#include "task.h"
/*
 *TIM3定时器实现us级延时
 */


//输出
void DHT11_OUT(void ){
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*Configure GPIO pin : PB12 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
//输入
void DHT11_IN(void ){
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*Configure GPIO pin : PB12 */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
//复位DHT11
void DHT11_Reset(void ){
    DHT11_OUT();                                          //设置为输出
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);     //拉低引脚
    vTaskDelay(20);                                             //延迟20ms
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);       //拉高引脚
    Delay_us(30);
}
//等待DHT11回应
uint8_t DHT11_Check(void)
{
    uint8_t retry=0;
    DHT11_IN();                                           //设置为输入
    while(GPIO_PIN_SET==HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) && retry<100)
    {
        retry++;
        Delay_us(1);
    }
    if(retry>=100)
        return 1;
    else
        retry=0;

    while(GPIO_PIN_RESET==HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) && retry<100)
    {
        retry++;
        Delay_us(1);
    }
    if(retry>=100)
        return 1;
    return 0;
}
//从DHT11读取一个位
uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry=0;
    while(GPIO_PIN_SET==HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) && retry<100)
    {
        retry++;
        Delay_us(1);
    }
    retry=0;

    while(GPIO_PIN_RESET==HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) && retry<100)
    {
        retry++;
        Delay_us(1);
    }
    Delay_us(40);

    if(GPIO_PIN_SET==HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0))
        return 1;
    else
        return 0;
}

//从DHT11读取一个字节
uint8_t DHT11_Read_Byte(void)
{
    uint8_t dat=0;
    for(uint8_t i=0;i<8;i++)
    {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}
//从DHT11读取一次数据
uint8_t DHT11_Read_Data(uint8_t* temp,uint8_t* temp_,uint8_t* humi,uint8_t* humi_)
{
  uint8_t buf[5];
  uint8_t rst;
  DHT11_Reset();
    if(DHT11_Check() == 0)
    {
        for(uint8_t i=0;i<5;i++)
            buf[i]=DHT11_Read_Byte();
        if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
        {
          *humi=buf[0];
					*humi_=buf[1];
					*temp=buf[2]; 
					*temp_=buf[3]; 
					rst=8;
        }else{
					*humi=0;
					*humi_=0;
					*temp=0; 
					*temp_=0; 
					rst=0;
				}
    }else{
			
			rst=1;
		}
		//printf("%s temp:%d.%d humi:%d.%d\n",rst==8?"success":"waiting a minute :",buf[2],buf[3],buf[0],buf[1]);
    return rst;
}

/**
  * @brief  在OLED上显示温湿度数据
  * @param  temp 温度整数部分
  * @param  temp_ 温度小数部分
  * @param  humi 湿度整数部分
  * @param  humi_ 湿度小数部分
  * @retval 无
  */
void OLED_ShowTempHumidity(uint8_t temp, uint8_t temp_, uint8_t humi, uint8_t humi_)
{
    // 清屏
    //OLED_Clear();
    
    // 显示标题
    OLED_ShowString(1, 1, "Temp:");
    OLED_ShowString(3, 1, "Humi:");
    
    // 显示温度值（格式：XX.X°C）
    OLED_ShowNum(1, 6, temp, 2);          // 显示整数部分
    OLED_ShowChar(1, 8, '.');             // 显示小数点
    OLED_ShowNum(1, 9, temp_, 1);         // 显示小数部分
    OLED_ShowChar(1, 10, 0xB0);           // 显示度符号（°）
    OLED_ShowChar(1, 11, 'C');            // 显示C
    
    // 显示湿度值（格式：XX.X%）
    OLED_ShowNum(3, 6, humi, 2);          // 显示整数部分
    OLED_ShowChar(3, 8, '.');             // 显示小数点
    OLED_ShowNum(3, 9, humi_, 1);         // 显示小数部分
    OLED_ShowChar(3, 10, '%');            // 显示百分比符号
    
    // 可选：添加单位说明或装饰性内容
    OLED_ShowString(2, 1, "-----------");
    OLED_ShowString(4, 1, "-----------");
}


void DHT11_Read_And_Display(void)
{
    uint8_t temp = 0, temp_ = 0, humi = 0, humi_ = 0;
    uint8_t result = 0;
    
    // 读取DHT11数据
    result = DHT11_Read_Data(&temp, &temp_, &humi, &humi_);
    
    if(result == 8) // 读取成功
    {
        // 在OLED上显示温湿度
        OLED_ShowTempHumidity(temp, temp_, humi, humi_);
        
        // 可选：在串口输出调试信息
       
    }
    else // 读取失败
    {
       
        OLED_ShowString(1, 1, "DHT11 Error!");
        OLED_ShowString(2, 1, "Code:");
        OLED_ShowNum(2, 7, result, 1);
        
       
    }
}
