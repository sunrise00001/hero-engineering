//
// Created by jiang on 2026/10/31.
//

#ifndef ARM_3508_H
#define ARM_3508_H
#include <stdint.h>
#include <sys/types.h>


typedef struct
{
    float angle;
    float speed;
    int16_t current;
    uint8_t temp;
    int16_t ctrl_iq;
    uint8_t state;
}motor_3508_t;

extern pid_t speed;
extern motor_3508_t motor_3508;
void motor_3508_pid(motor_3508_t *motor,float target_speed);
void motor_3508_init(void);
void motor_3508feedback(uint8_t *data);
void motor3508_senddata(uint16_t iq);
#endif //ARM_3508_H
