//
// Created by jiang on 2026/7/28.
//

#ifndef ARM_PID_H
#define ARM_PID_H

typedef struct
{
    float kp;
    float ki;
    float kd;
    float target;           //目标值
    float actual;           //反馈值
    float error0;           //当前误差
    float error1;           //上次误差
    float errorint;         //误差积分
    float error_i_min;      //积分下限
    float error_i_max;      //积分上限
    float out_min;          //输出下限
    float out_max;          //输出上限
    float out;
}Pid_t;

//初始化 增益、积分限幅、输出限幅
void pid_init(Pid_t *pid,float kp,float ki,float kd,float out_min,float out_max);
//清空状态 保留参数
void pid_reset(Pid_t *pid);
//PID计算 固定周期调用
float pid_calc(Pid_t *pid,float target,float actual);

#endif //ARM_PID_H
