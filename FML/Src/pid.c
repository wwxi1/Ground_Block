/**
 * @file    pid.c
 * @brief   Scalar and 2D vector PID implementation.
 */
#include "pid.h"

void PID_Init(PIDType *pid, float KP, float KI, float KD, uint8_t mode)
{
    pid->KP = KP;  pid->KI = KI;  pid->KD = KD;
    pid->mode = mode;
    pid->iLimit   = 10000.0f;
    pid->deadband = 0.0f;
    PID_Reset(pid);
}

void PID_Reset(PIDType *pid)
{
    pid->err[0] = pid->err[1] = pid->err[2] = 0.0f;
    pid->SumError = 0.0f;
    pid->output = 0.0f;
    pid->lastCurVal = pid->CurVal;
}


float PID_Calculate(PIDType *pid)
{
    pid->err[0] = pid->SetVal - pid->CurVal;

    if (ABS(pid->err[0]) < pid->deadband)
    {
        pid->err[0] = 0.0f;
    }

    switch (pid->mode)
    {
    case PIDINC:
        pid->output = pid->KP * (pid->err[0] - pid->err[1]) +
                      pid->KI * pid->err[0] +
                      pid->KD * (pid->err[0] - 2.0f * pid->err[1] + pid->err[2]);
        pid->err[2] = pid->err[1];
        pid->err[1] = pid->err[0];
        break;

    case PIDPOS:
        pid->SumError += pid->err[0];
        if (ABS(pid->SumError) > pid->iLimit)
        {
            pid->SumError = GetSign(pid->SumError) * pid->iLimit;
        }
        pid->output = pid->KP * pid->err[0] +
                      pid->KI * pid->SumError +
                      pid->KD * -(pid->CurVal - pid->lastCurVal);
        pid->err[1] = pid->err[0];
        pid->lastCurVal = pid->CurVal;
        break;

    default:
        break;
    }
    return pid->output;
}

void vector2fPIDInit(Vector2fPID *pid, float *param, uint8_t mode)
{
    pid->kp = param[0];
    pid->ki = param[1];
    pid->kd = param[2];
    pid->mode = mode;
    vector2fPIDReset(pid);
}

/* 清空动态状态(含 target/input),保留增益与模式 */
void vector2fPIDReset(Vector2fPID *pid)
{
    pid->target.x = 0.0f;
    pid->target.y = 0.0f;
    pid->input.x = 0.0f;
    pid->input.y = 0.0f;
    pid->output.x = 0.0f;
    pid->output.y = 0.0f;

    for (int i = 0; i < 3; i++) {
        pid->err[i].x = 0.0f;
        pid->err[i].y = 0.0f;
    }
}

/**
 * @brief 限制积分项的范围，防止积分饱和
 * @param integral 要限制的积分项向量
 * @return 限幅后的积分项向量
 */
static vector2d vector2fPIDLimitIntegral(vector2d integral)
{
    if (ABS(integral.x) > 1000.0f) {
        integral.x = GetSign(integral.x) * 1000.0f;
    }
    if (ABS(integral.y) > 1000.0f) {
        integral.y = GetSign(integral.y) * 1000.0f;
    }
    return integral;
}

/**
 * @brief 位置式PID输出计算
 * @param pid PID控制器结构体指针
 * @return PID输出向量
 */
static vector2d vector2fPIDPosCalculate(Vector2fPID *pid)
{
    // 更新积分项
    pid->err[2] = Vector_Add(Vector_MultiplyNum(pid->err[2], 0.5f),
                             Vector_MultiplyNum(pid->err[0], 0.5f));
    // 限制积分项范围
    pid->err[2] = vector2fPIDLimitIntegral(pid->err[2]);
    // 计算PID输出
    pid->output = Vector_MultiplyNum(pid->err[0], pid->kp);
    pid->output = Vector_Add(pid->output, Vector_MultiplyNum(pid->err[2], pid->ki));
    pid->output = Vector_Add(pid->output, Vector_MultiplyNum(Vector_Minus(pid->err[0], pid->err[1]), pid->kd));
    pid->err[1] = pid->err[0];
    return pid->output;
}

/**
 * @brief 增量式PID输出计算
 * @param pid PID控制器结构体指针
 * @return PID输出向量
 */
static vector2d vector2fPIDIncCalculate(Vector2fPID *pid)
{
    // 计算PID输出
    pid->output = Vector_MultiplyNum(Vector_Minus(pid->err[0], pid->err[1]), pid->kp);
    pid->output = Vector_Add(pid->output, Vector_MultiplyNum(pid->err[0], pid->ki));
    pid->output = Vector_Add(pid->output,
                             Vector_MultiplyNum(Vector_Minus(Vector_Add(pid->err[0], pid->err[2]),
                                                             Vector_MultiplyNum(pid->err[1], 2.0f)), pid->kd));
    pid->err[2] = pid->err[1];
    pid->err[1] = pid->err[0];
    return pid->output;
}

vector2d vector2fPIDOperation(Vector2fPID *pid)
{
    pid->err[0] = Vector_Minus(pid->target, pid->input);

    switch (pid->mode) {
    case PIDPOS:
        pid->output = vector2fPIDPosCalculate(pid);
        break;

    case PIDINC:
        pid->output = vector2fPIDIncCalculate(pid);
        break;

    default:
        break;
    }

    return pid->output;
}
