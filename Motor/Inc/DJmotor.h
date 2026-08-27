/**
 * @file    DJmotor.h
 * @brief   DJI M2006/M3508 CAN motor driver, ported from R2 chassis.
 */
#ifndef DJMOTOR_H
#define DJMOTOR_H

#include <stdbool.h>
#include "main.h"
#include "pid.h"
#include "motor_config.h"
#include "can.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define M3508_NUM MOTOR_M3508_COUNT //  定义M3508电机的数量
#define M2006_NUM MOTOR_M2006_COUNT //  定义M2006电机的数量
#define USE_DJNUM MOTOR_DJI_COUNT //  定义使用的电机编号数量
#define M2006_RATIO MOTOR_M2006_REDUCTION_RATIO //  定义M2006电机的减速比
#define M3508_RATIO MOTOR_M3508_REDUCTION_RATIO //  定义M3508电机的减速比
#define Zero_Distance 15 //  定义零点距离阈值

    typedef enum
    {
        DJ_Disable = 0,  /* 关: transmit 0 current */
        DJ_RPM = 1,      /* 速度 mode          */
        DJ_Position = 2, /* 位置 mode       */
        DJ_Zero = 3,     /* 寻零 mode       */
        DJ_Current = 4,  /* 电流/扭矩        */

    } DJmotor_mode_t;

    typedef struct
    {
        volatile int16_t current_raw;  // 直接设置电流
        volatile float angle_deg;      // 输出角度, degree
        volatile int16_t speed_rpm;    // valSet: 输出轴 rpm;valNow: 转子 rpm(原始反馈),m2006额定转速为416rpm,m3508为469rpm
        volatile float current_A;      // 反馈电流, A
        volatile int16_t PulseRead;    // raw encoder pulse
        volatile int16_t PulseGap;     // pulse delta
        volatile int32_t PulseTotal;   // accumulated pulse
        volatile int8_t temperature_C; // ℃
    } DJmotorVal;

    typedef struct
    {
        uint16_t PulsePerRound;   //           8191
        float Gear_ratio;         // mechanism ratio
        float Reduction_ratio;    // motor reducer ratio
        uint32_t ParamID;         // CAN receive ID base
        int16_t CurrentLimit_raw; // output current limit, raw
    } DJmotorParam;

    typedef struct
    {
        bool RPMLimitFlag;
        bool PosAngleLimitFlag;
        bool PosRPMFlag;
        bool CurrentLimitFlag;
        float MaxAngle_deg;           // degree
        float MinAngle_deg;           // degree
        int16_t SpeedRPMLimit;        // rpm
        int32_t PosRPMLimit;          // rpm
        int16_t ZeroRPMLimit;         // rpm
        int16_t ZeroCurrentLimit_raw; // raw
        bool IsLooseStuck;

        int16_t PosDeadband;   // 位置死区, 单位:编码器脉冲(带内不再追位置, 消除到位反冲)
        

    } DJmotorLimit;

    typedef struct
    {
        volatile bool ZeroFlag; //  零标志位，用于表示电机是否处于零位状态
        volatile bool Overtimeflag; //  过时标志位，用于表示电机是否运行超时
        volatile bool StuckFlag; //  卡住标志位，用于表示电机是否可能被卡住
        volatile bool IsSetZero; //  零位置设置标志位，用于表示零位置是否已设置
    } DJmotorStatus;

    typedef struct
    {
        volatile int32_t pulseLock; // 原先用于使能固定的字段,现在已无用
        uint16_t zeroCnt;
        uint16_t GapCnt;
    } DJmotorArgum;

    typedef struct
    {
        volatile uint32_t lastRxTime; //上次接收数据的时间戳，使用volatile关键字防止编译器优化
        uint16_t stuckCount; //  电机卡住计数器，用于检测电机是否卡住
        uint16_t timeoutCount; //  超时计数器，用于检测通信超时
    } DJmotorError; //  DJmotorError结构体，用于存储电机错误状态相关信息

    typedef struct
    {
        PIDType posPID;
        PIDType velPID;
    } DJmotorPID;

    typedef struct
    {
        uint8_t ID;
        volatile bool Begin;              // true 运行 MODE;false 失能
        volatile DJmotor_mode_t MODE_Set; // DJ_Disable 即失能(发 0 电流)
        volatile DJmotor_mode_t MODE_Cur; // 实际运行模式,任务层可读

        DJmotorParam param;
        DJmotorVal valSet;
        DJmotorVal valNow;
        DJmotorVal valPre;
        DJmotorStatus statusFlag;
        DJmotorLimit limit;
        DJmotorArgum argum;
        DJmotorError error;
        PIDType posPID;
        PIDType velPID;
    } DJMotor, *DJMotorPointer;

#if USE_DJ
    extern DJMotor DJmotor[USE_DJNUM];

    void DJmotor_Init(void);
    void DJmotor_Func(void);
    void DJmotor_Receive(CAN_RxHeaderTypeDef Rxheader, uint8_t *Rx_data);
    void DJmotor_PID_Reload(DJMotorPointer motor, DJmotorPID pid_reload);

#endif /* USE_DJ */

#ifdef __cplusplus
}
#endif

#endif /* DJMOTOR_H */
