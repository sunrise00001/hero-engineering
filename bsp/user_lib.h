/**
 * @file user_lib.h
 * @brief 斜坡发生器、一阶低通滤波器及限幅、死区、角度映射接口。
 * @author jiang
 * @date 2026-10-29
 */

#ifndef ARM_USER_LIB_H
#define ARM_USER_LIB_H
#define  fp32 float // 32 位浮点类型别名
#include <stdint.h>

/** @brief 斜坡发生器状态：按变化速率积分，并对输出限幅。 */
typedef struct
{
    fp32 input;           // 最近一次输入的变化速率，单位为输出量的单位每秒
    fp32 out;             // 累加并限幅后的斜坡输出
    fp32 min_value;       // 斜坡输出下限
    fp32 max_value;       // 斜坡输出上限
    fp32 frame_period;    // 调用更新函数对应的时间间隔，单位：s
} ramp_function_source_t;

/** @brief 一阶低通滤波器状态与参数。 */
typedef struct
{
    fp32 input;           // 最近一次输入的采样值
    fp32 out;             // 当前滤波输出，也是下次计算使用的历史输出
    fp32 num[1];          // num[0] 为滤波时间常数，单位：s，应不小于 0
    fp32 frame_period;    // 调用更新函数对应的时间间隔，单位：s
} first_order_filter_type_t;

/** @brief 估算 1 / sqrt(num)，num 应为正的有限值。 */
extern fp32 invSqrt(fp32 num);

/** @brief 设置斜坡周期（s）、上限和下限，并将输入、输出清零。 */
void ramp_init(ramp_function_source_t *ramp_source_type, fp32 frame_period, fp32 max, fp32 min);

/** @brief 按 input（变化速率）积分并限幅，结果保存在结构体的 out 中。 */
void ramp_calc(ramp_function_source_t *ramp_source_type, fp32 input);

/** @brief 初始化一阶低通滤波器；周期和 num[0]（时间常数）的单位均为 s。 */
extern void first_order_filter_init(first_order_filter_type_t *first_order_filter_type, fp32 frame_period, const fp32 num[1]);

/** @brief 输入当前采样值并更新滤波输出，结果保存在结构体的 out 中。 */
extern void first_order_filter_cali(first_order_filter_type_t *first_order_filter_type, fp32 input);

/** @brief 将 *num 原地限制到 [-Limit, Limit]，Limit 应不小于 0。 */
extern void abs_limit(fp32 *num, fp32 Limit);

/** @brief 输入大于等于 0 时返回 1.0f，否则返回 -1.0f。 */
extern fp32 sign(fp32 value);

/** @brief 浮点死区：严格位于 (minValue, maxValue) 内返回 0，否则返回原值。 */
extern fp32 fp32_deadline(fp32 Value, fp32 minValue, fp32 maxValue);

/** @brief 16 位整数死区：严格位于 (minValue, maxValue) 内返回 0，否则返回原值。 */
extern int16_t int16_deadline(int16_t Value, int16_t minValue, int16_t maxValue);

/** @brief 将浮点数限制到闭区间 [minValue, maxValue]。 */
extern fp32 fp32_constrain(fp32 Value, fp32 minValue, fp32 maxValue);

/** @brief 将 16 位整数限制到闭区间 [minValue, maxValue]。 */
extern int16_t int16_constrain(int16_t Value, int16_t minValue, int16_t maxValue);

/** @brief 循环映射到 [minValue, maxValue]；要求有限输入及有限正区间宽度。 */
extern fp32 loop_fp32_constrain(fp32 Input, fp32 minValue, fp32 maxValue);

/** @brief 将有限角度循环映射到 [-180, 180]，单位：度，保留两端点。 */
extern fp32 theta_format(fp32 Ang);

/** @brief 将有限弧度循环映射到 [-PI, PI]；使用前需定义 PI，保留两端点。 */
#define rad_format(Ang) loop_fp32_constrain((Ang), -PI, PI)

#endif // ARM_USER_LIB_H
