// //
// // Created by jiang on 2026/9/21.
// //
//
// #ifndef J4310_F407_UNITREE_A1_H
// #define J4310_F407_UNITREE_A1_H
// #include <stdint.h>
// #include "main.h"
//
// typedef enum
// {
//     motormode_stop = 0,
//     motormode_slow = 5,
//     motormode_foc =10
// }motormode;
//
// typedef struct
// {
//     uint8_t id;
//     uint8_t mode;
//     float tor;                //目标力矩
//     float angvel;             //目标角速度
//     float pos;                //目标位置
//     float kp;                 //位置刚度
//     float kd;                 //速度刚度
// }Motorcmd;
//
// #pragma pack(1)
//
// typedef union {
//     int32_t L;
//     uint8_t u8[4];
//     uint16_t u16[2];
//     uint32_t u32;
//     float F;
// } COMData32;
//
// //数据包头
// typedef struct
// {
//     uint8_t start[2];       //包头 0xFE OXEE
//     uint8_t motorID;        //电机ID
//     uint8_t reserved;       //预留位，可忽略
// }COMHead;
//
// //发送主体
// typedef struct
// {
//     uint8_t mode;        //0:停转   5：开环缓慢转动    10：闭环伺服控制
//     uint8_t ModifyBit;       //可忽略
//     uint8_t ReadBit;         //可忽略
//     uint8_t reserved;        //可忽略
//     COMData32 Modify;         //可忽略
//
//     int16_t T;            //目标扭矩(T*256)
//     int16_t W;            //目标速度(W*128)
//     int32_t Pos;          //目标位置(Pos*16384/2pi)
//     int16_t K_P;          //位置刚度(K_P*2048)
//     int16_t K_W;          //速度刚度(K_W*1024)
//
//     uint8_t LowHzMotorCmdIndex;    //可忽略
//     uint8_t LowHzMotorCmdByte;
//
//     COMData32 Res[1];
// }MasterComdV3;
//
// //发送包
// typedef struct
// {
//     COMHead head;
//     MasterComdV3 data;
//     COMData32 CRCdata;
// }sentpackage;
//
// //接收主体
// typedef struct
// {
//     uint8_t mode;        //运行模式
//     int8_t ReadBit;
//     uint8_t Temp;        //电机平均温度
//     uint8_t MError;      //电机报错信息
//     uint8_t Read[4];
//
//     int16_t T;           //当前力矩(T*256)
//     int16_t W;           //当前转速(W*128)
//     int32_t LW;
//
//     int16_t W2;
//     int32_t LW2;
//
//     int16_t Acc;         //当前电机转动加速度
//     int16_t OutAcc;
//     int32_t Pos;         //当前电机角度(Pos*16384/2pi)
//     int32_t Pos2;
//
//     int16_t gyro[3];     //角速度(gyro*2000/32768*2pi/360)   x,y,z   rad/s
//     int16_t acc[3];      //线速度(acc*8*9.80665/32768)     m/s*s
//     int16_t Fgyro[3];
//     int16_t Facc[3];
//     int16_t Fmag[3];
//     uint8_t Ftemp;
//     uint16_t Forcd16;
//     uint8_t Forcd8;
//     uint8_t FError;
//     uint8_t Res;
// }ServoComdV3;
//
// //完整接收包
// typedef struct
// {
//     COMHead head;
//     ServoComdV3 data;
//     COMData32 CRCdata;    // CRC32
// }recevepackage;
//
// //解码
// typedef struct
// {
//     uint8_t id;
//     uint8_t mode;
//     float T;
//     float W;
//     float Pos;
//     int8_t Temp;
//     uint8_t MError;
//
//     float gyro[3];
//     float acc[3];
//
//     uint32_t invalid_cnt; // CRC 错误计数
//     uint8_t  valid;       // 数据有效标志
// }unitreemotorstate;
//
// #pragma pack()
//
// extern unitreemotorstate unitreemotorstate_A1;
// void Unitree_Init(UART_HandleTypeDef *huart);
// void Unitree_senddata(Motorcmd *cmd);
// void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
// void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
// #endif //J4310_F407_UNITREE_A1_H
