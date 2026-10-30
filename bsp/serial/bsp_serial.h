//
// Created by jiang on 2026/10/17.
//

#ifndef ARM_BSP_SERIAL_H
#define ARM_BSP_SERIAL_H
#include "stm32f4xx_hal.h"

/**
 * @brief 模块接收处理回调的类型：无参数、无返回值。
 * @note 将用户定义的 void OnSerialReceived(void) 赋给 config.callback 即可注册。
 *       当前实现直接调用该指针，接收模式下必须提供有效函数，不能为 NULL。
 *       回调通常在中断上下文中执行，应尽快返回，避免延时或长时间阻塞。
 */
typedef void(*serial_callback)(void);




typedef enum
{
    serial_send,
    serial_receive,
    serial_send_receive,
}serial_mode_t;


typedef struct
{
    serial_mode_t mode;
    UART_HandleTypeDef *huart;
    uint8_t RXdata[100];
    uint8_t rx_DLC;
    uint8_t rxbuffer[100];
    serial_callback callback;
}Serial_Instance_t;

typedef struct
{
    serial_mode_t mode;
    UART_HandleTypeDef *huart;
    uint8_t rx_DLC;
    serial_callback callback;
}serial_config_t;



/**
 * @brief 将串口配置复制到内部实例中，不负责初始化 UART/DMA 外设或启动接收。
 * @param config 配置地址，不能为 NULL。huart 应指向已初始化的串口句柄；
 *               mode 为 serial_send、serial_receive 或 serial_send_receive；
 *               接收时 rx_DLC 应为 1~100，callback 应指向有效的模块处理函数。
 * @par 用法
 * @code
 * // OnSerialReceived 是用户定义的 void OnSerialReceived(void) 函数。
 * serial_config_t config = {
 *     .mode = serial_send_receive,
 *     .huart = &huart1,
 *     .rx_DLC = 20,
 *     .callback = OnSerialReceived
 * };
 * Serial_Init(&config);
 * @endcode
 * @warning 当前 .c 实现没有在保存配置后递增 idx，后续接收遍历不会找到实例；
 *          重复初始化还会覆盖已有配置。使用接收流程前需先修复实例注册逻辑。
 */
void Serial_Init(serial_config_t *config);

/**
 * @brief 启动 DMA 接收，等待串口空闲事件或接收缓冲区写满后由 HAL 通知。
 * @param huart 已初始化并配置接收 DMA 的串口句柄，不能为 NULL。
 * @note 调用后立即返回；接收长度取自实例的 rx_DLC，并关闭 DMA 半传输中断。
 *       应先注册接收/收发模式的实例；普通 DMA 模式下，现有回调会再次调用
 *       本函数以启动下一次接收，不应在主循环中反复调用。
 * @par 用法
 * @code
 * Serial_Receive(&huart1);  // 外设初始化、实例注册完成后启动首次接收
 * @endcode
 * @warning 当前 .c 中查找实例时没有匹配 huart；
 *          使用前需修复，否则可能无法接收或选错接收缓冲区。
 */
void Serial_Receive(UART_HandleTypeDef *huart);

/**
 * @brief 将对应串口的 DMA 接收区 RXdata 复制到调用者提供的缓冲区。
 * @param huart 已注册且处于接收/收发模式的串口句柄。
 * @param data 输出缓冲区，不能为 NULL，容量必须至少为该实例的 rx_DLC 字节。
 * @note 固定复制 rx_DLC 字节，不返回实际接收长度，也不会自动追加字符串 '\0'。
 * @par 用法
 * @code
 * uint8_t data[100];
 * Serial_getRxdata(&huart1, data);  // 需确保目标串口有效且 RXdata 内的数据可用
 * @endcode
 * @warning 当前实现读取 RXdata，而两个接收回调会先清空 RXdata，再调用模块回调。
 *          因此在模块回调中调用本函数无法取到刚保存到 rxbuffer 的数据；
 *          若用于读取已完成的数据，应先调整 .c 实现中的读取来源。
 *          当前实现未找到匹配实例时仍会读取下标 0，请勿传入未注册串口。
 */
void Serial_getRxdata(UART_HandleTypeDef *huart,uint8_t *data);

/**
 * @brief 标准 UART 接收完成回调：保存接收数据、执行模块回调并尝试重启接收。
 * @param huart 由 HAL 传入的、本次接收完成的串口句柄。
 * @note 用法：由 HAL 的标准中断/DMA 接收完成流程自动调用，业务代码无需手动调用。
 *       当前实现复制 rx_DLC 字节到 rxbuffer，清空 RXdata，再执行注册的 callback。
 *       本模块的 ReceiveToIdle DMA 接收应由 HAL_UARTEx_RxEventCallback 处理。
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

/**
 * @brief ReceiveToIdle 接收事件回调：保存接收数据、执行模块回调并尝试重启接收。
 * @param huart 由 HAL 传入的、发生接收事件的串口句柄。
 * @param Size HAL 通知的接收缓冲区有效数据结束位置；普通 DMA 单次接收时
 *             可用于确定本次收到的字节数，循环 DMA 时需结合上次位置处理。
 * @note 用法：调用 Serial_Receive 后，HAL 在串口空闲或 DMA 传输完成时自动调用。
 *       本模块关闭了 DMA 半传输中断；模块 callback 通常也在中断上下文中执行。
 * @warning 当前 .c 实现未使用 Size，始终复制 rx_DLC 字节；收到短帧时，
 *          rxbuffer 后部不一定属于本帧，解析前应先完善实际接收长度的处理。
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);

/**
 * @brief UART 错误回调：对已注册的串口尝试重新启动 DMA 接收。
 * @param huart 由 HAL 传入的、发生错误的串口句柄。
 * @note 用法：串口错误由 HAL 检测并进入错误回调流程时自动调用，无需业务代码调用。
 *       当前函数仅调用 Serial_Receive，不检查重启结果，不保证所有错误均已恢复。
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);


void Serial_Printf(UART_HandleTypeDef *huart,char *format, ...);

void huart_start(UART_HandleTypeDef *huart,uint8_t *data);













#endif //ARM_BSP_SERIAL_H
