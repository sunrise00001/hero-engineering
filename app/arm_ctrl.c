//
// Created by jiang on 2026/10/6.
//

#include "arm_ctrl.h"
#include "can.h"
#include "cmsis_os2.h"
#include "unitree_a1.h"
#include "RemoteController.h"


dm_motor_t dm_motor[7] = {0};
void control_update(void);

void dm_cmd_init(void)
{
    dm_motor_init(&dm_motor[ARM1],ARM1,mit_mode,0,0,0,0,0);
    dm_motor_init(&dm_motor[ARM2],ARM2,mit_mode,0,0,0,0,0);
    dm_motor_init(&dm_motor[ARM3],ARM3,mit_mode,0,0,0,0,0);
    dm_motor_init(&dm_motor[ARM4],ARM4,mit_mode,0,0,0,0,0);
    dm_motor_init(&dm_motor[ARM5],ARM5,mit_mode,0,0,0,0,0);
    dm_motor_init(&dm_motor[ARM6],ARM6,mit_mode,0,0,0,0,0);

    dm_motor_enable(&hcan1,&dm_motor[ARM1]);
    osDelay(20);
    dm_motor_enable(&hcan1,&dm_motor[ARM2]);
    osDelay(20);
    dm_motor_enable(&hcan1,&dm_motor[ARM3]);
    osDelay(20);
    dm_motor_enable(&hcan1,&dm_motor[ARM4]);
    osDelay(20);
    dm_motor_enable(&hcan1,&dm_motor[ARM5]);
    osDelay(20);
    dm_motor_enable(&hcan1,&dm_motor[ARM6]);

    s_ramp_init(&dm_motor[ARM1].ramp_data,RAMP_K,dm_motor[ARM1].ctrl.pos_set,dm_motor[ARM1].feedback.pos);
	s_ramp_init(&dm_motor[ARM2].ramp_data,RAMP_K,dm_motor[ARM2].ctrl.pos_set,dm_motor[ARM2].feedback.pos);
	s_ramp_init(&dm_motor[ARM3].ramp_data,RAMP_K,dm_motor[ARM3].ctrl.pos_set,dm_motor[ARM3].feedback.pos);
	s_ramp_init(&dm_motor[ARM4].ramp_data,RAMP_K,dm_motor[ARM4].ctrl.pos_set,dm_motor[ARM4].feedback.pos);
	s_ramp_init(&dm_motor[ARM5].ramp_data,RAMP_K,dm_motor[ARM5].ctrl.pos_set,dm_motor[ARM5].feedback.pos);
	s_ramp_init(&dm_motor[ARM6].ramp_data,RAMP_K,dm_motor[ARM6].ctrl.pos_set,dm_motor[ARM6].feedback.pos);
};


void ARM_CTRL(void *argument)//ÈÎÎñº¯Êý
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


     	dm_motor_ctrl(&hcan1,&dm_motor[ARM1],&dm4310para,0,0,0,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM2],&dm4340para,0,0,0,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM3],&dm4310para,0,0,0,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM4],&dm4340para,0,0,0,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM5],&dm8009para,0,0,0,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM6],&dm8009para,0,0,0,0,0);
			
		}
		else if(GET_SWITCH_LEFT() == RC_SW){


        dm_motor_ctrl(&hcan1,&dm_motor[ARM1],&dm4310para,3,1,0,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM2],&dm4340para,15,4,0,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM3],&dm4310para,5,2,0,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM4],&dm4340para,55,10,0,0,10);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM5],&dm8009para,0,0,0,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM6],&dm8009para,0,0,0,0,0);
	}
		else{

        dm_motor_ctrl(&hcan1,&dm_motor[ARM1],&dm4310para,3,1,dm_motor[ARM1].ctrl.pos_set,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM2],&dm4340para,8,3,dm_motor[ARM2].ctrl.pos_set,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM3],&dm4310para,5,2,dm_motor[ARM3].ctrl.pos_set,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM4],&dm4340para,45,15,dm_motor[ARM4].ctrl.pos_set,0,2);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM5],&dm8009para,40,15,dm_motor[ARM5].ctrl.pos_set,0,0);
        dm_motor_ctrl(&hcan1,&dm_motor[ARM6],&dm8009para,0,0,dm_motor[ARM6].ctrl.pos_set,0,0);
		}
}