/**
 * @file    DJmotor.c
 * @brief   DJI M2006/M3508 CAN motor driver.
 *
 * Ported from R2_Chassis-chassis_main/Motor/src/DJmotor.c.  The control flow
 * and PID structure are kept, only the bus selection and a few safety details
 * were generalized for this template.
 */
#include "DJmotor.h"

#if USE_DJ //当使用电机时，编译以下代码 ~line404

DJMotor DJmotor[USE_DJNUM];//定义DJNUM个电机djmotor类型的数组

/* 按 MOTOR_DJI_CAN_BUS 取总线句柄(0=CAN1,1=CAN2),返回对应的CAN外设句柄指针  */
/*inline：建议编译器把函数体直接内联到调用点，省掉一次函数调用的进出栈开销*/
static inline CAN_HandleTypeDef *DJmotor_GetCanHandle(void)
{
    switch (MOTOR_DJI_CAN_BUS)//MOTOR_DJI_CAN_BUS在motor_config.h中定义
    {
    case 0:
        return &hcan1;
    case 1:
        return &hcan2;
    default:
        return 0;
    }
}

/*获取总减速比：转子与输出轴之间的换算关系*/
static inline float Get_Total_Ratio(DJMotorPointer motor)
{
    return motor->param.Gear_ratio * motor->param.Reduction_ratio;
}

#if M3508_NUM > 0
static inline bool is_M3508(uint8_t ID)
{
    return (ID > M2006_NUM);
}
#else
static inline bool is_M3508(uint8_t ID)
{
    return false;
}
#endif

