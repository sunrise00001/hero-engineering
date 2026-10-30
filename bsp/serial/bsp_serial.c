//
// Created by jiang on 2026/10/17.
//

#include "bsp_serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

Serial_Instance_t serial_instance[4];
static uint8_t idx =0;

//串口发送
static void Serial_sendstring(UART_HandleTypeDef *huart,char *string)
{
    HAL_UART_Transmit(huart, (uint8_t*)string, strlen(string), 100);
}

//初始化
void Serial_Init(serial_config_t *config)
{
    memset(serial_instance,0,sizeof(Serial_Instance_t));

    serial_instance[idx].mode = config->mode;
    serial_instance[idx].huart   = config->huart;
    serial_instance[idx].rx_DLC = config->rx_DLC;
    serial_instance[idx].callback = config->callback;
}

//接收函数
void Serial_Receive(UART_HandleTypeDef *huart)
{
    uint8_t index_id = 0;
    uint8_t findflag = 0;

    for (uint8_t i = 0; i < idx; i++)
    {
        if(serial_instance[i].mode == serial_receive || serial_instance[i].mode ==serial_send_receive)
        {
            index_id = i;
            findflag = 1;
            break;
        }
    }
    //未注册为接收模式或无接收长度, 不使用DMA接收
    if(findflag == 0 || serial_instance[index_id].rx_DLC == 0)
        return;
    HAL_UARTEx_ReceiveToIdle_DMA(huart,serial_instance[index_id].RXdata,serial_instance[index_id].rx_DLC);
    __HAL_DMA_DISABLE_IT(huart->hdmarx,DMA_IT_HT);

}

void Serial_getRxdata(UART_HandleTypeDef *huart,uint8_t *data)
{
    uint8_t index_id = 0;
    for(uint8_t i = 0; i < idx; i++)
    {
        if(huart == serial_instance[i].huart)
        {
            if(serial_instance[i].mode == serial_receive || serial_instance[i].mode == serial_send_receive)
            {
                index_id = i;
                break;
            }
        }
    }
    memcpy(data,serial_instance[index_id].RXdata,serial_instance[index_id].rx_DLC);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    for(uint8_t i = 0; i < idx; i++)
    {
        if(huart == serial_instance[i].huart)
        {
            if(serial_instance[i].mode == serial_receive || serial_instance[i].mode == serial_send_receive)
            {
                memcpy(serial_instance[i].rxbuffer, serial_instance[i].RXdata, serial_instance[i].rx_DLC);
                memset(serial_instance[i].RXdata,0,sizeof(serial_instance[i].RXdata));
                serial_instance[i].callback();      //调用第 i 个串口实例中保存的回调函数
            }
        }
    }
    Serial_Receive(huart);
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    for(uint8_t i = 0; i < idx; i++)
    {
        if(huart == serial_instance[i].huart)
        {
            if(serial_instance[i].mode == serial_receive || serial_instance[i].mode ==serial_send_receive)
            {
                memcpy(serial_instance[i].rxbuffer, serial_instance[i].RXdata, serial_instance[i].rx_DLC);
                memset(serial_instance[i].RXdata,0,sizeof(serial_instance[i].RXdata));
                serial_instance[i].callback();
            }
        }
    }
    Serial_Receive(huart);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    for(uint8_t i = 0; i < idx; i++)
    {
        if(huart == serial_instance[i].huart)
        {
            Serial_Receive(huart);
            return;
        }
    }
}
//开启空闲中断接收和DMA接收
void huart_start(UART_HandleTypeDef *huart,uint8_t *data)
{
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    HAL_UART_Receive_DMA(huart,data,256);
}

//串口打印
void Serial_Printf(UART_HandleTypeDef *huart,char *format, ...)
{
    char String[160];
    va_list arg;
    va_start(arg, format);
    vsnprintf(String, sizeof(String), format, arg);   //原 vsprintf 无界, 数值变大时会写越界 -> 串口出现乱码/粘行
    va_end(arg);

    Serial_sendstring(huart,String);
}













