//
// Created by jiang on 2026/7/28.
//

#include "pid.h"
#include <stddef.h>

//初始化
void pid_init(Pid_t *pid,float kp,float ki,float kd,float out_min,float out_max)
{
    if (pid == NULL)
        return;

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->out_min = out_min;
    pid->out_max = out_max;

}

//清空状态
void pid_reset(Pid_t *pid)
{
    if (pid == NULL)
        return;

    pid->target = 0.0f;
    pid->actual = 0.0f;
    pid->error0 = 0.0f;
    pid->error1 = 0.0f;
    pid->errorint = 0.0f;
    pid->out = 0.0f;
}

//PID计算
float pid_calc(Pid_t *pid,float target,float actual)
{
    if (pid == NULL)
        return 0.0f;

    pid->target = target;
    pid->actual = actual;
    pid->error1 = pid->error0;
    pid->error0 = pid->target-pid->actual;

    pid->out = pid->kp*pid->error0+pid->ki*pid->errorint+pid->kd*(pid->error0-pid->error1);

    //输出限幅
    if (pid->out > pid->out_max)
        pid->out = pid->out_max;
    if (pid->out < pid->out_min)
        pid->out = pid->out_min;

    return pid->out;
}
