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


motorpara_t dm4310para = {DM_4310_PMAX,DM_4310_VMAX,DM_4310_TMAX};
motorpara_t dm4340para = {DM_4340_PMAX,DM_4340_VMAX,DM_4340_TMAX};
motorpara_t dm8009para = {DM_8009_PMAX,DM_8009_VMAX,DM_8009_TMAX};

void dm_motor_init(dm_motor_t *motor,uint16_t id,uint8_t mode,float kp_set,float kd_set,float pos_set,float vel_set,float tor_set)
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
}

void dm_motor_ctrl(hcan_t* hcan,dm_motor_t *motor,motorpara_t *para,float kp,float kd,float pos,float vel,float tor)
{

        switch(motor->ctrl.mode)
        {
        case mit_mode:
            motor->ramp_out= s_ramp_calc(&motor->ramp_data,pos);
            mit_ctrl(hcan, motor,para,motor->ctrl.id,kp,kd,motor->ramp_out,vel,tor);
            break;
        case pos_mode:
            pos_ctrl(hcan, motor->ctrl.id, pos, vel);
            break;
        case vel_mode:
            vel_ctrl(hcan, motor->ctrl.id,vel);
            break;
        case psi_mode:
            psi_ctrl(hcan, motor->ctrl.id,pos, vel, 0);
            break;
        }
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

    // dm_motor[ARM1].ramp_out = s_ramp_calc(&dm_motor[ARM1].ramp_data,dm_motor[ARM1].ctrl.pos_set);
    // dm_motor[ARM2].ramp_out = s_ramp_calc(&dm_motor[ARM2].ramp_data,dm_motor[ARM2].ctrl.pos_set);
    // dm_motor[ARM3].ramp_out = s_ramp_calc(&dm_motor[ARM3].ramp_data,dm_motor[ARM3].ctrl.pos_set);
    // dm_motor[ARM4].ramp_out = s_ramp_calc(&dm_motor[ARM4].ramp_data,dm_motor[ARM4].ctrl.pos_set);
    // dm_motor[ARM5].ramp_out = s_ramp_calc(&dm_motor[ARM5].ramp_data,dm_motor[ARM5].ctrl.pos_set);
    // dm_motor[ARM6].ramp_out = s_ramp_calc(&dm_motor[ARM6].ramp_data,dm_motor[ARM6].ctrl.pos_set);
}

void can1_rx_callback(void)
{
    uint16_t rec_id;
    uint8_t rx_data[8] = {0};
    canx_receive(&hcan1, &rec_id, rx_data);
    uint8_t temp_id = (rx_data[0])&0x0F;
    switch (temp_id)
    {

        case 0x01: dm_4310_motor_feedback(&dm_motor[1],rx_data); break;
        case 0x02: dm_4340_motor_feedback(&dm_motor[2],rx_data); break;
        case 0x03: dm_4310_motor_feedback(&dm_motor[3],rx_data); break;
        case 0x04: dm_4340_motor_feedback(&dm_motor[4],rx_data); break;
        case 0x05: dm_8009_motor_feedback(&dm_motor[5],rx_data); break;
        case 0x06: dm_8009_motor_feedback(&dm_motor[6],rx_data); break;

    }
}


/*------------------------电机使能&使能------------------------------------*/
void dm_motor_enable(hcan_t* hcan,dm_motor_t* motor)
{
    switch (motor->ctrl.mode)
    {
    case mit_mode:
        enable_motor_mode(hcan, motor->ctrl.id, MIT_MODE);
        break;
    case pos_mode:
        enable_motor_mode(hcan, motor->ctrl.id, POS_MODE);
        break;
    case vel_mode:
        enable_motor_mode(hcan, motor->ctrl.id, SPD_MODE);
        break;
    case psi_mode:
        enable_motor_mode(hcan, motor->ctrl.id, PSI_MODE);
        break;
    }
}


void dm_motor_disable(hcan_t* hcan, dm_motor_t *motor)
{
    switch(motor->ctrl.mode)
    {
    case mit_mode:
        disable_motor_mode(hcan, motor->ctrl.id, MIT_MODE);
        break;
    case pos_mode:
        disable_motor_mode(hcan, motor->ctrl.id, POS_MODE);
        break;
    case vel_mode:
        disable_motor_mode(hcan, motor->ctrl.id, SPD_MODE);
        break;
    case psi_mode:
        disable_motor_mode(hcan, motor->ctrl.id, PSI_MODE);
        break;
    }
    dm_motor_clear_para(motor);
}


