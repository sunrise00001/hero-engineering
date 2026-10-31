//
// Created by jiang on 2026/10/6.
//

#include "arm_ctrl.h"
#include "can.h"
#include "cmsis_os2.h"
#include "unitree_a1.h"
#include "RemoteController.h"


dm_motor_t dm_motor[7] = {0};


void dm_cmd_init(void)
{
    dm_motor_init(&dm_motor[ARM1],ARM1,mit_mode,0,0,0,0,0,DM_4310_PMAX,DM_4310_VMAX,DM_4310_TMAX,ARM1_POS_MAX,ARM1_POS_MIN);
    dm_motor_init(&dm_motor[ARM2],ARM2,mit_mode,0,0,0,0,0,DM_4340_PMAX,DM_4340_VMAX,DM_4340_TMAX,ARM2_POS_MAX,ARM2_POS_MIN);
    dm_motor_init(&dm_motor[ARM3],ARM3,mit_mode,0,0,0,0,0,DM_4310_PMAX,DM_4310_VMAX,DM_4310_TMAX,ARM3_POS_MAX,ARM3_POS_MIN);
    dm_motor_init(&dm_motor[ARM4],ARM4,mit_mode,0,0,0,0,0,DM_4340_PMAX,DM_4340_VMAX,DM_4340_TMAX,ARM4_POS_MAX,ARM4_POS_MIN);
    dm_motor_init(&dm_motor[ARM5],ARM5,mit_mode,0,0,0,0,0,DM_8009_PMAX,DM_8009_VMAX,DM_8009_TMAX,ARM5_POS_MAX,ARM5_POS_MIN);
    dm_motor_init(&dm_motor[ARM6],ARM6,mit_mode,0,0,0,0,0,DM_8009_PMAX,DM_8009_VMAX,DM_8009_TMAX,ARM6_POS_MAX,ARM6_POS_MIN);

    dm_motor_enable(&dm_motor[ARM1]);
    osDelay(20);
    dm_motor_enable(&dm_motor[ARM2]);
    osDelay(20);
    dm_motor_enable(&dm_motor[ARM3]);
    osDelay(20);
    dm_motor_enable(&dm_motor[ARM4]);
    osDelay(20);
    dm_motor_enable(&dm_motor[ARM5]);
    osDelay(20);
    dm_motor_enable(&dm_motor[ARM6]);

    s_ramp_init(&dm_motor[ARM1].ramp_data,RAMP_K,dm_motor[ARM1].ctrl.pos_set,dm_motor[ARM1].feedback.pos);
	s_ramp_init(&dm_motor[ARM2].ramp_data,RAMP_K,dm_motor[ARM2].ctrl.pos_set,dm_motor[ARM2].feedback.pos);
	s_ramp_init(&dm_motor[ARM3].ramp_data,RAMP_K,dm_motor[ARM3].ctrl.pos_set,dm_motor[ARM3].feedback.pos);
	s_ramp_init(&dm_motor[ARM4].ramp_data,RAMP_K,dm_motor[ARM4].ctrl.pos_set,dm_motor[ARM4].feedback.pos);
	s_ramp_init(&dm_motor[ARM5].ramp_data,RAMP_K,dm_motor[ARM5].ctrl.pos_set,dm_motor[ARM5].feedback.pos);
	s_ramp_init(&dm_motor[ARM6].ramp_data,RAMP_K,dm_motor[ARM6].ctrl.pos_set,dm_motor[ARM6].feedback.pos);
};


void ARM_CTRL(void *argument)//ÈÎÎñº¯Êı
{
    osDelay(3000);
    dm_cmd_init();
	  while(1){

	  	dm_motor_upgrate(dm_motor,&custom_robot_data);

		control_update();
		osDelay(2);
		}		
	  
}


void control_update(){
     if (GET_SWITCH_LEFT() == RC_SW_DOWN) {


     	dm_motor_ctrl(&dm_motor[ARM1],0,0,0,0,0);
        dm_motor_ctrl(&dm_motor[ARM2],0,0,0,0,0);
        dm_motor_ctrl(&dm_motor[ARM3],0,0,0,0,0);
        dm_motor_ctrl(&dm_motor[ARM4],0,0,0,0,0);
        dm_motor_ctrl(&dm_motor[ARM5],0,0,0,0,0);
        dm_motor_ctrl(&dm_motor[ARM6],0,0,0,0,0);
			
		}
		else if(GET_SWITCH_LEFT() == RC_SW){


        dm_motor_ctrl(&dm_motor[ARM1],3,1,0,0,0);
        dm_motor_ctrl(&dm_motor[ARM2],15,4,0,0,0);
        dm_motor_ctrl(&dm_motor[ARM3],5,2,0,0,0);
        dm_motor_ctrl(&dm_motor[ARM4],55,10,0,0,10);
        dm_motor_ctrl(&dm_motor[ARM5],0,0,0,0,0);
        dm_motor_ctrl(&dm_motor[ARM6],0,0,0,0,0);
	}
		else{
			pos_limit(&dm_motor[ARM1]);
			pos_limit(&dm_motor[ARM2]);
			pos_limit(&dm_motor[ARM3]);
			pos_limit(&dm_motor[ARM4]);
			pos_limit(&dm_motor[ARM5]);
			pos_limit(&dm_motor[ARM6]);

        dm_motor_ctrl(&dm_motor[ARM1],3,1,dm_motor[ARM1].ctrl.pos_set,0,0);
        dm_motor_ctrl(&dm_motor[ARM2],8,3,dm_motor[ARM2].ctrl.pos_set,0,0);
        dm_motor_ctrl(&dm_motor[ARM3],5,2,dm_motor[ARM3].ctrl.pos_set,0,0);
        dm_motor_ctrl(&dm_motor[ARM4],45,15,dm_motor[ARM4].ctrl.pos_set,0,2);
        dm_motor_ctrl(&dm_motor[ARM5],40,15,dm_motor[ARM5].ctrl.pos_set,0,0);
        dm_motor_ctrl(&dm_motor[ARM6],0,0,dm_motor[ARM6].ctrl.pos_set,0,0);
		}
}


void pos_limit(dm_motor_t *motor)
{
	if (motor->ctrl.pos_set > motor->ctrl.pos_max)
	{
		motor->ctrl.pos_set = motor->ctrl.pos_max;
	}

	if (motor->ctrl.pos_set < motor->ctrl.pos_min)
	{
		motor->ctrl.pos_set = motor->ctrl.pos_min;
	}


}