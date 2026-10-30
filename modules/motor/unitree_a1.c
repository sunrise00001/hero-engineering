// //
// // Created by jiang on 2026/10/6.
// //
//
// #include "unitree_a1.h"
//
// #include <string.h>
//
// #include "cmsis_os2.h"
// static UART_HandleTypeDef *motor_huart;
//
// unitreemotorstate unitreemotorstate_A1;
//
// volatile uint8_t wating_reply =0;
//
// //前置声明
// static uint32_t crc32_core(uint32_t *ptr, uint32_t len);
//
// //接收
// #define  UNITREE_RX_BUF_SIZE 256
// static uint8_t rx_buffer[UNITREE_RX_BUF_SIZE];
//
//
// void Unitree_Init(UART_HandleTypeDef *huart)
// {
//     motor_huart = huart;
//     memset(&unitreemotorstate_A1, 0, sizeof(unitreemotorstate_A1));
//
//     if (motor_huart)
//     {
//         HAL_UARTEx_ReceiveToIdle_DMA(motor_huart, rx_buffer, UNITREE_RX_BUF_SIZE);
//         __HAL_UART_ENABLE_IT(motor_huart, UART_IT_ERR);
//     }
// }
//
//
// /**************数据提取&crc接收校验*******************/
// static void unitree_extract_data(recevepackage *package)
// {
//     //CRC接收校验
//     uint32_t cal_crc = crc32_core((uint32_t*)package,18);        //len长度为18的原因：见文档
//     if (cal_crc != package->CRCdata.u32)
//         return;
//     //搬运数据
//     unitreemotorstate_A1.valid = 1;                  //crc校验通过，数据合法
//     unitreemotorstate_A1.mode = package->data.mode;
//     unitreemotorstate_A1.id = package->head.motorID;
//     unitreemotorstate_A1.T = package->data.T/256.0f;
//     unitreemotorstate_A1.Pos =package->data.Pos* 6.2831853f / 16384.0f;
//     unitreemotorstate_A1.W = package->data.W/ 128.0f;
//     unitreemotorstate_A1.MError = package->data.MError;
//
//     for (int i = 0; i < 3; i++)
//     {
//         unitreemotorstate_A1.acc[i] = package->data.acc[i]* 8.0f * (9.80665f/32768.0f);
//         unitreemotorstate_A1.gyro[i] = package->data.gyro[i]* (2000.0f/32768.0f) * (6.2831853f/360.0f);
//     }
// }
//
//
// /***********接收数据*************/
// void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
// {
//     if (huart->Instance == motor_huart->Instance)
//     {
//         for (int i = 0; i < UNITREE_RX_BUF_SIZE - sizeof(recevepackage); i++)
//         {
//             if (rx_buffer[i] == 0xFE && rx_buffer[i+1] == 0xEE)
//             {
//                 recevepackage package_copy;
//                 memcpy(&package_copy,&rx_buffer[i], sizeof(recevepackage));
//                 if (package_copy.head.motorID < 10)                  //粗筛
//                 {
//                     unitree_extract_data(&package_copy);
//                 }
//             }
//         }
//         wating_reply = 0;          //标志总线空闲
//     }
// }
//
// /*********************发送数据******************/
// void Unitree_senddata(Motorcmd *cmd)
// {
//     if (!motor_huart)
//         return;
//
//
//     sentpackage package;
//     memset(&package,0,sizeof(sentpackage));
//     package.head.start[0] = 0xFE;
//     package.head.start[1] = 0xEE;
//     package.head.motorID = cmd->id;
//     package.head.reserved = 0x00;
//
//     package.data.mode = cmd->mode;
//     package.data.ModifyBit = 0xFF;            //0xFF 默认不修改数据
//     package.data.ReadBit = 0x00;
//     package.data.reserved = 0x00;
//
//     package.data.T = (int16_t)(cmd->tor * 256.0f);
//     package.data.W = (int16_t)(cmd->angvel* 128.0f);
//     package.data.Pos = (int32_t)(cmd->pos * 16384.0f / 6.2831853f);
//
//     package.data.K_P = (int16_t)(cmd->kp * 2048.0f);
//     package.data.K_W = (int16_t)(cmd->kd * 1024.0f);
//
//     package.data.LowHzMotorCmdIndex = 0;
//     package.data.LowHzMotorCmdByte = 0;
//
//     package.CRCdata.u32 =0;
//
//     package.CRCdata.u32 = crc32_core((uint32_t*)&package,7);
//
//     if (wating_reply == 0)
//     HAL_UART_Transmit(motor_huart, (uint8_t*)&package, sizeof(package), 10);
//     else
//     {
//         osDelay(2);
//         HAL_UART_Transmit(motor_huart, (uint8_t*)&package, sizeof(package), 10);
//     }
//     wating_reply = 1;
// }
//
//
// /**
//  * @brief  Unitree Go1/A1 参考代码使用的 CRC32 算法 (Word-based)
//  * CRC校验
//  */
// static uint32_t crc32_core(uint32_t *ptr, uint32_t len)
// {
//     uint32_t xbit = 0;
//     uint32_t data = 0;
//     uint32_t CRC32 = 0xFFFFFFFF;
//     const uint32_t dwPolynomial = 0x04c11db7;
//
//     for (uint32_t i = 0; i < len; i++) {
//         xbit = 1 << 31;
//         data = ptr[i];
//         for (uint32_t bits = 0; bits < 32; bits++) {
//             if (CRC32 & 0x80000000) {
//                 CRC32 <<= 1;
//                 CRC32 ^= dwPolynomial;
//             } else {
//                 CRC32 <<= 1;
//             }
//             if (data & xbit)
//                 CRC32 ^= dwPolynomial;
//             xbit >>= 1;
//         }
//     }
//     return CRC32;
// }
//
