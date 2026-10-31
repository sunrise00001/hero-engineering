//
// Created by jiang on 2026/10/6.
//

#include "dm.h"
#include <string.h>


#include "arm_ctrl.h"
#include "can.h"
#include "cmsis_os2.h"
#include "referee.h"
#include "tool.h"


void dm_motor_init(dm_motor_t *motor,uint16_t id,uint8_t mode,float kp_set,float kd_set,float pos_set,float vel_set,float tor_set,float PMAX,float VMAX,float TMAX,float p_max,float p_min)
{
    memset(motor, 0, sizeof(*motor));
    motor->ctrl.id = id;
    motor->ctrl.mst_id = 0x00;
    motor->ctrl.mode = mode;
    motor->ctrl.kp_set =kp_set;
    motor->ctrl.kd_set =kd_set;
    motor->ctrl.pos_set =pos_set;
    motor->ctrl.vel_set =vel_set;
    motor->ctrl.cur_set =0;
    motor->ctrl.tor_set =tor_set;
    motor->para.PMAX = PMAX;
    motor->para.VMAX = VMAX;
    motor->para.TMAX = TMAX;
    motor->ctrl.pos_max = p_max;
    motor->ctrl.pos_min = p_min;
}

/**
 *      计算斜坡
 *      can发送
 *
 **/

void dm_motor_ctrl(dm_motor_t *motor,float kp,float kd,float pos,float vel,float tor)
{
        pos_limit(motor);
        motor->ramp_out= s_ramp_calc(&motor->ramp_data,pos);              //斜坡

        CAN_TxHeaderTypeDef	tx_header;
        uint32_t mailbox;
        uint8_t data[8];

        tx_header.StdId =motor->ctrl.id;
        tx_header.IDE   = 0;
        tx_header.RTR   = 0;
        tx_header.DLC   = 0x08;

        uint32_t pos_tmp = float_to_uint(motor->ramp_out, -motor->para.PMAX, motor->para.PMAX, 16);
        uint32_t vel_tmp = float_to_uint(vel, -motor->para.VMAX, motor->para.VMAX, 12);
        uint32_t tor_tmp = float_to_uint(tor, -motor->para.TMAX, motor->para.TMAX, 12);
        uint32_t kp_tmp  = float_to_uint(kp,  KP_MIN, KP_MAX, 12);
        uint32_t kd_tmp  = float_to_uint(kd,  KD_MIN, KD_MAX, 12);

        data[0] = (pos_tmp >> 8);
        data[1] = pos_tmp;
        data[2] = (vel_tmp >> 4);
        data[3] = ((vel_tmp&0xF)<<4)|(kp_tmp>>8);
        data[4] = kp_tmp;
        data[5] = (kd_tmp >> 4);
        data[6] = ((kd_tmp&0xF)<<4)|(tor_tmp>>8);
        data[7] = tor_tmp;

        HAL_CAN_AddTxMessage(&hcan1,&tx_header,data,&mailbox);

        osDelay(1);
}


void dm_motor_upgrate(dm_motor_t *motor,message_arm *custom_robot_data)
{
    dm_motor[ARM1].ctrl.pos_set = custom_robot_data->mt_ang.angle[0]*PI/180.f;
    dm_motor[ARM2].ctrl.pos_set = custom_robot_data->mt_ang.angle[1]*PI/180.f;
    dm_motor[ARM3].ctrl.pos_set = custom_robot_data->mt_ang.angle[2]*PI/180.f;
    dm_motor[ARM4].ctrl.pos_set = custom_robot_data->mt_ang.angle[3]*PI/180.f;
    dm_motor[ARM5].ctrl.pos_set = custom_robot_data->mt_ang.angle[4]*PI/180.f;
    dm_motor[ARM6].ctrl.pos_set = custom_robot_data->mt_ang.angle[5]*PI/180.f;

}

void can1_rx_callback(void)
{
    uint16_t rec_id;
    uint8_t rx_data[8] = {0};
    canx_receive(&hcan1, &rec_id, rx_data);
    uint8_t temp_id = (rx_data[0])&0x0F;
    switch (temp_id)
    {

        case 0x01: dm_motor_feedback(&dm_motor[1],rx_data); break;
        case 0x02: dm_motor_feedback(&dm_motor[2],rx_data); break;
        case 0x03: dm_motor_feedback(&dm_motor[3],rx_data); break;
        case 0x04: dm_motor_feedback(&dm_motor[4],rx_data); break;
        case 0x05: dm_motor_feedback(&dm_motor[5],rx_data); break;
        case 0x06: dm_motor_feedback(&dm_motor[6],rx_data); break;

    }
}