void DJmotor_Init(void)
{
    DJmotorParam dj2006_param;
    DJmotorParam dj3508_param;
    DJmotorLimit limit;
    DJmotorStatus statusFlag;
    DJmotorArgum argum;
    DJmotorError error;

    /*2006电机初始化，设定id,减速比，电流限制以及编码器分辨率*/
    dj2006_param.ParamID = 0x1ffU;
    dj2006_param.Gear_ratio = 1.0f;
    dj2006_param.Reduction_ratio = M2006_RATIO;
    dj2006_param.PulsePerRound = 8191U;
    dj2006_param.CurrentLimit_raw = 4500;

    /*3508电机初始化，设定id,减速比，电流限制以及编码器分辨率*/
    dj3508_param.ParamID = 0x200U;
    dj3508_param.Gear_ratio = 1.0f;
    dj3508_param.Reduction_ratio = M3508_RATIO;
    dj3508_param.PulsePerRound = 8191U;
    dj3508_param.CurrentLimit_raw = 10000;

    limit.CurrentLimitFlag = true;//开启电流限制，当前无代码调用
    limit.IsLooseStuck = false;//关闭堵转时失能功能，在DJmotor_Monitor中调用

    limit.MaxAngle_deg = 270.0f;
    limit.MinAngle_deg = -270.0f;//设定角度限制值
    limit.PosAngleLimitFlag = false;//关闭位置模式下的角度限制
    limit.PosRPMFlag = true;//开启位置模式下的速度限制
    limit.PosRPMLimit = 440;//位置模式下输出轴的最大转速

    limit.RPMLimitFlag = false;//关闭速度模式下的速度限制,在DJmotor_SpeedMode中调用
    limit.SpeedRPMLimit = 400;//速度模式下的最大输出轴转速
    limit.ZeroCurrentLimit_raw = 3000;//寻零模式下原始数值未转换电流值限制
    limit.ZeroRPMLimit = 50;//寻零模式下输出轴速度限制

    statusFlag.IsSetZero = true; //  将IsSetZero标志设置为true，表示已设置零值
    statusFlag.Overtimeflag = false; //  将Overtimeflag标志设置为false，表示未超时
    statusFlag.StuckFlag = false; //  将StuckFlag标志设置为false，表示未卡住
    statusFlag.ZeroFlag = false; //  将ZeroFlag标志设置为false，表示零值标志未激活

    argum.pulseLock = 0; //  脉冲锁状态标志，初始化为0，表示未锁定
    argum.zeroCnt = 0; //  零值计数器，初始化为0，用于计数零值出现的次数
    argum.GapCnt = 0; //  间隙计数器，初始化为0，用于计数间隙的次数

    error.lastRxTime = 0; //  将最后接收时间设置为0，表示尚未接收到数据
    error.stuckCount = 0; //  将卡住计数器清零，用于跟踪连续卡住的次数
    error.timeoutCount = 0; //  将超时计数器清零，用于跟踪超时发生的次数

    /*  循环初始化电机数组，设置每个电机的初始状态和参数 * USE_DJNUM 表示电机数量 */
    for (uint32_t i = 0; i < USE_DJNUM; i++) 
    {
        DJmotor[i].Begin = false; //  初始化后电机未开始运行
        DJmotor[i].MODE_Set = DJ_Disable; /* 上电失能:发 0 电流 */
        DJmotor[i].statusFlag = statusFlag; //  设置状态标志位结构体
        DJmotor[i].limit = limit; //  设置限制参数结构体
        DJmotor[i].argum = argum; //  设置电机参数结构体
        DJmotor[i].error = error; //  设置错误状态结构体
        DJmotor[i].valSet.current_raw = 0; //  设置目标电流值为0
        DJmotor[i].valSet.angle_deg = 0.0f; //  设置目标角度为0度
        DJmotor[i].valSet.speed_rpm = 0; //  设置目标转速为0 RPM
        DJmotor[i].valSet.PulseTotal = 0; //  设置目标脉冲总数为0
        DJmotor[i].valNow.PulseTotal = 0; //  设置当前脉冲总数为0
        DJmotor[i].valPre.PulseRead = 0; //  设置上一次读取的脉冲值为0
    }

    // 这里可以使用表封装的参数进行替换赋值
    for (uint32_t i = 0; i < M2006_NUM; i++) //  循环遍历所有M2006电机
    {
        DJmotor[i].ID = (uint8_t)(i + 1U); //  设置电机ID，从1开始递增
        DJmotor[i].param = dj2006_param; //  为每个电机设置相同的参数配置
    }

    for (uint32_t i = 0; i < M3508_NUM; i++) //  循环遍历所有M3508电机
    {
        DJmotor[i + M2006_NUM].ID = (uint8_t)(i + M2006_NUM + 1U); 
        //  设置电机ID，从M2006_NUM+1开始编号 ↑
        DJmotor[i + M2006_NUM].param = dj3508_param; //  设置电机参数为预设的dj3508_param
    }

    for (uint32_t i = 0; i < USE_DJNUM; i++)//初始化使用电机的PID系数
    {
        PID_Init(&DJmotor[i].posPID, 0.015f, 0.0005f, 0.005f, PIDPOS);
        PID_Init(&DJmotor[i].velPID, 2.975f, 0.045f, 0.001f, PIDINC); 
    }
}

/*用于重新加载指定电机的PID参数，包括位置PID和速度PID的参数配置
motor：指向电机结构体的指针，用于指定要重新加载PID参数的电机
pid_reload：包含新的位置PID和速度PID参数的结构体*/
void DJmotor_PID_Reload(DJMotorPointer motor, DJmotorPID pid_reload)
{
    PID_Init(&motor->posPID, 
             pid_reload.posPID.KP, pid_reload.posPID.KI, pid_reload.posPID.KD,
             pid_reload.posPID.mode);
    PID_Init(&motor->velPID,
             pid_reload.velPID.KP, pid_reload.velPID.KI, pid_reload.velPID.KD, 
             pid_reload.velPID.mode);
}

/*将电机对象的状态重置为零位，清除零位标记，并重置角度值、脉冲总数和脉冲锁参数。
motor：指向DJMotor结构体的指针，表示需要重置的电机对象。*/
void DJmotor_SetZero(DJMotorPointer motor)
{
    motor->statusFlag.IsSetZero = false;
    motor->valNow.angle_deg = 0.0f;
    motor->valNow.PulseTotal = 0;
    motor->argum.pulseLock = 0;
}