/*---------------------电机控制&反馈------------------------------------*/
void dm_motor_ctrl_send(hcan_t* hcan, dm_motor_t *motor,motorpara_t* para)
{
    switch(motor->ctrl.mode)
    {
    case mit_mode:
        mit_ctrl(hcan, motor,para, motor->ctrl.id, motor->ctrl.kp_set, motor->ctrl.kd_set, motor->ctrl.pos_set, motor->ctrl.vel_set, motor->ctrl.tor_set);
        break;
    case pos_mode:
        pos_ctrl(hcan, motor->ctrl.id, motor->ctrl.pos_set, motor->ctrl.vel_set);
        break;
    case vel_mode:
        vel_ctrl(hcan, motor->ctrl.id, motor->ctrl.vel_set);
        break;
    case psi_mode:
        psi_ctrl(hcan, motor->ctrl.id,motor->ctrl.pos_set, motor->ctrl.vel_set, motor->ctrl.cur_set);
        break;
    }
}


void dm_4310_motor_feedback(dm_motor_t *motor, uint8_t *rx_data)
{
    int p_temp;
    int v_temp;
    int t_temp;
    motor->feedback.id = (rx_data[0])&0x0F;
    motor->feedback.state = (rx_data[0])>>4;
    p_temp=(rx_data[1]<<8)|rx_data[2];
    v_temp=(rx_data[3]<<4)|(rx_data[4]>>4);
    t_temp=((rx_data[4]&0xF)<<8)|rx_data[5];
    motor->feedback.pos = uint_to_float(p_temp, -DM_4310_PMAX, DM_4310_PMAX, 16);
    motor->feedback.vel = uint_to_float(v_temp, -DM_4310_VMAX , DM_4310_VMAX , 12);
    motor->feedback.tor = uint_to_float(t_temp, -DM_4310_TMAX,DM_4310_TMAX, 12);
    motor->feedback.Tmos = (float)(rx_data[6]);
    motor->feedback.Tcoil = (float)(rx_data[7]);
}

void dm_4340_motor_feedback(dm_motor_t *motor, uint8_t *rx_data)
{
    int p_temp;
    int v_temp;
    int t_temp;
    motor->feedback.id = (rx_data[0])&0x0F;
    motor->feedback.state = (rx_data[0])>>4;
    p_temp=(rx_data[1]<<8)|rx_data[2];
    v_temp=(rx_data[3]<<4)|(rx_data[4]>>4);
    t_temp=((rx_data[4]&0xF)<<8)|rx_data[5];
    motor->feedback.pos = uint_to_float(p_temp, -DM_4340_PMAX, DM_4340_PMAX, 16);
    motor->feedback.vel = uint_to_float(v_temp, -DM_4340_VMAX , DM_4340_VMAX , 12);
    motor->feedback.tor = uint_to_float(t_temp, -DM_4340_TMAX,DM_4340_TMAX, 12);
    motor->feedback.Tmos = (float)(rx_data[6]);
    motor->feedback.Tcoil = (float)(rx_data[7]);
}

void dm_8009_motor_feedback(dm_motor_t *motor, uint8_t *rx_data)
{
    int p_temp;
    int v_temp;
    int t_temp;
    motor->feedback.id = (rx_data[0])&0x0F;
    motor->feedback.state = (rx_data[0])>>4;
    p_temp=(rx_data[1]<<8)|rx_data[2];
    v_temp=(rx_data[3]<<4)|(rx_data[4]>>4);
    t_temp=((rx_data[4]&0xF)<<8)|rx_data[5];
    motor->feedback.pos = uint_to_float(p_temp, -DM_8009_PMAX, DM_8009_PMAX, 16);
    motor->feedback.vel = uint_to_float(v_temp, -DM_8009_VMAX , DM_8009_VMAX , 12);
    motor->feedback.tor = uint_to_float(t_temp, -DM_8009_TMAX,DM_8009_TMAX, 12);
    motor->feedback.Tmos = (float)(rx_data[6]);
    motor->feedback.Tcoil = (float)(rx_data[7]);
}


/****************************保存零点*************************************/
void save_pos_zero(hcan_t* hcan, uint16_t motor_id,uint16_t mode_id)
{
    uint8_t data[8];
    uint16_t id = motor_id + mode_id;
    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0xFF;
    data[3] = 0xFF;
    data[4] = 0xFF;
    data[5] = 0xFF;
    data[6] = 0xFF;
    data[7] = 0xFE;
    canx_send_data(hcan, id, data, 8);
}


