//
// Created by jiang on 2026/10/6.
//

#ifndef ARM_DM_H
#define ARM_DM_H
#include "bsp_can.h"
#include "referee.h"

#define MIT_MODE 			0x000
#define POS_MODE			0x100
#define SPD_MODE			0x200
#define PSI_MODE		  	0x300

#define KP_MIN 0.0f
#define KP_MAX 500.0f
#define KD_MIN 0.0f
#define KD_MAX 5.0f


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

}motor_ctrl_t;

typedef struct
{
    motor_feedback_t feedback;
    motor_ctrl_t ctrl;

}dm_motor_t;

extern dm_motor_t dm_motor[8];
extern motorpara_t dm4310para;
extern motorpara_t dm4340para;
extern motorpara_t dm8009para;
void dm_motor_upgrate(dm_motor_t *motor,message_arm *custom_robot_data,uint8_t index);
void dm_motor_ctrl(hcan_t* hcan,dm_motor_t *motor,motorpara_t *para,float kp,float kd,float pos,float vel,float tor);
void dm_motor_init(dm_motor_t *motor,uint16_t id,uint8_t mode,float kp_set,float kd_set,float pos_set,float vel_set,float tor_set);
void dm_motor_ctrl_send(hcan_t* hcan, dm_motor_t *motor,motorpara_t* para);
void dm_motor_enable(hcan_t* hcan,dm_motor_t* motor);
void dm_motor_disable(hcan_t* hcan, dm_motor_t *motor);
void dm_4310_motor_feedback(dm_motor_t *motor, uint8_t *rx_data);
void dm_4340_motor_feedback(dm_motor_t *motor, uint8_t *rx_data);
void dm_8009_motor_feedback(dm_motor_t *motor, uint8_t *rx_data);
void enable_motor_mode(hcan_t* hcan, uint16_t motor_id, uint16_t mode);
void disable_motor_mode(hcan_t* hcan, uint16_t motor_id, uint16_t mode_id);
void dm_motor_clear_para(dm_motor_t *motor);
void mit_ctrl(hcan_t* hcan, dm_motor_t *motor,motorpara_t *para, uint16_t motor_id,float kp,float kd,float pos,float vel,float tor);
int float_to_uint(float x_float, float x_min, float x_max, int bits);
float uint_to_float(int x_int, float x_min, float x_max, int bits);
void pos_ctrl(hcan_t* hcan,uint16_t motor_id, float pos, float vel);
void vel_ctrl(hcan_t* hcan, uint16_t motor_id, float vel);
void psi_ctrl(hcan_t* hcan, uint16_t motor_id, float pos, float vel, float cur);
#endif //ARM_DM_H