/*用于计算电机的角度值，并根据电机的脉冲变化情况更新电机的状态。*/
void DJmotor_AngleCalculate(DJMotorPointer motor)
{
    motor->valNow.PulseGap = (int16_t)(motor->valNow.PulseRead - motor->valPre.PulseRead); 
    //  计算当前脉冲读数与上一次脉冲读数之间的差值

    if (ABS(motor->valNow.PulseGap) > 4096) 
    //  检查脉冲差值是否超过4096(半圈)，如果超过则进行脉冲周期修正
    {
        motor->valNow.PulseGap = (int16_t)(motor->valNow.PulseGap 
                                         - GetSign(motor->valNow.PulseGap) 
                                         * (int32_t)motor->param.PulsePerRound);
    }

    motor->valNow.PulseTotal += motor->valNow.PulseGap; //  更新总脉冲数

    motor->valNow.angle_deg = (float)motor->valNow.PulseTotal * 360.0f / 
                              ((float)motor->param.PulsePerRound * Get_Total_Ratio(motor));
    //  计算当前输出轴角度（度）↑

    if (motor->Begin) // 废弃字段 
    {
        motor->argum.pulseLock = motor->valNow.PulseTotal;
    }

    if (motor->statusFlag.IsSetZero) //  如果设置了零位标志，则执行电机零位设置
    {
        DJmotor_SetZero(motor);
        motor->statusFlag.IsSetZero = false;
    }

    motor->valPre = motor->valNow; //  更新上一次的值为当前值，为下一次计算做准备
}

/*该函数用于处理接收到的CAN总线数据，根据电机ID解析并更新电机的状态信息，
包括温度、电流、转速等参数，并重置错误接收时间

Rxheader：CAN接收头信息，包含标准ID、帧类型等信息
Rx_data：接收到的CAN数据指针，包含电机的原始数据*/
void DJmotor_Receive(CAN_RxHeaderTypeDef Rxheader, uint8_t *Rx_data)
{
    if ((Rxheader.IDE != CAN_ID_STD) || //  检查CAN帧类型是否为标准数据帧
        (Rxheader.RTR != CAN_RTR_DATA) || //  检查是否为数据帧
        (Rxheader.StdId < 0x201U) || (Rxheader.StdId > 0x208U)) 
        //检查ID是否在0x201-0x208范围内
    {
        return; //  如果不符合条件，直接返回
    }

    uint8_t card_id = (uint8_t)(Rxheader.StdId - 0x200U); /* 1..8 *, 计算电机卡号，范围1-8*/

    /* Init 保证 ID = 索引 + 1,直接索引免循环查找 */
    if (card_id > USE_DJNUM) //  检查卡片ID是否大于可使用的DJ电机数量
    {
        return; //  如果超过，直接返回
    }

    DJMotorPointer motor = &DJmotor[card_id - 1U]; 
    //  根据卡片ID获取对应的电机指针 数组索引从0开始，
    // 所以需要将card_id减1 使用1U表示无符号常量1，确保计算正确

    motor->valNow.PulseRead = (int16_t)(((uint16_t)Rx_data[0] << 8) | Rx_data[1]);
     //  读取电机当前位置脉冲值
    int16_t speed_raw = (int16_t)(((uint16_t)Rx_data[2] << 8) | Rx_data[3]); 
    //  读取转子原始速度值
    motor->valNow.current_raw = (int16_t)(((uint16_t)Rx_data[4] << 8) | Rx_data[5]); 
    //  读取原始电流值

     /*根据电机型号(M3508)进行不同的数据处理，M3508电机有特定的温度和电流转换公式 */
    if (is_M3508(motor->ID))
    {
        motor->valNow.temperature_C = (int8_t)Rx_data[6]; //  读取温度值
        motor->valNow.current_A = (float)motor->valNow.current_raw * 0.0012207f; 
        //  转换为实际电流值，0.0012207f=±20/16348(2^14)
    }
    else
    {
        motor->valNow.current_A = (float)motor->valNow.current_raw / 10000.0f * 10.0f;
    }

    motor->valNow.speed_rpm = (float)speed_raw / Get_Total_Ratio(motor); 
    //  将转子原始速度值转换为输出轴实际转速(rpm)，使用总减速比进行换算

    motor->error.lastRxTime = 0; //  重置上次接收时间为0，表示刚接收到新数据
    DJmotor_AngleCalculate(motor); //  计算电机角度
}

