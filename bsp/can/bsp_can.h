//
// Created by jiang on 2026/10/7.
//

#ifndef ARM_BSP_CAN_H
#define ARM_BSP_CAN_H
#include "main.h"
typedef CAN_HandleTypeDef hcan_t;





void bsp_can_init(void);
void can_filter_init(void);
uint8_t canx_send_data(CAN_HandleTypeDef *hcan, uint16_t id, uint8_t *data, uint32_t len);
uint8_t canx_receive(hcan_t *hcan, uint16_t *rec_id, uint8_t *buf);
void HAL_CAN_RxFifo0MsgPendingCallback(hcan_t *hcan);
void can1_rx_callback(void);
void can2_rx_callback(void);
#endif //ARM_BSP_CAN_H
