//
// Created by jiang on 2026/10/6.
//

#ifndef ARM_DM_H
#define ARM_DM_H
#include "bsp_can.h"
#include "referee.h"
#include "../../bsp/ramp/ramp.h"


#define KP_MIN 0.0f
#define KP_MAX 500.0f
#define KD_MIN 0.0f
#define KD_MAX 5.0f

#define RAMP_K     0.005


#define DM_4310_PMAX   12.5
#define DM_4310_VMAX   30
#define DM_4310_TMAX   10

#define DM_4340_PMAX   12.5
#define DM_4340_VMAX   10
#define DM_4340_TMAX   28

#define DM_8009_PMAX   12.5
#define DM_8009_VMAX   45
#define DM_8009_TMAX   54

typedef struct
{
    float PMAX;
    float VMAX;
    float TMAX;
}motorpara_t;

typedef enum
{
    mit_mode =1,
    pos_mode =2,
    vel_mode =3,
    psi_mode =4
}mode_e;


//电机反馈数据
typedef struct
{
    int id;
    int state;
    float pos;
    float vel;
    float tor;
    float Tmos;
    float Tcoil;
}motor_feedback_t;

//电机参数设置
typedef struct
{
    uint16_t id;
    uint16_t mst_id;
    uint8_t mode;
    float pos_set;
    float vel_set;
    float tor_set;
    float cur_set;
    float kp_set;
    float kd_set;
    float pos_max;                  //位置限幅用
    float pos_min;                  //
}motor_ctrl_t;

typedef struct
{
    motor_feedback_t feedback;
    motor_ctrl_t ctrl;
    motorpara_t para;          //电机的参数 PMAX VMAX TMAX
    ramp_type_t ramp_data;     //斜坡数据
    float ramp_out;               //斜坡输出
}dm_motor_t;

extern dm_motor_t dm_motor[7];
void dm_motor_upgrate(dm_motor_t *motor,message_arm *custom_robot_data);
void dm_motor_ctrl(dm_motor_t *motor,float kp,float kd,float pos,float vel,float tor);
void dm_motor_init(dm_motor_t *motor,uint16_t id,uint8_t mode,float kp_set,float kd_set,float pos_set,float vel_set,float tor_set,float PMAX,float VMAX,float TMAX,float p_max,float p_min);
void dm_motor_enable(dm_motor_t* motor);
void dm_motor_disable(hcan_t* hcan, dm_motor_t *motor);
void dm_motor_feedback(dm_motor_t *motor, uint8_t *rx_data);
void dm_motor_clear_para(dm_motor_t *motor);
int float_to_uint(float x_float, float x_min, float x_max, int bits);
float uint_to_float(int x_int, float x_min, float x_max, int bits);
#endif //ARM_DM_H
