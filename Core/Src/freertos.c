/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "DTH11.h"
#include "OLED.h"
#include <stdio.h>
#include "led.h"
#include "gpio.h"
#include "usart.h"
#include <string.h>
#include "jdy.h"
#include "usart.h"
#include "asr.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern UART_HandleTypeDef huart2;   // 来自 usart2.c

// 任务句柄
TaskHandle_t xDisplayHandle = NULL;
TaskHandle_t xSensorHandle = NULL;
TaskHandle_t xInfraredHandle = NULL;
TaskHandle_t xBluetoothHandle = NULL;
TaskHandle_t xASRTaskHandle = NULL;
// 互斥锁保护OLED
SemaphoreHandle_t xOLEDMutex;

// 显示缓冲区
static char display_buffer[20];


SensorData_t current_data = {0};


/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void vDisplayTask(void *pvParameters);
void vSensorTask(void *pvParameters);
void vInfraredTask(void *pvParameters);
void vBluetoothTask(void *pvParameters);
void asr_task(void *pvParameters);
/* USER CODE END FunctionPrototypes */

void MX_FREERTOS_Init(void);
/**
  * @brief FreeRTOS初始化
  */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN RTOS_MUTEX */
  // 创建互斥锁保护OLED
  xOLEDMutex = xSemaphoreCreateMutex();
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_THREADS */
  // 创建显示任务
  xTaskCreate(vDisplayTask, "Display", 256, NULL, 4, &xDisplayHandle);

  // 创建传感器任务
  xTaskCreate(vSensorTask, "Sensor", 256, NULL, 5, &xSensorHandle);

  // 创建红外检测任务
 
  xTaskCreate(vInfraredTask, "Infrared", 256, NULL, 4, &xInfraredHandle);
	
  //创建蓝牙任务
  xTaskCreate(vBluetoothTask,"Bluetooth",256,NULL, 4 ,&xBluetoothHandle);
	
	
  //创建天问语音任务
  xTaskCreate(asr_task, "ASR", 300, NULL, 4, &xASRTaskHandle);


  /* USER CODE END RTOS_THREADS */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
  * @brief 安全的OLED显示函数（使用互斥锁）
  */
void OLED_DisplaySafe(SensorData_t *data)
{
    // 获取OLED使用权（阻塞等待）
    if(xSemaphoreTake(xOLEDMutex, portMAX_DELAY) == pdTRUE)
    {
       
        
        // 显示温度（第1行）
        OLED_ShowString(1, 1, "Temp:");
        sprintf(display_buffer, "%d.%dC  ", data->temp_int, data->temp_decimal);
        OLED_ShowString(1, 6, display_buffer);
       
        // 显示湿度（第2行）
        OLED_ShowString(2, 1, "Humi:");
        sprintf(display_buffer, "%d.%d%%      ", data->humi_int, data->humi_decimal);
        OLED_ShowString(2, 6, display_buffer);
        xSemaphoreGive(xOLEDMutex);
    }
}

/**
  * @brief 显示任务
  */
void vDisplayTask(void *pvParameters)
{
    OLED_Init();
    OLED_Clear();
    
    // 显示启动界面
    if(xSemaphoreTake(xOLEDMutex, portMAX_DELAY) == pdTRUE)
    {
        OLED_ShowString(1, 1, "System Start");
        OLED_ShowString(2, 1, "Please wait...");
        xSemaphoreGive(xOLEDMutex);
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    for(;;)
    {
        // 安全显示数据
        OLED_DisplaySafe(&current_data);
        
        vTaskDelay(pdMS_TO_TICKS(1000));  // 500ms刷新一次
    }
}

/**
  * @brief 传感器任务
  */
void vSensorTask(void *pvParameters)
{
    uint8_t result;
    for(;;)
    {
		
        // 读取温湿度传感器（使用uint8_t参数）
        result = DHT11_Read_Data(&current_data.temp_int, 
                                &current_data.temp_decimal,
                                &current_data.humi_int,
                                &current_data.humi_decimal);
        
        // 如果读取失败，设置错误值
        if(result != 8)
        {
            current_data.temp_int = 0;
            current_data.temp_decimal = 0;
            current_data.humi_int = 0;
            current_data.humi_decimal = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));  // 2秒采集一次
    }
}

/**
  * @brief 红外检测任务
  */
void vInfraredTask(void *pvParameters)
{
    

    for (;;) {
		
        /* 读取探头电平 */
        uint8_t status = HAL_GPIO_ReadPin(infrared_GPIO_Port, infrared_Pin);
		
        if (status == GPIO_PIN_SET) {   // 
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); // LED亮
        } else {                          
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);   // LED灭
        }

        vTaskDelay(pdMS_TO_TICKS(50));    
    }
}



void vBluetoothTask(void *pvParameters)
{
    /* 上电先发一次提示 */
	
    JDY_SendString("Bluetooth online\r\n");
	 
    for (;;)
    {     
        /* 读全局传感器数据 */
		
        uint8_t t_i = current_data.temp_int;
        uint8_t t_d = current_data.temp_decimal;
        uint8_t h_i = current_data.humi_int;
        uint8_t h_d = current_data.humi_decimal;

        /* 发到手机 */
       JDY_SendSensorData(t_i, t_d, h_i, h_d);
		
        vTaskDelay(pdMS_TO_TICKS(1500));   // 1 s 刷新
    }
}

void asr_task(void *pvParameters)
{
    HAL_UARTEx_ReceiveToIdle_IT(&huart2, rxBuf, sizeof(rxBuf));
    vTaskDelay(pdMS_TO_TICKS(600));
    for (;;){ 
		if (rxReady){               // 轮询标志
            rxReady = 0;            // 清零标志
            asr_exec((char *)rxLine);
        }
    }
}
/* USER CODE END Application */
