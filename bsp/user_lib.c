/**
 * @file user_lib.c
 * @brief 常用控制算法与数值处理函数的实现。
 * @author jiang
 * @date 2026-10-29
 */
#include "user_lib.h"

/**
 * @brief  快速估算平方根的倒数，即 1 / sqrt(num)。
 * @param  num 输入值，应为正的有限浮点数。
 * @return 经过一次牛顿迭代修正的近似结果。
 */
fp32 invSqrt(fp32 num)
{
    fp32 halfnum = 0.5f * num;
    fp32 y = num;
    long i = *(long *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(fp32 *)&i;
    y = y * (1.5f - (halfnum * y * y));
    return y;
}

/**
 * @brief  初始化斜坡发生器，设置更新周期和输出范围，并将输入、输出清零。
 * @param[out] ramp_source_type 斜坡状态，指针不能为空。
 * @param[in]  frame_period 每次计算对应的时间间隔，单位：s，应大于 0。
 * @param[in]  max 输出上限，应不小于 min。
 * @param[in]  min 输出下限。
 * @note   初始输出固定为 0；输出限幅在调用 ramp_calc 时执行。
 */
 void ramp_init(ramp_function_source_t *ramp_source_type, fp32 frame_period, fp32 max, fp32 min)
{
    ramp_source_type->frame_period = frame_period;
    ramp_source_type->max_value = max;
    ramp_source_type->min_value = min;
    ramp_source_type->input = 0.0f;
    ramp_source_type->out = 0.0f;
}

/**
 * @brief  按变化速率累加斜坡输出，并将结果限制在配置的范围内。
 * @param[in,out] ramp_source_type 已初始化的斜坡状态，指针不能为空。
 * @param[in]     input 输出变化速率，单位为输出量的单位每秒。
 * @note   每次调用执行 out += input * frame_period，结果保存在 out 中。
 */
void ramp_calc(ramp_function_source_t *ramp_source_type, fp32 input)
{
    ramp_source_type->input = input;
    ramp_source_type->out += ramp_source_type->input * ramp_source_type->frame_period;
    if (ramp_source_type->out > ramp_source_type->max_value)
    {
        ramp_source_type->out = ramp_source_type->max_value;
    }
    else if (ramp_source_type->out < ramp_source_type->min_value)
    {
        ramp_source_type->out = ramp_source_type->min_value;
    }
}

/**
 * @brief  初始化一阶低通滤波器，并将输入、输出清零。
 * @param[out] first_order_filter_type 滤波状态，指针不能为空。
 * @param[in]  frame_period 采样周期，单位：s，应大于 0。
 * @param[in]  num 参数数组，指针不能为空；num[0] 为时间常数，单位：s，应不小于 0。
 * @note   时间常数越大，输出越平滑，响应越慢。
 */
void first_order_filter_init(first_order_filter_type_t *first_order_filter_type, fp32 frame_period, const fp32 num[1])
{
    first_order_filter_type->frame_period = frame_period;
    first_order_filter_type->num[0] = num[0];
    first_order_filter_type->input = 0.0f;
    first_order_filter_type->out = 0.0f;
}

/**
 * @brief  输入一个采样值，更新一阶低通滤波输出。
 * @param[in,out] first_order_filter_type 已初始化的滤波状态，指针不能为空。
 * @param[in]     input 当前采样值。
 * @note   令 tau = num[0]、dt = frame_period，则
 *         out = tau / (tau + dt) * out + dt / (tau + dt) * input。
 *         更新结果保存在 out 中，tau + dt 不能为 0。
 */
void first_order_filter_cali(first_order_filter_type_t *first_order_filter_type, fp32 input)
{
    first_order_filter_type->input = input;
    first_order_filter_type->out =
        first_order_filter_type->num[0] / (first_order_filter_type->num[0] + first_order_filter_type->frame_period) * first_order_filter_type->out + first_order_filter_type->frame_period / (first_order_filter_type->num[0] + first_order_filter_type->frame_period) * first_order_filter_type->input;
}

/**
 * @brief  将指定变量原地限制在 [-Limit, Limit] 内。
 * @param[in,out] num 待限幅的变量，指针不能为空。
 * @param[in]     Limit 绝对值上限，应不小于 0。
 */
void abs_limit(fp32 *num, fp32 Limit)
{
    if (*num > Limit)
    {
        *num = Limit;
    }
    else if (*num < -Limit)
    {
        *num = -Limit;
    }
}

/**
 * @brief  根据输入的正负返回符号值。
 * @param  value 待判断的数值。
 * @return value >= 0 时返回 1.0f，否则返回 -1.0f；输入为 0 时返回 1.0f。
 */
fp32 sign(fp32 value)
{
    if (value >= 0.0f)
    {
        return 1.0f;
    }
    else
    {
        return -1.0f;
    }
}

/**
 * @brief  对浮点数应用死区：严格位于 (minValue, maxValue) 内时置零。
 * @param  Value 输入值。
 * @param  minValue 死区下边界，应小于 maxValue。
 * @param  maxValue 死区上边界。
 * @return 死区内部返回 0，边界上及死区外返回原值。
 */
fp32 fp32_deadline(fp32 Value, fp32 minValue, fp32 maxValue)
{
    if (Value < maxValue && Value > minValue)
    {
        Value = 0.0f;
    }
    return Value;
}

/**
 * @brief  对 16 位有符号整数应用死区，区间边界不包含在死区内。
 * @param  Value 输入值。
 * @param  minValue 死区下边界，应小于 maxValue。
 * @param  maxValue 死区上边界。
 * @return 严格位于 (minValue, maxValue) 内时返回 0，否则返回原值。
 */
int16_t int16_deadline(int16_t Value, int16_t minValue, int16_t maxValue)
{
    if (Value < maxValue && Value > minValue)
    {
        Value = 0;
    }
    return Value;
}

/**
 * @brief  将浮点数限制在闭区间 [minValue, maxValue] 内。
 * @param  Value 输入值。
 * @param  minValue 下限，应不大于 maxValue。
 * @param  maxValue 上限。
 * @return 小于下限时返回下限，大于上限时返回上限，否则返回原值。
 */
fp32 fp32_constrain(fp32 Value, fp32 minValue, fp32 maxValue)
{
    if (Value < minValue)
        return minValue;
    else if (Value > maxValue)
        return maxValue;
    else
        return Value;
}

/**
 * @brief  将 16 位有符号整数限制在闭区间 [minValue, maxValue] 内。
 * @param  Value 输入值。
 * @param  minValue 下限，应不大于 maxValue。
 * @param  maxValue 上限。
 * @return 小于下限时返回下限，大于上限时返回上限，否则返回原值。
 */
int16_t int16_constrain(int16_t Value, int16_t minValue, int16_t maxValue)
{
    if (Value < minValue)
        return minValue;
    else if (Value > maxValue)
        return maxValue;
    else
        return Value;
}

/**
 * @brief  反复加减区间宽度，将输入循环映射到 [minValue, maxValue] 内。
 * @param  Input 待映射的有限浮点数。
 * @param  minValue 区间下限，应小于 maxValue；两边界及区间宽度应为有限值。
 * @param  maxValue 区间上限。
 * @return 映射后的值；原值在区间内（含边界）时保持不变。
 * @note   maxValue < minValue 时直接返回原值；两边界相等且输入不等于边界时
 *         循环无法退出，因此调用者必须保证区间宽度大于 0。
 */
fp32 loop_fp32_constrain(fp32 Input, fp32 minValue, fp32 maxValue)
{
    if (maxValue < minValue)
    {
        return Input;
    }

    if (Input > maxValue)
    {
        fp32 len = maxValue - minValue;
        while (Input > maxValue)
        {
            Input -= len;
        }
    }
    else if (Input < minValue)
    {
        fp32 len = maxValue - minValue;
        while (Input < minValue)
        {
            Input += len;
        }
    }
    return Input;
}

/**
 * @brief  将角度循环映射到 [-180, 180] 范围内。
 * @param  Ang 输入角度，单位：度，应为有限值。
 * @return 映射后的角度，单位：度；-180 和 180 均保持原值。
 */
fp32 theta_format(fp32 Ang)
{
    return loop_fp32_constrain(Ang, -180.0f, 180.0f);
}