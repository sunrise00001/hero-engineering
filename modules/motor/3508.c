//
// Created by jiang on 2026/10/31.
//

#include "3508.h"


#include "can.h"
#include "stm32f4xx_hal_can.h"
#include "pid.h"

motor_3508_t motor_3508;
Pid_t motor_3508_speed_pid;

void motor_3508_init(void)
{
    pid_init(&motor_3508_speed_pid,10,1,0,-50,50,-2000,2000);
    motor_3508.state = 1;
}

void motor_3508_pid(motor_3508_t *motor,float target_speed)
{
    motor->ctrl_iq = pid_calc(&motor_3508_speed_pid,target_speed,motor->speed);
}



void motor3508_senddata(uint16_t iq)
{
    uint8_t data[8];
    uint32_t mailbox;

    CAN_TxHeaderTypeDef tx_header;
    tx_header.StdId = 0x200;
    tx_header.ExtId = 0;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 8;
    tx_header.TransmitGlobalTime = DISABLE;

    data[0] = iq>>8;
    data[1] = iq;
    data[2] = iq>>8;
    data[3] = iq;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x00;
    data[7] = 0x00;

    HAL_CAN_AddTxMessage(&hcan1,&tx_header,data,&mailbox);
}

void motor_3508feedback(uint8_t *data)
{
        motor_3508.angle = data[0]<<8 | data[1];
        motor_3508.speed = (int16_t)data[2]<<8 | data[3];
        motor_3508.current = (int16_t)data[4]<<8 | data[5] /16384*20 ;
        motor_3508.temp = data[6];
}