/*------------------------电机使能&使能------------------------------------*/
void dm_motor_enable(dm_motor_t* motor)
{
    CAN_TxHeaderTypeDef	tx_header;
    uint32_t mailbox;
    tx_header.StdId = motor->ctrl.id;
    tx_header.ExtId = 0;
    tx_header.IDE   = 0;
    tx_header.RTR   = 0;
    tx_header.DLC   = 8;

    uint8_t data[8];
    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0xFF;
    data[3] = 0xFF;
    data[4] = 0xFF;
    data[5] = 0xFF;
    data[6] = 0xFF;
    data[7] = 0xFC;

    HAL_CAN_AddTxMessage(&hcan1,&tx_header,data,&mailbox);
}


void dm_motor_disable(hcan_t* hcan, dm_motor_t *motor)
{
    CAN_TxHeaderTypeDef	tx_header;
    uint32_t mailbox;
    tx_header.StdId = motor->ctrl.id;
    tx_header.ExtId = 0;
    tx_header.IDE   = 0;
    tx_header.RTR   = 0;
    tx_header.DLC   = 8;

    uint8_t data[8];
    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0xFF;
    data[3] = 0xFF;
    data[4] = 0xFF;
    data[5] = 0xFF;
    data[6] = 0xFF;
    data[7] = 0xFD;

    HAL_CAN_AddTxMessage(&hcan1,&tx_header,data,&mailbox);
    dm_motor_clear_para(motor);
}



void dm_motor_feedback(dm_motor_t *motor, uint8_t *rx_data)
{
    int p_temp;
    int v_temp;
    int t_temp;
    motor->feedback.id = (rx_data[0])&0x0F;
    motor->feedback.state = (rx_data[0])>>4;
    p_temp=(rx_data[1]<<8)|rx_data[2];
    v_temp=(rx_data[3]<<4)|(rx_data[4]>>4);
    t_temp=((rx_data[4]&0xF)<<8)|rx_data[5];
    motor->feedback.pos = uint_to_float(p_temp, -motor->para.PMAX, motor->para.PMAX, 16);
    motor->feedback.vel = uint_to_float(v_temp, -motor->para.VMAX , motor->para.VMAX , 12);
    motor->feedback.tor = uint_to_float(t_temp, -motor->para.TMAX,motor->para.TMAX, 12);
    motor->feedback.Tmos = (float)(rx_data[6]);
    motor->feedback.Tcoil = (float)(rx_data[7]);
}



/****************************保存零点*************************************/
void save_pos_zero(hcan_t* hcan, uint16_t motor_id,uint16_t mode_id)
{
    CAN_TxHeaderTypeDef	tx_header;
    uint32_t mailbox;
    tx_header.StdId = motor_id;
    tx_header.ExtId = 0;
    tx_header.IDE   = 0;
    tx_header.RTR   = 0;
    tx_header.DLC   = 8;

    uint8_t data[8];
    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0xFF;
    data[3] = 0xFF;
    data[4] = 0xFF;
    data[5] = 0xFF;
    data[6] = 0xFF;
    data[7] = 0xFE;
    HAL_CAN_AddTxMessage(&hcan1,&tx_header,data,&mailbox);
}


void dm_motor_clear_para(dm_motor_t *motor)
{
    motor->ctrl.kp_set =0;
    motor->ctrl.kd_set = 0;
    motor->ctrl.pos_set = 0;
    motor->ctrl.vel_set = 0;
    motor->ctrl.tor_set = 0;
    motor->ctrl.cur_set = 0;
}


int float_to_uint(float x_float, float x_min, float x_max, int bits)
{
    /* Converts a float to an unsigned int, given range and number of bits */
    float span = x_max - x_min;
    float offset = x_min;
    if (x_float < x_min) x_float = x_min;
    if (x_float > x_max) x_float = x_max;
    return (int) ((x_float-offset)*((float)((1<<bits)-1))/span);
}

float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
    /* converts unsigned int to float, given range and number of bits */
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int)*span/((float)((1<<bits)-1)) + offset;
}









