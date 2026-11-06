//
// Created by jiang on 2026/10/31.
//

#ifndef ARM_3508_H
#define ARM_3508_H
#include <stdint.h>
#include <sys/types.h>

#include "pid.h"


typedef struct
{
    uint16_t angle;
    int16_t speed;
    int16_t current;
    uint8_t temp;
    int16_t ctrl_iq;
    uint8_t state;
}motor_3508_t;

extern Pid_t motor_3508_speed_pid;
extern motor_3508_t motor_3508;
void graipper_ctrl_init(void);
void gripper_ctrl(const int16_t speed);
void motor_3508_pid(motor_3508_t *motor,int16_t target_speed);
void motor_3508_init(void);
void motor_3508feedback(uint8_t *data);
void motor3508_senddata(uint16_t iq);
#endif //ARM_3508_H