/*通过CAN总线发送电机电流指令。根据电机的ID选择不同的标准标识符，
并将电流值打包到指定位置的数据帧中，最后通过CAN总线发送*/
void DJmotor_CurrentTransmit(DJMotorPointer motor)
{
    static uint8_t tx_data[8] = {0}; //  发送数据缓冲区，大小为8字节
    CAN_TxHeaderTypeDef tx_header = {0};
    uint32_t tx_mailbox = 0; //  定义用于存储发送邮箱号的变量
    uint8_t tag = 0; //  定义用于存储标签值的变量

    /* 电流限幅由各模式函数负责,此处只打包发送 */
    tx_header.IDE = CAN_ID_STD; /* 设置CAN标准帧的ID类型为标准帧 */
    tx_header.RTR = CAN_RTR_DATA; /* 设置CAN帧类型为数据帧 */
    tx_header.DLC = 8U; /* 设置数据长度为8字节 */
    tx_header.TransmitGlobalTime = DISABLE; /* 禁用全局时间戳 */

    if (motor->ID <= 4U) /* 根据电机ID判断使用哪个标准ID */
    {
        tx_header.StdId = 0x200U; /* 当电机ID小于等于4时，使用0x200作为标准ID */
        tag = (uint8_t)((motor->ID - 1U) * 2U); /* 计算数据标签位置，每个电机占用2个字节 */
    }
    else
    {
        tx_header.StdId = 0x1FFU; /* 当电机ID大于4时，使用0x1FF作为标准ID */
        tag = (uint8_t)((motor->ID - 5U) * 2U); /* 计算数据标签位置，每个电机占用2个字节 */
    }

    EncodeS16Data(&motor->valSet.current_raw, &tx_data[tag]); 
    /* 编码原始电流数据并存储到发送数据缓冲区 */
    ChangeDataByte(&tx_data[tag], &tx_data[tag + 1U]); /* 改变数据的字节顺序 */

    if (motor->ID == 4U || motor->ID == 8U) /* 特殊处理ID为4和8的电机，发送CAN消息 */
    {
        HAL_CAN_AddTxMessage(DJmotor_GetCanHandle(), &tx_header, tx_data, &tx_mailbox); 
        /* 添加发送消息到CAN总线，使用获取的CAN句柄 */
    }
}

/*设置电机速度模式，并根据限制条件调整电机的速度和电流值*/
void DJmotor_SpeedMode(DJMotorPointer motor)
{
    if (motor->limit.RPMLimitFlag) //  如果电机设置了转速限制标志
    {
        //  对设定的速度值进行限幅处理，确保不超过最大允许转速
        motor->valSet.speed_rpm = ClampPeak(motor->velPID.SetVal,  
                                            motor->limit.SpeedRPMLimit);
    }
    motor->velPID.SetVal = (float)motor->valSet.speed_rpm * Get_Total_Ratio(motor); 
    //  将设定的转子速度值(RPM)转换为输出轴的实际速度值，考虑总减速比
    motor->velPID.CurVal = (float)motor->valNow.speed_rpm * Get_Total_Ratio(motor); 
    //  将当前的转子速度值(RPM)转换为输出轴的实际速度值，考虑总减速比
    motor->valSet.current_raw += PID_Calculate(&motor->velPID); 
    //  根据PID计算结果更新电机输出电流值
    motor->valSet.current_raw = (int16_t)ClampPeak(motor->valSet.current_raw,
                                                   motor->param.CurrentLimit_raw);
    //钳位
}

