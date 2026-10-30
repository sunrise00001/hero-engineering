# RoboMaster 重装机器人机械臂控制程序

本项目是 RoboMaster 重装机器人机械臂部分的 STM32 嵌入式控制代码，基于 STM32 HAL 和 FreeRTOS，通过 CAN1 控制六个达妙电机。程序从 USART6 接收自定义控制器的关节角度数据，并通过 USART3 接收遥控器数据，使用拨杆切换机械臂的控制状态。

当前应用以关节角度跟随和固定目标控制为主，尚未包含机械臂正逆运动学、轨迹规划或自动作业流程。

## 主要功能

- 初始化并使能六个达妙电机，使用 MIT 模式发送控制指令。
- 解析电机反馈的位置、速度、力矩、状态和温度数据。
- 接收并解析 18 字节遥控器数据，使用 DMA 双缓冲及串口空闲中断。
- 接收裁判系统格式的数据帧，校验 CRC8、CRC16，提取 `0x0302` 自定义控制数据。
- 将六个目标角度从角度制转换为弧度，按关节顺序下发。

## 硬件与软件配置

| 项目 | 当前配置 |
| --- | --- |
| 主控芯片 | STM32F407IGH6，Cortex-M4F |
| 系统时钟 | 168 MHz，外部晶振 12 MHz |
| 实时系统 | FreeRTOS，CMSIS-RTOS V2 接口，Tick 为 1 ms |
| 底层驱动 | STM32F4 HAL、CMSIS |
| 电机通信 | CAN1，1 Mbit/s |
| 遥控器接收 | USART3，100000 baud，偶校验，1 位停止位 |
| 自定义控制数据接收 | USART6，115200 baud，8N1 |
| 工程形式 | Keil MDK 工程、CMake + Ninja 工程 |

引脚来自当前外设初始化代码，实际板卡连接器位置需结合硬件原理图确认。

| 外设 | RX 引脚 | TX 引脚 | 用途 |
| --- | --- | --- | --- |
| CAN1 | PD0 | PD1 | 六个关节电机控制与反馈 |
| CAN2 | PB5 | PB6 | 已配置外设，BSP 中未启动 |
| USART3 | PC11 | PC10 | 遥控器接收，当前为仅接收模式 |
| USART6 | PG9 | PG14 | 裁判系统格式 / 自定义控制器数据接收 |
| USART1 | PB7 | PA9 | 已配置为 4800000 baud、8N1，当前机械臂控制流程未使用 |

USART3 的 HAL 配置为 `UART_WORDLENGTH_8B` 和 `UART_PARITY_EVEN`；接入遥控器时应核对实际数据位配置与接收数据，不能仅根据波特率判断链路正确。

## 目录结构

```text
ARM/
├── app/
│   └── arm_ctrl.c/.h           # 机械臂任务、关节编号、拨杆模式与控制参数
├── bsp/
│   ├── can/                   # CAN 初始化、收发与中断分发
│   ├── serial/                # 串口工具及 DMA 接收入口
│   ├── ramp/                  # 斜坡工具
│   ├── tool/                  # 通用工具及 PI 常量
│   └── user_lib.c/.h          # 通用算法工具
├── modules/
│   ├── motor/
│   │   ├── dm.c/.h            # 达妙电机协议、控制与反馈解析
│   │   └── unitree_a1.c/.h    # 保留的宇树 A1 代码，当前内容已注释
│   ├── RC/                    # 遥控器协议解析与 DMA 双缓冲配置
│   └── define_rx/             # 裁判系统格式数据解析、自定义角度数据结构
├── Core/                      # 主函数、FreeRTOS 任务、外设和中断代码
├── Drivers/                   # STM32 HAL 与 CMSIS
├── Middlewares/               # FreeRTOS 源码
├── MDK-ARM/ARM.uvprojx        # Keil 工程入口
├── cmake/                     # 交叉编译工具链及 CubeMX 构建配置
├── ARM.ioc                    # STM32CubeMX 配置
├── CMakeLists.txt              # CMake 构建入口
├── CMakePresets.json           # Debug / Release 构建预设
└── STM32F407XX_FLASH.ld        # GCC 链接脚本
```

## 关节与电机映射

当前六个电机均连接到 CAN1，初始化模式均为 `mit_mode`。

| 关节 | 电机 ID | 电机参数类型 | 目标角度来源 | 反馈存储位置 |
| --- | --- | --- | --- | --- |
| ARM1 | 1 | DM4310 | `mt_ang.angle[0]` | `dm_motor[1].feedback` |
| ARM2 | 2 | DM4340 | `mt_ang.angle[1]` | `dm_motor[2].feedback` |
| ARM3 | 3 | DM4310 | `mt_ang.angle[2]` | `dm_motor[3].feedback` |
| ARM4 | 4 | DM4340 | `mt_ang.angle[3]` | `dm_motor[4].feedback` |
| ARM5 | 5 | DM8009 | `mt_ang.angle[4]` | `dm_motor[5].feedback` |
| ARM6 | 6 | DM8009 | `mt_ang.angle[5]` | `dm_motor[6].feedback` |

`dm_motor` 数组使用下标 1～6，输入角度数组使用下标 0～5。各关节对应的实际机械轴名称、正方向和零点需按机械结构标定。

## 控制流程

