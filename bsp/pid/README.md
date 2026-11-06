# PID 模块

`pid.c/.h` 为不依赖电机驱动、HAL 或 RTOS 的位置式 PID。每个控制环创建一个
`pid_t`，通过 `pid_init()` 初始化，通过 `pid_calc()` 传入目标值与反馈值。
`pid_reset()` 清除运行状态并保留增益和限幅，适合停机或切换模式时调用。

## 角度环与速度环串级示例

```c
#include "pid.h"

static pid_t angle_pid;
static pid_t speed_pid;

/* 启动时调用一次，参数沿用原 PID 文件。 */
void control_pid_init(void)
{
    pid_init(&angle_pid, 3.0f, 0.0f, 0.0f, -50.0f, 50.0f, -20.0f, 20.0f);
    pid_init(&speed_pid, 0.2f, 0.01f, 0.0f, -50.0f, 50.0f, -3.0f, 3.0f);
}

/* 在固定周期任务中调用，反馈由调用方从实际电机结构中读取。 */
float control_pid_calc(float target_angle, float actual_angle, float actual_speed)
{
    float target_speed = pid_calc(&angle_pid, target_angle, actual_angle);
    return pid_calc(&speed_pid, target_speed, actual_speed);
}

void control_pid_reset(void)
{
    pid_reset(&angle_pid);
    pid_reset(&speed_pid);
}
```

此示例仅说明接口用法；输出单位由增益与反馈单位共同决定，接入电机前需匹配驱动
要求的速度、力矩或电流单位。应用当前没有调用旧 PID，本次未改动机械臂任务或电机指令。

## 计算约定

- 保留原离散公式：`kp * error + ki * sum(error) + kd * (error - last_error)`。
  不额外乘除 `dt`；`ki`、`kd` 包含采样周期影响，改变周期时需重新调整参数。
- 积分上下限限制误差累加值 `errorint`，并非 `ki * errorint`；输出另外限幅。
- `ki == 0` 时清除积分，非零增益（包括负值）正常累加。
- 初始化和复位后，上次误差为零，首次计算的微分项为 `kd * error`。
- 所有数值输入应为有限值，限幅下界应不大于上界。同一实例由一个控制任务维护。
- 原来的 `angle_pidcalc()`、`speed_pidcalc()` 及全局实例已移除，调用方自行持有实例。

CMake 与 Keil 工程已加入模块。数值测试可在项目根目录使用本机 GCC 运行：

```powershell
gcc -std=c11 -Wall -Wextra -Werror -pedantic -I bsp/pid bsp/pid/pid.c tests/pid_test.c -o "$env:TEMP/pid_test.exe"
& "$env:TEMP/pid_test.exe"
```