/*电机进入位置模式，根据目标角度或速度限制计算所需的脉冲总数，
并通过PID控制器计算并设置电机的目标电流值*/
void DJmotor_PositionMode(DJMotorPointer motor)
{

    if (motor->limit.PosAngleLimitFlag) //  如果设置了位置角度限制标志
    {
        motor->valSet.angle_deg = Clamp(motor->valSet.angle_deg, 
                                        motor->limit.MinAngle_deg, 
                                        motor->limit.MaxAngle_deg);//钳位
    }

     motor->valSet.PulseTotal = (int32_t)(motor->valSet.angle_deg 
                                        * Get_Total_Ratio(motor) 
                                        * (float)motor->param.PulsePerRound / 360.0f); 
                                        //  将目标角度转换为脉冲总数
    motor->posPID.SetVal = (float)motor->valSet.PulseTotal; 
    //  设置位置PID的设定值为目标脉冲总数
    motor->posPID.CurVal = (float)motor->valNow.PulseTotal; 
    //  设置位置PID的当前值为当前脉冲总数

    /* 1) 先算位置环, 更新 posPID 误差历史, 再决定用不用 */
    motor->velPID.SetVal = PID_Calculate(&motor->posPID); 
    //  计算位置PID的设定值，并将其赋值给速度PID的设定值

    /* 2) 位置死区: 误差落在带内则不再追位置, 交给速度环自然刹停 */
    if (ABS(motor->valSet.PulseTotal - motor->valNow.PulseTotal) 
                                    < (int32_t)motor->limit.PosDeadband)
    {
        motor->velPID.SetVal = 0.0f; /* 如果在死区范围内，则设置速度PID的目标值为0 */
    }
    else if (motor->limit.PosRPMFlag) /* 检查是否启用了位置RPM限制标志 */
    {
        motor->velPID.SetVal = ClampPeak(motor->velPID.SetVal, 
                               motor->limit.PosRPMLimit * Get_Total_Ratio(motor)); 
                /* 如果启用了限制，则对速度PID的目标值进行限幅处理 */
    }


    motor->velPID.CurVal = (float)motor->valNow.speed_rpm * Get_Total_Ratio(motor); 
    /* 计算当前转子速度值，考虑总传动比 */

    motor->valSet.current_raw += PID_Calculate(&motor->velPID); 
    /* 计算速度PID并更新到目标电流值 */
    motor->valSet.current_raw = (int16_t)ClampPeak(motor->valSet.current_raw,
                                         motor->param.CurrentLimit_raw);
    
}

/*控制电机进入寻零模式，通过PID计算和位置检测，实现电机自动归零的功能*/
void DJmotor_ZeroMode(DJMotorPointer motor)
{
    motor->velPID.SetVal = (float)motor->limit.ZeroRPMLimit * Get_Total_Ratio(motor); 
    //  设置转子目标值，考虑零转速限制和总传动比
    motor->velPID.CurVal = (float)motor->valNow.speed_rpm * Get_Total_Ratio(motor); 
    //  设置转子当前值，考虑当前转速和总传动比
    motor->valSet.current_raw += PID_Calculate(&motor->velPID); 
    //  根据速度PID计算结果更新电流输出值
    motor->valSet.current_raw = (int16_t)ClampPeak(motor->valSet.current_raw, 
                                                   motor->limit.ZeroCurrentLimit_raw); 
                            //  限制电流输出值在零电流限制范围内

    if (ABS(motor->valNow.PulseGap) < Zero_Distance) 
    //  如果相邻周期内脉冲计数变化量距离小于零点判定距离
    {
        if (motor->argum.zeroCnt++ > 100U) //  如果零点计数器超过100次
        {
            motor->argum.zeroCnt = 0; //  重置零点计数器
            motor->statusFlag.ZeroFlag = true; //  设置零点标志为真
            motor->Begin = false; //  设置电机开始标志为假
            /* 寻零结束不走 SwitchMode,这里手动清 PID 历史,重新使能时从零起步 */
            PID_Reset(&motor->posPID); //  重置位置PID控制器
            PID_Reset(&motor->velPID); //  重置速度PID控制器
            DJmotor_SetZero(motor); //  将电机设置为零位
        }
    }
}