1. `main()` 初始化时钟、GPIO、DMA、CAN 和串口。
2. `bsp_can_init()` 启动 CAN1 接收；`Remote_Controler_init()` 启动遥控器接收；`huart_start()` 启动 USART6 DMA 接收。
3. 启动 FreeRTOS，创建 `motor_init_TASK` 和 `arm_ctrl_TASK`。
4. `ARM_CTRL()` 延时约 3 秒，初始化六个电机，并依次发送使能指令，电机之间间隔约 20 ms。
5. 循环更新六个目标角度，按拨杆状态发送控制指令，再执行 `osDelay(2)`。

电机初始化实际在 `ARM_CTRL()` 内完成，`motor_init_TASK` 当前只循环延时。每次 `dm_motor_ctrl()` 内部还会执行 `osDelay(1)`，因此整轮控制包含六次 1 Tick 延时和末尾 2 Tick 延时，不能将控制周期直接视为 2 ms。

### 遥控器拨杆模式

控制逻辑位于 `app/arm_ctrl.c` 的 `control_update()`，判断对象为 `GET_SWITCH_LEFT()`，即 `RC_ctrldata.s[1]`。遥控解析函数中的左右注释与此宏命名存在差异，调试时应实际核对拨杆对应关系。

| 拨杆值 | 代码分支 | 实际行为 |
| --- | --- | --- |
| 下位，`RC_SW_DOWN = 2` | 第一分支 | 六个电机的 Kp、Kd、目标位置、目标速度和前馈力矩均设为 0 |
| 中位，`RC_SW = 3` | 第二分支 | ARM1～ARM4 使用固定的零位置目标；ARM4 额外给定前馈力矩 10；ARM5、ARM6 参数为 0 |
| 上位，`RC_SW_UP = 1` | `else` 分支 | 使用自定义控制器角度作为目标；ARM4 额外给定前馈力矩 2 |

下位分支持续发送零参数控制帧，没有调用 `dm_motor_disable()`。`else` 分支也会匹配上位之外的其他数值，包括初始化时的 0；当前尚未增加拨杆有效性及遥控器在线状态判断。

ARM6 在跟随分支中的 Kp、Kd 和前馈力矩仍为 0，虽然发送了目标角度，但当前未配置位置跟随增益。

### 自定义控制数据

USART6 的数据经 `Core/Src/stm32f4xx_it.c` 中的空闲中断交给 `parse()`，解析结果写入全局变量 `custom_robot_data`。

机械臂使用命令 ID `0x0302`，有效载荷长度为 30 字节：

| 有效载荷偏移 | 长度 | 内容 |
| --- | --- | --- |
| 0～23 | 24 字节 | 六个 `float` 关节角度，单位为度，按 ARM1～ARM6 排列 |
| 24 | 1 字节 | 按钮位：机械臂升降、泵开关及预留按钮 |
| 25～29 | 5 字节 | 预留数据 |

角度数据按 STM32 上的 32 位小端浮点表示直接复制。按钮位定义见 `message_arm.mt_ang`，当前机械臂控制任务只使用角度字段，尚未消费按钮指令。

```c
pos_set = custom_robot_data.mt_ang.angle[index] * PI / 180.0f;
```

数据帧由 5 字节帧头（包含 `0xA5`、长度、序号和 CRC8）、2 字节命令 ID、有效载荷和 2 字节 CRC16 组成；该命令完整帧长度为 39 字节。具体字段及校验实现以 `modules/define_rx/referee.c` 为准。

## 调试与参数修改

| 修改内容 | 位置 |
| --- | --- |
| 关节 ID、输入角度下标 | `app/arm_ctrl.h` |
| 拨杆分支、Kp、Kd、前馈力矩 | `app/arm_ctrl.c` |
| 电机位置、速度、力矩编码范围 | `modules/motor/dm.h` |
| 角度转换与电机反馈解析 | `modules/motor/dm.c` |
| 自定义数据结构与命令解析 | `modules/define_rx/referee.h`、`referee.c` |
| 串口波特率、CAN 时序与引脚 | `Core/Src/usart.c`、`can.c`、`ARM.ioc` |
| 任务优先级与栈大小 | `Core/Src/freertos.c` |

调试时可观察 `RC_ctrldata.s[1]`、`custom_robot_data.mt_ang.angle[]`、`dm_motor[1..6].ctrl.pos_set` 及各电机的 `feedback` 字段，分别检查模式输入、目标输入与电机反馈。

当前 MIT 编码将 Kp 限制在 0～500、Kd 限制在 0～5。应用中部分 Kd 参数大于 5，发送时会被截断到 5；调整控制参数时需要同时核对应用设定和协议编码范围。

## 当前实现边界

- 上电约 3 秒后会自动使能电机，当前没有等待有效遥控器或有效角度帧的启动条件。
- 尚未实现遥控器 / 自定义控制器断连超时处理、关节机械限位、目标变化率限制和自动故障停机。
- 数据解析包含 CRC 校验，但完整帧长度检查和跨 DMA 接收的半包缓存尚需完善。
- 电机温度和状态已解析保存，当前控制任务未据此执行保护动作。

首次联调应结合实际机械零点、正方向与运动范围核对参数，再逐关节验证控制效果。重新使用 CubeMX 生成工程时，应检查自定义任务入口、串口中断处理和用户构建配置是否保留。
