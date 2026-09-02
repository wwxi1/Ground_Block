
#include "Ground_Block.h"

uint8_t solenoid_is_open = 0; // 夹爪张开与关闭状态
uint8_t solenoid_enable = 0;  // 夹爪使能状态
volatile uint8_t gb_cmd = 0;
static uint8_t gb_data[8];
static uint8_t gb_ready_signal = 0;
// 使能电机控制，设置为失能模式
void Ground_Block_Enable()
{
    solenoid_enable = 1;
    DJmotor[GROUND_BLOCK_dji_num].Begin = 1;
    DJmotor[GROUND_BLOCK_dji_num].MODE_Set = DJ_Disable;
    BEEP_ON();
    osDelay(300);
    BEEP_OFF();
}

// 失能电机控制
void Ground_Block_Disable()
{
    solenoid_enable = 0;                                 // 关闭电磁阀使能
    DJmotor[GROUND_BLOCK_dji_num].Begin = 0;             // 停止电机运行
    DJmotor[GROUND_BLOCK_dji_num].MODE_Set = DJ_Disable; // 设置电机为禁用模式
    BEEP_ON();
    osDelay(300);
    BEEP_OFF();
}

// 初始化
void Ground_Block_Init()
{
    solenoid_on(3, 0x00);
    Ground_Block_Disable();
}

void Ground_Block_reset()
{
    //  下降高度，收爪收杆
    __set_FAULTMASK(1); // 关闭所有的中断，确保执行复位时不被中断打断
    NVIC_SystemReset(); // 系统软件复位，配置好的外设寄存器也一起复位,此处内含了beep_off
}

void Ground_Block_close()
{
    // 先下降高度，再收夹爪,再收主杆

    DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = 0.0f;
    osDelay(4000);
    solenoid_on(3, 0x4);
    solenoid_is_open = 0;
    solenoid_on(3, 0x00);
    osDelay(1000);
}

void Ground_Block_GetReady() // 预取大地块
{
    // 杆先推出，延时，夹爪张开
    if(gb_ready_signal == 0){
    solenoid_on(3, 0x04);
    osDelay(800);
    gb_ready_signal = 1;
    }
    solenoid_on(3, 0x07);
    solenoid_is_open = 1;
    DJmotor[GROUND_BLOCK_dji_num].MODE_Set = DJ_Position;
    DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -15.0f;
    osDelay(500);
}

// 接收数据 并执行对应动作，此处osdelay需更改，目前测试用
void Ground_Block_Fetch(uint8_t *Rxdata)
{
    switch (Rxdata[1])
    {
    case 1:
        if (solenoid_is_open == 1)
        {
            solenoid_on(3, 0x04); // 夹爪闭合
            solenoid_is_open = 0;
            osDelay(500);
        }
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -450.0f;
        osDelay(500);

        break;
    case 2:
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -850.0f;
        osDelay(500);
        if (solenoid_is_open == 1)
        {
            solenoid_on(3, 0x04); // 夹爪闭合
            solenoid_is_open = 0;
            osDelay(500);
        }
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -1200.0f;
        osDelay(500);
        break;
    default:
        break;
    }
}

void Ground_Block_Lay(uint8_t *Rxdata)
{
    switch (Rxdata[1])
    {
    case 1:
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -80.0f;
        osDelay(500);
        if (solenoid_is_open == 0)
        {
            solenoid_on(3, 0x07); // 夹爪张开
            solenoid_is_open = 1;
            osDelay(500);
        }
        break;
    case 2:
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -840.0f;
        osDelay(400);
        if (solenoid_is_open == 0)
        {
            solenoid_on(3, 0x07); // 夹爪张开
            solenoid_is_open = 1;
            osDelay(500);
        }
        break;
    default:
        break;
    }
}

void Ground_Block_Split()
{
    if (solenoid_is_open == 0)
    {
        solenoid_on(3, 0x07); // 夹爪张开
        solenoid_is_open = 1;
        osDelay(500);
    }
    DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -850.0f;
    osDelay(800);
    if (solenoid_is_open == 1)
    {
        solenoid_on(3, 0x04); // 夹爪闭合
        solenoid_is_open = 0;
        osDelay(500);
    }
    DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -1200.0f;
    osDelay(500);
}

void Ground_Block_Func(CAN_RxHeaderTypeDef RxHeader, uint8_t *Rxdata)
{
    if (RxHeader.ExtId == 0x01010301 && Rxdata[0] == 0x4D)
    {
        switch (Rxdata[1])
        {
        case 0:
            Ground_Block_Disable(); // 失能
            return;
        case 1:
            Ground_Block_Enable(); // 使能
            return;
        }
    }
    else if (RxHeader.ExtId == 0x010103FF && Rxdata[0] == 0x52)
    {
        gb_cmd = Ground_Block_reset_Flag;
    }
    else if (solenoid_enable == 1 && DJmotor[GROUND_BLOCK_dji_num].Begin == 1)
    {
        memcpy(gb_data, Rxdata, 8);
        if (RxHeader.ExtId == 0x01010302 && Rxdata[0] == 0x50)
            gb_cmd = Ground_Block_GetReady_Flag;
        else if (RxHeader.ExtId == 0x01010303 && Rxdata[0] == 0x47)
            gb_cmd = Ground_Block_Fetch_Flag;
        else if (RxHeader.ExtId == 0x01010304 && Rxdata[0] == 0X50)
            gb_cmd = Ground_Block_Lay_Flag;
        else if (RxHeader.ExtId == 0x01010305 && Rxdata[0] == 0x53)
            gb_cmd = Ground_Block_Split_Flag;
    }
}

void Ground_Block_Process(void)
{
    if (gb_cmd == 0)
    {
        return;
    }
    uint8_t cmd = gb_cmd;
    gb_cmd = 0;
    switch (cmd)
    {
    case Ground_Block_GetReady_Flag:
        Ground_Block_GetReady();
        break;
    case Ground_Block_Fetch_Flag:
        Ground_Block_Fetch(gb_data);
        break;
    case Ground_Block_Lay_Flag:
        Ground_Block_Lay(gb_data);
        break;
    case Ground_Block_Split_Flag:
        Ground_Block_Split();
        break;
    case Ground_Block_reset_Flag:
        Ground_Block_reset();
        break;
    default:
        break;
    }
}
