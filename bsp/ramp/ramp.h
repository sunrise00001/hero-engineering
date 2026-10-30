//
// Created by jiang on 2026/10/29.
//

#ifndef ARM_RAMP_H
#define ARM_RAMP_H

typedef struct{
    float k;            //比例系数
    float target;       //目标值
    float cur_target;    //当前目标值
}ramp_type_t;

/**
 * @brief 斜坡函数初始化
 *
 * @param ramp      斜坡函数结构体
 * @param k         斜坡斜率
 * @param target    目标值
 * @param current   当前值
 */
void s_ramp_init(ramp_type_t* ramp, float k, float target, float current);

/**
 * @brief 斜坡函数计算
 *
 * @param ramp      斜坡函数结构体
 * @param target    目标值
 * @return float
 */
float s_ramp_calc(ramp_type_t* ramp, float target);

float s_ramp_reset(ramp_type_t* ramp, float reset);










#endif //ARM_RAMP_H
