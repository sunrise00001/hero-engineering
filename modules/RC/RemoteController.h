//
// Created by jiang on 2026/10/10.
//

#ifndef ARM_REMOTECONTROLLER_H
#define ARM_REMOTECONTROLLER_H

#define RC_CH_VALUE_MIN                 ((uint16_t)364)
#define RC_CH_VALUE_OFFSET              ((uint16_t)1024)
#define RC_CH_VALUE_MAX                 ((uint16_t)1684)

#define RC_SWITCH_LEFT			1
#define RC_SWITCH_RIGHT			0

#define RC_SW_UP                ((uint16_t)1)
#define RC_SW                   ((uint16_t)3)
#define RC_SW_DOWN              ((uint16_t)2)
#define switch_is_down(s)       (s == RC_SW_DOWN)
#define switch_is_mid(s)        (s == RC_SW_MID)
#define switch_is_up(s)         (s == RC_SW_UP)

#define GET_SWITCH_LEFT() (RC_ctrldata.s[RC_SWITCH_LEFT])
#define GET_SWITCH_RIGHT() (RC_ctrldata.s[RC_SWITCH_RIGHT])

#define KEY_PRESSED_W                   ((uint16_t)0X01<<0)
#define KEY_PRESSED_S                   ((uint16_t)0X01<<1)
#define KEY_PRESSED_A                   ((uint16_t)0X01<<2)
#define KEY_PRESSED_D                   ((uint16_t)0X01<<3)
#define KEY_PRESSED_Q                   ((uint16_t)0X01<<4)
#define KEY_PRESSED_E                   ((uint16_t)0X01<<5)
#define KEY_PRESSED_Shift               ((uint16_t)0X01<<6)
#define KEY_PRESSED_Ctrl                ((uint16_t)0X01<<7)

#define SBUS_RX_BUF_NUM 36u
#define RC_FRAME_LENGTH 18u

#include <stdint.h>



typedef struct
{
    int16_t ch0;
    int16_t ch1;
    int16_t ch2;
    int16_t ch3;
    int16_t ch4;

    char s[2];

    int16_t mouse_x;
    int16_t mouse_y;
    int16_t mouse_z;

    uint8_t mouse_left;
    uint8_t mouse_right;

    uint16_t key;
}RC_ctrl_t;



void Remote_Controler_init(void);
extern RC_ctrl_t RC_ctrldata;
extern uint8_t sbus_rx_buf[2][SBUS_RX_BUF_NUM];
void RemoteData_extract(volatile const uint8_t *sbus_buf,RC_ctrl_t* data);
void RC_Init(uint8_t *rx1_buf, uint8_t *rx2_buf, uint16_t dma_buf_num);

#endif //ARM_REMOTECONTROLLER_H
