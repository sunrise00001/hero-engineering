//
// Created by jiang on 2026/10/31.
//

#include "3508.h"

#include <math.h>


#include "can.h"
#include "stm32f4xx_hal_can.h"
#include "pid.h"

motor_3508_t motor_3508;
Pid_t motor_3508_speed_pid;

void motor_3508_init(void)
{
    pid_init(&motor_3508_speed_pid,25,2,0,-5000,5000);
    motor_3508.state = 1;
}

void motor_3508_pid(motor_3508_t *motor,int16_t target_speed)
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
    data[4] = iq>>8;
    data[5] = iq;
    data[6] = 0x00;
    data[7] = 0x00;

    HAL_CAN_AddTxMessage(&hcan1,&tx_header,data,&mailbox);
}

void motor_3508feedback(uint8_t *data)
{
        motor_3508.angle = (uint16_t)data[0]<<8 | data[1];
        motor_3508.speed = (int16_t)(data[2]<<8) | data[3];
        motor_3508.current = (int16_t)(data[4]<<8) | data[5];
        motor_3508.temp = data[6];
        // motor_3508.angle = motor_3508.angle/8191*360;
}



// Pid_t first_speed;
// Pid_t gripper_speed;
// Pid_t gripper_angle;
// void graipper_ctrl_init(void)
// {
//     pid_init(&first_speed,25,2,0,-5000,5000);
//     pid_init(&gripper_speed,0,0,0,-5000,5000);
//     pid_init(&gripper_angle,0,0,0,-5000,5000);
//
//     motor_3508.state = 1;
// }
// uint8_t zero_flag = 0;
// void gripper_ctrl(const int16_t speed)
// {
//     int16_t step_angle;
//     int16_t last_angle;
//
//     int16_t speed_out;
//     uint8_t counter = 0;
//
//
//     motor_3508.ctrl_iq = pid_calc(&first_speed,-90,motor_3508.speed);
//     motor3508_senddata(motor_3508.ctrl_iq);
//
//
//
//
//     if (fabsf(motor_3508.current)>=1500&&fabsf(motor_3508.speed)<20)
//     {
//         motor_3508.state = 0;                    //转到最里面，停
//     }
//
//     if (zero_flag == 0)
//     {
//         step_angle = motor_3508.angle;          //读取当前编码器角度，只进入一次
//         zero_flag = 1;
//         motor_3508.state =1;
//     }
//
//     motor_3508.angle = motor_3508.angle - step_angle;    //角度校零
//
//     //计圈判断
//     last_angle = motor_3508.angle;
//     if (last_angle - motor_3508.angle > 180)
//     {
//         counter++;
//     }
//     if (last_angle - motor_3508.angle < -180)
//     {
//         counter--;
//     }
//
//     motor_3508.angle = motor_3508.angle +counter*360;
//
//     speed_out = pid_calc(&gripper_angle,0,motor_3508.angle);
//     motor_3508.ctrl_iq = pid_calc(&gripper_speed,speed_out,motor_3508.speed);
//
// }


