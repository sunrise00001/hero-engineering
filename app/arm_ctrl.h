//
// Created by jiang on 2026/10/6.

#ifndef ARM_ARM_CTRL_H
#define ARM_ARM_CTRL_H
#include "dm.h"

#define ARM1                 1
#define ARM2                 2
#define ARM3                 3
#define ARM4                 4
#define ARM5                 5
#define ARM6                 6

#define ARM1_INDEX           0
#define ARM2_INDEX           1
#define ARM3_INDEX           2
#define ARM4_INDEX           3
#define ARM5_INDEX           4
#define ARM6_INDEX           5

extern dm_motor_t dm_motor[8];
extern uint8_t ic_define_angle[256];
#endif //ARM_ARM_CTRL_H