void enable_motor_mode(hcan_t* hcan, uint16_t motor_id, uint16_t mode)
{
    uint8_t data[8];
    uint16_t id = motor_id + mode;

    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0xFF;
    data[3] = 0xFF;
    data[4] = 0xFF;
    data[5] = 0xFF;
    data[6] = 0xFF;
    data[7] = 0xFC;

    canx_send_data(hcan, id, data, 8);
}

void disable_motor_mode(hcan_t* hcan, uint16_t motor_id, uint16_t mode_id)
{
    uint8_t data[8];
    uint16_t id = motor_id + mode_id;

    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0xFF;
    data[3] = 0xFF;
    data[4] = 0xFF;
    data[5] = 0xFF;
    data[6] = 0xFF;
    data[7] = 0xFD;

    canx_send_data(hcan, id, data, 8);
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



void mit_ctrl(hcan_t* hcan, dm_motor_t *motor,motorpara_t *para, uint16_t motor_id,float kp,float kd,float pos,float vel,float tor)
{
    uint8_t data[8];
    uint32_t pos_tmp,vel_tmp,kp_tmp,kd_tmp,tor_tmp;
    uint16_t id = motor_id + MIT_MODE;

    pos_tmp = float_to_uint(pos, -para->PMAX, para->PMAX, 16);
    vel_tmp = float_to_uint(vel, -para->VMAX, para->VMAX, 12);
    tor_tmp = float_to_uint(tor, -para->TMAX, para->TMAX, 12);
    kp_tmp  = float_to_uint(kp,  KP_MIN, KP_MAX, 12);
    kd_tmp  = float_to_uint(kd,  KD_MIN, KD_MAX, 12);

    data[0] = (pos_tmp >> 8);
    data[1] = pos_tmp;
    data[2] = (vel_tmp >> 4);
    data[3] = ((vel_tmp&0xF)<<4)|(kp_tmp>>8);
    data[4] = kp_tmp;
    data[5] = (kd_tmp >> 4);
    data[6] = ((kd_tmp&0xF)<<4)|(tor_tmp>>8);
    data[7] = tor_tmp;

    canx_send_data(hcan, id, data, 8);
}


void pos_ctrl(hcan_t* hcan,uint16_t motor_id, float pos, float vel)
{
    uint16_t id;
    uint8_t *pbuf, *vbuf;
    uint8_t data[8];

    id = motor_id + POS_MODE;
    pbuf=(uint8_t*)&pos;
    vbuf=(uint8_t*)&vel;

    data[0] = *pbuf;
    data[1] = *(pbuf+1);
    data[2] = *(pbuf+2);
    data[3] = *(pbuf+3);

    data[4] = *vbuf;
    data[5] = *(vbuf+1);
    data[6] = *(vbuf+2);
    data[7] = *(vbuf+3);

    canx_send_data(hcan, id, data, 8);
}

void vel_ctrl(hcan_t* hcan, uint16_t motor_id, float vel)
{
    uint16_t id;
    uint8_t *vbuf;
    uint8_t data[4];

    id = motor_id + SPD_MODE;
    vbuf=(uint8_t*)&vel;

    data[0] = *vbuf;
    data[1] = *(vbuf+1);
    data[2] = *(vbuf+2);
    data[3] = *(vbuf+3);

    canx_send_data(hcan, id, data, 4);
}

void psi_ctrl(hcan_t* hcan, uint16_t motor_id, float pos, float vel, float cur)
{
    uint16_t id;
    uint8_t *pbuf, *vbuf, *ibuf;
    uint8_t data[8];

    uint16_t u16_vel = vel*100;
    uint16_t u16_cur  = cur*10000;

    id = motor_id + PSI_MODE;
    pbuf=(uint8_t*)&pos;
    vbuf=(uint8_t*)&u16_vel;
    ibuf=(uint8_t*)&u16_cur;

    data[0] = *pbuf;
    data[1] = *(pbuf+1);
    data[2] = *(pbuf+2);
    data[3] = *(pbuf+3);

    data[4] = *vbuf;
    data[5] = *(vbuf+1);

    data[6] = *ibuf;
    data[7] = *(ibuf+1);

    canx_send_data(hcan, id, data, 8);
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