/*监控电机状态，检测电机是否卡死或通信超时，并在检测到异常时设置相应的错误标志和禁用电机*/
static void DJmotor_Monitor(DJMotorPointer motor)
{

    if (motor->valNow.PulseGap < 5 && motor->valNow.current_raw > 3000) 
    /* 检查电机是否卡住的逻辑判断 当当前脉冲间隔小于5且原始电流值大于3000时，认为电机可能卡住 */
    {
        if (motor->error.stuckCount++ > 500U) /* 增加计数器，如果超过500次，则认为确实卡住了 */
        {
            motor->error.stuckCount = 0; /* 重置堵转计数器 */
            motor->statusFlag.StuckFlag = true; /* 设置堵转标志为真 */
            if (motor->limit.IsLooseStuck) /* 如果是堵转时失能状态，则失能电机 */
            {
                motor->MODE_Set = DJ_Disable;
            }
        }
    }
    else /* 如果不满足堵转条件，则重置堵转计数器 */
    {
        motor->error.stuckCount = 0;
    }

    if (motor->error.lastRxTime++ > 50U) 
    /* 检查通信超时的逻辑判断 如果自上次接收数据以来时间超过50个单位 */
    {
        if (motor->error.timeoutCount++ > 20U) 
        /* 增加超时计数器，如果超过20次，则认为通信超时 */
        {
            motor->error.timeoutCount = 0; /* 重置超时计数器 */
            motor->MODE_Set = DJ_Disable; /* 禁用电机 */
            motor->statusFlag.Overtimeflag = true; /* 设置超时标志为真 */
        }
    }
}

/*用于切换电机的运行模式，当设置的模式与当前模式不同时，会更新当前模式，并重置相关参数和状态标志*/
static void DJmotor_SwitchMode(DJMotorPointer motor)
{
    if (motor->MODE_Set != motor->MODE_Cur) /* 检查电机当前模式与设定模式是否不一致 */
    {
        motor->MODE_Cur = motor->MODE_Set; /* 更新当前模式为设定模式 */
        motor->valSet.current_raw = 0; /* 重置电流设定值为0 */
        motor->valSet.speed_rpm = 0; /* 重置速度设定值为0 */
        motor->valSet.angle_deg = motor->valNow.angle_deg; /* 保持当前角度值不变 */
        /* 清误差历史与位置环累加的目标速度(velPID.SetVal),避免残留值冲击新模式 */
        PID_Reset(&motor->posPID); //  重置位置PID控制器参数
        PID_Reset(&motor->velPID); //  重置速度PID控制器参数
        motor->statusFlag.ZeroFlag = false; //  清除零位标志
        motor->statusFlag.Overtimeflag = false; //  清除超时标志
        motor->statusFlag.StuckFlag = false; //  清除堵转标志
    }
}

/*控制多个DJ电机，根据电机的不同模式执行相应的控制逻辑，
包括禁用模式、转速模式和电流模式，并确保电机电流的合理输出*/
void DJmotor_Func(void)
{
    for (uint32_t i = 0; i < USE_DJNUM; i++) //  遍历所有DJ电机
    {

        if (DJmotor[i].Begin) //  检查电机是否已初始化并开始工作
        {
            // DJmotor_Monitor(&DJmotor[i]); //  电机监控功能,后续可能选择开启
            DJmotor_SwitchMode(&DJmotor[i]); //  切换电机工作模式

            switch (DJmotor[i].MODE_Cur) //  根据当前电机模式执行相应操作
            {
            case DJ_Disable: //  电机禁用模式
                DJmotor[i].valSet.current_raw = 0; //  设置电流值为0
                DJmotor_CurrentTransmit(&DJmotor[i]); //  发送电流值到电机
                continue; //  跳过后续处理，继续下一个电机
            case DJ_RPM: //  速度模式
                DJmotor_SpeedMode(&DJmotor[i]); //  执行速度控制
                break;
            case DJ_Position: //  电机位置模式
                DJmotor_PositionMode(&DJmotor[i]); //  切换到位置模式控制
                break;
            case DJ_Zero: //  电机寻零模式
                DJmotor_ZeroMode(&DJmotor[i]); //  执行电机寻零操作
                break;
            case DJ_Current: //  直通电流模式
                /* 直通电流:任务层每周期写 valSet.current_raw,这里补限幅 */
                ClampPeak(DJmotor[i].valSet.current_raw, DJmotor[i].param.CurrentLimit_raw);
                break;
            default:
                break; //  默认情况，不做任何处理
            }
        }
        else
        {
            /* Begin=false(未初始化/寻零完成):强制 0 电流,防止残留累加电流持续输出 */
            DJmotor[i].valSet.current_raw = 0; 
            //  当电机未初始化或寻零完成时，将电流值强制设置为0，防止残留电流持续输出
        }

        DJmotor_CurrentTransmit(&DJmotor[i]);
    }
}
#endif /* USE_DJ */
