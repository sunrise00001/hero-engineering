//
// Created by jiang on 2026/10/29.
//

#include "ramp.h"

#define ABS(__num__) 	\
((__num__) > 0 ? (__num__) : ( - (__num__)))

void s_ramp_init(ramp_type_t* ramp, float k, float target, float current)
{
    ramp->k = k;
    ramp->target = target;
    ramp->cur_target = current;
}

float s_ramp_calc(ramp_type_t* ramp, float target)
{
    ramp->target = target;
    if (ABS(ramp->target - ramp->cur_target) < ramp->k) {
        ramp->cur_target = ramp->target;
        return ramp->cur_target;
    }

    if (ramp->target > ramp->cur_target) {
        ramp->cur_target += ramp->k;
    } else if (ramp->target < ramp->cur_target) {
        ramp->cur_target -= ramp->k;
    }

    return ramp->cur_target;
}

float s_ramp_reset(ramp_type_t* ramp, float reset)
{
    ramp->cur_target = reset;
}










