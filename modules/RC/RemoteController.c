//
// Created by jiang on 2026/10/10.
//

#include "RemoteController.h"

#include <string.h>


#include "main.h"
#include "stm32f4xx_hal_uart.h"
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;

//接收原始数据，为18个字节，给了36个字节长度，防止DMA传输越界
uint8_t sbus_rx_buf[2][SBUS_RX_BUF_NUM];
RC_ctrl_t RC_ctrldata;


void Remote_Controler_init(void)
{
    RC_Init(sbus_rx_buf[0],sbus_rx_buf[1], SBUS_RX_BUF_NUM);
}



void RemoteData_extract(volatile const uint8_t *sbus_buf,RC_ctrl_t* data)
{
    if (sbus_buf == NULL || data == NULL)
    {
        return;
    }

    data->ch0 = (sbus_buf[0] | (sbus_buf[1] << 8)) & 0x07ff;        //!< Channel 0
    data->ch1 = ((sbus_buf[1] >> 3) | (sbus_buf[2] << 5)) & 0x07ff; //!< Channel 1
    data->ch2 = ((sbus_buf[2] >> 6) | (sbus_buf[3] << 2) |          //!< Channel 2
                  (sbus_buf[4] << 10)) &0x07ff;
    data->ch3 = ((sbus_buf[4] >> 1) | (sbus_buf[5] << 7)) & 0x07ff; //!< Channel 3
    data->s[0] = ((sbus_buf[5] >> 4) & 0x0003);                  //!< Switch left
    data->s[1]= ((sbus_buf[5] >> 4) & 0x000C) >> 2;                       //!< Switch right
    data->mouse_x = sbus_buf[6] | (sbus_buf[7] << 8);                    //!< Mouse X axis
    data->mouse_y = sbus_buf[8] | (sbus_buf[9] << 8);                    //!< Mouse Y axis
    data->mouse_z = sbus_buf[10] | (sbus_buf[11] << 8);                  //!< Mouse Z axis
    data->mouse_left = sbus_buf[12];                                   //鼠标 左
    data->mouse_right = sbus_buf[13];                                  //鼠标 右
    data->key = sbus_buf[14] | (sbus_buf[15] << 8);                    //按键
    data->ch4 = sbus_buf[16] | (sbus_buf[17] << 8);                 //波轮

    data->ch0 -= RC_CH_VALUE_OFFSET;
    data->ch1 -= RC_CH_VALUE_OFFSET;
    data->ch2 -= RC_CH_VALUE_OFFSET;
    data->ch3 -= RC_CH_VALUE_OFFSET;
    data->ch4 -= RC_CH_VALUE_OFFSET;

};




void RC_Init(uint8_t *rx1_buf, uint8_t *rx2_buf, uint16_t dma_buf_num)
{

    //enable the DMA transfer for the receiver request
    //使能DMA串口接收
    SET_BIT(huart3.Instance->CR3, USART_CR3_DMAR);

    //enalbe idle interrupt
    //使能空闲中断
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

    //disable DMA
    //失效DMA
    __HAL_DMA_DISABLE(&hdma_usart3_rx);
    while(hdma_usart3_rx.Instance->CR & DMA_SxCR_EN)
    {
        __HAL_DMA_DISABLE(&hdma_usart3_rx);
    }

    hdma_usart3_rx.Instance->PAR = (uint32_t) & (USART3->DR);
    //memory buffer 1
    //内存缓冲区1
    hdma_usart3_rx.Instance->M0AR = (uint32_t)(rx1_buf);
    //memory buffer 2
    //内存缓冲区2
    hdma_usart3_rx.Instance->M1AR = (uint32_t)(rx2_buf);
    //data length
    //数据长度
    hdma_usart3_rx.Instance->NDTR = dma_buf_num;
    //enable double memory buffer
    //使能双缓冲区
    SET_BIT(hdma_usart3_rx.Instance->CR, DMA_SxCR_DBM);

    //enable DMA
    //使能DMA
    __HAL_DMA_ENABLE(&hdma_usart3_rx);


}
void RC_unable(void)
{
    __HAL_UART_DISABLE(&huart3);
}
void RC_restart(uint16_t dma_buf_num)
{
    __HAL_UART_DISABLE(&huart3);
    __HAL_DMA_DISABLE(&hdma_usart3_rx);

    hdma_usart3_rx.Instance->NDTR = dma_buf_num;

    __HAL_DMA_ENABLE(&hdma_usart3_rx);
    __HAL_UART_ENABLE(&huart3);

}








